#include "ltrx_cmds_beacon.h"
#include "ltrx_app.h"
#include "ltrx_eventids.h"
#include "ltrx_session.h"
#include "ltrx.h"

#include "cfe.h"

#include <string.h>

#ifndef LTRX_DOWNLINK_MAX_LEN
#define LTRX_DOWNLINK_MAX_LEN 240
#endif

#ifndef LTRX_UPLINK_MAX_LEN
#define LTRX_UPLINK_MAX_LEN 240
#endif

#ifndef LTRX_UPLINK_PART_REQ_LEN
#define LTRX_UPLINK_PART_REQ_LEN 240
#endif

#define LTRX_HK_COMBINED_PAYLOAD_OFFSET \
    (sizeof(CFE_MSG_TelemetryHeader_t) - sizeof(((CFE_MSG_TelemetryHeader_t *)0)->Spare))

/* Uplink part retry (CRC mismatch etc.) */
#ifndef LTRX_UPLINK_MAX_PART_RETRY
#define LTRX_UPLINK_MAX_PART_RETRY 5
#endif

#define LTRX_UPLINK_BITMAP_BYTES  ((LTRX_UPLINK_MAX_LEN + 7u) / 8u)

/* ---------------- DOWNLINK (OBC -> GS) storage ---------------- */

static uint8_t  s_DownBuf[LTRX_DOWNLINK_MAX_LEN];
static uint16_t s_DownLen    = 0;
static uint32_t s_DownMsgId  = 0;
static uint32_t s_DownMsgCrc = 0;
static bool     s_DownReady  = false;

static uint8_t  s_PendingDownBuf[LTRX_DOWNLINK_MAX_LEN];
static uint16_t s_PendingDownLen   = 0;
static uint32_t s_PendingDownMsgId = 0;
static bool     s_PendingDownReady = false;

/* Downstream gate (Bus Beacon staging on/off). Default: enabled. */
static bool s_DownstreamEnabled = true;
static uint16_t s_DownstreamBeaconPeriod = LTRX_BUS_BEACON_PERIOD_DEFAULT;
static uint16_t s_DownstreamBeaconCount  = 0u;

void LTRX_Downstream_SetEnabled(bool enabled)
{
    s_DownstreamEnabled = enabled;
    s_DownstreamBeaconCount = 0u;
    LTRX_APP_printf("LTRX: downstream %s\n", enabled ? "enabled" : "disabled");
}

bool LTRX_Downstream_IsEnabled(void)
{
    return s_DownstreamEnabled;
}

void LTRX_Downstream_SetBeaconPeriod(uint16_t period)
{
    s_DownstreamBeaconPeriod = period;
    s_DownstreamBeaconCount  = 0u;
}

uint16_t LTRX_Downstream_GetBeaconPeriod(void)
{
    return s_DownstreamBeaconPeriod;
}

uint16_t LTRX_Downstream_GetBeaconCount(void)
{
    return s_DownstreamBeaconCount;
}


int32_t LTRX_Downlink_SetMessage(uint32_t MessageID,
                                const uint8_t *data,
                                uint16_t length)
{
    if (data == NULL || length == 0)
    {
        return LTRX_ERROR_NULL_PTR;
    }

    if (length > LTRX_DOWNLINK_MAX_LEN)
    {
        return LTRX_ERROR_PROTOCOL;
    }

    memcpy(s_DownBuf, data, length);
    s_DownLen    = length;
    s_DownMsgId  = MessageID;
    s_DownMsgCrc = LTRX_CalculateCRC32(s_DownBuf, s_DownLen);
    s_DownReady  = true;
    LTRX_APP_printf("LTRX: downlink staged id=%u len=%u crc=0x%08X\n", (unsigned)s_DownMsgId, (unsigned)s_DownLen, (unsigned)s_DownMsgCrc);

    return LTRX_SUCCESS;
}

void LTRX_Downlink_ClearMessage(void)
{
    memset(s_DownBuf, 0, sizeof(s_DownBuf));
    s_DownLen    = 0;
    s_DownMsgId  = 0;
    s_DownMsgCrc = 0;
    s_DownReady  = false;
}

bool     LTRX_Downlink_IsReady(void)      { return s_DownReady; }
bool     LTRX_Downlink_HasPending(void)   { return s_PendingDownReady; }
uint32_t LTRX_Downlink_GetMessageId(void) { return s_DownMsgId; }
uint16_t LTRX_Downlink_GetLength(void)    { return s_DownLen; }

static int32_t LTRX_Downlink_CommitPending(void)
{
    if (!s_PendingDownReady)
    {
        return LTRX_SUCCESS;
    }

    int32_t rc = LTRX_Downlink_SetMessage(s_PendingDownMsgId,
                                          s_PendingDownBuf,
                                          s_PendingDownLen);
    if (rc == LTRX_SUCCESS)
    {
        s_PendingDownReady = false;
    }

    return rc;
}

