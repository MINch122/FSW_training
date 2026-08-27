/**
 * @file thrust_cmds_list.c
 * @brief iG4U Device 레이어 구현 (ICD D02 기준)
 *
 * [ICD 내부 표기 주의]
 *   일부 표의 Length 값은 바로 아래 필드/오프셋 합계와 다르다.
 *   이 구현은 필드 정의와 바이너리 오프셋으로 계산되는 payload 길이를 사용한다.
 *
 * [직렬화 방식]
 *   모든 멀티바이트 값은 ICD의 LSB First에 맞춰 명시적으로 직렬화한다.
 *
 * [수신 방식]
 *   RecvPacket()에서 데이터를 수신
 *   헤더를 1바이트씩 읽어온 후, 0xAA55가 맞는 건지 확인
 *   이후 Version, MsgType, MsgID, Length 순으로 한 바이트씩 읽어온다
 *   Length를 고려하여 Payload와 CRC를 한 번에 읽은 후 CRC를 검증한다.
 *
 * [역할 분리]
 *   Device 레이어: 패킷 송수신 + CRC 검증 + DefaultResponse 파싱 → outResp 전달
 *   App 레이어:    outResp 해석 + AppData 갱신 + EVS 이벤트 (PostCmdProcess)
 *
 * [TBD]
 *   THRUST_RECV_TIMEOUT_MS — 장치 응답 시간 측정 후 확정 필요
 */

#include "thrust_cmds_list.h"
#include "thrust_packet_defs.h"
#include <stdbool.h>
#include "thrust_mission_cfg.h"
#include "thrust_crc.h"
#include "thrust_hal.h"
#include <string.h>

/* 송수신 버퍼 크기
 * Header(2)+Version(1)+MsgType(1)+MsgID(1)+Length(1)+Payload(N)+CRC(2) */
#define THRUST_TX_BUF_SIZE  (6 + THRUST_N_MAX + 2)
#define THRUST_RX_BUF_SIZE  (6 + THRUST_N_MAX + 2)


static THRUST_TransportFn_t thrust_transport = NULL;
static uint8_t thrust_last_response[THRUST_RX_BUF_SIZE];
static uint16_t thrust_last_response_size = 0;

static void THRUST_ResetLastResponse(void)
{
    memset(thrust_last_response, 0, sizeof(thrust_last_response));
    thrust_last_response_size = 0;
}

static void THRUST_RecordLastResponse(const uint8_t *data, uint16_t size)
{
    size_t copy_size;

    if (data == NULL || size == 0 || thrust_last_response_size >= sizeof(thrust_last_response))
    {
        return;
    }

    copy_size = size;
    if (copy_size > sizeof(thrust_last_response) - thrust_last_response_size)
    {
        copy_size = sizeof(thrust_last_response) - thrust_last_response_size;
    }

    memcpy(&thrust_last_response[thrust_last_response_size], data, copy_size);
    thrust_last_response_size += (uint16_t)copy_size;
}

