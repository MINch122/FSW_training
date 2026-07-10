/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include <string.h>
#include <gs/csp/csp.h>
#include <gs/param/internal/types.h>
#include <gs/param/rparam.h>
#include <gs/param/table.h>

#include "pay_slt_utils.h"
#include "pay_slt_eventids.h"
#include "pay_slt_msgids.h"
#include "pay_slt_msg.h"
#include "pay_slt_app.h"

#define PAY_SLT_RPARAM_SET          0xFFU
#define PAY_SLT_RPARAM_SET_OK       1U
#define PAY_SLT_RPARAM_HEADER_SIZE  10U
#define PAY_SLT_RPARAM_ADDR_SIZE    2U

static void PAY_SLT_StoreBe16(uint8 *buf, uint16 value)
{
    buf[0] = (uint8)((value >> 8) & 0xFFU);
    buf[1] = (uint8)(value & 0xFFU);
}

static uint16 PAY_SLT_LoadBe16(const uint8 *buf)
{
    return (uint16)(((uint16)buf[0] << 8) | (uint16)buf[1]);
}

static size_t PAY_SLT_ParamSize(uint8 type)
{
    switch (type) {
        case GS_PARAM_UINT8:  return 1U;
        case GS_PARAM_UINT16: return 2U;
        case GS_PARAM_UINT32: return 4U;
        default:              return 0U;
    }
}

static int32 PAY_SLT_SetParamCompat(uint8 node, uint8 table, uint16 addr, uint8 type, uint32 raw_value)
{
    uint8 query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE + sizeof(uint32)] = {0};
    uint8 reply[256] = {0};
    size_t value_size = PAY_SLT_ParamSize(type);
    size_t payload_size;
    size_t query_size;
    int reply_len;

    if (value_size == 0U) {
        return GS_ERROR_ARG;
    }

    payload_size = PAY_SLT_RPARAM_ADDR_SIZE + value_size;
    query_size = PAY_SLT_RPARAM_HEADER_SIZE + payload_size;

    query[0] = PAY_SLT_RPARAM_SET;
    query[1] = table;
    PAY_SLT_StoreBe16(&query[2], (uint16)payload_size);
    PAY_SLT_StoreBe16(&query[4], GS_RPARAM_MAGIC_CHECKSUM);
    PAY_SLT_StoreBe16(&query[6], 0U);
    PAY_SLT_StoreBe16(&query[8], 0U);
    PAY_SLT_StoreBe16(&query[PAY_SLT_RPARAM_HEADER_SIZE], addr);

    switch (type) {
        case GS_PARAM_UINT8:
            query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE] = (uint8)raw_value;
            break;
        case GS_PARAM_UINT16:
            PAY_SLT_StoreBe16(&query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE], (uint16)raw_value);
            break;
        case GS_PARAM_UINT32:
            query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE + 0U] = (uint8)((raw_value >> 24) & 0xFFU);
            query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE + 1U] = (uint8)((raw_value >> 16) & 0xFFU);
            query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE + 2U] = (uint8)((raw_value >> 8) & 0xFFU);
            query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE + 3U] = (uint8)(raw_value & 0xFFU);
            break;
        default:
            return GS_ERROR_ARG;
    }

    /*
     * Some payload rparam servers reply to SET with the full SET_OK packet,
     * while gs_rparam_set() in this tree expects exactly one byte.
     */
    reply_len = csp_transaction2(CSP_PRIO_HIGH, node, GS_CSP_PORT_RPARAM, SLT_IFB_RPARAM_TIMEOUT_MS,
                                 query, (int)query_size, reply, -1, CSP_O_CRC32);
    if (reply_len <= 0) {
        return GS_ERROR_IO;
    }

    if ((reply_len == 1) && (reply[0] == PAY_SLT_RPARAM_SET_OK)) {
        return CFE_SUCCESS;
    }

    if ((reply_len >= (int)PAY_SLT_RPARAM_HEADER_SIZE) &&
        (reply[0] == PAY_SLT_RPARAM_SET_OK) &&
        (PAY_SLT_LoadBe16(&reply[2]) == payload_size)) {
        return CFE_SUCCESS;
    }

    CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                      "PAY_SLT_SetParam reply invalid: len=%d action=0x%02X payload_len=%u",
                      reply_len, reply[0], (reply_len >= 4) ? PAY_SLT_LoadBe16(&reply[2]) : 0U);
    return GS_ERROR_DATA;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* FETCH_PARAM_SIMPLE:                                                        */