/* ---------------- UPLINK (GS -> OBC) receive state ---------------- */

typedef struct
{
    bool     in_progress;
    bool     complete;

    uint32_t msg_id;
    uint16_t msg_len;
    uint32_t msg_crc;

    uint8_t  buf[LTRX_UPLINK_MAX_LEN];
    uint8_t  recv_bitmap[LTRX_UPLINK_BITMAP_BYTES];
    uint16_t received_bytes;

    /* retry tracking */
    uint16_t last_part_start;
    uint16_t last_part_len;
    uint8_t  last_part_retry;

    uint16_t next_request_offset;
} LTRX_UplinkRx_t;

static LTRX_UplinkRx_t s_Up;

static void Uplink_Reset(void)
{
    memset(&s_Up, 0, sizeof(s_Up));
}

/* Mark byte range at bitmap */
static void Uplink_MarkReceived(uint16_t start, uint16_t len)
{
    uint16_t i;
    uint16_t new_bytes = 0;
    for (i = start; i < (uint16_t)(start + len) && i < LTRX_UPLINK_MAX_LEN; i++)
    {
        uint16_t byte_idx = (uint16_t)(i / 8u);
        uint8_t  bit_mask = (uint8_t)(1u << (i % 8u));
        if (!(s_Up.recv_bitmap[byte_idx] & bit_mask))
        {
            s_Up.recv_bitmap[byte_idx] |= bit_mask;
            new_bytes++;
        }
    }
    s_Up.received_bytes = (uint16_t)(s_Up.received_bytes + new_bytes);
}

static bool Uplink_AllReceived(void)
{
    return (s_Up.in_progress && s_Up.received_bytes >= s_Up.msg_len);
}

/* Scan bitmap */
static bool Uplink_FindNextGap(uint16_t *out_start, uint16_t *out_len)
{
    uint16_t i;
    uint16_t gap_start = 0;
    bool     in_gap = false;

    for (i = 0; i < s_Up.msg_len; i++)
    {
        uint16_t byte_idx = (uint16_t)(i / 8u);
        uint8_t  bit_mask = (uint8_t)(1u << (i % 8u));
        bool received = (s_Up.recv_bitmap[byte_idx] & bit_mask) != 0;

        if (!received && !in_gap)
        {
            gap_start = i;
            in_gap = true;
        }
        else if (received && in_gap)
        {
            *out_start = gap_start;
            uint16_t gap_len = (uint16_t)(i - gap_start);
            if (gap_len > (uint16_t)LTRX_UPLINK_PART_REQ_LEN)
            {
                gap_len = (uint16_t)LTRX_UPLINK_PART_REQ_LEN;
            }
            *out_len = gap_len;
            return true;
        }
    }

    if (in_gap)
    {
        *out_start = gap_start;
        uint16_t gap_len = (uint16_t)(s_Up.msg_len - gap_start);
        if (gap_len > (uint16_t)LTRX_UPLINK_PART_REQ_LEN)
        {
            gap_len = (uint16_t)LTRX_UPLINK_PART_REQ_LEN;
        }
        *out_len = gap_len;
        return true;
    }

    return false;  /* All received */
}

bool LTRX_Uplink_HasCompleteMessage(void)
{
    return s_Up.complete;
}

int32_t LTRX_Uplink_CopyOut(uint32_t *MessageID_out,
                            uint8_t  *dst,
                            uint16_t *dst_len_inout)
{
    if (!s_Up.complete)
    {
        return LTRX_ERROR_PROTOCOL;
    }
    if (MessageID_out == NULL || dst == NULL || dst_len_inout == NULL)
    {
        return LTRX_ERROR_NULL_PTR;
    }

    uint16_t cap = *dst_len_inout;
    if (cap < s_Up.msg_len)
    {
        *dst_len_inout = s_Up.msg_len;
        return LTRX_ERROR_PROTOCOL;
    }

    memcpy(dst, s_Up.buf, s_Up.msg_len);
    *dst_len_inout  = s_Up.msg_len;
    *MessageID_out = s_Up.msg_id;

    Uplink_Reset();
    return LTRX_SUCCESS;
}

void LTRX_Uplink_Clear(void)
{
    Uplink_Reset();
}

/* ---------------- helpers ---------------- */

