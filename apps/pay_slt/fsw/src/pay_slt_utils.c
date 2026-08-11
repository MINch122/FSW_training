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

#define PAY_SLT_RPARAM_GET               0x00U
#define PAY_SLT_RPARAM_REPLY             0x55U
#define PAY_SLT_RPARAM_SET               0xFFU
#define PAY_SLT_RPARAM_SET_OK            1U
#define PAY_SLT_RPARAM_HEADER_SIZE       10U
#define PAY_SLT_RPARAM_ADDR_SIZE         2U
#define PAY_SLT_RPARAM_MAX_REPLY_PAYLOAD 180U

static void PAY_SLT_StoreBe16(uint8 *buf, uint16 value)
{
    buf[0] = (uint8)((value >> 8) & 0xFFU);
    buf[1] = (uint8)(value & 0xFFU);
}

static uint16 PAY_SLT_LoadBe16(const uint8 *buf)
{
    return (uint16)(((uint16)buf[0] << 8) | (uint16)buf[1]);
}

static uint32 PAY_SLT_LoadBe32(const uint8 *buf)
{
    return ((uint32)buf[0] << 24) | ((uint32)buf[1] << 16) | ((uint32)buf[2] << 8) | (uint32)buf[3];
}

static size_t PAY_SLT_ParamElementSize(uint8 type)
{
    switch (type) {
        case GS_PARAM_UINT8:
        case GS_PARAM_INT8:
        case GS_PARAM_STRING:
            return 1U;
        case GS_PARAM_UINT16:
        case GS_PARAM_INT16:
            return 2U;
        case GS_PARAM_UINT32:
        case GS_PARAM_FLOAT:
            return 4U;
        default:
            return 0U;
    }
}

static size_t PAY_SLT_ParamElementCapacity(uint8 type)
{
    switch (type) {
        case GS_PARAM_UINT8:
            return sizeof(((PAY_SLT_Params_t *)0)->param.u8) / sizeof(((PAY_SLT_Params_t *)0)->param.u8[0]);
        case GS_PARAM_UINT16:
            return sizeof(((PAY_SLT_Params_t *)0)->param.u16) / sizeof(((PAY_SLT_Params_t *)0)->param.u16[0]);
        case GS_PARAM_UINT32:
            return sizeof(((PAY_SLT_Params_t *)0)->param.u32) / sizeof(((PAY_SLT_Params_t *)0)->param.u32[0]);
        case GS_PARAM_INT8:
            return sizeof(((PAY_SLT_Params_t *)0)->param.i8) / sizeof(((PAY_SLT_Params_t *)0)->param.i8[0]);
        case GS_PARAM_INT16:
            return sizeof(((PAY_SLT_Params_t *)0)->param.i16) / sizeof(((PAY_SLT_Params_t *)0)->param.i16[0]);
        case GS_PARAM_STRING:
            return sizeof(((PAY_SLT_Params_t *)0)->param.str);
        case GS_PARAM_FLOAT:
            return sizeof(((PAY_SLT_Params_t *)0)->param.flt) / sizeof(((PAY_SLT_Params_t *)0)->param.flt[0]);
        default:
            return 0U;
    }
}

static void PAY_SLT_CopyParamValueFromBe(uint8 type, uint8 *dst, const uint8 *src)
{
    switch (type) {
        case GS_PARAM_UINT8:
        case GS_PARAM_INT8:
        case GS_PARAM_STRING:
            *dst = *src;
            break;
        case GS_PARAM_UINT16: {
            uint16 value = PAY_SLT_LoadBe16(src);
            memcpy(dst, &value, sizeof(value));
            break;
        }
        case GS_PARAM_INT16: {
            int16 value = (int16)PAY_SLT_LoadBe16(src);
            memcpy(dst, &value, sizeof(value));
            break;
        }
        case GS_PARAM_UINT32: {
            uint32 value = PAY_SLT_LoadBe32(src);
            memcpy(dst, &value, sizeof(value));
            break;
        }
        case GS_PARAM_FLOAT: {
            uint32 raw_value = PAY_SLT_LoadBe32(src);
            memcpy(dst, &raw_value, sizeof(raw_value));
            break;
        }
        default:
            break;
    }
}

