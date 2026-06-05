#include "ltrx_session.h"
#include "ltrx_child.h"
#include "ltrx_cmds_beacon.h"
#include "ltrx_app.h"
#include "ltrx_msg.h"
#include "ltrx_eventids.h"

#include "osapi.h"
#include "cfe.h"

#include <string.h>
#include <stdbool.h>

#define LTRX_REQ_Q_NAME   "LTRX_REQ_Q"
#define LTRX_REQ_Q_DEPTH  16


typedef enum
{
    LTRX_WAIT_NONE = 0,
    LTRX_WAIT_TYPE3,        /* Downlink: waiting Beacon's Type3 request     */
    LTRX_WAIT_DL_DONE11,    /* Downlink: waiting Beacon's Type11 confirm    */
    LTRX_WAIT_UL_HEADER5,   /* Uplink: waiting Beacon's Type5 header        */
    LTRX_WAIT_UL_PARTS,     /* Uplink: waiting Beacon's Type9 parts / done  */
    LTRX_WAIT_GNSS21,
    LTRX_WAIT_BCN23
} LTRX_WaitKind_t;

static osal_id_t           s_ReqQ = OS_OBJECT_ID_UNDEFINED;
static LTRX_SessionState_t s_State;

static volatile uint8  s_LastRxType;
static volatile uint8  s_LastRxStatus;
static volatile bool   s_HaveRx;
static volatile uint32 s_LastRxMsgId;

static LTRX_WaitKind_t s_WaitKind;

static uint32 s_RetryCount;
static uint32 s_MaxRetry  = 3;
static uint32 s_TimeoutMs = 2000;
static uint32 s_WaitStartMs;

/* Tracks current downlink */
static uint32 s_DownlinkMsgId;

static uint32 LTRX_NowMs(void)
{
    OS_time_t now;
    OS_GetLocalTime(&now);
    return (uint32)(OS_TimeGetTotalSeconds(now) * 1000U +
                    (OS_TimeGetFractionalPart(now) / 1000000U));
}

static void LTRX_SetState(LTRX_SessionState_t st) { s_State = st; }

static void LTRX_StartWait(LTRX_WaitKind_t kind)
{
    s_WaitKind    = kind;
    s_RetryCount  = 0;
    s_HaveRx      = false;
    s_LastRxType  = 0;
    s_LastRxStatus= 0;
    s_LastRxMsgId = 0;
    s_WaitStartMs = LTRX_NowMs();
}

static void LTRX_TouchProgress(void)
{
    s_WaitStartMs = LTRX_NowMs();
}