static uint16_t THRUST_ReadU16LE(const uint8_t *buf)
{
    return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

#if THRUST_DEBUG_OS_PRINT
static void THRUST_DebugPrintHexLine(const char *label, uint8_t msg_id,
                                     const uint8_t *buf, uint32_t len)
{
    uint32_t i;

    OS_printf("[THRUST][DBG] %s MsgID=%u Len=%lu HEX=",
              label, (unsigned int)msg_id, (unsigned long)len);

    for (i = 0; i < len; ++i)
    {
        OS_printf("%02X", (unsigned int)buf[i]);
        if ((i + 1) < len)
        {
            OS_printf(" ");
        }
    }

    OS_printf("\n");
}

static void THRUST_DebugPrintDefaultResponse(uint8_t msg_id,
                                             const THRUST_DefaultResponse_t *resp)
{
    OS_printf("[THRUST][DBG] RX Parsed DefaultResp MsgID=%u ResultCode=%u CurrentStatus=0x%04X FaultFlags=0x%04X InterlockFlags=0x%04X\n",
              (unsigned int)msg_id,
              (unsigned int)resp->ResultCode,
              (unsigned int)resp->CurrentStatus,
              (unsigned int)resp->FaultFlags,
              (unsigned int)resp->InterlockFlags);
}

static void THRUST_DebugPrintHKData(const THRUST_HKData_Payload_t *hk)
{
    OS_printf("[THRUST][DBG] RX Parsed HK Timestamp_ms=%lu Pressure_CH0=%f Pressure_CH1=%f Pressure_CH2=%f Pressure_CH3=%f\n",
              (unsigned long)hk->Timestamp_ms,
              (double)hk->Pressure_CH0,
              (double)hk->Pressure_CH1,
              (double)hk->Pressure_CH2,
              (double)hk->Pressure_CH3);
    OS_printf("[THRUST][DBG] RX Parsed HK Temp_CH0=%f Temp_CH1=%f Temp_CH2=%f Temp_CH3=%f Temp_CH4=%f Temp_CH5=%f Status=0x%08lX\n",
              (double)hk->Temp_CH0,
              (double)hk->Temp_CH1,
              (double)hk->Temp_CH2,
              (double)hk->Temp_CH3,
              (double)hk->Temp_CH4,
              (double)hk->Temp_CH5,
              (unsigned long)hk->Status);
}

static void THRUST_DebugPrintStatusData(const THRUST_StatusData_Payload_t *status)
{
    OS_printf("[THRUST][DBG] RX Parsed Status CurrentMode=%lu LastMsgId=%u LastResult=%u ResultCode=%u CurrentStatus=0x%04X FaultFlags=0x%04X InterlockFlags=0x%04X Reserved=0x%04X\n",
              (unsigned long)status->CurrentMode,
              (unsigned int)status->LastMsgId,
              (unsigned int)status->LastResult,
              (unsigned int)status->ResultCode,
              (unsigned int)status->CurrentStatus,
              (unsigned int)status->FaultFlags,
              (unsigned int)status->InterlockFlags,
              (unsigned int)status->Reserved);
}

static void THRUST_DebugPrintFaultLog(const THRUST_FaultLogData_Payload_t *faultLog)
{
    uint32_t i;

    OS_printf("[THRUST][DBG] RX Parsed FaultLog ReservedLen=%lu ReservedHEX=",
              (unsigned long)sizeof(faultLog->Reserved));

    for (i = 0; i < sizeof(faultLog->Reserved); ++i)
    {
        OS_printf("%02X", (unsigned int)faultLog->Reserved[i]);
        if ((i + 1) < sizeof(faultLog->Reserved))
        {
            OS_printf(" ");
        }
    }

    OS_printf("\n");
}
#else
#define THRUST_DebugPrintHexLine(label, msg_id, buf, len) \
    do {                                                  \
        (void)(label);                                    \
        (void)(msg_id);                                   \
        (void)(buf);                                      \
        (void)(len);                                      \
    } while (0)
#define THRUST_DebugPrintDefaultResponse(msg_id, resp) \
    do {                                               \
        (void)(msg_id);                                \
        (void)(resp);                                  \
    } while (0)
#define THRUST_DebugPrintHKData(hk) \
    do {                            \
        (void)(hk);                 \
    } while (0)
#define THRUST_DebugPrintStatusData(status) \
    do {                                    \
        (void)(status);                     \
    } while (0)
#define THRUST_DebugPrintFaultLog(faultLog) \
    do {                                    \
        (void)(faultLog);                   \
    } while (0)
#endif

void THRUST_RegisterTransport(THRUST_TransportFn_t fn)
{
    thrust_transport = fn;
}

void THRUST_GetLastResponse(const uint8_t **data, uint16_t *size)
{
    if (data != NULL)
    {
        *data = thrust_last_response;
    }
    if (size != NULL)
    {
        *size = thrust_last_response_size;
    }
}

static int32 ValidatePacketMeta(const uint8_t *buf, uint8_t payload_len,
                                uint8_t expected_msg_type,
                                uint8_t expected_msg_id,
                                uint8_t expected_payload_len)
{
    if (buf[3] != expected_msg_type ||
        buf[4] != expected_msg_id ||
        payload_len != expected_payload_len)
    {
        return THRUST_ERR_PACKET;
    }

    return CFE_SUCCESS;
}


/* ===================================================================
 * 내부 헬퍼: 패킷 구성 및 송신
 *
 * [패킷 조립 순서]
 *   tx_buf[0]   = 0xAA (Header MSB)
 *   tx_buf[1]   = 0x55 (Header LSB)
 *   tx_buf[2]   = Version
 *   tx_buf[3]   = MsgType
 *   tx_buf[4]   = MsgID
 *   tx_buf[5]   = Length
 *   tx_buf[6..] = Payload
 *   tx_buf[6+N] = CRC LSB
 *   tx_buf[7+N] = CRC MSB
 *
 * [CRC 범위 - ICD Section 4.3]
 *   tx_buf[2](Version)부터 tx_buf[5+N](Payload 마지막)까지
 * =================================================================== */
static int32 SendPacket(uint8_t msg_type, uint8_t msg_id,
                         uint8_t payload_len, const uint8_t *payload)
{
    uint8_t  tx_buf[THRUST_TX_BUF_SIZE];
    uint32_t idx = 0;
    uint16_t crc;

    THRUST_ResetLastResponse();

    tx_buf[idx++] = 0xAA;
    tx_buf[idx++] = 0x55;
    tx_buf[idx++] = THRUST_PKT_VERSION;
    tx_buf[idx++] = msg_type;
    tx_buf[idx++] = msg_id;
    tx_buf[idx++] = payload_len;

    if (payload != NULL && payload_len > 0)
    {
        memcpy(&tx_buf[idx], payload, payload_len);
    }
    idx += payload_len;

    /* CRC: Version(tx_buf[2])부터 Payload 마지막까지 */
    crc = THRUST_CalcCRC16(&tx_buf[2], 4 + (uint32_t)payload_len);
    tx_buf[idx++] = (uint8_t)( crc       & 0xFF);  /* LSB */
    tx_buf[idx++] = (uint8_t)((crc >> 8) & 0xFF);  /* MSB */
    if (thrust_transport == NULL)
    {
        return THRUST_ERR_WRITE;
    }

    THRUST_DebugPrintHexLine("TX", msg_id, tx_buf, idx);

    if (thrust_transport(tx_buf, idx, NULL, 0, 0) < 0)
    {
        return THRUST_ERR_WRITE;
    }
    return CFE_SUCCESS;
}

/* ===================================================================
 * 내부 헬퍼: 패킷 수신 (바이트 단위, 헤더 동기화 포함)
 *
 * 흐름:
 *   1. 1바이트씩 읽어 0xAA 탐색 (헤더 동기화)
 *   2. 다음 바이트 0x55 확인 → 불일치 시 1로 재시도
 *   3. Version, MsgType, MsgID 각 1바이트 읽기
 *   4. Length(5번째 필드, buf[5]) 읽기
 *   5. Length 바이트만큼 Payload 읽기
 *   6. CRC 2바이트 읽기 + 검증
 *
 * 출력 버퍼 레이아웃:
 *   buf[0-1]        = Header (0xAA, 0x55)
 *   buf[2]          = Version
 *   buf[3]          = MsgType
 *   buf[4]          = MsgID
 *   buf[5]          = Length (= N)
 *   buf[6..6+N-1]   = Payload
 *   buf[6+N]        = CRC LSB
 *   buf[6+N+1]      = CRC MSB
 * =================================================================== */
static int32 RecvPacket(uint8_t *buf, uint8_t *out_payload_len)
{
    // uint8_t  byte;
    uint32_t  payload_len;
    uint16_t crc_calc;
    uint16_t crc_recv;
    uint32_t i;

    if (thrust_transport == NULL)
    {
        return THRUST_ERR_TIMEOUT;
    }

    /* Step 1-2: 헤더 동기화
     * 0xAA를 찾고, 다음 바이트가 0x55이면 패킷 시작.
     * 0x55가 아니면 0xAA 탐색부터 재시도. */
    // int32_t ret;

    // 1. 헤더 2바이트 읽기
    for (i = 0; i <= 1; i++)
    {
        if (thrust_transport( NULL, 0, &buf[i], 1, THRUST_RECV_TIMEOUT1_MS) < 0)
        {
            return THRUST_ERR_TIMEOUT;
        }
        THRUST_RecordLastResponse(&buf[i], 1);
    }

    // 2. 헤더 즉시 검증
    if (buf[0] != 0xAA || buf[1] != 0x55) 
    {
        THRUST_DebugPrintHexLine("RX", 0, buf, 2);
        return THRUST_ERR_HEADER;
    }

    /* Step 3: Version(buf[2]), MsgType(buf[3]), MsgID(buf[4]) — 각 1바이트 */
    for (i = 2; i <= 5; i++)
    {
        if (thrust_transport( NULL, 0, &buf[i], 1, THRUST_RECV_TIMEOUT1_MS) < 0)
        {
            return THRUST_ERR_TIMEOUT;
        }
        THRUST_RecordLastResponse(&buf[i], 1);
    }
    payload_len = buf[5];
    if (payload_len > THRUST_N_MAX)
    {
        THRUST_DebugPrintHexLine("RX", buf[4], buf, 6);
        return THRUST_ERR_PACKET;
    }

    /* Step 5: Payload (payload_len 바이트) + 2byte로 crc를 같이 가져옴 */
    if (thrust_transport( NULL, 0, &buf[6], payload_len + 2, THRUST_RECV_TIMEOUT2_MS) < 0)
    {
        return THRUST_ERR_TIMEOUT;
    }
    THRUST_RecordLastResponse(&buf[6], (uint16_t)(payload_len + 2));

    THRUST_DebugPrintHexLine("RX", buf[4], buf, 6 + payload_len + 2);

    /* CRC 검증: Version(buf[2])부터 Payload 마지막까지 */
    crc_calc = THRUST_CalcCRC16(&buf[2], 4 + (uint32_t)payload_len);
    crc_recv = (uint16_t)buf[6 + payload_len] |
               ((uint16_t)buf[6 + payload_len + 1] << 8);
    if (crc_calc != crc_recv)
    {
        return THRUST_ERR_CRC;
    }

    *out_payload_len = payload_len;
    return CFE_SUCCESS;
}

/* ===================================================================
 * 내부 헬퍼: Default Response 수신 및 파싱
 *
 * RecvPacket() 후 buf[6..13]에서 ResultCode, CurrentStatus,
 * FaultFlags, InterlockFlags를 추출하여 outResp에 채운다.
 * AppData 갱신은 App 팀(PostCmdProcess) 담당.
 *
 * 수신 버퍼 레이아웃:
 *   buf[0-1] = Header
 *   buf[2]   = Version
 *   buf[3]   = MsgType
 *   buf[4]   = MsgID
 *   buf[5]   = Length (= payload_len)
 *   buf[6..] = Payload
 *   buf[6+payload_len]   = CRC LSB
 *   buf[6+payload_len+1] = CRC MSB
 * =================================================================== */
static int32 RecvDefaultResponse(uint8_t requested_msg_id,
                                 THRUST_DefaultResponse_t *outResp)
{
    uint8_t rx_buf[THRUST_RX_BUF_SIZE];
    uint8_t payload_len = 0;
    int32   ret;

    ret = RecvPacket(rx_buf, &payload_len);
    if (ret != CFE_SUCCESS) { return ret; }

    ret = ValidatePacketMeta(rx_buf, payload_len, THRUST_PKT_MSGTYPE_TM,
                             requested_msg_id, THRUST_N_DEFAULT_RESP);
    if (ret != CFE_SUCCESS) { return ret; }

    if (outResp != NULL)
    {
        uint16_t result_code = THRUST_ReadU16LE(&rx_buf[6]);

        outResp->ResultCode     = result_code;
        outResp->CurrentStatus  = THRUST_ReadU16LE(&rx_buf[8]);
        outResp->FaultFlags     = THRUST_ReadU16LE(&rx_buf[10]);
        outResp->InterlockFlags = THRUST_ReadU16LE(&rx_buf[12]);

        THRUST_DebugPrintDefaultResponse(requested_msg_id, outResp);

        if (result_code != 0) { return THRUST_ERR_NACK; }
    }

    return CFE_SUCCESS;
}

/* ===================================================================
 * Group 1: 기본 제어 명령
 * =================================================================== */

/* MsgID=1  PING (Length=0) */
int32 THRUST_Ping(THRUST_DefaultResponse_t *outResp)
{
    int32 ret;

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 1, THRUST_N_PING, NULL);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(1, outResp);
}