static int32 PAY_SLT_FetchParamCompat(PAY_SLT_Params_t *Payload)
{
    uint8 query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_MAX_REPLY_PAYLOAD] = {0};
    uint8 reply[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_MAX_REPLY_PAYLOAD] = {0};
    size_t element_size;
    size_t capacity;
    size_t request_count;
    size_t query_payload_size;
    size_t reply_payload_size;
    int reply_len;

    element_size = PAY_SLT_ParamElementSize(Payload->type);
    capacity     = PAY_SLT_ParamElementCapacity(Payload->type);
    if ((element_size == 0U) || (capacity == 0U) || (Payload->len == 0U)) {
        return GS_ERROR_ARG;
    }

    if (Payload->type == GS_PARAM_STRING) {
        request_count = 1U;
        if (Payload->len > capacity) {
            return GS_ERROR_ARG;
        }
        if (((size_t)PAY_SLT_RPARAM_ADDR_SIZE + Payload->len) > PAY_SLT_RPARAM_MAX_REPLY_PAYLOAD) {
            return GS_ERROR_ARG;
        }
    } else {
        request_count = Payload->len;
        if (request_count > capacity) {
            return GS_ERROR_ARG;
        }
        if ((request_count * PAY_SLT_RPARAM_ADDR_SIZE) > PAY_SLT_RPARAM_MAX_REPLY_PAYLOAD) {
            return GS_ERROR_ARG;
        }
        if ((request_count * (PAY_SLT_RPARAM_ADDR_SIZE + element_size)) > PAY_SLT_RPARAM_MAX_REPLY_PAYLOAD) {
            return GS_ERROR_ARG;
        }
    }

    query_payload_size = request_count * PAY_SLT_RPARAM_ADDR_SIZE;

    query[0] = PAY_SLT_RPARAM_GET;
    query[1] = Payload->table;
    PAY_SLT_StoreBe16(&query[2], (uint16)query_payload_size);
    PAY_SLT_StoreBe16(&query[4], GS_RPARAM_MAGIC_CHECKSUM);
    PAY_SLT_StoreBe16(&query[6], 0U);
    PAY_SLT_StoreBe16(&query[8], 0U);

    for (size_t i = 0; i < request_count; ++i) {
        uint16 item_addr = (Payload->type == GS_PARAM_STRING) ? Payload->addr : (uint16)(Payload->addr + (element_size * i));
        PAY_SLT_StoreBe16(&query[PAY_SLT_RPARAM_HEADER_SIZE + (i * PAY_SLT_RPARAM_ADDR_SIZE)], item_addr);
    }

    reply_len = csp_transaction2(CSP_PRIO_HIGH, Payload->node, GS_CSP_PORT_RPARAM, SLT_IFB_RPARAM_TIMEOUT_MS,
                                 query, (int)(PAY_SLT_RPARAM_HEADER_SIZE + query_payload_size),
                                 reply, -1, CSP_O_CRC32);
    if (reply_len <= 0) {
        return GS_ERROR_IO;
    }
    if (reply_len < (int)PAY_SLT_RPARAM_HEADER_SIZE) {
        return GS_ERROR_DATA;
    }
    if ((reply[0] != PAY_SLT_RPARAM_REPLY) || (reply[1] != Payload->table)) {
        return GS_ERROR_DATA;
    }

    reply_payload_size = PAY_SLT_LoadBe16(&reply[2]);
    if (reply_payload_size > (size_t)(reply_len - (int)PAY_SLT_RPARAM_HEADER_SIZE)) {
        return GS_ERROR_DATA;
    }
    if (reply_payload_size < PAY_SLT_RPARAM_ADDR_SIZE) {
        return GS_ERROR_DATA;
    }

    if (PAY_SLT_LoadBe16(&reply[PAY_SLT_RPARAM_HEADER_SIZE]) != Payload->addr) {
        return GS_ERROR_DATA;
    }

    if (Payload->type == GS_PARAM_STRING) {
        size_t string_size = reply_payload_size - PAY_SLT_RPARAM_ADDR_SIZE;
        if (string_size > Payload->len) {
            return GS_ERROR_DATA;
        }

        memcpy(Payload->param.str, &reply[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE], string_size);
        if (string_size < sizeof(Payload->param.str)) {
            Payload->param.str[string_size] = '\0';
        }
        Payload->len = (uint8)string_size;
        return CFE_SUCCESS;
    }

    {
        const size_t reply_element_size = PAY_SLT_RPARAM_ADDR_SIZE + element_size;
        size_t actual_count;

        if ((reply_payload_size % reply_element_size) != 0U) {
            return GS_ERROR_DATA;
        }

        actual_count = reply_payload_size / reply_element_size;
        if ((actual_count == 0U) || (actual_count > request_count)) {
            return GS_ERROR_DATA;
        }

        for (size_t i = 0; i < actual_count; ++i) {
            const size_t item_offset = PAY_SLT_RPARAM_HEADER_SIZE + (i * reply_element_size);
            const uint16 expected_addr = (uint16)(Payload->addr + (element_size * i));

            if (PAY_SLT_LoadBe16(&reply[item_offset]) != expected_addr) {
                return GS_ERROR_DATA;
            }

            PAY_SLT_CopyParamValueFromBe(Payload->type,
                                         ((uint8 *)&Payload->param) + (i * element_size),
                                         &reply[item_offset + PAY_SLT_RPARAM_ADDR_SIZE]);
        }

        Payload->len = (uint8)actual_count;
    }

    return CFE_SUCCESS;
}