static CFE_Status_t LTRX_PushReq(LTRX_SessionReqType_t t)
{
    LTRX_SessionReq_t r;

    if (!OS_ObjectIdDefined(s_ReqQ))
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    r.type    = t;
    r.arg_u32 = 0;

    if (OS_QueuePut(s_ReqQ, &r, sizeof(r), 0) != OS_SUCCESS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    LTRX_ChildWake();
    return CFE_SUCCESS;
}

static bool LTRX_PopReq(LTRX_SessionReq_t *out)
{
    size_t actual = 0;

    if (!OS_ObjectIdDefined(s_ReqQ))
    {
        return false;
    }

    if (OS_QueueGet(s_ReqQ, out, sizeof(*out), &actual, OS_CHECK) == OS_SUCCESS)
    {
        return (actual == sizeof(*out));
    }

    return false;
}

void LTRX_SessionInit(void)
{
    if (!OS_ObjectIdDefined(s_ReqQ))
    {
        int32 osrc = OS_QueueCreate(&s_ReqQ, LTRX_REQ_Q_NAME,
                                   sizeof(LTRX_SessionReq_t),
                                   LTRX_REQ_Q_DEPTH, 0);
        if (osrc != OS_SUCCESS)
        {
            LTRX_SetState(LTRX_SESS_ERROR);
            CFE_EVS_SendEvent(LTRX_SESS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX session: queue create failed (RC=%ld)", (long)osrc);
            return;
        }
    }

    s_State        = LTRX_SESS_IDLE;
    s_WaitKind     = LTRX_WAIT_NONE;

    s_RetryCount   = 0;
    s_WaitStartMs  = 0;

    s_LastRxType   = 0;
    s_LastRxStatus = 0;
    s_LastRxMsgId  = 0;
    s_HaveRx       = false;

    s_DownlinkMsgId = 0;
}

void LTRX_SessionOnIcdRx(uint8 type_id, uint8 status)
{
    LTRX_SessionOnIcdRxEx(type_id, status, 0);
}

void LTRX_SessionOnIcdRxEx(uint8 type_id, uint8 status, uint32 msg_id)
{
    s_LastRxType   = type_id;
    s_LastRxStatus = status;
    s_LastRxMsgId  = msg_id;
    s_HaveRx       = true;

    LTRX_AppData.LastRxType   = type_id;
    LTRX_AppData.LastRxStatus = status;

    /* Any RX while waiting counts as progress */
    if (s_WaitKind != LTRX_WAIT_NONE)
    {
        LTRX_TouchProgress();
    }

    LTRX_ChildWake();
}

LTRX_SessionState_t LTRX_SessionGetState(void) { return s_State; }

typedef CFE_Status_t (*LTRX_TxRetryFn_t)(void);

static void LTRX_RetryOrFail(LTRX_TxRetryFn_t tx_fn, const char *tag)
{
    uint32 now = LTRX_NowMs();

    if ((now - s_WaitStartMs) < s_TimeoutMs)
    {
        return;
    }

    if (s_RetryCount < s_MaxRetry)
    {
        s_RetryCount++;
        s_WaitStartMs = now;

        if (tx_fn != NULL)
        {
            (void)tx_fn();
        }
    }
    else
    {
        s_WaitKind = LTRX_WAIT_NONE;
        LTRX_SetState(LTRX_SESS_ERROR);
        CFE_EVS_SendEvent(LTRX_SESS_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX session timeout: %s", tag);
    }
}

static bool LTRX_DownlinkDoneMsgMatches(void)
{
    /* If you don't track msg id, accept completion on Type11 */
    if (s_DownlinkMsgId == 0 || s_LastRxMsgId == 0)
    {
        return true;
    }
    return (s_LastRxMsgId == s_DownlinkMsgId);
}

void LTRX_SessionTick(void)
{
    LTRX_SessionReq_t req;

    while (LTRX_PopReq(&req))
    {
        switch (req.type)
        {
            case LTRX_REQ_START_DOWNLINK:
                s_DownlinkMsgId = LTRX_Downlink_GetMessageId();

                LTRX_StartWait(LTRX_WAIT_TYPE3);
                LTRX_SetState(LTRX_SESS_WAIT_BEACON_REQ);
                break;

            case LTRX_REQ_ABORT:
                s_WaitKind = LTRX_WAIT_NONE;
                LTRX_SetState(LTRX_SESS_ABORTED);
                break;

            case LTRX_REQ_RESET:
                LTRX_SessionInit();
                break;

            case LTRX_REQ_QUERY_BEACON_STATUS:
            case LTRX_REQ_QUERY_GNSS_INFO:
                break;

            default:
                break;
        }
    }

    switch (s_State)
    {
        /* DOWNLINK: waiting Beacon to send Type3 (request to transmit)        */
        case LTRX_SESS_WAIT_BEACON_REQ:
        {
            if (s_WaitKind != LTRX_WAIT_TYPE3)
            {
                break;
            }

            if (s_HaveRx)
            {
                s_HaveRx = false;

                if (s_LastRxType == (uint8)LTRX_BEACON_CMD_REQUEST_MSG_TX && s_LastRxStatus == 0)
                {
                    /* Type3 received and we responded with Type5 header OK */
                    LTRX_StartWait(LTRX_WAIT_DL_DONE11);
                    LTRX_SetState(LTRX_SESS_DL_IN_PROGRESS);
                }
                else if (s_LastRxStatus != 0)
                {
                    s_WaitKind = LTRX_WAIT_NONE;
                    LTRX_SetState(LTRX_SESS_ERROR);
                }
            }
            else
            {
                LTRX_RetryOrFail(NULL, "Type3 wait");
            }

            break;
        }

        /* DOWNLINK: parts sent, waiting for Beacon's Type11 confirm */
        case LTRX_SESS_DL_IN_PROGRESS:
        {
            if (s_WaitKind != LTRX_WAIT_DL_DONE11)
            {
                break;
            }

            if (s_HaveRx)
            {
                s_HaveRx = false;

                if (s_LastRxStatus != 0)
                {
                    if (s_LastRxType != (uint8)LTRX_BEACON_CMD_CONFIRM_PART_RX)
                    {
                        s_WaitKind = LTRX_WAIT_NONE;
                        LTRX_SetState(LTRX_SESS_ERROR);
                        break;
                    }
                    /* Type10 part error: wait */
                    LTRX_TouchProgress();
                    break;
                }

                if (s_LastRxType == (uint8)LTRX_BEACON_CMD_CONFIRM_MSG_RX)
                {
                    if (LTRX_DownlinkDoneMsgMatches())
                    {
                        s_WaitKind = LTRX_WAIT_NONE;
                        LTRX_SetState(LTRX_SESS_COMPLETE);
                    }
                    else
                    {
                        /* Type11 for a different msg id: wait */
                        LTRX_TouchProgress();
                    }
                }
                else if (s_LastRxType == (uint8)LTRX_BEACON_CMD_REQUEST_MSG_PART || 
                         s_LastRxType == (uint8)LTRX_BEACON_CMD_CONFIRM_PART_RX)
                {
                    LTRX_TouchProgress();
                }
                else if (s_LastRxType == (uint8)LTRX_BEACON_CMD_MSG_STATUS_UPDATE)
                {
                    if (s_LastRxStatus != 0)
                    {
                        s_WaitKind = LTRX_WAIT_NONE;
                        LTRX_SetState(LTRX_SESS_ERROR);
                        CFE_EVS_SendEvent(LTRX_SESS_ERR_EID, CFE_EVS_EventType_ERROR,
                                        "LTRX downlink: Type13 error (code=%u mid=%u)",
                                        (unsigned)s_LastRxStatus,
                                        (unsigned)s_LastRxMsgId);
                    }
                    else
                    {
                        LTRX_TouchProgress();
                    }
                }
                /* else: ignore other type IDs while waiting */
            }
            else
            {
                LTRX_RetryOrFail(NULL, "Downlink done wait(Type11)");
            }

            break;
        }

        /* DOWNLINK complete */
        case LTRX_SESS_COMPLETE:
        {
            LTRX_SetState(LTRX_SESS_IDLE);
            break;
        }

        
        /* UPLINK: Type6 received & Type8 sent, wait for Beacon's Type5 */
        case LTRX_SESS_UL_OFFERED:
        {
            if (s_WaitKind != LTRX_WAIT_UL_HEADER5)
            {
                break;
            }

            if (s_HaveRx)
            {
                s_HaveRx = false;

                if (s_LastRxType == (uint8)LTRX_BEACON_CMD_SEND_MSG_HEADER && s_LastRxStatus == 0)
                {
                    LTRX_StartWait(LTRX_WAIT_UL_PARTS);
                    LTRX_SetState(LTRX_SESS_UL_IN_PROGRESS);
                }
                else if (s_LastRxStatus != 0)
                {
                    s_WaitKind = LTRX_WAIT_NONE;
                    LTRX_SetState(LTRX_SESS_ERROR);
                    CFE_EVS_SendEvent(LTRX_SESS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "LTRX uplink: Type5 header error (status=%u)", (unsigned)s_LastRxStatus);
                }
            }
            else
            {
                LTRX_RetryOrFail(NULL, "Uplink header wait(Type5)");
            }

            break;
        }
        
        /* UPLINK: receiving parts (Type9), confirming (Type10), until done    */
        case LTRX_SESS_UL_IN_PROGRESS:
        {
            if (s_WaitKind != LTRX_WAIT_UL_PARTS)
            {
                break;
            }

            if (s_HaveRx)
            {
                s_HaveRx = false;

                if (s_LastRxType == (uint8)LTRX_BEACON_CMD_SEND_MSG_PART)
                {
                    if (s_LastRxStatus != 0)
                    {
                        /* Part CRC/length error */
                        LTRX_TouchProgress();
                    }
                    else if (LTRX_Uplink_HasCompleteMessage())
                    {
                        /* All parts received and full-message CRC verified */
                        s_WaitKind = LTRX_WAIT_NONE;
                        LTRX_SetState(LTRX_SESS_UL_COMPLETE);
                        CFE_EVS_SendEvent(LTRX_SESS_STATE_INF_EID, CFE_EVS_EventType_INFORMATION,
                                          "LTRX uplink: message complete (mid=%u)",
                                          (unsigned)s_LastRxMsgId);
                    }
                    /* else: partial receive, keep waiting */
                }
                else if (s_LastRxType == (uint8)LTRX_BEACON_CMD_SEND_MSG_HEADER)
                {
                    s_WaitKind = LTRX_WAIT_NONE;
                    LTRX_SetState(LTRX_SESS_ERROR);
                    CFE_EVS_SendEvent(LTRX_SESS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "LTRX uplink: unexpected Type5 during part rx");
                }
                /* else: other type IDs (e.g. Type21/23 telemetry) — ignore */
            }
            else
            {
                LTRX_RetryOrFail(NULL, "Uplink parts wait(Type9)");
            }

            break;
        }
        
        /* UPLINK complete: hold one tick so HK can read UL_COMPLETE */
        case LTRX_SESS_UL_COMPLETE:
        {
            LTRX_SetState(LTRX_SESS_IDLE);
            break;
        }

        default:
            break;
    }
}

CFE_Status_t LTRX_SessionRequestStartDownlink(void)      { return LTRX_PushReq(LTRX_REQ_START_DOWNLINK); }
CFE_Status_t LTRX_SessionRequestAbort(void)              { return LTRX_PushReq(LTRX_REQ_ABORT); }
CFE_Status_t LTRX_SessionRequestReset(void)              { return LTRX_PushReq(LTRX_REQ_RESET); }
CFE_Status_t LTRX_SessionRequestQueryBeaconStatus(void)  { return LTRX_PushReq(LTRX_REQ_QUERY_BEACON_STATUS); }
CFE_Status_t LTRX_SessionRequestQueryGnssInfo(void)      { return LTRX_PushReq(LTRX_REQ_QUERY_GNSS_INFO); }

/* Type6 is received and Type8 ConfirmReady has been sent successfully */
void LTRX_SessionNotifyUplinkOffered(void)
{
    /* Only accept a new uplink if we're not in the middle of a downlink */
    if (s_State == LTRX_SESS_IDLE      ||
        s_State == LTRX_SESS_COMPLETE  ||
        s_State == LTRX_SESS_ABORTED   ||
        s_State == LTRX_SESS_ERROR     ||
        s_State == LTRX_SESS_UL_COMPLETE)
    {
        LTRX_StartWait(LTRX_WAIT_UL_HEADER5);
        LTRX_SetState(LTRX_SESS_UL_OFFERED);
    }
}