/* MsgID=2  RESET MODULE (Length=4)
 * Payload: Target Mode (UINT32) */
int32 THRUST_ResetModule(uint32_t targetMode, THRUST_DefaultResponse_t *outResp)
{
    int32   ret;
    uint8_t payload[THRUST_N_RESET_MODULE];

    PACK_U32_LE(payload, targetMode);

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 2, THRUST_N_RESET_MODULE, payload);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(2, outResp);
}

/* MsgID=3  SET MODE (Length=4)
 * Payload: Target Mode (UINT32) [TBD: 실제 모드 값 확정 필요] */
int32 THRUST_SetMode(uint32_t targetMode, THRUST_DefaultResponse_t *outResp)
{
    int32   ret;
    uint8_t payload[THRUST_N_SET_MODE];

    PACK_U32_LE(payload, targetMode);

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 3, THRUST_N_SET_MODE, payload);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(3, outResp);
}

/* MsgID=4  ARM PROPULSION (Length=4)
 * Payload: Arm Key(0xA55A, UINT16) + Timeout(Second, UINT16) */
int32 THRUST_ArmPropulsion(uint16_t armKey, uint16_t timeoutSec,
                            THRUST_DefaultResponse_t *outResp)
{
    int32   ret;
    uint8_t payload[THRUST_N_ARM];

    PACK_U16_LE(&payload[0], armKey);
    PACK_U16_LE(&payload[2], timeoutSec);

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 4, THRUST_N_ARM, payload);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(4, outResp);
}