/*   파라미터 가져오기(따로 받는 용)                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 PAY_SLT_FetchParam_Simple(uint8 type, uint8 node, uint8 table, uint16 addr, uint8 len, void *out_ptr)
{
    PAY_SLT_Params_t req;
    memset(&req, 0, sizeof(PAY_SLT_Params_t));
    
    req.type = type;
    req.node = node;
    req.table = table;
    req.addr = addr;
    req.len = len;

    int32 Status = PAY_SLT_FetchParam(&req);

    if (Status == CFE_SUCCESS && out_ptr != NULL)
    {
        size_t element_size = 0;
        switch (type) {
            case GS_PARAM_UINT8:   element_size = 1; break;
            case GS_PARAM_UINT16:  element_size = 2; break;
            case GS_PARAM_UINT32:  element_size = 4; break;
            case GS_PARAM_INT16:   element_size = 2; break;
            case GS_PARAM_STRING:  element_size = 1; break;
            default:               element_size = 1; break;
        }
        
        // 데이터 복사
        memcpy(out_ptr, &req.param, (element_size * len));
    }

    return Status;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* FETCH_PARAM:                                                               */
/*   파라미터 가져오기                                                           */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 PAY_SLT_FetchParam(PAY_SLT_Params_t *Payload) {
    int32 Status;

    // null pointer check
    if (Payload == NULL)
    {
        return -1;
    }

    PAY_SLT_APP_printf("Before getRparam\n"); 

    // get Rparam csp에 string이 정의되지 않았음
    // 아니 애초에 CFE_SRL_ApiGetRparamCSP는 배열 데이터를 받을 수 없음
    // 직접 gs library 함수 호출해서 사용
    if (Payload->type == GS_PARAM_STRING) {
        // 문자열은 한 덩어리로 인식됨
        gs_error_t gs_err = gs_rparam_get(Payload->node, Payload->table, Payload->addr, Payload->type,
                                            GS_RPARAM_MAGIC_CHECKSUM, 5000, (void *)&(Payload->param), Payload->len);
        if (gs_err == GS_OK) {
            Status = CFE_SUCCESS;
        }
        else {
            Status = (int32)gs_err;
        }
    }
    else if (Payload->len >= 2) {
        size_t element_size = 0;
        switch(Payload->type) {
            case GS_PARAM_UINT8:   element_size = 1; break;
            case GS_PARAM_UINT16:  element_size = 2; break;
            case GS_PARAM_UINT32:  element_size = 4; break;
            case GS_PARAM_INT16:   element_size = 2; break;
            case GS_PARAM_FLOAT:   element_size = 4; break;
            default:               element_size = 1; break;
        }

        gs_error_t gs_err = gs_rparam_get_array(Payload->node, Payload->table, Payload->addr, Payload->type,
                                                GS_RPARAM_MAGIC_CHECKSUM, 5000, (void *)&(Payload->param), element_size, Payload->len);

        if (gs_err == GS_OK) {
            Status = CFE_SUCCESS;
        }
        else {
            Status = (int32)gs_err;
        }
    }
    else {
        Status = CFE_SRL_ApiGetRparamCSP(Payload->type, Payload->node, Payload->table,
                                     Payload->addr, (void *)&(Payload->param));
    }

    PAY_SLT_APP_printf("After getRparam\n");

    
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAY_SLT_FetchParam err: %d", Status);
    }

    return Status;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SET_PARAM:                                                                 */
