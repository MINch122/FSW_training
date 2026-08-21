/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include <string.h>
#include <stddef.h>
#include <gs/param/rparam.h>
#include <gs/param/table.h>
#include <gs/param/internal/types.h>

#include "pay_slt_app.h"
#include "pay_slt_cmds.h"
#include "pay_slt_eventids.h"
#include "pay_slt_msgids.h"
#include "pay_slt_version.h"
#include "pay_slt_utils.h"

#define PAY_SLT_EXP_A7_NODE  11
#define PAY_SLT_EXP_M7_NODE  12
#define PAY_SLT_IFB_NODE     13

#define TABLE_BOARD_PARAM          0
#define TABLE_DATA_CONTROL_PARAM   3
#define TABLE_TELEMETRY            4
#define PAY_SLT_PARAM_TABLE_MAX    7U
#define PAY_SLT_GET_FULL_TABLE_MAX_ATTEMPTS 3U
#define PAY_SLT_GET_FULL_TABLE_TIMEOUT_MS 1000U
#define PAY_SLT_GET_FULL_TABLE_RETRY_DELAY_MS 100U
#define PAY_SLT_DOWNLOAD_FLUSH_INTERVAL 20U
#define PAY_SLT_RS422_PART_LATCH_DELAY_MS 100U

static uint16 PAY_SLT_GetParamElementSize(uint8 type)
{
    switch (type)
    {
        case GS_PARAM_UINT8:
        case GS_PARAM_STRING:
            return sizeof(uint8);
        case GS_PARAM_INT8:
            return sizeof(int8);
        case GS_PARAM_UINT16:
            return sizeof(uint16);
        case GS_PARAM_UINT32:
            return sizeof(uint32);
        case GS_PARAM_INT16:
            return sizeof(int16);
        case GS_PARAM_FLOAT:
            return sizeof(float);
        default:
            return 0;
    }
}

static bool PAY_SLT_IsValidNode(uint8 node)
{
    return (node == PAY_SLT_EXP_A7_NODE) || (node == PAY_SLT_EXP_M7_NODE) || (node == PAY_SLT_IFB_NODE);
}

static bool PAY_SLT_IsValidTable(uint8 table)
{
    return table <= PAY_SLT_PARAM_TABLE_MAX;
}

static bool PAY_SLT_IsRetryableFullTableError(gs_error_t err)
{
    return (err == GS_ERROR_IO) || (err == GS_ERROR_TIMEOUT) || (err == GS_ERROR_DATA);
}

// getpar 명령의 Request/Report 구조체의 크기를 계산하는 함수
static uint16 PAY_SLT_GetParamReportSize(const PAY_SLT_Params_t *param)
{
    size_t report_size;
    uint16 element_size = PAY_SLT_GetParamElementSize(param->type);

    report_size = offsetof(PAY_SLT_Params_t, param) + ((size_t)param->len * element_size);
    if (report_size > sizeof(*param))
    {
        report_size = sizeof(*param);
    }

    return (uint16)report_size;
}

static gs_error_t PAY_SLT_GetTableDataSize(const gs_param_table_instance_t *tinst, size_t *data_size)
{
    size_t table_size = 0U;

    if ((tinst == NULL) || (tinst->rows == NULL) || (tinst->memory == NULL) || (data_size == NULL)) {
        return GS_ERROR_ARG;
    }

    for (unsigned int index = 0U; index < tinst->row_count; ++index) {
        const gs_param_table_row_t *row = &tinst->rows[index];
        const size_t addr = (size_t)GS_PARAM_ADDR(row);
        const size_t param_size = (size_t)GS_PARAM_SIZE(row);
        const size_t array_size = (size_t)GS_PARAM_ARRAY_SIZE(row);
        const size_t value_size = param_size * array_size;

        if ((param_size == 0U) || (array_size == 0U) || (addr > tinst->memory_size) ||
            (value_size > ((size_t)tinst->memory_size - addr))) {
            return GS_ERROR_DATA;
        }

        if ((addr + value_size) > table_size) {
            table_size = addr + value_size;
        }
    }

    if (table_size == 0U) {
        return GS_ERROR_DATA;
    }

    *data_size = table_size;
    return GS_OK;
}

// 파일 다운로드 경로 생성 함수
static void PAY_SLT_MakeDownloadPath(uint32 file_index, const char *src_name, char *filepath, size_t filepath_size)
{
    char safe_name[OS_MAX_FILE_NAME] = {0};
    size_t src_len;
    size_t i;

    if (filepath == NULL || filepath_size == 0U) {
        return;
    }

    filepath[0] = '\0';

    // ft_file_name이 NULL이거나 빈 문자열이면 기본 경로 사용
    if (src_name == NULL || src_name[0] == '\0') {
        snprintf(filepath, filepath_size, "/cf/payload_%lu.bin", (unsigned long)file_index);
        return;
    }

    src_len = strlen(src_name);
    if (src_len >= sizeof(safe_name)) {
        src_len = sizeof(safe_name) - 1U;
    }

    // 파일 이름에서 안전하지 않은 문자를 '_'로 대체
    for (i = 0; i < src_len; ++i) {
        char c = src_name[i];
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            safe_name[i] = '_';
        } else {
            safe_name[i] = c;
        }
    }
    safe_name[src_len] = '\0';

    // 파일 이름이 비어있으면 기본 이름 사용
    if (safe_name[0] == '\0') {
        snprintf(safe_name, sizeof(safe_name), "payload_%lu", (unsigned long)file_index);
    }

    // 파일 이름이 OS_MAX_FILE_NAME보다 길면 잘라내기
    if (strlen(safe_name) >= OS_MAX_FILE_NAME) {
        safe_name[OS_MAX_FILE_NAME - 1] = '\0';
    }

    snprintf(filepath, filepath_size, "/cf/%s", safe_name);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UPDATE_IFB_CACHE:                                                          */