/* MsgID=5  DISARM PROPULSION (Length=0) */
int32 THRUST_DisarmPropulsion(THRUST_DefaultResponse_t *outResp)
{
    int32 ret;

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 5, THRUST_N_DISARM, NULL);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(5, outResp);
}

/* ===================================================================
 * Group 2: HK / 상태 요청
 * Payload 전체를 App 계층으로 전달 — 해석은 App 팀 담당
 * =================================================================== */

/* MsgID=10 REQUEST HK DATA (Length=0)
 * 응답: Reply HK Data (48바이트) → THRUST_HKData_Payload_t */
int32 THRUST_GetHKData(THRUST_HKData_Payload_t *returnVal)
{
    uint8_t rx_buf[THRUST_RX_BUF_SIZE];
    uint8_t payload_len = 0;
    int32   ret;

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 10, THRUST_N_REQUEST_HK, NULL);
    if (ret != CFE_SUCCESS) { return ret; }

    ret = RecvPacket(rx_buf, &payload_len);
    if (ret != CFE_SUCCESS) { return ret; }

    ret = ValidatePacketMeta(rx_buf, payload_len, THRUST_PKT_MSGTYPE_TM,
                             10, THRUST_N_REPLY_HK);
    if (ret != CFE_SUCCESS) { return ret; }

    if (returnVal != NULL)
    {
        memcpy(returnVal, &rx_buf[6], sizeof(THRUST_HKData_Payload_t));
        THRUST_DebugPrintHKData(returnVal);
    }

    return CFE_SUCCESS;
}

