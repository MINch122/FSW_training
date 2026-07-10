#include "ltrx_dispatch.h"
#include "ltrx_app.h"
#include "ltrx_fcncodes.h"
#include "ltrx_msg.h"
#include "ltrx_cmds.h"
#include "ltrx_cmds_beacon.h"
#include "hk_msgids.h"
#include "ltrx_eventids.h"

#include "cfe.h"

#include <stdbool.h>
#include <stddef.h>

/* ---- counters for commands that cmds.c does NOT count ---- */
static inline void LTRX_CountCmdResult(CFE_Status_t status)
{
    if (status == CFE_SUCCESS)
    {
        LTRX_AppData.CmdCounter++;
    }
    else
    {
        LTRX_AppData.AppErrCounter++;
    }
}

/* local length verify (dispatch-local helper) */
static bool LTRX_VerifyCmdLength(const CFE_SB_Buffer_t *SBBufPtr, size_t ExpectedSize, uint16 CC)
{
    CFE_Status_t status;
    size_t       ActualSize = 0;

    status = CFE_MSG_GetSize(&SBBufPtr->Msg, &ActualSize);
    if (status != CFE_SUCCESS)
    {
        LTRX_AppData.AppErrCounter++;
        CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: CFE_MSG_GetSize failed (CC=%u, RC=0x%08lX)",
                          (unsigned)CC, (unsigned long)status);
        return false;
    }

    if (ActualSize != ExpectedSize)
    {
        LTRX_AppData.AppErrCounter++;
        CFE_EVS_SendEvent(LTRX_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: Invalid cmd length (CC=%u, Expected=%u, Actual=%u)",
                          (unsigned)CC, (unsigned)ExpectedSize, (unsigned)ActualSize);
        return false;
    }

    return true;
}

/* ====================== Task pipe ================== */
void LTRX_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_Status_t  status;
    CFE_SB_MsgId_t MsgId;

    if (SBBufPtr == NULL)
    {
        LTRX_AppData.AppErrCounter++;
        CFE_EVS_SendEvent(LTRX_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: TaskPipe got NULL buffer");
        return;
    }

    status = CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
    if (status != CFE_SUCCESS)
    {
        LTRX_AppData.AppErrCounter++;
        CFE_EVS_SendEvent(LTRX_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: GetMsgId failed (RC=0x%08lX)", (unsigned long)status);
        return;
    }

    const uint16 mid = (uint16)CFE_SB_MsgIdToValue(MsgId);

    if (mid == LTRX_CMD_MID)
    {
        LTRX_DispatchCommand(SBBufPtr);
    }
    else if (mid == LTRX_SEND_HK_MID)
    {
        (void)LTRX_SendHkCmd(SBBufPtr);
    }
    else if (mid == LTRX_SEND_BCN_MID)
    {
        LTRX_SendBcnTlm();
    }
    else if (mid == HK_COMBINED_PKT1_MID)
    {
        LTRX_OnBusBeaconReceived(SBBufPtr);
    }
    else
    {
        LTRX_AppData.AppErrCounter++;
        CFE_EVS_SendEvent(LTRX_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: Invalid MID=0x%04X", (unsigned)mid);
    }
}

/* ===================== Dispatch command ==================== */
void LTRX_DispatchCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_Status_t     status;
    CFE_MSG_FcnCode_t cc = 0;

    status = CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &cc);
    if (status != CFE_SUCCESS)
    {
        LTRX_AppData.AppErrCounter++;
        CFE_EVS_SendEvent(LTRX_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: GetFcnCode failed (RC=0x%08lX)", (unsigned long)status);
        return;
    }

    const uint16 CC = (uint16)cc;
    const size_t ExpectedNoArgsSize = sizeof(LTRX_NoArgsCmd_t);

    switch (CC)
    {
        /* ---- Basic app management (cmds.c handles counters itself) ---- */
        case LTRX_NOOP_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            (void)LTRX_NoopCmd((const LTRX_NoopCmd_t *)SBBufPtr);
            break;

        case LTRX_RESET_COUNTERS_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            (void)LTRX_ResetCountersCmd((const LTRX_ResetCountersCmd_t *)SBBufPtr);
            break;

        case LTRX_RESET_APP_CMD_COUNTERS_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            (void)LTRX_ResetAppCmdCountersCmd((const LTRX_ResetAppCmdCountersCmd_t *)SBBufPtr);
            break;

        case LTRX_RESET_DEVICE_CMD_COUNTERS_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            (void)LTRX_ResetDeviceCmdCountersCmd((const LTRX_ResetDeviceCmdCountersCmd_t *)SBBufPtr);
            break;

        /* ---- Session triggers (dispatch counts result) ---- */
        case LTRX_SESSION_START_DOWNLINK_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            status = LTRX_SessionStartDownlinkCmd((const LTRX_SessionStartDownlinkCmd_t *)SBBufPtr);
            LTRX_CountCmdResult(status);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: SESSION_START_DOWNLINK failed (RC=0x%08lX)", (unsigned long)status);
            }
            break;

        case LTRX_SESSION_ABORT_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            status = LTRX_SessionAbortCmd((const LTRX_SessionAbortCmd_t *)SBBufPtr);
            LTRX_CountCmdResult(status);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: SESSION_ABORT failed (RC=0x%08lX)", (unsigned long)status);
            }
            break;

        case LTRX_SESSION_RESET_STATE_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            status = LTRX_SessionResetStateCmd((const LTRX_SessionResetStateCmd_t *)SBBufPtr);
            LTRX_CountCmdResult(status);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: SESSION_RESET_STATE failed (RC=0x%08lX)", (unsigned long)status);
            }
            break;

        /* ---- Downstream gating (dispatch counts result) ---- */
        case LTRX_DOWNSTREAM_ENABLE_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            status = LTRX_DownstreamEnableCmd((const LTRX_DownstreamEnableCmd_t *)SBBufPtr);
            LTRX_CountCmdResult(status);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: DOWNSTREAM_ENABLE failed (RC=0x%08lX)", (unsigned long)status);
            }
            break;

        case LTRX_DOWNSTREAM_DISABLE_CC:
            if (!LTRX_VerifyCmdLength(SBBufPtr, ExpectedNoArgsSize, CC)) return;
            status = LTRX_DownstreamDisableCmd((const LTRX_DownstreamDisableCmd_t *)SBBufPtr);
            LTRX_CountCmdResult(status);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: DOWNSTREAM_DISABLE failed (RC=0x%08lX)", (unsigned long)status);
            }
            break;

        default:
            LTRX_AppData.AppErrCounter++;
            CFE_EVS_SendEvent(LTRX_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX: Invalid CC=%u", (unsigned)CC);
            break;
    }
}