/*   파라미터 설정 (uint8 ,uint16, uint32만 취급함)                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 PAY_SLT_SetParam(uint8 node, uint8 table, uint16 addr, uint8 type, void *value) {
    int32 Status;
    void *pass_ptr = NULL; // 하위 API에 최종적으로 넘겨줄 포인터

    // null pointer check
    if (value == NULL) return -1;

    union {
        uint8  u8;
        uint16 u16;
        uint32 u32;
    } param_val;

    uint32 raw_value = *(uint32 *)value;

    switch(type) {
        case GS_PARAM_UINT8:  
            param_val.u8 = (uint8)raw_value;
            pass_ptr = &param_val.u8; 
            break;
            
        case GS_PARAM_UINT16: 
            param_val.u16 = (uint16)raw_value;
            pass_ptr = &param_val.u16;
            break;
            
        case GS_PARAM_UINT32: 
            param_val.u32 = raw_value;
            pass_ptr = &param_val.u32;
            break;

        default:
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, 
                              "PAY_SLT_SetParam err: Unsupported type %d", type);
            return -1;
    }

    PAY_SLT_APP_printf("Before SetRparam\n");

    (void)pass_ptr;
    Status = PAY_SLT_SetParamCompat(node, table, addr, type, raw_value);

    PAY_SLT_APP_printf("After SetRparam\n");

    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAY_SLT_SetParam err: %d", Status);
    }

    return Status;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* READ_EXP_I2C_CHUNK:                                                        */
/*     I2C read part/chunk data                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 PAY_SLT_ReadExpI2CChunk(CFE_SRL_IO_Handle_t *handle, uint32 start_addr, void *data, size_t size)
{
    int32 status;
    uint8 exp_i2c_addr = 0;
    uint8 addr_buf[PAY_SLT_I2C_ADDR_BYTES];
    CFE_SRL_IO_Param_t params = {0};

    if ((handle == NULL) || (data == NULL) || (size == 0U)) {
        return SLT_IFB_DEVICE_BAD_ARG;
    }

    // I2C addr : uint8, table 0 , 0x0017, 
    status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_EXP_A7_NODE, TABLE_BOARD_PARAM, 0x0017, 1, &exp_i2c_addr);
    if (status != CFE_SUCCESS) {
        return status;
    }

    PAY_SLT_APP_printf("PAY_SLT_ReadExpI2CChunk: start_addr=0x%08X, size=%zu, exp_i2c_addr=0x%02X\n", start_addr, size, exp_i2c_addr);

    addr_buf[0] = (uint8)((start_addr >> 24) & 0xFFU);
    addr_buf[1] = (uint8)((start_addr >> 16) & 0xFFU);
    addr_buf[2] = (uint8)((start_addr >> 8) & 0xFFU);
    addr_buf[3] = (uint8)(start_addr & 0xFFU);

    /*
     * This I2C adapter does not support I2C_RDWR combined messages, so use
     * the driver's interval path: write the memory address, wait, then read.
     */
    params.TxData = addr_buf;
    params.TxSize = sizeof(addr_buf);
    params.RxData = data;
    params.RxSize = size;
    params.Timeout = SLT_IFB_RPARAM_TIMEOUT_MS;
    params.Interval = 10;
    params.Addr = exp_i2c_addr;

    return CFE_SRL_ApiRead(handle, &params);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UPDATE_CRC32:                                                              */
/*     지속적인 업데이트가 가능한 CRC32 연산 함수                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
uint32 PAY_SLT_UpdateCRC32(uint32 current_crc, const uint8 *data, uint32 length) {
    for (uint32 i = 0; i < length; i++) {
        current_crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (current_crc & 1) {
                current_crc = (current_crc >> 1) ^ 0xEDB88320;
            } else {
                current_crc = (current_crc >> 1);
            }
        }
    }
    return current_crc;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* CALCULATE_CRC32:                                                           */