/*    Update the latest status of the InterFace Board to computer's memory    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static void PAY_SLT_KeepFirstError(int32 *result, int32 status)
{
    if ((*result == CFE_SUCCESS) && (status != CFE_SUCCESS)) {
        *result = status;
    }
}

static int32 PAY_SLT_UpdateIfbCache(void) {
    int32 status = CFE_SUCCESS;

    // SLT-IFB data (node 13)
    // [ICD 0x0000] sys_status (INT16, 길이 1)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY,
                                                              0x0000, 1, &PAY_SLT_Data.sys_status));
    // [ICD 0x0002] sys_uptime (UINT32, 길이 1)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_IFB_NODE, TABLE_TELEMETRY,
                                                              0x0002, 1, &PAY_SLT_Data.sys_uptime));
    // [ICD 0x0006] sys_now (UINT32, 길이 1)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_IFB_NODE, TABLE_TELEMETRY,
                                                              0x0006, 1, &PAY_SLT_Data.sys_now));
    // [ICD 0x000A] boot_cnt (UINT16, 길이 1)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY,
                                                              0x000A, 1, &PAY_SLT_Data.boot_cnt));
    // [ICD 0x000C] boot_his (UINT8, 길이 8)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_IFB_NODE, TABLE_TELEMETRY,
                                                              0x000C, 8, PAY_SLT_Data.boot_his));

    return status;
}

CFE_Status_t PAY_SLT_SendHkCmd(const PAY_SLT_SendHkCmd_t *Msg) {
    int32 status = CFE_SUCCESS;
    PAY_SLT_HkTlm_Payload_t HkPayload;

    (void)Msg;

    if (PAY_SLT_Data.HkEnabled == 1) {
    PAY_SLT_HkTlm_Payload_t *HkPkt = &HkPayload;
    memset(HkPkt, 0, sizeof(*HkPkt));

    PAY_SLT_KeepFirstError(&status, PAY_SLT_UpdateIfbCache());

    // PAY-EXP
    // [ICD 0x0000] sys_status (INT8, 길이 2)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT8, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0000, 2, HkPkt->sys_status));
    // [ICD 0x0026] brm_data (INT16, 길이 3)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0026, 3, HkPkt->brm_data));
    // [ICD 0x003C] ntc_data (INT16, 길이 8)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x003C, 8, HkPkt->ntc_data));

    // PAY-IFB
    // [ICD 0x001A] ntc_data_ifb (INT16, 길이 4)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x001A, 4, HkPkt->ntc_data_ifb));

    if (status != CFE_SUCCESS)
    {
        PAY_SLT_Data.ErrCounter++;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT HK collection failed, not transmitting partial packet, status=%ld",
                          (long)status);
        return status;
    }

    memcpy(&PAY_SLT_Data.HkTlm.Payload, HkPkt, sizeof(PAY_SLT_Data.HkTlm.Payload));
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAY_SLT_Data.HkTlm.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(PAY_SLT_Data.HkTlm.TelemetryHeader), true);

    }

    return status;
}

CFE_Status_t PAY_SLT_SendBeaconCmd(const PAY_SLT_SendBcnCmd_t *Msg) {
    int32 status = CFE_SUCCESS;

    (void)Msg;
    
    if (PAY_SLT_Data.BcnEnabled == 1){
    PAY_SLT_BcnTlm_Payload_t *BcnPkt = &PAY_SLT_Data.BcnTlm.Payload;
    memset(BcnPkt, 0, sizeof(*BcnPkt));

    PAY_SLT_KeepFirstError(&status, PAY_SLT_UpdateIfbCache());

    BcnPkt->CmdCounter = PAY_SLT_Data.CmdCounter;
    BcnPkt->ErrCounter = PAY_SLT_Data.ErrCounter;

    // PAY-EXP-A7
    // System status [MPU MCU] (I8 Array[2], Addr: 0x0000)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT8, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0000, 2, BcnPkt->sys_status_a7));
    // System uptime in sec (U32, Addr: 0x0002)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0002, 1, &BcnPkt->sys_uptime_a7));
    // System current time (U32, Addr: 0x0006)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0006, 1, &BcnPkt->sys_now_a7));
    // System boot count - MPU (U16, Addr: 0x000A)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x000A, 1, &BcnPkt->boot_cnt_p));
    // System boot cause code - MPU (U8 Array[8], Addr: 0x000C)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x000C, 8, BcnPkt->boot_his_p));
    // System boot count - MCU (U16, Addr: 0x0014)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0014, 1, &BcnPkt->boot_cnt_c));
    // System boot cause code - MCU (U8 Array[8], Addr: 0x0016)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0016, 8, BcnPkt->boot_his_c));
    // Board temperature (I16, Addr: 0x001E)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x001E, 1, &BcnPkt->brd_temp_a7));
    // SLF sensor data (I16 Array[3], Addr: 0x0020)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0020, 3, BcnPkt->slf_data));
    // Barometric sensor data (I16 Array[3], Addr: 0x0026)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x0026, 3, BcnPkt->brm_data));
    // IMU sensor data (I16 Array[8], Addr: 0x002C)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x002C, 8, BcnPkt->imu_data));
    // NTC sensor data (I16 Array[8], Addr: 0x003C)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x003C, 8, BcnPkt->ntc_data_a7));
    // System power voltage measures in mV (U16 Array[8], Addr: 0x004C)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x004C, 8, BcnPkt->pwr_volt));
    // System power current measures in mA (U16 Array[8], Addr: 0x005C)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE,
                                                              TABLE_TELEMETRY, 0x005C, 8, BcnPkt->pwr_current));

    // PAY-IFB 
    // System status (I16, Addr: 0x0000)
    BcnPkt->sys_status_ifb = PAY_SLT_Data.sys_status;
    // System uptime in sec (U32, Addr: 0x0002)
    BcnPkt->sys_uptime_ifb = PAY_SLT_Data.sys_uptime;
    // System boot count (U16, Addr: 0x000A)
    BcnPkt->boot_cnt = PAY_SLT_Data.boot_cnt;
    // System boot cause code (U8 Array[8], Addr: 0x000C)
    memcpy(BcnPkt->boot_his, PAY_SLT_Data.boot_his, sizeof(PAY_SLT_Data.boot_his));
    // System current time (Unix epoch, sec) (U32, Addr: 0x0006)
    BcnPkt->sys_now_ifb = PAY_SLT_Data.sys_now;
    // Time left before WDT causes reboot (U32, Addr: 0x0014)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x0014, 1, &BcnPkt->wdt_left_ifb));
    // Board temperature (val/10 °C) (I16, Addr: 0x0018)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x0018, 1, &BcnPkt->brd_temp_ifb));
    // NTC sensor data (val/10 °C) (I16 Array[4], Addr: 0x001A)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x001A, 4, BcnPkt->ntc_data_ifb));
    // System power current measures in mA (U16 Array[2], Addr: 0x0022)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x0022, 2, BcnPkt->pw_cur));
    // System power voltage measures in mV (U16 Array[2], Addr: 0x0026)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x0026, 2, BcnPkt->pw_vol));
    // Sensor data: sen_online (U8, Addr: 0x002A)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x002A, 1, &BcnPkt->sen_online));
    // Sensor data: sen_qlvl (U8, Addr: 0x002B)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x002B, 1, &BcnPkt->sen_qlvl));
    // Sensor data: att_ql (U8, Addr: 0x002C)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x002C, 1, &BcnPkt->att_ql));
    // Sensor data: att_q (F32 Array[4], Addr: 0x002D)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x002D, 4, BcnPkt->att_q));
    // Sensor data: rot_r (F32 Array[3], Addr: 0x003D)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x003D, 3, BcnPkt->rot_r));
    // Sensor data: lin_acc (F32 Array[3], Addr: 0x0049)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x0049, 3, BcnPkt->lin_acc));
    // Sensor data: fld_vec (F32 Array[3], Addr: 0x0055)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x0055, 3, BcnPkt->fld_vec));
    // Sensor data: sen_rst (U16, Addr: 0x0061)
    PAY_SLT_KeepFirstError(&status, PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE,
                                                              TABLE_TELEMETRY, 0x0061, 1, &BcnPkt->sen_rst));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAY_SLT_Data.BcnTlm.TelemetryHeader));
    PAY_SLT_KeepFirstError(&status, CFE_SB_TransmitMsg(CFE_MSG_PTR(PAY_SLT_Data.BcnTlm.TelemetryHeader), true));
    }

    return status;
} 

/* command */

/* No-op command */
CFE_Status_t PAY_SLT_NoopCmd(const PAY_SLT_NoopCmd_t *Msg) {
    (void)Msg;
    CFE_EVS_SendEvent(PAY_SLT_APP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "PAY_SLT: NOOP command %s",
                      PAY_SLT_VERSION);
    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_NOOP_CC, false, NULL, 0);
}

/* RS422 Ping command */
CFE_Status_t PAY_SLT_RS422PingCmd(const PAY_SLT_RS422PingCmd_t *Msg) {
    uint8 rs422_ping_tx[7] = {0x40, 0x50, 0x00, 0x00, 0x00, 0x00, 0x0D};
    uint8 rs422_ping_rx[10] = {0};
    CFE_SRL_IO_Param_t ping_params = {0};
    CFE_Status_t status;

    (void)Msg;

    if (PAY_SLT_Data.RS422Handle == NULL) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "RS422 ping test skipped: handle=NULL");
        return PAY_SLT_HandleReport(CFE_STATUS_EXTERNAL_RESOURCE_FAIL, PAY_SLT_RS422_PING_CC, false, NULL, 0);
    }

    ping_params.TxData = rs422_ping_tx;
    ping_params.TxSize = sizeof(rs422_ping_tx);
    ping_params.RxData = rs422_ping_rx;
    ping_params.RxSize = sizeof(rs422_ping_rx);
    ping_params.Timeout = SLT_IFB_RPARAM_TIMEOUT_MS;

    PAY_SLT_APP_printf("RS422 ping TX:");
    for (uint32 j = 0; j < sizeof(rs422_ping_tx); j++) {
        PAY_SLT_APP_printf("0x%02X ", rs422_ping_tx[j]);
    }
    PAY_SLT_APP_printf("\n");

    status = CFE_SRL_ApiRead(PAY_SLT_Data.RS422Handle, &ping_params);

    if (status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "RS422 ping test success");

        PAY_SLT_APP_printf("RS422 ping RX (Chunk %u): ", (unsigned int)sizeof(rs422_ping_rx));
        for (uint32 j = 0; j < sizeof(rs422_ping_rx); j++) {
            PAY_SLT_APP_printf("0x%02X ", rs422_ping_rx[j]);
        }
        PAY_SLT_APP_printf("\n");
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RS422 ping test send failed, status=0x%08X", (unsigned int)status);
    }

    return PAY_SLT_HandleReport(status, PAY_SLT_RS422_PING_CC, true, rs422_ping_rx, sizeof(rs422_ping_rx));
}

/* Reset Counters */
CFE_Status_t PAY_SLT_ResetCountersCmd(const PAY_SLT_ResetCountersCmd_t *Msg) {
    (void)Msg;
    PAY_SLT_Data.CmdCounter = 0;
    PAY_SLT_Data.ErrCounter = 0;
    PAY_SLT_Data.AppErrCounter = 0;
    PAY_SLT_Data.DeviceErrCounter = 0;
    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_RESET_COUNTERS_CC, false, NULL, 0);
}