static inline uint16_t ReadLe16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline uint32_t ReadLe32(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static inline int16_t ReadLeI16(const uint8_t *p)
{
    return (int16_t)ReadLe16(p);
}

static inline int32_t ReadLeI32(const uint8_t *p)
{
    return (int32_t)ReadLe32(p);
}

static inline uint64_t ReadLe64(const uint8_t *p)
{
    return (uint64_t)p[0]
         | ((uint64_t)p[1] << 8)
         | ((uint64_t)p[2] << 16)
         | ((uint64_t)p[3] << 24)
         | ((uint64_t)p[4] << 32)
         | ((uint64_t)p[5] << 40)
         | ((uint64_t)p[6] << 48)
         | ((uint64_t)p[7] << 56);
}

static int32_t Uplink_RequestPart(uint16_t start, uint16_t len)
{
    s_Up.last_part_start = start;
    s_Up.last_part_len   = len;
    return LTRX_RequestMessagePart(s_Up.msg_id, start, len);
}

static int32_t Uplink_RequestNextPart(void)
{
    uint16_t start = 0;
    uint16_t len   = 0;

    if (!s_Up.in_progress)
    {
        return LTRX_ERROR_PROTOCOL;
    }

    if (Uplink_AllReceived())
    {
        return LTRX_SUCCESS;
    }

    if (!Uplink_FindNextGap(&start, &len))
    {
        return LTRX_SUCCESS;
    }

    s_Up.last_part_retry = 0;
    return Uplink_RequestPart(start, len);
}

/* Helper: send bcn type15 ACK if type7 error */
static void SendErrorAckForRx(const LTRX_BeaconCmdHeader_t *hdr,
                               uint8_t status_code,
                               const char *desc)
{
    (void)LTRX_SendCommandAck(hdr, status_code, desc);
}

/* ---------------- RX-driven protocol handling ---------------- */

int32_t LTRX_BcnProcessOneRx(uint32_t timeout_ms)
{
    LTRX_BeaconCmdHeader_t hdr;
    uint8_t  payload[2048];
    uint16_t actual_len = 0;

    int32_t rc = LTRX_ReceiveCommand(&hdr, payload, sizeof(payload), &actual_len, timeout_ms);
    if (rc != LTRX_SUCCESS)
    {
        return rc;
    }

    const uint8_t type_id = hdr.TypeID;
    LTRX_APP_printf("LTRX_BcnProcessOneRx: from=%u to=%u type=%u payload_len=%u actual_len=%u\n",
              (unsigned)hdr.FromID,
              (unsigned)hdr.ToID,
              (unsigned)type_id,
              (unsigned)hdr.Length,
              (unsigned)actual_len);

    switch (type_id)
    {
        /* ---------- DOWNLINK (OBC -> GS) ---------- */

        case LTRX_BEACON_CMD_REQUEST_MSG_TX: /* 3 */
        {
            LTRX_APP_printf("LTRX case Type3 REQUEST_MSG_TX\n");

            rc = LTRX_Downlink_CommitPending();
            if (rc != LTRX_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_BCN_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: Type3 pending downlink commit failed rc=%ld", (long)rc);
                LTRX_SessionOnIcdRx(type_id, 1);
                return rc;
            }

            if (s_DownReady)
            {
                LTRX_SessionSetDownlinkMsgId(s_DownMsgId);
                rc = LTRX_SendMessageHeader(s_DownMsgId, s_DownLen, s_DownMsgCrc);
            }
            else
            {
                LTRX_SessionSetDownlinkMsgId(0);
                rc = LTRX_SendMessageHeader(0, 0, 0);
            }

            if (rc != LTRX_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_BCN_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: Type3 response SendMessageHeader failed rc=%ld", (long)rc);
            }

            LTRX_SessionOnIcdRx(type_id, (rc == LTRX_SUCCESS) ? 0 : 1);
            return rc;
        }

        case LTRX_BEACON_CMD_REQUEST_MSG_PART: /* 7 */
        {
            LTRX_APP_printf("LTRX case Type7 REQUEST_MSG_PART\n");
            /* ICD: MessageID(4) + PartStart(2) + PartLen(2) = 8 bytes */
            if (actual_len < 8)
            {
                SendErrorAckForRx(&hdr, LTRX_ACK_STATUS_LENGTH_UNEXPECTED, "Type7 too short");
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            uint32_t mid   = ReadLe32(&payload[0]);
            uint16_t start = ReadLe16(&payload[4]);
            uint16_t plen  = ReadLe16(&payload[6]);

            if (!s_DownReady || mid != s_DownMsgId)
            {
                SendErrorAckForRx(&hdr, LTRX_ACK_STATUS_OTHER_ERR, "no msg or mid mismatch");
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            if ((uint32_t)start + (uint32_t)plen > (uint32_t)s_DownLen)
            {
                SendErrorAckForRx(&hdr, LTRX_ACK_STATUS_PARSE_FAILED, "part out of range");
                LTRX_SessionOnIcdRx(type_id, 10);
                return LTRX_ERROR_PROTOCOL;
            }

            rc = LTRX_SendMessagePart(mid, start, plen, &s_DownBuf[start]);
            if (rc != LTRX_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_BCN_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: Type7 response SendMessagePart failed rc=%ld", (long)rc);
            }

            LTRX_APP_printf(" hex dump %x\n", s_DownBuf[start]);

            LTRX_SessionOnIcdRx(type_id, (rc == LTRX_SUCCESS) ? 0 : 1);
            return rc;
        }

        case LTRX_BEACON_CMD_CONFIRM_PART_RX: /* 10 */
        {
            LTRX_APP_printf("LTRX case Type10 CONFIRM_PART_RX\n");
            /* ICD: MessageID(4)+Start(2)+Len(2)+Status(1)=9, +ErrorDescription(25)=34 */
            if (actual_len < 34)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: Type10 too short (len=%u)", (unsigned)actual_len);
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            uint32_t mid   = ReadLe32(&payload[0]);
            uint16_t start = ReadLe16(&payload[4]);
            uint16_t plen  = ReadLe16(&payload[6]);
            uint8_t  st    = payload[8];

            if (st != 0 && actual_len >= 34)
            {
                char ErrorDescription[26];
                memset(ErrorDescription, 0, sizeof(ErrorDescription));
                memcpy(ErrorDescription, &payload[9], 25);
                ErrorDescription[25] = '\0';

                CFE_EVS_SendEvent(LTRX_BCN_PROTO_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: PartReceipt err st=%u mid=%u start=%u len=%u ErrorDescription='%s'",
                                  (unsigned)st, (unsigned)mid, (unsigned)start, (unsigned)plen, ErrorDescription);
            }

            LTRX_SessionOnIcdRx(type_id, st);
            return LTRX_SUCCESS;
        }

        case LTRX_BEACON_CMD_CONFIRM_MSG_RX: /* 11 */
        {
            LTRX_APP_printf("LTRX case Type11 CONFIRM_MSG_RX\n");
            if (actual_len < 5)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                "LTRX: Type11 too short (len=%u)", (unsigned)actual_len);
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            uint32_t mid = ReadLe32(&payload[0]);
            LTRX_SessionOnIcdRxEx(type_id, 0, mid);
            return LTRX_SUCCESS;
        }

        case LTRX_BEACON_CMD_MSG_STATUS_UPDATE: /* 13 */
        {
            LTRX_APP_printf("LTRX case Type13 MSG_STATUS_UPDATE\n");
            /* ICD: MessageID(4) + CurrentNode(1) + UpdateTimestamp(8) + ErrorCode(1) + ErrorDescription(25) = 39 */
            if (actual_len < 39)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                "LTRX: Type13 too short (len=%u)", (unsigned)actual_len);
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            LTRX_AppData.HaveMsgStatus = true;
            LTRX_AppData.LastMsgStatus.MessageID   = ReadLe32(&payload[0]);
            LTRX_AppData.LastMsgStatus.CurrentNode = payload[4];
            LTRX_AppData.LastMsgStatus.UpdateTimestamp    = ReadLe64(&payload[5]);
            LTRX_AppData.LastMsgStatus.ErrorCode   = payload[13];

            memset(LTRX_AppData.LastMsgStatus.ErrorDescription, 0, 25);
            memcpy(LTRX_AppData.LastMsgStatus.ErrorDescription, &payload[14], 25);

            LTRX_AppData.LastMsgStatus.ErrorDescription[24] = '\0';

            LTRX_SessionOnIcdRxEx(type_id, LTRX_AppData.LastMsgStatus.ErrorCode,
                       LTRX_AppData.LastMsgStatus.MessageID);
            return LTRX_SUCCESS;
        }

        case LTRX_BEACON_CMD_PREV_CMD_ACK: /* 15 */
        {
            LTRX_APP_printf("LTRX case Type15 PREV_CMD_ACK\n");
            if (actual_len < 35)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: Type15 too short (len=%u)", (unsigned)actual_len);
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            uint8_t  prev_from = payload[0];
            uint8_t  prev_to   = payload[1];
            uint8_t  prev_type = payload[2];
            uint16_t prev_len  = ReadLe16(&payload[3]);
            uint32_t prev_crc  = ReadLe32(&payload[5]);
            uint8_t  status    = payload[9];

            char ErrorDescription[26];
            memset(ErrorDescription, 0, sizeof(ErrorDescription));
            if (actual_len >= 35)
            {
                memcpy(ErrorDescription, &payload[10], 25);
                ErrorDescription[25] = '\0';
            }

            if (status != 0)
            {
                CFE_EVS_SendEvent(LTRX_BCN_PROTO_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: PrevCmdAck err st=%u prev(hdr:%u->%u type=%u len=%u crc=0x%08X) ErrorDescription='%s'",
                                  (unsigned)status,
                                  (unsigned)prev_from, (unsigned)prev_to, (unsigned)prev_type,
                                  (unsigned)prev_len, (unsigned)prev_crc,
                                  ErrorDescription);
                LTRX_SessionOnIcdRx(type_id, status);
            }
            else
            {
                CFE_EVS_SendEvent(LTRX_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "LTRX: PrevCmdAck ok prev(type=%u len=%u)",
                                  (unsigned)prev_type, (unsigned)prev_len);
                LTRX_SessionOnIcdRx(type_id, 0);
            }

            return LTRX_SUCCESS;
        }

        /* ---------- UPLINK (GS -> OBC) ---------- */

        case LTRX_BEACON_CMD_OFFER_RECEIVE_MSG: /* 6 */
        {
            LTRX_APP_printf("LTRX case Type6 OFFER_RECEIVE_MSG\n");
            /* ICD: MessageID(4) + MessageLength(2) + MessageCRC(4) = 10 bytes */
            if (actual_len < 10)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: Type6 too short (len=%u)", (unsigned)actual_len);
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            uint32_t offered_mid  = ReadLe32(&payload[0]);
            uint16_t offered_mlen = ReadLe16(&payload[4]);
            uint32_t offered_mcrc = ReadLe32(&payload[6]);

            if (offered_mlen == 0 || offered_mlen > LTRX_UPLINK_MAX_LEN)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: Type6 offered length invalid (%u)", (unsigned)offered_mlen);
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            Uplink_Reset();
            s_Up.in_progress = true;
            s_Up.msg_id      = offered_mid;
            s_Up.msg_len     = offered_mlen;
            s_Up.msg_crc     = offered_mcrc;

            rc = LTRX_ConfirmReadyForMessage(); /* 8 */
            if (rc != LTRX_SUCCESS)
            {
                CFE_EVS_SendEvent(LTRX_BCN_TX_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LTRX: Type6 response ConfirmReady failed rc=%ld", (long)rc);
                Uplink_Reset();
            }

            LTRX_SessionOnIcdRx(type_id, (rc == LTRX_SUCCESS) ? 0 : 1);
            if (rc == LTRX_SUCCESS)
            {
                LTRX_SessionNotifyUplinkOffered();
            }
            return rc;
        }

        case LTRX_BEACON_CMD_SEND_MSG_HEADER: /* 5 */
        {
            LTRX_APP_printf("LTRX case Type5 SEND_MSG_HEADER\n");
            if (actual_len < 10)
            {
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            uint32_t mid  = ReadLe32(&payload[0]);
            uint16_t mlen = ReadLe16(&payload[4]);
            uint32_t mcrc = ReadLe32(&payload[6]);

            if (mlen == 0 || mlen > LTRX_UPLINK_MAX_LEN)
            {
                LTRX_SessionOnIcdRx(type_id, 10);
                return LTRX_ERROR_PROTOCOL;
            }

            /* If Type6 already pre-populated s_Up, validate consistency */
            if (s_Up.in_progress && s_Up.msg_id != 0)
            {
                if (s_Up.msg_id != mid || s_Up.msg_len != mlen || s_Up.msg_crc != mcrc)
                {
                    CFE_EVS_SendEvent(LTRX_BCN_PROTO_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "LTRX: Type5 mismatch with Type6 offer (mid %u!=%u or len %u!=%u)",
                                      (unsigned)s_Up.msg_id, (unsigned)mid,
                                      (unsigned)s_Up.msg_len, (unsigned)mlen);
                    Uplink_Reset();
                    LTRX_SessionOnIcdRx(type_id, 30);
                    return LTRX_ERROR_PROTOCOL;
                }
                /* consistent - reset bitmap and start */
                s_Up.received_bytes = 0;
                s_Up.next_request_offset = 0;                       
                memset(s_Up.recv_bitmap, 0, sizeof(s_Up.recv_bitmap));
            }
            else
            {
                /* Type6 not received or was skipped - init fresh */
                Uplink_Reset();
                s_Up.in_progress = true;
                s_Up.msg_id      = mid;
                s_Up.msg_len     = mlen;
                s_Up.msg_crc     = mcrc;
            }

            rc = Uplink_RequestNextPart();
            LTRX_SessionOnIcdRx(type_id, (rc == LTRX_SUCCESS) ? 0 : 1);
            return rc;
        }

        case LTRX_BEACON_CMD_SEND_MSG_PART: /* 9 */
        {
            LTRX_APP_printf("LTRX case Type9 SEND_MSG_PART\n");
            /* ICD: MessageID(4)+Start(2)+Len(2)+PartCRC(4)+Bytes(N) */
            if (actual_len < 12)
            {
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            uint32_t mid   = ReadLe32(&payload[0]);
            uint16_t start = ReadLe16(&payload[4]);
            uint16_t plen  = ReadLe16(&payload[6]);
            uint32_t pcrc  = ReadLe32(&payload[8]);

            const uint16_t data_len = (uint16_t)(actual_len - 12);

            if (!s_Up.in_progress || mid != s_Up.msg_id)
            {
                (void)LTRX_ConfirmMessagePartReceipt(mid, start, plen, 30, "unexpected");
                LTRX_SessionOnIcdRx(type_id, 30);
                return LTRX_ERROR_PROTOCOL;
            }

            if (plen != data_len)
            {
                (void)LTRX_ConfirmMessagePartReceipt(mid, start, plen, 10, "len mismatch");
                LTRX_SessionOnIcdRx(type_id, 10);
                return LTRX_ERROR_PROTOCOL;
            }

            if ((uint32_t)start + (uint32_t)plen > (uint32_t)s_Up.msg_len)
            {
                (void)LTRX_ConfirmMessagePartReceipt(mid, start, plen, 10, "out of range");
                LTRX_SessionOnIcdRx(type_id, 10);
                return LTRX_ERROR_PROTOCOL;
            }

            const uint8_t *bytes = &payload[12];
            uint32_t calc = LTRX_CalculateCRC32(bytes, plen);

            if (calc != pcrc)
            {
                (void)LTRX_ConfirmMessagePartReceipt(mid, start, plen, 20, "crc mismatch");

                if (start == s_Up.last_part_start && plen == s_Up.last_part_len)
                {
                    if (s_Up.last_part_retry < LTRX_UPLINK_MAX_PART_RETRY)
                    {
                        s_Up.last_part_retry++;
                        (void)Uplink_RequestPart(start, plen);
                    }
                    else
                    {
                        CFE_EVS_SendEvent(LTRX_SESS_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "LTRX: uplink part retry exceeded (mid=%u start=%u len=%u)",
                                          (unsigned)mid, (unsigned)start, (unsigned)plen);
                        Uplink_Reset();
                    }
                }
                else
                {
                    s_Up.last_part_retry = 0;
                    (void)Uplink_RequestPart(start, plen);
                }

                LTRX_SessionOnIcdRx(type_id, 20);
                return LTRX_SUCCESS;
            }

            memcpy(&s_Up.buf[start], bytes, plen);
            Uplink_MarkReceived(start, plen);

            (void)LTRX_ConfirmMessagePartReceipt(mid, start, plen, 0, NULL);
            LTRX_SessionOnIcdRx(type_id, 0);

            if (Uplink_AllReceived())
            {
                uint32_t full = LTRX_CalculateCRC32(s_Up.buf, s_Up.msg_len);
                if (full == s_Up.msg_crc)
                {
                    (void)LTRX_ConfirmMessageReceipt(s_Up.msg_id, LTRX_NODE_OBC);
                    s_Up.complete    = true;
                    s_Up.in_progress = false;
                }
                else
                {
                    CFE_EVS_SendEvent(LTRX_SESS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "LTRX: uplink full CRC mismatch (mid=%u)", (unsigned)s_Up.msg_id);
                    Uplink_Reset();
                    LTRX_SessionOnIcdRx(type_id, 30);
                }

                return LTRX_SUCCESS;
            }

            (void)Uplink_RequestNextPart();
            return LTRX_SUCCESS;
        }

        case LTRX_BEACON_CMD_GNSS_INFO: /* 21 */
        {
            LTRX_APP_printf("LTRX case Type21 GNSS_INFO\n");
            /* ICD payload: UTCTime(4) Lat(4) Lon(4) Alt(4) Fix(1) NumSat(1) = 18 */
            if (actual_len < 18)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                "LTRX: Type21 too short (len=%u)", (unsigned)actual_len);
                return LTRX_ERROR_PROTOCOL;
            }

            LTRX_AppData.LastGnss.UTCTimeMs       = ReadLe32(&payload[0]);
            LTRX_AppData.LastGnss.Latitude      = ReadLeI32(&payload[4]);
            LTRX_AppData.LastGnss.Longitude     = ReadLeI32(&payload[8]);
            LTRX_AppData.LastGnss.Altitude      = ReadLe32(&payload[12]);
            LTRX_AppData.LastGnss.FixQuality    = payload[16];
            LTRX_AppData.LastGnss.NumSatellites = payload[17];
            LTRX_AppData.HaveGnss               = true;

            /* optional: session notify */
            LTRX_SessionOnIcdRx(type_id, 0);
            return LTRX_SUCCESS;
        }

        case LTRX_BEACON_CMD_BEACON_STATUS: /* 23 */
        {
            LTRX_APP_printf("LTRX case Type23 BEACON_STATUS\n");
            /* ICD payload size: 52 bytes */
            if (actual_len < 52)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                "LTRX: Type23 too short (len=%u)", (unsigned)actual_len);
                return LTRX_ERROR_PROTOCOL;
            }

            LTRX_BeaconStatus_Payload_t *st = &LTRX_AppData.LastBeaconStatus;

            st->Version           = ReadLe16(&payload[0]);
            st->Temperature       = ReadLeI16(&payload[2]);
            st->AngularVelocityX  = ReadLeI32(&payload[4]);
            st->AngularVelocityY  = ReadLeI32(&payload[8]);
            st->AngularVelocityZ  = ReadLeI32(&payload[12]);
            st->AccelerationX     = ReadLeI32(&payload[16]);
            st->AccelerationY     = ReadLeI32(&payload[20]);
            st->AccelerationZ     = ReadLeI32(&payload[24]);
            st->ConnectionQuality = payload[28];
            st->BatteryIsCharging = payload[29];
            st->BatteryCapacity   = ReadLe16(&payload[30]);
            memcpy(st->Reserved, &payload[32], sizeof(st->Reserved));


            LTRX_AppData.HaveBeaconStatus = true;

            LTRX_SessionOnIcdRx(type_id, 0);
            return LTRX_SUCCESS;
        }

        case LTRX_BEACON_CMD_BEACON_STATUS_FULL: /* 24 */
        {
            LTRX_APP_printf("LTRX case Type24 BEACON_STATUS_FULL\n");
            if (actual_len < 70)
            {
                CFE_EVS_SendEvent(LTRX_BCN_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                                "LTRX: Type24 too short (len=%u)", (unsigned)actual_len);
                return LTRX_ERROR_PROTOCOL;
            }

            /* GNSS portion (offset 0..17) -> update LastGnss */
            LTRX_AppData.LastGnss.UTCTimeMs       = ReadLe32(&payload[0]);
            LTRX_AppData.LastGnss.Latitude         = ReadLeI32(&payload[4]);
            LTRX_AppData.LastGnss.Longitude        = ReadLeI32(&payload[8]);
            LTRX_AppData.LastGnss.Altitude         = ReadLe32(&payload[12]);
            LTRX_AppData.LastGnss.FixQuality       = payload[16];
            LTRX_AppData.LastGnss.NumSatellites    = payload[17];
            LTRX_AppData.HaveGnss                  = true;

            /* Status portion (offset 18..69) -> update LastBeaconStatus */
            LTRX_BeaconStatus_Payload_t *st = &LTRX_AppData.LastBeaconStatus;
            st->Version           = ReadLe16(&payload[18]);
            st->Temperature       = ReadLeI16(&payload[20]);
            st->AngularVelocityX  = ReadLeI32(&payload[22]);
            st->AngularVelocityY  = ReadLeI32(&payload[26]);
            st->AngularVelocityZ  = ReadLeI32(&payload[30]);
            st->AccelerationX     = ReadLeI32(&payload[34]);
            st->AccelerationY     = ReadLeI32(&payload[38]);
            st->AccelerationZ     = ReadLeI32(&payload[42]);
            st->ConnectionQuality = payload[46];
            st->BatteryIsCharging = payload[47];
            st->BatteryCapacity   = ReadLe16(&payload[48]);
            memcpy(st->Reserved, &payload[50], sizeof(st->Reserved));
            LTRX_AppData.HaveBeaconStatus = true;

            /* Store combined struct */
            LTRX_AppData.HaveBeaconStatusFull = true;
            memcpy(&LTRX_AppData.LastBeaconStatusFull, payload, 70);

            LTRX_APP_printf("LTRX Type24 GNSS: utc=%u lat=%ld lon=%ld alt=%u fix=%u sats=%u\n",
                      (unsigned)LTRX_AppData.LastGnss.UTCTimeMs,
                      (long)LTRX_AppData.LastGnss.Latitude,
                      (long)LTRX_AppData.LastGnss.Longitude,
                      (unsigned)LTRX_AppData.LastGnss.Altitude,
                      (unsigned)LTRX_AppData.LastGnss.FixQuality,
                      (unsigned)LTRX_AppData.LastGnss.NumSatellites);

            LTRX_APP_printf("LTRX Type24 STATUS: ver=%u temp=%d ang=(%ld,%ld,%ld) acc=(%ld,%ld,%ld) cq=%u charging=%u batt=%u\n",
                      (unsigned)LTRX_AppData.LastBeaconStatus.Version,
                      (int)LTRX_AppData.LastBeaconStatus.Temperature,
                      (long)LTRX_AppData.LastBeaconStatus.AngularVelocityX,
                      (long)LTRX_AppData.LastBeaconStatus.AngularVelocityY,
                      (long)LTRX_AppData.LastBeaconStatus.AngularVelocityZ,
                      (long)LTRX_AppData.LastBeaconStatus.AccelerationX,
                      (long)LTRX_AppData.LastBeaconStatus.AccelerationY,
                      (long)LTRX_AppData.LastBeaconStatus.AccelerationZ,
                      (unsigned)LTRX_AppData.LastBeaconStatus.ConnectionQuality,
                      (unsigned)LTRX_AppData.LastBeaconStatus.BatteryIsCharging,
                      (unsigned)LTRX_AppData.LastBeaconStatus.BatteryCapacity);

            LTRX_SessionOnIcdRx(type_id, 0);
            return LTRX_SUCCESS;
        }

        default:
        {
            LTRX_APP_printf("LTRX case UNKNOWN type=%u actual_len=%u\n",
                      (unsigned)type_id, (unsigned)actual_len);
            /* Type15 ACK for unknown type id*/
            SendErrorAckForRx(&hdr, LTRX_ACK_STATUS_PARSE_FAILED, "unknown type id");
            LTRX_SessionOnIcdRx(type_id, 0);
            return LTRX_SUCCESS;
        }
    }
}