/* MsgID=11 REQUEST STATUS (Length=0)
 * 응답: Reply Status 필드/오프셋 합계 16바이트 → THRUST_StatusData_Payload_t */
int32 THRUST_GetStatus(THRUST_StatusData_Payload_t *returnVal)
{
    uint8_t rx_buf[THRUST_RX_BUF_SIZE];
    uint8_t payload_len = 0;
    int32   ret;

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 11, THRUST_N_REQUEST_STATUS, NULL);
    if (ret != CFE_SUCCESS) { return ret; }

    ret = RecvPacket(rx_buf, &payload_len);
    if (ret != CFE_SUCCESS) { return ret; }

    ret = ValidatePacketMeta(rx_buf, payload_len, THRUST_PKT_MSGTYPE_TM,
                             11, THRUST_N_REPLY_STATUS);
    if (ret != CFE_SUCCESS) { return ret; }

    if (returnVal != NULL)
    {
        memcpy(returnVal, &rx_buf[6], sizeof(THRUST_StatusData_Payload_t));
        THRUST_DebugPrintStatusData(returnVal);
    }

    return CFE_SUCCESS;
}

/* ===================================================================
 * Group 3: 주추력기 명령
 * =================================================================== */

/* MsgID=20 MAIN THRUSTER FIRE (Length=8)
 * Payload: Fire Time(UINT32) + Ignition Time(UINT16) + Fire Key(UINT16) */