CFE_Status_t PAY_SLT_OutputEnabledCmd(const PAY_SLT_OutputEnabledCmd_t *Msg) {
    // 값 검증
    if (Msg->Payload.HkEnabled > 1U || Msg->Payload.BcnEnabled > 1U) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: OUTPUT_ENABLED rejected, Args must be 0 or 1");
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_OUTPUT_ENABLED_CC, false, NULL, 0);
    }

    PAY_SLT_Data.HkEnabled = Msg->Payload.HkEnabled;
    PAY_SLT_Data.BcnEnabled = Msg->Payload.BcnEnabled;

    CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                  "PAY_SLT: Output Cmd success (HK:%u, BCN:%u)", 
                  PAY_SLT_Data.HkEnabled, PAY_SLT_Data.BcnEnabled);

    uint8 result[2] = {PAY_SLT_Data.HkEnabled, PAY_SLT_Data.BcnEnabled};

    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_OUTPUT_ENABLED_CC, false, result, sizeof(result));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* COMMAND                                                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Set Parameter                                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_ParSetCmd(const PAY_SLT_ParSetCmd_t *Msg) {
    int32 Status;
    bool device_error = true;

    PAY_SLT_ParSet_Payload_t Payload = Msg->Payload;

    if (!PAY_SLT_IsValidNode(Payload.node) || !PAY_SLT_IsValidTable(Payload.table))
    {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT_ParSetCmd rejected: node=%u table=%u", (unsigned)Payload.node,
                          (unsigned)Payload.table);
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_PAR_SET_CC, false, &Payload, sizeof(Payload));
    }

    switch (Payload.type) {
        case GS_PARAM_STRING:
            Payload.data.str[sizeof(Payload.data.str) - 1] = '\0';
            Status = PAY_SLT_SetParam(Payload.node, Payload.table, Payload.addr, Payload.type, Payload.data.str);
            break;
        case GS_PARAM_UINT8:
        case GS_PARAM_UINT16:
        case GS_PARAM_UINT32:
            Status = PAY_SLT_SetParam(Payload.node, Payload.table, Payload.addr, Payload.type, &Payload.data.value);
            break;
        default:
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT_ParSetCmd err: Unsupported type %d", Payload.type);
            Status = CFE_ES_BAD_ARGUMENT;
            device_error = false;
            break;
    }

    return PAY_SLT_HandleReport(Status, PAY_SLT_PAR_SET_CC, device_error, NULL, 0);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Set Array Parameter                                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_ParSetArrayCmd(const PAY_SLT_ParSetArrayCmd_t *Msg)
{
    int32  Status      = CFE_SUCCESS;
    bool   device_error = true;
    const PAY_SLT_ParSetArray_Payload_t *Payload = &Msg->Payload;
    uint16 element_size;

    if (!PAY_SLT_IsValidNode(Payload->node) || !PAY_SLT_IsValidTable(Payload->table))
    {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT_ParSetArrayCmd rejected: node=%u table=%u",
                          (unsigned)Payload->node, (unsigned)Payload->table);
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_PAR_SET_ARRAY_CC, false, NULL, 0);
    }

    if ((Payload->type != GS_PARAM_UINT8) &&
        (Payload->type != GS_PARAM_UINT16) &&
        (Payload->type != GS_PARAM_UINT32))
    {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT_ParSetArrayCmd rejected: unsupported type=%u", (unsigned)Payload->type);
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_PAR_SET_ARRAY_CC, false, NULL, 0);
    }

    if ((Payload->len == 0U) || (Payload->len > PAY_SLT_PAR_SET_ARRAY_MAX_LEN))
    {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT_ParSetArrayCmd rejected: len=%u (max=%u)",
                          (unsigned)Payload->len, (unsigned)PAY_SLT_PAR_SET_ARRAY_MAX_LEN);
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_PAR_SET_ARRAY_CC, false, NULL, 0);
    }

    element_size = PAY_SLT_GetParamElementSize(Payload->type);

    for (uint8 i = 0; i < Payload->len; i++)
    {
        uint16 element_addr = (uint16)(Payload->addr + ((uint16)element_size * (uint16)i));
        void  *value_ptr;

        switch (Payload->type) {
            case GS_PARAM_UINT8:  value_ptr = (void *)&Payload->data.u8[i];  break;
            case GS_PARAM_UINT16: value_ptr = (void *)&Payload->data.u16[i]; break;
            case GS_PARAM_UINT32: value_ptr = (void *)&Payload->data.u32[i]; break;
            default:              value_ptr = NULL; break;
        }

        Status = PAY_SLT_SetParam(Payload->node, Payload->table, element_addr, Payload->type, value_ptr);
        if (Status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT_ParSetArrayCmd: idx=%u addr=0x%04X failed, status=0x%08X",
                              (unsigned)i, (unsigned)element_addr, (unsigned)Status);
            break;
        }
    }

    return PAY_SLT_HandleReport(Status, PAY_SLT_PAR_SET_ARRAY_CC, device_error, NULL, 0);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Get Parameter                                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_ParGetCmd(const PAY_SLT_ParGetCmd_t *Msg) {
    static PAY_SLT_Params_t ReqPayload;
    bool device_error;
    memset(&ReqPayload, 0, sizeof(PAY_SLT_Params_t));

    ReqPayload.node = Msg->Payload.node;
    ReqPayload.table = Msg->Payload.table;
    ReqPayload.addr = Msg->Payload.addr;
    ReqPayload.type = Msg->Payload.type;
    ReqPayload.len = Msg->Payload.len;

    if (!PAY_SLT_IsValidNode(ReqPayload.node) || !PAY_SLT_IsValidTable(ReqPayload.table) ||
        (PAY_SLT_GetParamElementSize(ReqPayload.type) == 0U) || (ReqPayload.len == 0U))
    {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT_ParGetCmd rejected: node=%u table=%u type=%u len=%u",
                          (unsigned)ReqPayload.node, (unsigned)ReqPayload.table, (unsigned)ReqPayload.type,
                          (unsigned)ReqPayload.len);
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_PAR_GET_CC, false, &ReqPayload,
                                    PAY_SLT_GetParamReportSize(&ReqPayload));
    }


    PAY_SLT_APP_printf("Before\n");
    PAY_SLT_APP_printf("type: %u,  len: %u\n", ReqPayload.type, ReqPayload.len);

    int32 Status = PAY_SLT_FetchParam(&ReqPayload);
    device_error = (Status != GS_ERROR_ARG);

    PAY_SLT_APP_printf("After\n");
    PAY_SLT_APP_printf("type: %u,  len: %u\n", ReqPayload.type, ReqPayload.len);

    if (Status == CFE_SUCCESS) {
        char ValBuf[128] = {0}; 
        int offset = 0;

        switch (ReqPayload.type) {    // 출력을 위해 문자열로 조합
            case GS_PARAM_UINT8:
                if (ReqPayload.len > 1) {
                    offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "[");

                    for (int i = 0; i < ReqPayload.len; i++) {
                        if (i == ReqPayload.len - 1) {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%u]", ReqPayload.param.u8[i]);
                        } else {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%u ", ReqPayload.param.u8[i]);
                        }

                        // prevent overflow
                        if (offset >= sizeof(ValBuf) - 1) {
                            break;
                        }
                    }
                } else {
                    snprintf(ValBuf, sizeof(ValBuf), "%u", ReqPayload.param.u8[0]);
                }
                break;

            case GS_PARAM_UINT16:
                if (ReqPayload.len > 1) {
                    offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "[");

                    for (int i = 0; i < ReqPayload.len; i++) {
                        if (i == ReqPayload.len - 1) {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%u]", ReqPayload.param.u16[i]);
                        } else {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%u ", ReqPayload.param.u16[i]);
                        }

                        // prevent overflow
                        if (offset >= sizeof(ValBuf) - 1) {
                            break;
                        }
                    }
                    
                    PAY_SLT_APP_printf("[PAY-SLT] Data: ");
                        for (int i = 0; i < ReqPayload.len; i++) PAY_SLT_APP_printf("%u ", (unsigned int)ReqPayload.param.u16[i]);
                    PAY_SLT_APP_printf("\n");    
                } else {
                    snprintf(ValBuf, sizeof(ValBuf), "%u", ReqPayload.param.u16[0]);
                }
                break;

            case GS_PARAM_UINT32:
                snprintf(ValBuf, sizeof(ValBuf), "%u", (unsigned int)ReqPayload.param.u32[0]);
                break;

            case GS_PARAM_INT8:
                if (ReqPayload.len > 1) {
                    offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "[");
                    for (int i = 0; i < ReqPayload.len; i++) {
                        if (i == ReqPayload.len - 1) {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%d]", ReqPayload.param.i8[i]);
                        } else {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%d ", ReqPayload.param.i8[i]);
                        }
                        if (offset >= sizeof(ValBuf) - 1) { break; }
                    }
                } else {
                    snprintf(ValBuf, sizeof(ValBuf), "%d", ReqPayload.param.i8[0]);
                }
                break;

            case GS_PARAM_INT16:
                if (ReqPayload.len > 1) {
                    offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "[");

                    for (int i = 0; i < ReqPayload.len; i++) {
                        if (i == ReqPayload.len - 1) {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%d]", ReqPayload.param.i16[i]);
                        } else {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%d ", ReqPayload.param.i16[i]);
                        }

                        // prevent overflow
                        if (offset >= sizeof(ValBuf) - 1) {
                            break;
                        }
                    }
                } else {
                    snprintf(ValBuf, sizeof(ValBuf), "%d", ReqPayload.param.i16[0]);
                }
                break;

            case GS_PARAM_FLOAT:
                if (ReqPayload.len > 1) {
                    offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "[");

                    for (int i = 0; i < ReqPayload.len; i++) {
                        if (i == ReqPayload.len - 1) {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%f]", ReqPayload.param.flt[i]);
                        } else {
                            offset += snprintf(ValBuf + offset, sizeof(ValBuf) - offset, "%f ", ReqPayload.param.flt[i]);
                        }

                        // prevent overflow
                        if (offset >= sizeof(ValBuf) - 1) {
                            break;
                        }
                    }
                } else {
                    snprintf(ValBuf, sizeof(ValBuf), "%f", ReqPayload.param.flt[0]);
                }
                break;

            case GS_PARAM_STRING:
                // null 삽입
                if (ReqPayload.len < sizeof(ReqPayload.param.str)) {
                    ReqPayload.param.str[ReqPayload.len] = '\0';
                } else {
                    ReqPayload.param.str[sizeof(ReqPayload.param.str) - 1] = '\0';
                }
                snprintf(ValBuf, sizeof(ValBuf), "%s", ReqPayload.param.str);
                break;

            default:
                snprintf(ValBuf, sizeof(ValBuf), "Unknown Type");
                break;
        }

        PAY_SLT_APP_printf("[PAY-SLT] PAR_GET: Table %u, Addr 0x%04X, Value = %s\n", 
                ReqPayload.table, ReqPayload.addr, ValBuf);
                
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "[PAY-SLT] PAR_GET: Node %u, Table %u, Addr 0x%04X = %s", 
                      (unsigned int)ReqPayload.node, 
                      (unsigned int)ReqPayload.table, 
                      (unsigned int)ReqPayload.addr, 
                      ValBuf);
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAR_GET: CSP transaction failed: %d", (int)Status);
    }

    // 지상으로 쏠 때는 ReqPayload 중 유효한 응답 데이터까지만 잘라서 보냄
    uint16 total_size = PAY_SLT_GetParamReportSize(&ReqPayload);

    return PAY_SLT_HandleReport(Status, PAY_SLT_PAR_GET_CC, device_error, &ReqPayload, total_size);

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Get Full Table:                                                            */
/*   Get all params of the table                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_GetFullTableCmd(const PAY_SLT_GetFullTableCmd_t *Msg) {
    gs_param_table_instance_t tinst = {0};
    uint8 table_data[sizeof(PAY_SLT_Data.RptPkt.Report.ReturnValue)] = {0};
    size_t table_data_size;
    gs_error_t err = GS_ERROR_UNKNOWN;
    uint8 attempt;
    uint8 attempts_used = 0U;

    OS_printf("[PAY-SLT] 1 GET_FULL_TABLE CMD: node=%u table=%u\n", Msg->Payload.node, Msg->Payload.table);

    if (!PAY_SLT_IsValidNode(Msg->Payload.node) || !PAY_SLT_IsValidTable(Msg->Payload.table))
    {
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_GET_FULL_TABLE_CC, false, &Msg->Payload,
                                    sizeof(Msg->Payload));
    }


    for (attempt = 1U; attempt <= PAY_SLT_GET_FULL_TABLE_MAX_ATTEMPTS; ++attempt) {
        attempts_used = attempt;
        err = PAY_SLT_GetFullTable(Msg->Payload.node, Msg->Payload.table, &tinst,
                                   PAY_SLT_GET_FULL_TABLE_TIMEOUT_MS);
        if (err == GS_OK) {
            break;
        }

        PAY_SLT_APP_printf("[PAY-SLT] RParam Get Full Table attempt %u/%u FAILED: node=%u table=%u err=%d\n",
                           (unsigned int)attempt, (unsigned int)PAY_SLT_GET_FULL_TABLE_MAX_ATTEMPTS,
                           (unsigned int)Msg->Payload.node, (unsigned int)Msg->Payload.table, err);

        if ((attempt >= PAY_SLT_GET_FULL_TABLE_MAX_ATTEMPTS) || !PAY_SLT_IsRetryableFullTableError(err)) {
            break;
        }

        OS_TaskDelay(PAY_SLT_GET_FULL_TABLE_RETRY_DELAY_MS);
    }

    if (err != GS_OK) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: RParam Get Full Table command failed after %u attempt(s), err=%d",
                          (unsigned int)attempts_used, err);
        PAY_SLT_APP_printf("[PAY-SLT] RParam Get Full Table FAILED: node=%u table=%u attempts=%u err=%d\n",
                           Msg->Payload.node, Msg->Payload.table, attempts_used, err);
        return PAY_SLT_HandleReport(CFE_STATUS_EXTERNAL_RESOURCE_FAIL, PAY_SLT_GET_FULL_TABLE_CC, true, &err,
                                    sizeof(err));
    }

    if (attempts_used > 1U) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "PAY_SLT: RParam Get Full Table succeeded after %u attempt(s)",
                          (unsigned int)attempts_used);
    }

    OS_printf("[PAY-SLT] 2 GET_FULL_TABLE CMD: node=%u table=%u, getfulltable err: %d\n", Msg->Payload.node, Msg->Payload.table, err);

    PAY_SLT_PrintParamTable(Msg->Payload.node, Msg->Payload.table, &tinst);

    OS_printf("[PAY-SLT] 3 GET_FULL_TABLE CMD: node=%u table=%u\n", Msg->Payload.node, Msg->Payload.table);

    err = PAY_SLT_GetTableDataSize(&tinst, &table_data_size);
    if (err != GS_OK) {
        gs_param_table_free(&tinst);
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Full table data layout invalid, err=%d", err);
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_GET_FULL_TABLE_CC, false, &err, sizeof(err));
    }

    if (table_data_size > sizeof(table_data)) {
        gs_param_table_free(&tinst);
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: Full table is %u bytes, RPT limit is %u bytes",
                          (unsigned int)table_data_size, (unsigned int)sizeof(table_data));
        return PAY_SLT_HandleReport(CFE_STATUS_WRONG_MSG_LENGTH, PAY_SLT_GET_FULL_TABLE_CC, false,
                                    &table_data_size, sizeof(table_data_size));
    }

    memcpy(table_data, tinst.memory, table_data_size);

    PAY_SLT_APP_printf("[PAY-SLT] GET_FULL_TABLE RPT: node=%u table=%u size=%u data:\n",
                       (unsigned int)Msg->Payload.node, (unsigned int)Msg->Payload.table,
                       (unsigned int)table_data_size);
    for (size_t index = 0U; index < table_data_size; ++index) {
        if ((index % 16U) == 0U) {
            PAY_SLT_APP_printf("%04X: ", (unsigned int)index);
        }

        PAY_SLT_APP_printf("%02X%s", table_data[index], ((index % 16U) == 15U || (index + 1U) == table_data_size) ? "\n" : " ");
    }

    gs_param_table_free(&tinst);

    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_GET_FULL_TABLE_CC, false, table_data, (uint16)table_data_size);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* File scan                                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_ScanFilesCmd(const PAY_SLT_ScanFilesCmd_t *Msg) {
    uint32 scan_val = 1;

    uint8 node = Msg->Payload.node;

    if (node != PAY_SLT_EXP_A7_NODE && node != PAY_SLT_EXP_M7_NODE) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Node should be 11 or 12. Received node: %d", node);
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_SCAN_FILES_CC, false, &node, sizeof(node));
    }

    // [Table3 0x0041] ft_scan set 1
    int32 Status = PAY_SLT_SetParam(node, TABLE_DATA_CONTROL_PARAM, 0x0041, GS_PARAM_UINT32, &scan_val);

    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "Payload File Scan Success. scanned node: %d", node);
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Payload File Scan Failed. Status: 0x%06X", Status);
        return PAY_SLT_HandleReport(Status, PAY_SLT_SCAN_FILES_CC, true, NULL, 0);
    }

    OS_TaskDelay(1000); // scan & table update 대기 1초

    PAY_SLT_ScanFileRpl_t Reply = {0};

    // ft_snap_id: uint32, table3, 0x0045
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, node, TABLE_DATA_CONTROL_PARAM, 0x0045, 1,
                                       &Reply.snap_id);
    // ft_file_count: uint32, table3, 0x0049
    int32 Status1 = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, node, TABLE_DATA_CONTROL_PARAM, 0x0049, 1,
                                              &Reply.file_count);

    if (Status == CFE_SUCCESS && Status1 == CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "File Scan: snap_id=%u, file count=%u", Reply.snap_id, Reply.file_count);
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Scan Params Read failed. Status: 0x%06X(snap_id), 0x%06X(file_count)", Status, Status1);
    }

    int32 final_status = (Status == CFE_SUCCESS && Status1 == CFE_SUCCESS) ? CFE_SUCCESS :
                                                                                 CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    return PAY_SLT_HandleReport(final_status, PAY_SLT_SCAN_FILES_CC, true, &Reply, sizeof(Reply));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Scan File Download - I2C                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

// download file index
static uint32 PAY_SLT_TargetFileIndex = 0;
static uint32 PAY_SLT_DownloadStartChunk = 0;
static uint32 PAY_SLT_DownloadTaskID;
static bool PAY_SLT_DownloadInProgress = false;

static CFE_Status_t PAY_SLT_SendDownloadResult(int32 status, uint8 command_code, const char *file_path,
                                                uint32 expected_file_size, bool device_error)
{
    PAY_SLT_DownloadFile_RPT_t result = {0};

    if (file_path != NULL) {
        snprintf(result.file_path, sizeof(result.file_path), "%s", file_path);
    }
    result.expected_file_size = expected_file_size;

    PAY_SLT_APP_printf("[PAY-SLT] DOWNLOAD RPT: cc=%u status=0x%08X size=%u file_path=%s expected_file_size=%u\n",
                       (unsigned int)command_code, (unsigned int)status, (unsigned int)sizeof(result),
                       result.file_path, (unsigned int)result.expected_file_size);
    for (size_t index = 0U; index < sizeof(result); ++index) {
        if ((index % 16U) == 0U) {
            PAY_SLT_APP_printf("%04X: ", (unsigned int)index);
        }

        PAY_SLT_APP_printf("%02X%s", ((const uint8 *)&result)[index],
                           ((index % 16U) == 15U || (index + 1U) == sizeof(result)) ? "\n" : " ");
    }

    return PAY_SLT_HandleReport(status, command_code, device_error, &result, sizeof(result));
}


void PAY_SLT_DownloadChildTask_I2C(void) {
    int32 Status;
    uint32 file_index = PAY_SLT_TargetFileIndex;
    uint32 file_npart = 0;
    uint32 part_ready = 0;
    uint32 start_chunk = PAY_SLT_DownloadStartChunk;
    uint32 expected_file_size = 0U;
    int32  file_fd = -1;
    uint8 chunk_buffer[1024]; // default: 1024바이트 조각 버퍼. 일단 크게 받고 이후에 chunk_size만큼 잘라쓰기
    char ft_file_name[65] = {0}; // STR(64) 필드: 64바이트 + null 보장
    char filepath[128] = {0};
    bool download_completed = false;
    bool report_device_error = true;

    // 0. Chunk size(ft_chunk_size, addr 0x009C)
    uint32 chunk_size = 0;
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x009D, 1, &chunk_size);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: Get RParam(ft_chunk_size) failed. Status: 0x%06X", Status);
        goto TASK_EXIT;
    }

    // chunk size overflow 방지
    if ((chunk_size == 0U) || (chunk_size >= sizeof(chunk_buffer))) {
        Status = CFE_ES_BAD_ARGUMENT;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid chunk size (%d) or Fetch failed.", chunk_size);
        goto TASK_EXIT;
    }

    // 1. Load file at <index> (ft_file_load, addr 0x004D)
    Status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x004D, GS_PARAM_UINT32,
                              &file_index);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: Set RParam(ft_file_load) failed. Status: 0x%06X", Status);
        goto TASK_EXIT;
    }

    // 탑재체가 파일을 로드할 시간을 잠깐
    OS_TaskDelay(100); 

    // 2. 총 조각 수(ft_file_npart, addr 0x0095) 읽기 (FetchParam 사용)
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0095, 1, &file_npart);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: Get RParam(ft_file_npart) failed. Status: 0x%06X", Status);
        goto FILE_CLEANUP;
    }

    if (start_chunk >= file_npart) {
        Status = CFE_ES_BAD_ARGUMENT;
        report_device_error = false;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: start_chunk %u is out of range (file_npart=%u)",
                          (unsigned int)start_chunk, (unsigned int)file_npart);
        goto FILE_CLEANUP;
    }

    // 3. OBC 로컬 파일 시스템에 빈 파일 열기 (OSAL API)
    // 파일명: /cf/payload_data.bin? ft_file_name(str, 0x0050)?
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_STRING, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0051, sizeof(ft_file_name) - 1, ft_file_name);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: Get RParam(ft_file_name) failed. Status: 0x%06X", Status);
        goto FILE_CLEANUP;
    }

    PAY_SLT_MakeDownloadPath(file_index, ft_file_name, filepath, sizeof(filepath));

    // debug
    PAY_SLT_APP_printf("[PAY-SLT] DownloadChildTask: Target file path: %s\n", filepath);

    uint32 open_flags = OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE;
    if (start_chunk != 0U) {
        open_flags = OS_FILE_FLAG_CREATE | OS_FILE_FLAG_APPEND;
    }

    Status = OS_OpenCreate(&file_fd, filepath, open_flags, OS_READ_WRITE);
    if (Status != CFE_SUCCESS) {
        report_device_error = false;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: OpenCreate File failed. Status: 0x%06X", Status);
        goto FILE_CLEANUP;
    }

    // 이어받기 모드일경우 파일 작성 시작점을 맨 끝으로
    if (start_chunk != 0U) {
        Status = OS_lseek(file_fd, 0, OS_SEEK_END);
        if (Status < 0) {
            report_device_error = false;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Seek to file end failed. Status: 0x%06X", Status);
            goto FILE_CLEANUP;
        }
    }

    // 전체 파일 CRC 누적용 초기값
    uint32 total_crc = 0xFFFFFFFF;
    uint32 total_written_bytes = 0U;
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0091, 1, &expected_file_size);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: Get RParam(ft_file_size) failed. Status: 0x%06X", Status);
        goto FILE_CLEANUP;
    }

    // 4. 조각(Chunk) 무한 다운로드 루프
    for (uint32 i = start_chunk; i < file_npart; i++)
    {
        uint8 retry_count = 0; // 다운로드 반복 횟수

CHUNK_RETRY:
        if (retry_count > 3) {
                Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Max retries reached at chunk %d. Aborting.", i);
                goto FILE_CLEANUP; // 3번 연속 실패하면 다운로드 중단
        }

        // 4-1. Load part/chunk at <index> (ft_part_load, addr 0x00A1)
        Status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00A1, GS_PARAM_UINT32, &i);
        if (Status != CFE_SUCCESS) {
            retry_count++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Set ft_part_load failed at chunk %d. Status: 0x%06X. Download Retry",
                              i, Status);
            OS_TaskDelay(100);
            goto CHUNK_RETRY;
        }

        // 4-2. 조각이 준비될 때까지 기다림 (상태 체크 루프)
        part_ready = 0;
        uint32 timeout_cnt = 0;
        OS_TaskDelay(100); // give payload time to latch the new part index and update the I2C buffer
        do {
            // ft_part_ready(0x00AD): 0 = loading, 1 = ready, 2 = error
            Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00AD, 1, &part_ready);
            if (Status != CFE_SUCCESS) {
                retry_count++;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Get ft_part_ready failed at chunk %d. Status: 0x%06X. Download Retry",
                                  i, Status);
                goto CHUNK_RETRY;
            }

            timeout_cnt++;
            if (timeout_cnt > 500) part_ready = 2;   // 5초 이상 응답 없음이면 에러 처리
            if (part_ready == 0) OS_TaskDelay(10); // 10ms 대기 (CPU 점유율 방지)
        } while (part_ready == 0); // 1: ready 상태가 될 때까지 대기

        if (part_ready != 1U) {
            retry_count ++;
            Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Unexpected part-ready state %u at chunk %d. Download Retry",
                              (unsigned int)part_ready, i);
            goto CHUNK_RETRY; // error 발생시 다운로드 재시도
        }

        // 4-3. I2C 데이터 읽기 (이 부분은 하드웨어 통신 API 사용)
        uint32 data_buffer_addr = 0x00000000; // ICD: Memory address is start from 0x00000000

        uint32 part_size = 0;
        uint32 valid_data_len = 0;
        // ft_part_size(0x00A5)
        Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00A5, 1, &part_size);
        if (Status != CFE_SUCCESS) {
            retry_count++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Get ft_part_size failed at chunk %d. Status: 0x%06X. Download Retry",
                              i, Status);
            goto CHUNK_RETRY;
        }

        if ((part_size == 0U) || (part_size > chunk_size) || (part_size >= sizeof(chunk_buffer))) {
            Status = CFE_ES_BAD_ARGUMENT;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid I2C part size %u at chunk %d. Max payload=%u", (unsigned int)part_size, i,
                              (unsigned int)(sizeof(chunk_buffer) - 1U));
            goto FILE_CLEANUP;
        }

        if ((i * chunk_size) >= expected_file_size) {
            Status = CFE_ES_BAD_ARGUMENT;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid I2C chunk range at chunk %d. Offset=%lu, FileSize=%lu",
                              i, (unsigned long)(i * chunk_size), (unsigned long)expected_file_size);
            goto FILE_CLEANUP;
        }

        valid_data_len = expected_file_size - (i * chunk_size);
        if (valid_data_len > part_size) {
            valid_data_len = part_size;
        }

        if (valid_data_len == 0U) {
            Status = CFE_ES_BAD_ARGUMENT;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid I2C valid data length at chunk %d. PartSize=%lu, FileSize=%lu",
                              i, (unsigned long)part_size, (unsigned long)expected_file_size);
            goto FILE_CLEANUP;
        }

        // I2C 실제 데이터 = [쓰레기 값 1 byte] + 실제 데이터
        Status = PAY_SLT_ReadExpI2CChunk(PAY_SLT_Data.I2c1Handle, data_buffer_addr, chunk_buffer, part_size+1);
        if (Status != CFE_SUCCESS) {
            retry_count ++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "I2C Read Error at chunk %d. Download Retry", i);
            goto CHUNK_RETRY; // I2C 통신 끊기면 다운로드 재시도
        }
        memmove(chunk_buffer, &chunk_buffer[1], part_size);

        PAY_SLT_APP_printf("Chunk Data (Chunk %d): ", i);
        for (uint32 j = 0; j < part_size; j++) {
            PAY_SLT_APP_printf("%02X ", chunk_buffer[j]);
        }
        PAY_SLT_APP_printf("\n");

        // 4-4. CRC check - chunk data
        uint32 chunk_crc = PAY_SLT_CalculateCRC32(chunk_buffer, part_size);
        uint32 valid_chunk_crc = chunk_crc;

        // ft_part_crc32, addr 0x00A8
        uint32 ft_part_crc32 = 0;
        Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00A9, 1, &ft_part_crc32);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: Get RParam(ft_part_crc32) failed. Status: 0x%06X", Status);
            goto FILE_CLEANUP; // Rparam 실패는 바로 끝냄
        }

        if (valid_data_len != part_size) {
            valid_chunk_crc = PAY_SLT_CalculateCRC32(chunk_buffer, valid_data_len);
            CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "I2C last chunk trimmed at chunk %d. ReadLen=%lu, ValidLen=%lu",
                              i, (unsigned long)part_size, (unsigned long)valid_data_len);
        }
        
        if (valid_data_len != part_size) {
            PAY_SLT_APP_printf("Chunk CRC Check: Calc: 0x%08X, ValidCalc: 0x%08X, Payload: 0x%08X\n",
                               chunk_crc, valid_chunk_crc, ft_part_crc32);
        } else {
            PAY_SLT_APP_printf("Chunk CRC Check: Calc: 0x%08X, Payload: 0x%08X\n", chunk_crc, ft_part_crc32);
        }

        if ((chunk_crc != ft_part_crc32) && (valid_chunk_crc != ft_part_crc32)) {
            // CRC가 불일치하는 경우 다운로드 재시도
            retry_count ++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "CRC Mismatch at chunk %d. Calc: 0x%08X, Payload: 0x%08X. Download Retry", i, chunk_crc, ft_part_crc32);
            goto CHUNK_RETRY;
        }


        // 4-5. 파일에 직접 쓰기
        int32 bytes_written = OS_write(file_fd, chunk_buffer, valid_data_len);
        if ((bytes_written < 0) || ((size_t)bytes_written != valid_data_len)) {
            report_device_error = false;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: File write failed at chunk %u. Written=%d, Expected=%u",
                              (unsigned int)i, (int)bytes_written, valid_data_len);
            goto FILE_CLEANUP;
        }

        // 4-6. 전체 CRC 변수에 현재 청크 데이터 누적 (청크 검증이 통과된 찐 데이터만 기록)
        total_crc = PAY_SLT_UpdateCRC32(total_crc, chunk_buffer, valid_data_len);

        // 4-7. 파일 핸들을 계속 유지하며 실제로 쓴 바이트 수 누적
        total_written_bytes += (uint32)bytes_written;

        // 4-8. 몇 청크마다 파일을 닫고 다시 열어둬서 중간에 실패해도 이전 데이터가 안전하게 저장
        if (((i + 1U) % PAY_SLT_DOWNLOAD_FLUSH_INTERVAL) == 0U || ((i + 1U) == file_npart)) {
            if (file_fd >= 0) {
                OS_close(file_fd);
                file_fd = -1;
            }

            Status = OS_OpenCreate(&file_fd, filepath, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_APPEND, OS_READ_WRITE);
            if (Status != CFE_SUCCESS) {
                report_device_error = false;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Reopen file for append failed at chunk %u. Status: 0x%06X",
                                  (unsigned int)i, Status);
                goto FILE_CLEANUP;
            }

            // APPEND 플래그가 무시될 수 있음 -> 커서 가장 끝으로 고정
            Status = OS_lseek(file_fd, 0, OS_SEEK_END);
            if (Status < 0) {
                report_device_error = false;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Seek after reopen failed at chunk %u. Status: 0x%06X",
                                  (unsigned int)i, Status);
                goto FILE_CLEANUP;
            }
        }

        // 터미널에서 진행률 확인
        if (i % 10 == 0) PAY_SLT_APP_printf("Downloading... %d / %d\n", i, file_npart);
    }

    // 5. CRC check - full data
    if (start_chunk == 0) {
        // 처음부터 다운로드한 경우에만 전체 CRC 검사 수행
        total_crc ^= 0xFFFFFFFF; // 다운로드가 끝났으므로 최종 반전(XOR Out)

        // ft_file_crc32, addr 0x0099
        uint32 ft_file_crc32 = 0;
        Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM,
                                           0x0099, 1, &ft_file_crc32);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Get ft_file_crc32 failed. Status: 0x%06X", Status);
            goto FILE_CLEANUP;
        }

        if (total_crc != ft_file_crc32) {
            Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                              "Total CRC Mismatch. Calc: 0x%08X, Payload: 0x%08X", total_crc, ft_file_crc32);
            goto FILE_CLEANUP;
        } else {
            CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, 
                              "Total CRC Match Success! File securely downloaded.");
        }
    } else {
        // 이어받기 모드일 경우 CRC 검사 생략 메시지 출력
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, 
                          "Resume Download Complete. (Total CRC check skipped, started from chunk %u)", 
                          (unsigned int)start_chunk);
    }

    // 최종 검사 전 저장
    if (file_fd >= 0) {
        OS_close(file_fd);
        file_fd = -1;
    }

    // 6. file 최종 검사 - status, size
    download_completed = true;
    {
        os_fstat_t file_stat;
        int32 stat_status = OS_stat(filepath, &file_stat);
        if (stat_status != OS_SUCCESS) {
            download_completed = false;
            Status = stat_status;
            report_device_error = false;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Stat failed for %s. Status: 0x%06X", filepath, stat_status);
        } else {
            CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "DownloadChildTask: File size check: actual=%lu expected=%lu written=%lu",
                              (unsigned long)OS_FILESTAT_SIZE(file_stat), (unsigned long)expected_file_size,
                              (unsigned long)total_written_bytes);
            if (OS_FILESTAT_SIZE(file_stat) != expected_file_size) {
                download_completed = false;
                Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: File size mismatch. actual=%lu expected=%lu",
                                  (unsigned long)OS_FILESTAT_SIZE(file_stat), (unsigned long)expected_file_size);
            }
            if ((start_chunk == 0U) && (total_written_bytes != expected_file_size)) {
                download_completed = false;
                Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Byte count mismatch. written=%lu expected=%lu",
                                  (unsigned long)total_written_bytes, (unsigned long)expected_file_size);
            }
        }
    }

    FILE_CLEANUP:
    // 7. 파일 닫기 - error 발생 시 안전망
    if (file_fd >= 0) {
        OS_close(file_fd);
        file_fd = -1;
    }

    // 8. Release loaded file (ft_release, addr 0x00B0)
    uint32 release_val = 1;
    int32 release_status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00B2,
                                             GS_PARAM_UINT32, &release_val);
    if ((release_status != CFE_SUCCESS) && download_completed) {
        download_completed = false;
        Status = release_status;
    }

    CFE_EVS_SendEvent(download_completed ? PAY_SLT_CMD_INF_EID : PAY_SLT_CMD_ERR_EID,
                      download_completed ? CFE_EVS_EventType_INFORMATION : CFE_EVS_EventType_ERROR,
                      download_completed ? "Payload Data Download Complete." : "Payload Data Download Aborted.");

    // 스레드 종료
TASK_EXIT:
    (void)PAY_SLT_SendDownloadResult(download_completed ? CFE_SUCCESS :
                                         ((Status == CFE_SUCCESS) ? CFE_STATUS_EXTERNAL_RESOURCE_FAIL : Status),
                                     PAY_SLT_DOWNLOAD_FILE_I2C_CC, filepath, expected_file_size,
                                     report_device_error);
    PAY_SLT_DownloadInProgress = false;
    CFE_ES_ExitChildTask();
}

CFE_Status_t PAY_SLT_DownloadFileI2CCmd(const PAY_SLT_DownloadFileCmd_t *Msg) {
    if (PAY_SLT_DownloadInProgress)
    {
        return PAY_SLT_HandleReport(CFE_STATUS_REQUEST_ALREADY_PENDING, PAY_SLT_DOWNLOAD_FILE_I2C_CC, false,
                                    &Msg->Payload, sizeof(Msg->Payload));
    }

    PAY_SLT_TargetFileIndex = Msg->Payload.file_index;
    PAY_SLT_DownloadStartChunk = Msg->Payload.start_chunk;
    PAY_SLT_DownloadInProgress = true;

    int32 Status = CFE_ES_CreateChildTask(
        &PAY_SLT_DownloadTaskID,
        "PAY_DL_I2C",                 // 스레드 이름
        PAY_SLT_DownloadChildTask_I2C, // 실행할 스레드 함수 이름
        NULL,                          // 스택 포인터 (NULL이면 자동 할당)
        16384,                         // 스택 사이즈 (16KB면 넉넉?)
        100,                           // 우선순위 (낮게 설정)
        0                              // 플래그
    );

    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, 
                          "Download Task Started for index %d", PAY_SLT_TargetFileIndex);
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadFile: CreateChildTask Error. Status: 0x%06X", Status);
    }

    if (Status != CFE_SUCCESS) {
        PAY_SLT_DownloadInProgress = false;
        return PAY_SLT_SendDownloadResult(Status, PAY_SLT_DOWNLOAD_FILE_I2C_CC, NULL, 0U, false);
    }

    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_DOWNLOAD_FILE_I2C_CC, false, &Msg->Payload,
                                sizeof(Msg->Payload));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Scan File Download - RS422                                                 */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void PAY_SLT_DownloadChildTask_RS422(void) {
    int32 Status;
    uint32 file_index = PAY_SLT_TargetFileIndex;
    uint32 file_npart = 0;
    uint32 part_ready = 0;
    uint32 start_chunk = PAY_SLT_DownloadStartChunk;
    uint32 expected_file_size = 0U;
    int32  file_fd = -1;
    uint8 chunk_buffer[1024]; // default: 1024바이트 조각 버퍼. 일단 크게 받고 이후에 chunk_size만큼 잘라쓰기
    char ft_file_name[65] = {0}; // STR(64) 필드: 64바이트 + null 보장
    char filepath[128] = {0};
    bool download_completed = false;
    bool report_device_error = true;

    // 0. Chunk size(ft_chunk_size, addr 0x009C)
    uint32 chunk_size = 0;
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x009D, 1, &chunk_size);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: Get RParam(ft_chunk_size) failed. Status: 0x%06X", Status);
        goto TASK_EXIT;
    }

    // chunk size overflow 방지
    if ((chunk_size == 0U) || (chunk_size > (sizeof(chunk_buffer) - 8U))) {
        Status = CFE_ES_BAD_ARGUMENT;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid chunk size (%d) or Fetch failed.", chunk_size);
        goto TASK_EXIT;
    }

    // 1. Load file at <index> (ft_file_load, addr 0x004C)
    Status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x004D, GS_PARAM_UINT32,
                              &file_index);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: Set RParam(ft_file_load) failed. Status: 0x%06X", Status);
        goto TASK_EXIT;
    }

    // 탑재체가 파일을 로드할 시간을 잠깐
    // OS_TaskDelay(100);

    // 2. 총 조각 수(ft_file_npart, addr 0x0094) 읽기 (FetchParam 사용)
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0095, 1, &file_npart);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: Get RParam(ft_file_npart) failed. Status: 0x%06X", Status);
        goto FILE_CLEANUP;
    }

    if (start_chunk >= file_npart) {
        Status = CFE_ES_BAD_ARGUMENT;
        report_device_error = false;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: start_chunk %u is out of range (file_npart=%u)",
                          (unsigned int)start_chunk, (unsigned int)file_npart);
        goto FILE_CLEANUP;
    }

    // 3. OBC 로컬 파일 시스템에 빈 파일 열기 (OSAL API)
    // 파일명: /cf/payload_data.bin? ft_file_name(str, 0x0050)?
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_STRING, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0051, sizeof(ft_file_name) - 1, ft_file_name);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: Get RParam(ft_file_name) failed. Status: 0x%06X", Status);
        goto FILE_CLEANUP;
    }

    PAY_SLT_MakeDownloadPath(file_index, ft_file_name, filepath, sizeof(filepath));

    // debug
    PAY_SLT_APP_printf("[PAY-SLT] DownloadChildTask: Target file path: %s\n", filepath);

    uint32 open_flags = OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE;
    if (start_chunk != 0U) {
        open_flags = OS_FILE_FLAG_CREATE | OS_FILE_FLAG_APPEND;
    }

    Status = OS_OpenCreate(&file_fd, filepath, open_flags, OS_READ_WRITE);
    if (Status != CFE_SUCCESS) {
        report_device_error = false;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: OpenCreate File failed. Status: 0x%06X", Status);
        goto FILE_CLEANUP;
    }

    // 이어받기 모드일경우 파일 작성 시작점을 맨 끝으로
    if (start_chunk != 0U) {
        Status = OS_lseek(file_fd, 0, OS_SEEK_END);
        if (Status < 0) {
            report_device_error = false;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Seek to file end failed. Status: 0x%06X", Status);
            goto FILE_CLEANUP;
        }
    }

    // 전체 파일 CRC 누적용 초기값
    uint32 total_crc = 0xFFFFFFFF;
    uint32 total_written_bytes = 0U;
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0091, 1, &expected_file_size);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: Get RParam(ft_file_size) failed. Status: 0x%06X", Status);
        goto FILE_CLEANUP;
    }

    // 4. 조각(Chunk) 무한 다운로드 루프
    for (uint32 i = start_chunk; i < file_npart; i++)
    {
        uint8 retry_count = 0; // 다운로드 반복 횟수

CHUNK_RETRY:
        if (retry_count > 3) {
                Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Max retries reached at chunk %d. Aborting.", i);
                goto FILE_CLEANUP; // 3번 연속 실패하면 다운로드 중단
        }

        // 4-1. Load part/chunk at <index> (ft_part_load, addr 0x00A0)
        Status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00A1, GS_PARAM_UINT32, &i);
        if (Status != CFE_SUCCESS) {
            retry_count++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Set ft_part_load failed at chunk %d. Status: 0x%06X. Download Retry",
                              i, Status);
            OS_TaskDelay(100);
            goto CHUNK_RETRY;
        }

        /* Let the payload latch the new part index before reading ready/data. */
        OS_TaskDelay(PAY_SLT_RS422_PART_LATCH_DELAY_MS);

        // 4-2. 조각이 준비될 때까지 기다림 (상태 체크 루프)
        part_ready = 0;
        uint32 timeout_cnt = 0;
        do {
            // ft_part_ready(0x00AC): 0 = loading, 1 = ready, 2 = error
            Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00AD, 1, &part_ready);
            if (Status != CFE_SUCCESS) {
                retry_count++;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Get ft_part_ready failed at chunk %d. Status: 0x%06X. Download Retry", i, Status);
                goto CHUNK_RETRY;
            }

            timeout_cnt++;
            if (timeout_cnt > 500) part_ready = 2;   // 5초 이상 응답 없음이면 에러 처리
            if (part_ready == 0) OS_TaskDelay(50); // 10ms 대기 (CPU 점유율 방지)
        } while (part_ready == 0); // 1: ready 상태가 될 때까지 대기

        if (part_ready != 1U) {
            retry_count ++;
            Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Unexpected part-ready state %u at chunk %d. Download Retry",
                              (unsigned int)part_ready, i);
            goto CHUNK_RETRY; // error 발생시 다운로드 재시도
        }

        // 4-3. RS422 데이터 읽기 (이 부분은 하드웨어 통신 API 사용)
        uint32 part_size = 0;
        // ft_part_size(0x00A4)
        Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00A5, 1, &part_size);
        if (Status != CFE_SUCCESS) {
            retry_count++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Get ft_part_size failed at chunk %d. Status: 0x%06X. Download Retry",
                              i, Status);
            goto CHUNK_RETRY;
        }


        // RS422 데이터 = [헤더] + [실제 데이터]
        /*
        **  RS422 Data: 
        **  +------+----------------+--------------+----------------+------------+---------------+
        **   0x40  |      0x44      | 0x<counter>  | uint16 BE      | uint16 BE  | .. N Bytes ..
        **   Start | ACK(ASCII 'D') | Counter Byte | Message Length | Part Index |  Chunk Data
        **  +------+----------------+--------------+----------------+------------+---------------+
        **
        */
        uint8 frame_overhead = 8;   // START + ACK/ERR + CNT + LEN[2] + PN[2] + ... + END
        uint8 payload_offset = 7;   // START + ACK/ERR + CNT + LEN[2] + PN[2]
        if ((part_size == 0U) || (part_size > chunk_size) ||
            (part_size > (sizeof(chunk_buffer) - frame_overhead))) {
            Status = CFE_ES_BAD_ARGUMENT;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid RS422 part size %u at chunk %d. Max payload=%u",
                              (unsigned int)part_size, i, (unsigned int)(sizeof(chunk_buffer) - frame_overhead));
            goto FILE_CLEANUP;
        }

        Status = PAY_SLT_ReadExpRS422Chunk(PAY_SLT_Data.RS422Handle, chunk_buffer, sizeof(chunk_buffer));

        if (Status != CFE_SUCCESS) {
            retry_count ++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RS422 Read Error at chunk %d. Status=0x%08X, Expected=%u. Download Retry",
                              i, (unsigned int)Status, (unsigned int)(part_size + frame_overhead));
            goto CHUNK_RETRY; // RS422 통신 끊기면 다운로드 재시도
        }

        // parsing
        uint16 received_frame_len = (chunk_buffer[3] << 8) | chunk_buffer[4];
        uint16 received_part      = (chunk_buffer[5] << 8) | chunk_buffer[6];
        bool is_last_chunk = ((i + 1U) == file_npart);
        uint32 valid_data_len = part_size;
        if (received_frame_len < 2U) {
            retry_count ++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RS422 length too short at chunk %d. Len=%u. Download Retry",
                              i, received_frame_len);
            goto CHUNK_RETRY;
        }

        uint16 received_data_len = received_frame_len - 2U; /* LEN includes PN[1:0]. */
        size_t rx_total_size = (size_t)received_data_len + frame_overhead;

        if (chunk_buffer[0] != 0x40 || chunk_buffer[1] != 0x44) {
            retry_count ++;
            if (chunk_buffer[1] == 0xFF) {
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "RS422 DOWNLOAD device error at chunk %d. Cnt=0x%02X, Len=%u. Download Retry",
                                  i, chunk_buffer[2], received_frame_len);
            } else {
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Packet format error: Start 0x%02X, ACK 0x%02X. Download Retry",
                                  chunk_buffer[0], chunk_buffer[1]);
            }
            goto CHUNK_RETRY;
        }

        if (received_data_len != part_size) {
            if (is_last_chunk && ((uint32)received_data_len >= valid_data_len)) {
                CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "RS422 last chunk padded at chunk %d. DataLen=%u, ft_part_size=%u",
                                  i, received_data_len, (unsigned int)part_size);
            } else {
                retry_count ++;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "RS422 length mismatch at chunk %d. Len=%u, DataLen=%u, ft_part_size=%u. Download Retry",
                                  i, received_frame_len, received_data_len, (unsigned int)part_size);
                goto CHUNK_RETRY;
            }
        }

        // END byte 체크 (0x0D)
        uint32 end_byte_index = payload_offset + received_data_len;
        if (chunk_buffer[end_byte_index] != 0x0D) {
            retry_count++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                            "RS422 End byte mismatch at chunk %d. Expected 0x0D, Got 0x%02X",
                            i, chunk_buffer[end_byte_index]);
            goto CHUNK_RETRY;
        }

        if (received_part != (uint16)i) {
            retry_count ++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RS422 part mismatch. Requested=%u, Received=%u. Download Retry",
                              (unsigned int)i, received_part);
            OS_TaskDelay(PAY_SLT_RS422_PART_LATCH_DELAY_MS);
            goto CHUNK_RETRY;
        }

        PAY_SLT_APP_printf("Raw Data (Chunk %d, received part: %d, frame length: %d, data length: %d, valid length: %u): ",
                           i, received_part, received_frame_len, received_data_len, valid_data_len);
        for (uint32 j = 0; j < rx_total_size; j++) {
            PAY_SLT_APP_printf("%02X ", chunk_buffer[j]);
        }
        PAY_SLT_APP_printf("\n");

        // 진짜 데이터만 따로 뽑기
        memmove(chunk_buffer, &chunk_buffer[payload_offset], received_data_len);

        // 4-4. CRC check - chunk data
        uint32 chunk_crc = PAY_SLT_CalculateCRC32(chunk_buffer, valid_data_len);

        // ft_part_crc32, addr 0x00A8
        uint32 ft_part_crc32 = 0;
        Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00A9, 1, &ft_part_crc32);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: Get RParam(ft_part_crc32) failed. Status: 0x%06X", Status);
            goto FILE_CLEANUP; // Rparam 실패는 바로 끝냄
        }
        
        PAY_SLT_APP_printf("Chunk CRC Check: Calc: 0x%08X, Payload: 0x%08X\n", chunk_crc, ft_part_crc32);

        if (chunk_crc != ft_part_crc32) {
            // CRC가 불일치하는 경우 다운로드 재시도
            retry_count ++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "CRC Mismatch at chunk %d. Calc: 0x%08X, Payload: 0x%08X. Download Retry", i, chunk_crc, ft_part_crc32);
            goto CHUNK_RETRY;
        }


        // 4-5. 파일에 직접 쓰기
        int32 bytes_written = OS_write(file_fd, chunk_buffer, valid_data_len);
        if ((bytes_written < 0) || ((size_t)bytes_written != valid_data_len)) {
            report_device_error = false;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: File write failed at chunk %u. Written=%d, Expected=%u",
                              (unsigned int)i, (int)bytes_written, valid_data_len);
            goto FILE_CLEANUP;
        }

        // 4-6. 전체 CRC 변수에 현재 청크 데이터 누적 (청크 검증이 통과된 찐 데이터만 기록)
        total_crc = PAY_SLT_UpdateCRC32(total_crc, chunk_buffer, valid_data_len);

        // 4-7. 파일 핸들을 계속 유지하며 실제로 쓴 바이트 수 누적
        total_written_bytes += (uint32)bytes_written;

        // 4-8. 몇 청크마다 파일을 닫고 다시 열어둬서 중간에 실패해도 이전 데이터가 안전하게 저장
        if (((i + 1U) % PAY_SLT_DOWNLOAD_FLUSH_INTERVAL) == 0U || ((i + 1U) == file_npart)) {
            if (file_fd >= 0) {
                OS_close(file_fd);
                file_fd = -1;
            }

            Status = OS_OpenCreate(&file_fd, filepath, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_APPEND, OS_READ_WRITE);
            if (Status != CFE_SUCCESS) {
                report_device_error = false;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Reopen file for append failed at chunk %u. Status: 0x%06X",
                                  (unsigned int)i, Status);
                goto FILE_CLEANUP;
            }

            // APPEND 플래그가 무시될 수 있음 -> 커서 가장 끝으로 고정
            Status = OS_lseek(file_fd, 0, OS_SEEK_END);
            if (Status < 0) {
                report_device_error = false;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Seek after reopen failed at chunk %u. Status: 0x%06X",
                                  (unsigned int)i, Status);
                goto FILE_CLEANUP;
            }
        }

        // 터미널에서 진행률 확인
        if (i % 10 == 0) PAY_SLT_APP_printf("Downloading... %d / %d\n", i, file_npart);
    }

    // 5. CRC check - full data
    if (start_chunk == 0) {
        // 처음부터 다운로드한 경우에만 전체 CRC 검사 수행
        total_crc ^= 0xFFFFFFFF; // 다운로드가 끝났으므로 최종 반전(XOR Out)

        // ft_file_crc32, addr 0x0099
        uint32 ft_file_crc32 = 0;
        Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM,
                                           0x0099, 1, &ft_file_crc32);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Get ft_file_crc32 failed. Status: 0x%06X", Status);
            goto FILE_CLEANUP;
        }

        if (total_crc != ft_file_crc32) {
            Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                              "Total CRC Mismatch. Calc: 0x%08X, Payload: 0x%08X", total_crc, ft_file_crc32);
            goto FILE_CLEANUP;
        } else {
            CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, 
                              "Total CRC Match Success! File securely downloaded.");
        }
    } else {
        // 이어받기 모드일 경우 CRC 검사 생략 메시지 출력
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, 
                          "Resume Download Complete. (Total CRC check skipped, started from chunk %u)", 
                          (unsigned int)start_chunk);
    }

    // 최종 검사 전 저장
    if (file_fd >= 0) {
        OS_close(file_fd);
        file_fd = -1;
    }

    // 6. file 최종 검사 - status, size
    download_completed = true;
    {
        os_fstat_t file_stat;
        int32 stat_status = OS_stat(filepath, &file_stat);
        if (stat_status != OS_SUCCESS) {
            download_completed = false;
            Status = stat_status;
            report_device_error = false;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: Stat failed for %s. Status: 0x%06X", filepath, stat_status);
        } else {
            CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "DownloadChildTask: File size check: actual=%lu expected=%lu written=%lu",
                              (unsigned long)OS_FILESTAT_SIZE(file_stat), (unsigned long)expected_file_size,
                              (unsigned long)total_written_bytes);
            if (OS_FILESTAT_SIZE(file_stat) != expected_file_size) {
                download_completed = false;
                Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: File size mismatch. actual=%lu expected=%lu",
                                  (unsigned long)OS_FILESTAT_SIZE(file_stat), (unsigned long)expected_file_size);
            }
            if ((start_chunk == 0U) && (total_written_bytes != expected_file_size)) {
                download_completed = false;
                Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Byte count mismatch. written=%lu expected=%lu",
                                  (unsigned long)total_written_bytes, (unsigned long)expected_file_size);
            }
        }
	    }

