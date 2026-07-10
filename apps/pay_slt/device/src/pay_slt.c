#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <gs/param/rparam.h>

#include "cfe_srl.h"
#include "pay_slt.h"

static int32 SLT_IFB_Transaction(uint8 node, uint8 port, const void *tx_data, int tx_size, void *rx_data, int rx_size)
{
    return CFE_SRL_ApiTransactionCSP(node, port, (void *)tx_data, tx_size, rx_data, rx_size);
}

int32 PAY_SLT_GetRparam(uint8 type, uint8 node, uint8 table_id, uint16 addr, void *param)
{
    if (param == NULL)
    {
        return SLT_IFB_DEVICE_BAD_ARG;
    }

    return CFE_SRL_ApiGetRparamCSP(type, node, table_id, addr, param);
}

int32 PAY_SLT_SetRparam(uint8 type, uint8 node, uint8 table_id, uint16 addr, void *param)
{
    if (param == NULL)
    {
        return SLT_IFB_DEVICE_BAD_ARG;
    }

    return CFE_SRL_ApiSetRparamCSP(type, node, table_id, addr, param);
}

int32 PAY_SLT_SaveTable(uint8 node, uint8 table_id)
{
    return CFE_SRL_ApiRparamSaveCSP(node, SLT_IFB_RPARAM_TIMEOUT_MS, table_id, table_id);
}

// I2C 읽는 함수는 pay_slt_utils.c에 구현되어 있음
/*
int32 PAY_SLT_ReadExpI2CChunk(CFE_SRL_IO_Handle_t *handle, uint32 start_addr, void *data, size_t size)
{
    int32 status;
    uint8 exp_i2c_addr = 0;
    uint8 addr_buf[PAY_SLT_I2C_ADDR_BYTES];
    CFE_SRL_IO_Param_t params = {0};

    if ((handle == NULL) || (data == NULL) || (size == 0U)) {
        return SLT_IFB_DEVICE_BAD_ARG;
    }

    status = PAY_SLT_GetRparam(GS_PARAM_UINT8, CSP_NODE_PAY_EXP, BOARD_PARAMETER_TABLE, EXP_I2C_ADDR_ADDRESS,
                               &exp_i2c_addr);
    if (status != CFE_SUCCESS) {
        return status;
    }

    addr_buf[0] = (uint8)((start_addr >> 24) & 0xFFU);
    addr_buf[1] = (uint8)((start_addr >> 16) & 0xFFU);
    addr_buf[2] = (uint8)((start_addr >> 8) & 0xFFU);
    addr_buf[3] = (uint8)(start_addr & 0xFFU);

    params.TxData = addr_buf;
    params.TxSize = sizeof(addr_buf);
    params.Addr = exp_i2c_addr;
    status = CFE_SRL_ApiWrite(handle, &params);
    if (status != CFE_SUCCESS) {
        return status;
    }

    memset(&params, 0, sizeof(params));
    params.RxData = data;
    params.RxSize = size;
    params.Timeout = SLT_IFB_RPARAM_TIMEOUT_MS;
    params.Addr = exp_i2c_addr;

    return CFE_SRL_ApiRead(handle, &params);
} */

int32 PAY_SLT_CSP_CMP(uint8 node)
{
    return SLT_IFB_Transaction(node, CSP_CMP_PORT, NULL, 0, NULL, 0);
}

int32 PAY_SLT_CSP_PING(uint8 node)
{
    return CFE_SRL_ApiPingCSP(node, SLT_IFB_RPARAM_TIMEOUT_MS, 4, CSP_O_CRC32);
}

int32 PAY_SLT_CSP_PS(uint8 node)
{
    return SLT_IFB_Transaction(node, CSP_PS_PORT, NULL, 0, NULL, 0);
}

int32 PAY_SLT_CSP_MEM_FREE(uint8 node)
{
    return SLT_IFB_Transaction(node, CSP_MEM_FREE_PORT, NULL, 0, NULL, 0);
}

int32 PAY_SLT_CSP_REBOOT(uint8 node)
{
    uint32 magic_word = csp_hton32(CSP_REBOOT_MAGIC);
    return SLT_IFB_Transaction(node, CSP_REBOOT_PORT, &magic_word, sizeof(magic_word), NULL, 0);
}

int32 PAY_SLT_CSP_BUF_FREE(uint8 node)
{
    return SLT_IFB_Transaction(node, CSP_BUF_FREE_PORT, NULL, 0, NULL, 0);
}

int32 PAY_SLT_CSP_UPTIME(uint8 node)
{
    return SLT_IFB_Transaction(node, CSP_UPTIME_PORT, NULL, 0, NULL, 0);
}

int32 PAY_SLT_CSP_GNDWDT(uint8 node)
{
    return SLT_IFB_Transaction(node, CSP_GNDWDT_PORT, NULL, 0, NULL, 0);
}

int32 SLT_IFB_CSP_CMP(void)
{
    return PAY_SLT_CSP_CMP(CSP_NODE_PAY_IFB);
}

int32 SLT_IFB_CSP_PING(void)
{
    return PAY_SLT_CSP_PING(CSP_NODE_PAY_IFB);
}

int32 SLT_IFB_CSP_PS(void)
{
    return PAY_SLT_CSP_PS(CSP_NODE_PAY_IFB);
}

int32 SLT_IFB_CSP_MEM_FREE(void)
{
    return PAY_SLT_CSP_MEM_FREE(CSP_NODE_PAY_IFB);
}

int32 SLT_IFB_CSP_REBOOT(void)
{
    return PAY_SLT_CSP_REBOOT(CSP_NODE_PAY_IFB);
}

int32 SLT_IFB_CSP_BUF_FREE(void)
{
    return PAY_SLT_CSP_BUF_FREE(CSP_NODE_PAY_IFB);
}

int32 SLT_IFB_CSP_UPTIME(void)
{
    return PAY_SLT_CSP_UPTIME(CSP_NODE_PAY_IFB);
}

int32 SLT_IFB_CSP_GNDWDT(void)
{
    return PAY_SLT_CSP_GNDWDT(CSP_NODE_PAY_IFB);
}

int32 SLT_IFB_SAVE_TABLE0(void)
{
    return PAY_SLT_SaveTable(CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE);
}

int32 SLT_IFB_SAVE_TABLE1(void)
{
    return PAY_SLT_SaveTable(CSP_NODE_PAY_IFB, CONFIGURATION_PARAMETER_TABLE);
}

int32 SLT_IFB_SAVE_TABLE4(void)
{
    return PAY_SLT_SaveTable(CSP_NODE_PAY_IFB, TELEMETRY_TABLE);
}

int32 SLT_IFB_SAVE_ALL_TABLE(void)
{
    int32 status = CFE_SUCCESS;

    status |= SLT_IFB_SAVE_TABLE0();
    status |= SLT_IFB_SAVE_TABLE1();
    status |= SLT_IFB_SAVE_TABLE4();

    return status;
}