int32 THRUST_MainThrusterFire(uint32_t fireTime_ms, uint16_t ignitionTime_ms,
                               uint16_t fireKey,
                               THRUST_DefaultResponse_t *outResp)
{
    int32   ret;
    uint8_t payload[THRUST_N_MAIN_FIRE];

    PACK_U32_LE(&payload[0], fireTime_ms);
    PACK_U16_LE(&payload[4], ignitionTime_ms);
    PACK_U16_LE(&payload[6], fireKey);

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 20, THRUST_N_MAIN_FIRE, payload);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(20, outResp);
}

/* MsgID=21 MAIN THRUSTER ABORT (Length=0)
 * 응답: Default Response */
int32 THRUST_MainThrusterAbort(THRUST_DefaultResponse_t *outResp)
{
    int32 ret;

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 21, THRUST_N_MAIN_ABORT, NULL);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(21, outResp);
}

/* ===================================================================
 * Group 4: 냉가스 추력기 명령
 * =================================================================== */

/* MsgID=30 CG THRUSTER PULSE (Length=8)
 * Payload: Thruster ID(UINT16) + Pulse Width(UINT32) + Fire Key(UINT16) */
int32 THRUST_CGThrusterPulse(uint16_t thrusterId, uint32_t pulseWidth_ms,
                              uint16_t fireKey,
                              THRUST_DefaultResponse_t *outResp)
{
    int32   ret;
    uint8_t payload[THRUST_N_CG_PULSE];

    PACK_U16_LE(&payload[0], thrusterId);
    PACK_U32_LE(&payload[2], pulseWidth_ms);
    PACK_U16_LE(&payload[6], fireKey);

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 30, THRUST_N_CG_PULSE, payload);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(30, outResp);
}

/* MsgID=31 CG THRUSTER ABORT (Length=0)
 * 응답: Default Response */
int32 THRUST_CGThrusterAbort(THRUST_DefaultResponse_t *outResp)
{
    int32 ret;

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 31, THRUST_N_CG_ABORT, NULL);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(31, outResp);
}

/* ===================================================================
 * Group 5: Fault 관리 명령
 * =================================================================== */

/* MsgID=40 REQUEST FAULT LOG (Length=0)
 * 응답: Reply Fault Log (40바이트, TBD) → THRUST_FaultLogData_Payload_t */
int32 THRUST_GetFaultLog(THRUST_FaultLogData_Payload_t *returnVal)
{
    uint8_t rx_buf[THRUST_RX_BUF_SIZE];
    uint8_t payload_len = 0;
    int32   ret;

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 40, THRUST_N_REQUEST_FAULT, NULL);
    if (ret != CFE_SUCCESS) { return ret; }

    ret = RecvPacket(rx_buf, &payload_len);
    if (ret != CFE_SUCCESS) { return ret; }

    ret = ValidatePacketMeta(rx_buf, payload_len, THRUST_PKT_MSGTYPE_TM,
                             40, THRUST_N_REPLY_FAULT);
    if (ret != CFE_SUCCESS) { return ret; }

    if (returnVal != NULL)
    {
        memcpy(returnVal, &rx_buf[6], sizeof(THRUST_FaultLogData_Payload_t));
        THRUST_DebugPrintFaultLog(returnVal);
    }

    return CFE_SUCCESS;
}

/* MsgID=41 CLEAR FAULT (Length=0) */
int32 THRUST_ClearFault(THRUST_DefaultResponse_t *outResp)
{
    int32 ret;

    ret = SendPacket(THRUST_PKT_MSGTYPE_TC, 41, THRUST_N_CLEAR_FAULT, NULL);
    if (ret != CFE_SUCCESS) { return ret; }

    return RecvDefaultResponse(41, outResp);
}