FILE_CLEANUP:
    // 7. 파일 닫기 - error 발생 시 안전망
    if (file_fd >= 0) {
        OS_close(file_fd);
        file_fd = -1;
    }

    // 8. Release loaded file (ft_release, addr 0x00B0)
    uint32 release_val = 1;
    int32 release_status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x00B2,
                                             GS_PARAM_UINT32, &release_val);
    if ((release_status != CFE_SUCCESS) && download_completed) {
        download_completed = false;
        Status = release_status;
    }

    CFE_EVS_SendEvent(download_completed ? PAY_SLT_CMD_INF_EID : PAY_SLT_CMD_ERR_EID,
                      download_completed ? CFE_EVS_EventType_INFORMATION : CFE_EVS_EventType_ERROR,
                      download_completed ? "Payload Data Download Complete." : "Payload Data Download Aborted.");

    // 스레드 종료
TASK_EXIT:
    (void)PAY_SLT_SendDownloadResult(download_completed ? CFE_SUCCESS :
                                         ((Status == CFE_SUCCESS) ? CFE_STATUS_EXTERNAL_RESOURCE_FAIL : Status),
                                     PAY_SLT_DOWNLOAD_FILE_RS422_CC, filepath, expected_file_size,
                                     report_device_error);
    PAY_SLT_DownloadInProgress = false;
    CFE_ES_ExitChildTask();
}