/* ---- BUS BEACON -> DOWNLINK staging ---- */

void LTRX_OnBusBeaconReceived(const CFE_SB_Buffer_t *SBBufPtr)
{
    size_t msg_size = 0;

    if (!s_DownstreamEnabled)
    {
        LTRX_APP_printf("LTRX: bus beacon ignored, downstream disabled\n");
        return;
    }

    if (CFE_MSG_GetSize(&SBBufPtr->Msg, &msg_size) != CFE_SUCCESS ||
        msg_size <= LTRX_HK_COMBINED_PAYLOAD_OFFSET)
    {
        return;
    }

    s_DownstreamBeaconCount++;
    if (s_DownstreamBeaconCount < s_DownstreamBeaconPeriod)
    {
        return;
    }
    s_DownstreamBeaconCount = 0u;
 
    const uint8_t *payload = ((const uint8_t *)SBBufPtr) + LTRX_HK_COMBINED_PAYLOAD_OFFSET;
    uint16_t payload_len = (uint16_t)(msg_size - LTRX_HK_COMBINED_PAYLOAD_OFFSET);
 
    if (payload_len > LTRX_DOWNLINK_MAX_LEN)
    {
        payload_len = LTRX_DOWNLINK_MAX_LEN;
    }

    /* MessageID = CFE TIME seconds (monotonic across sessions) */
    static uint32_t s_LastMsgId = 0;
    uint32_t msg_id = (uint32_t)CFE_TIME_GetTime().Seconds;
    if (msg_id == s_LastMsgId)
    {
        msg_id++;
    }
    s_LastMsgId = msg_id;

    LTRX_APP_printf("LTRX: bus beacon received, stage msg_id=%u len=%u\n", (unsigned)msg_id, (unsigned)payload_len);
    memcpy(s_PendingDownBuf, payload, payload_len);
    s_PendingDownLen   = payload_len;
    s_PendingDownMsgId = msg_id;
    s_PendingDownReady = true;
}
 
/* ---- UPLINK FORWARD to SB ---- */
 
void LTRX_Uplink_ForwardToSB(void)
{
    if (!LTRX_Uplink_HasCompleteMessage())
    {
        return;
    }
    LTRX_APP_printf("LTRX: complete uplink pending SB forward\n");
 
    uint32_t msg_id = 0;
    uint8_t  buf[LTRX_UPLINK_MAX_LEN];
    uint16_t len = LTRX_UPLINK_MAX_LEN;
 
    if (LTRX_Uplink_CopyOut(&msg_id, buf, &len) != LTRX_SUCCESS)
    {
        return;
    }
 
    CFE_Status_t status = CFE_SB_TransmitMsg((CFE_MSG_Message_t *)buf, false);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LTRX_UL_FWD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: uplink forward to SB failed (mid=%u len=%u RC=0x%08lX)",
                          (unsigned)msg_id, (unsigned)len, (unsigned long)status);
    }
    else
    {
        CFE_EVS_SendEvent(LTRX_UL_FWD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "LTRX: uplink forwarded to SB (mid=%u len=%u)",
                          (unsigned)msg_id, (unsigned)len);
    }
}