static int32 PAY_SLT_SetParamCompat(uint8 node, uint8 table, uint16 addr, uint8 type, const void *value)
{
    uint8 query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE + 256U] = {0};
    uint8 reply[256] = {0};
    size_t value_size = 0U;
    size_t payload_size;
    size_t query_size;
    int reply_len;
    uint32 raw_value = 0U;
    const char *str_value = NULL;

    if (value == NULL) {
        return GS_ERROR_ARG;
    }

    switch (type) {
        case GS_PARAM_UINT8:
            value_size = 1U;
            raw_value = (uint32)*(const uint8 *)value;
            break;
        case GS_PARAM_UINT16:
            value_size = 2U;
            raw_value = (uint32)*(const uint16 *)value;
            break;
        case GS_PARAM_UINT32:
            value_size = 4U;
            raw_value = *(const uint32 *)value;
            break;
        case GS_PARAM_STRING:
            str_value = (const char *)value;
            if (str_value == NULL) {
                return GS_ERROR_ARG;
            }

            /* 자동으로 '\0' 포함 */
            value_size = 0;
            while (value_size < 128 && str_value[value_size] != '\0') {
                value_size ++;
            }
            value_size++;
            break;
        default:
            return GS_ERROR_ARG;
    }

    payload_size = PAY_SLT_RPARAM_ADDR_SIZE + value_size;
    query_size = PAY_SLT_RPARAM_HEADER_SIZE + payload_size;
    if (query_size > sizeof(query)) {
        return GS_ERROR_ARG;
    }

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
        case GS_PARAM_STRING:
            memcpy(&query[PAY_SLT_RPARAM_HEADER_SIZE + PAY_SLT_RPARAM_ADDR_SIZE], str_value, value_size);
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
            case GS_PARAM_INT8:    element_size = 1; break;
            case GS_PARAM_INT16:   element_size = 2; break;
            case GS_PARAM_STRING:  element_size = 1; break;
            case GS_PARAM_FLOAT:   element_size = 4; break;
            default:               element_size = 1; break;
        }
        
        // 데이터 복사
        memcpy(out_ptr, &req.param, (element_size * req.len));
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

    Status = PAY_SLT_FetchParamCompat(Payload);
    
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

    // null pointer check
    if (value == NULL) return -1;

    PAY_SLT_APP_printf("Before SetRparam\n");

    switch(type) {
        case GS_PARAM_UINT8: {
            uint8 param_val = *(uint8 *)value;
            Status = PAY_SLT_SetParamCompat(node, table, addr, type, &param_val);
            break;
        }

        case GS_PARAM_UINT16: {
            uint16 param_val = *(uint16 *)value;
            Status = PAY_SLT_SetParamCompat(node, table, addr, type, &param_val);
            break;
        }

        case GS_PARAM_UINT32: {
            uint32 param_val = *(uint32 *)value;
            Status = PAY_SLT_SetParamCompat(node, table, addr, type, &param_val);
            break;
        }

        case GS_PARAM_STRING: {
            const char *str_value = (const char *)value;
            Status = PAY_SLT_SetParamCompat(node, table, addr, type, str_value);
            break;
        }

        default:
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAY_SLT_SetParam err: Unsupported type %d", type);
            return -1;
    }

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
    status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_EXP_A7_NODE, TABLE_BOARD_PARAM, 29, 1, &exp_i2c_addr);
    if (status != CFE_SUCCESS) {
        return status;
    }

    addr_buf[0] = (uint8)((start_addr >> 24) & 0xFFU);
    addr_buf[1] = (uint8)((start_addr >> 16) & 0xFFU);
    addr_buf[2] = (uint8)((start_addr >> 8) & 0xFFU);
    addr_buf[3] = (uint8)(start_addr & 0xFFU);
    
    // download protocol = I2C
    handle = PAY_SLT_Data.I2c1Handle;

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