CFE_Status_t PAY_SLT_DownloadFileRS422Cmd(const PAY_SLT_DownloadFileCmd_t *Msg) {
    if (PAY_SLT_DownloadInProgress)
    {
        return PAY_SLT_HandleReport(CFE_STATUS_REQUEST_ALREADY_PENDING, PAY_SLT_DOWNLOAD_FILE_RS422_CC, false,
                                    &Msg->Payload, sizeof(Msg->Payload));
    }

    PAY_SLT_TargetFileIndex = Msg->Payload.file_index;
    PAY_SLT_DownloadStartChunk = Msg->Payload.start_chunk;
    PAY_SLT_DownloadInProgress = true;

    int32 Status = CFE_ES_CreateChildTask(
        &PAY_SLT_DownloadTaskID,
        "PAY_DL_RS422",            // 스레드 이름
        PAY_SLT_DownloadChildTask_RS422,// 실행할 스레드 함수 이름
        NULL,                     // 스택 포인터 (NULL이면 자동 할당)
        16384,                    // 스택 사이즈 (16KB면 넉넉?)
        100,                      // 우선순위 (낮게 설정)
        0                         // 플래그
    );

    if (Status == CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, 
                          "Download Task Started for index %d", PAY_SLT_TargetFileIndex);
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadFile: CreateChildTask Error. Status: 0x%06X", Status);
    }

    if (Status != CFE_SUCCESS) {
        PAY_SLT_DownloadInProgress = false;
        return PAY_SLT_SendDownloadResult(Status, PAY_SLT_DOWNLOAD_FILE_RS422_CC, NULL, 0U, false);
    }

    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_DOWNLOAD_FILE_RS422_CC, false, &Msg->Payload,
                                sizeof(Msg->Payload));
}