/*     표준 CRC-32 ISO HDLC (zlib 호환) 계산 함수                                */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* Polynomial: 0xEDB88320 (Reversed 0x04C11DB7)  */
uint32 PAY_SLT_CalculateCRC32(const uint8 *data, uint32 length) {
    uint32 crc0 = 0xFFFFFFFF; // 초기값

    uint32 crc = PAY_SLT_UpdateCRC32(crc0, data, length);

    return crc ^ 0xFFFFFFFF; // 최종 결과 반전(XOR Out)
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* GET_FULL_TABLE:                                                            */
/*     Get Full Table Params at once                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
gs_error_t PAY_SLT_GetFullTable(uint8_t node, uint8_t table,
                                     gs_param_table_instance_t *tinst, uint32_t timeout_ms)
{
    uint16 checksum;

    if (tinst == NULL)
        return GS_ERROR_ARG;

    memset(tinst, 0, sizeof(*tinst));

    gs_error_t result = gs_rparam_download_table_spec(tinst, NULL, node, table, timeout_ms, &checksum);
    if (result)
    {
        gs_param_table_free(tinst);
        return result;
    }

    gs_error_t err = gs_rparam_get_full_table(tinst, node, table, checksum, timeout_ms);
    if (err != GS_OK)
    {
        gs_param_table_free(tinst);
        return err;
    }

    return GS_OK;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PRINT_PARAM_TABLE:                                                         */
/*     Print Full Parameter Table. PAY_SLT_APP_printf                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static const char *PAY_SLT_ParamTypeLabel(gs_param_type_t type)
{
    switch (type) {
        case GS_PARAM_BOOL:   return "BL";
        case GS_PARAM_INT8:   return "I8";
        case GS_PARAM_UINT8:
        case PARAM_X8:        return "U8";
        case GS_PARAM_INT16:  return "I16";
        case GS_PARAM_UINT16:
        case PARAM_X16:       return "U16";
        case GS_PARAM_INT32:  return "I32";
        case GS_PARAM_UINT32:
        case PARAM_X32:       return "U32";
        case GS_PARAM_INT64:  return "I64";
        case GS_PARAM_UINT64:
        case PARAM_X64:       return "U64";
        case GS_PARAM_FLOAT:  return "FLT";
        case GS_PARAM_DOUBLE: return "DBL";
        case GS_PARAM_STRING: return "STR";
        case GS_PARAM_DATA:   return "DAT";
        default:              return "UNK";
    }
}

static void PAY_SLT_PrintParamTableRow(const gs_param_table_instance_t *tinst, const gs_param_table_row_t *row)
{
    char buf[128] = {0};
    uint16_t addr = GS_PARAM_ADDR(row);
    gs_param_type_t type = GS_PARAM_TYPE(row);
    size_t param_size = (size_t)GS_PARAM_SIZE(row);
    size_t array_size = (size_t)GS_PARAM_ARRAY_SIZE(row);
    size_t value_size = param_size * array_size;

    if (param_size == 0U) {
        PAY_SLT_APP_printf("  [%3u] %-14.14s <invalid size>\n", addr, row->name);
        return;
    }

    if ((size_t)addr >= tinst->memory_size || value_size > ((size_t)tinst->memory_size - (size_t)addr)) {
        PAY_SLT_APP_printf("  [%3u] %-14.14s <addr out of range>\n", addr, row->name);
        return;
    }

    PAY_SLT_APP_printf("  [%3u] %-14.14s %-4s", addr, row->name, PAY_SLT_ParamTypeLabel(type));

    if (array_size > 1U) {
        PAY_SLT_APP_printf("[");
    }

    for (size_t i = 0U; i < array_size; i++) {
        const void *value = (const uint8_t *)tinst->memory + addr + (param_size * i);
        gs_error_t err = gs_param_to_string2(row, value, false, 0, buf, sizeof(buf), 0, NULL);

        if (err != GS_OK) {
            PAY_SLT_APP_printf("<decode err=%d>\n", err);
            return;
        }

        PAY_SLT_APP_printf("%s%s", (i == 0U) ? "" : " ", buf);
    }

    if (array_size > 1U) {
        PAY_SLT_APP_printf("]");
    }

    PAY_SLT_APP_printf("\n");
}

void PAY_SLT_PrintParamTable(uint8 node, uint8 table, const gs_param_table_instance_t *tinst)
{
    char deviceName[16] = {0};
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_STRING, node, TABLE_BOARD_PARAM, 0x0000, 16, deviceName);

    PAY_SLT_APP_printf("\n================[PAY-SLT Param Table]===================\n");
    PAY_SLT_APP_printf("  Device: %s | Node: %u | Table: %u\n",
              deviceName, node, table);

    if (tinst == NULL)
    {
        PAY_SLT_APP_printf("  <no table instance>\n");
        PAY_SLT_APP_printf("=======================================================\n");
        return;
    }

    PAY_SLT_APP_printf("  Remote Name: %s | Rows: %u | Size: %u bytes\n",
              tinst->name ? tinst->name : "N/A", tinst->row_count, tinst->memory_size);

    if (tinst->rows == NULL || tinst->memory == NULL)
    {
        PAY_SLT_APP_printf("  <table rows or memory unavailable>\n");
        PAY_SLT_APP_printf("=======================================================\n");
        return;
    }

    for (unsigned int i = 0; i < tinst->row_count; i++)
        PAY_SLT_PrintParamTableRow(tinst, &tinst->rows[i]);

    PAY_SLT_APP_printf("=======================================================\n");
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* HANDLE_REPORT:                                                             */
/*   지상에서 보낸 명령이 성공했는지/실패했는지 기록, telemetry packet 작성 후 발송    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_HandleReport(int32 status, uint8 command_code, bool device_error, const void *read_data,
                                  uint16 read_size)
{
    CFE_SB_Buffer_t *BufPtr;
    SLT_IFB_RPT_t *Report;
    bool success = ((device_error && (status > 0)) || (!device_error && (status == CFE_SUCCESS)));
    size_t copy_size = read_size;

    BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(SLT_IFB_RPT_t));
    if (BufPtr == NULL)
    {
        return CFE_SB_BUF_ALOC_ERR;
    }

    Report = (SLT_IFB_RPT_t *)BufPtr;
    memset(Report, 0, sizeof(*Report));
    if (CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(PAY_SLT_RPT_TLM_MID), sizeof(*Report)) !=
        CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        return CFE_MSG_BAD_ARGUMENT;
    }

    Report->Report.MsgID = PAY_SLT_CMD_MID;
    Report->Report.CommandCode = command_code;
    Report->Report.ReturnCode = success ? CFE_SUCCESS : status;

    if (success)
    {
        Report->Report.ReturnType = RPT_RETTYPE_SUCCESS;
        SLT_IFB_Data.CmdCounter++;
    }
    else
    {
        Report->Report.ReturnType = device_error ? RPT_RETTYPE_HW : RPT_RETTYPE_APP;
        SLT_IFB_Data.CmdCounter++;
        SLT_IFB_Data.ErrCounter++;
        if (device_error)
        {
            SLT_IFB_Data.DeviceErrCounter++;
        }
        else
        {
            SLT_IFB_Data.AppErrCounter++;
        }
    }

    if ((read_data == NULL) || (read_size == 0U))
    {
        goto send_report;
    }

    if (copy_size > sizeof(Report->Report.ReturnValue))
    {
        copy_size = sizeof(Report->Report.ReturnValue);
    }

    memcpy(Report->Report.ReturnValue, read_data, copy_size);
    Report->Report.ReturnDataSize = (uint16)copy_size;

send_report:
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report->TelemetryHeader));
    if (CFE_SB_TransmitBuffer(BufPtr, true) != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        return CFE_SB_BUF_ALOC_ERR;
    }

    return success ? CFE_SUCCESS : status;
}