/* RS422 file chunks can take longer than CSP/RParam replies to become ready. */
#define PAY_SLT_RS422_DOWNLOAD_TIMEOUT_MS 5000U
#define PAY_SLT_RS422_DOWNLOAD_TX_INTERVAL_MS 100U
#define PAY_SLT_RS422_DOWNLOAD_HEADER_SIZE 7U
#define PAY_SLT_RS422_DOWNLOAD_END_SIZE 1U
#define PAY_SLT_RS422_DOWNLOAD_OVERHEAD_SIZE \
    (PAY_SLT_RS422_DOWNLOAD_HEADER_SIZE + PAY_SLT_RS422_DOWNLOAD_END_SIZE)

static int32 PAY_SLT_ReadExpRS422Bytes(CFE_SRL_IO_Handle_t *handle, uint8 *data, size_t size,
                                       const uint8 *tx_data, size_t tx_size, uint32 interval)
{
    int32 status;

    for (size_t offset = 0U; offset < size; ++offset) {
        CFE_SRL_IO_Param_t params = {0};

        if (offset == 0U) {
            params.TxData = (void *)tx_data;
            params.TxSize = tx_size;
            params.Interval = interval;
        }

        params.RxData = &data[offset];
        params.RxSize = 1U;
        params.Timeout = PAY_SLT_RS422_DOWNLOAD_TIMEOUT_MS;

        status = CFE_SRL_ApiRead(handle, &params);
        if (status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RS422 DOWNLOAD read failed at byte %u/%u. Status=0x%08X",
                              (unsigned int)offset, (unsigned int)size, (unsigned int)status);
            return status;
        }
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* READ_EXP_RS422_CHUNK:                                                      */
/*     RS422 read part/chunk data                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 PAY_SLT_ReadExpRS422Chunk(CFE_SRL_IO_Handle_t *handle, void *data, size_t size) {
    uint8 rs422_cmd_buf[7] = {0x40, 0x44, 0x00, 0x00, 0x00, 0x00, 0x0D}; // RS422 DOWNLOAD command frame [Start 0x40][Cmd 0x44][P1 - P4 0x00][End 0x0D]
    uint8 *rx_data = (uint8 *)data;
    uint16 payload_len;
    size_t remaining_size;
    int32 status;

    if ((handle == NULL) || (data == NULL) || (size < PAY_SLT_RS422_DOWNLOAD_OVERHEAD_SIZE)) {
        return SLT_IFB_DEVICE_BAD_ARG;
    }

    status = PAY_SLT_ReadExpRS422Bytes(handle, rx_data, PAY_SLT_RS422_DOWNLOAD_HEADER_SIZE,
                                       rs422_cmd_buf, sizeof(rs422_cmd_buf),
                                       PAY_SLT_RS422_DOWNLOAD_TX_INTERVAL_MS);
    if (status != CFE_SUCCESS) {
        return status;
    }

    payload_len = (uint16)(((uint16)rx_data[3] << 8) | (uint16)rx_data[4]);
    if (payload_len < 2U) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RS422 DOWNLOAD reply length %u is shorter than PN field. Header=%02X %02X %02X %02X %02X %02X %02X",
                          payload_len, rx_data[0], rx_data[1], rx_data[2], rx_data[3], rx_data[4], rx_data[5], rx_data[6]);
        return CFE_SRL_PARTIAL_READ_ERR;
    }

    remaining_size = (size_t)(payload_len - 2U); /* Chunk data only; PN was already read in header */

    if ((PAY_SLT_RS422_DOWNLOAD_OVERHEAD_SIZE + remaining_size) > size) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "RS422 DOWNLOAD reply length %u exceeds buffer payload max %u. Header=%02X %02X %02X %02X %02X %02X %02X",
                          payload_len, (unsigned int)(size - PAY_SLT_RS422_DOWNLOAD_OVERHEAD_SIZE),
                          rx_data[0], rx_data[1], rx_data[2], rx_data[3], rx_data[4], rx_data[5], rx_data[6]);
        return CFE_SRL_PARTIAL_READ_ERR;
    }

    return PAY_SLT_ReadExpRS422Bytes(handle, &rx_data[PAY_SLT_RS422_DOWNLOAD_HEADER_SIZE],
                                     remaining_size + PAY_SLT_RS422_DOWNLOAD_END_SIZE, NULL, 0U, 0U);
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
CFE_Status_t PAY_SLT_HandleReportForMid(uint16 message_id, int32 status, uint8 command_code, bool device_error,
                                        const void *read_data, uint16 read_size) {
    CFE_SB_Buffer_t *BufPtr;
    PAY_SLT_RPT_t *Report;
    CFE_Status_t report_status;
    bool success = (status == CFE_SUCCESS);
    size_t copy_size = read_size;

    if (!success) {
        PAY_SLT_Data.ErrCounter++;
        if (device_error) {
            PAY_SLT_Data.DeviceErrCounter++;
        } else {
            PAY_SLT_Data.AppErrCounter++;
        }
    }

    BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(PAY_SLT_RPT_t));
    if (BufPtr == NULL) {
        return CFE_SB_BUF_ALOC_ERR;
    }

    Report = (PAY_SLT_RPT_t *)BufPtr;
    memset(Report, 0, sizeof(*Report));
    report_status = CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(PAY_SLT_RPT_TLM_MID),
                                 sizeof(*Report));
    if (report_status != CFE_SUCCESS)
    {
        CFE_SB_ReleaseMessageBuffer(BufPtr);
        return report_status;
    }

    Report->Report.MsgID = message_id;
    Report->Report.CommandCode = command_code;
    Report->Report.ReturnCode = status;

    if (success) {
        Report->Report.ReturnType = RPT_RETTYPE_SUCCESS;
    } else {
        Report->Report.ReturnType = device_error ? RPT_RETTYPE_HW : RPT_RETTYPE_APP;
    }

    if ((read_data == NULL) || (read_size == 0U)) {
        goto send_report;
    }

    if (copy_size > sizeof(Report->Report.ReturnValue)) {
        copy_size = sizeof(Report->Report.ReturnValue);
    }

    memcpy(Report->Report.ReturnValue, read_data, copy_size);
    Report->Report.ReturnDataSize = (uint16)copy_size;

send_report:
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report->TelemetryHeader));
    report_status = CFE_SB_TransmitBuffer(BufPtr, true);
    if (report_status != CFE_SUCCESS) {
        return report_status;
    }

    return success ? CFE_SUCCESS : status;
}

CFE_Status_t PAY_SLT_HandleReport(int32 status, uint8 command_code, bool device_error, const void *read_data,
                                  uint16 read_size)
{
    return PAY_SLT_HandleReportForMid(PAY_SLT_CMD_MID, status, command_code, device_error, read_data, read_size);
}
