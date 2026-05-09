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

int32 PAY_SLT_ReadExpI2CChunk(CFE_SRL_IO_Handle_t *handle, uint32 start_addr, void *data, size_t size)
{
    int32 status;
    uint8 exp_i2c_addr = 0;
    uint8 addr_buf[PAY_SLT_I2C_ADDR_BYTES];
    CFE_SRL_IO_Param_t params = {0};

    if ((handle == NULL) || (data == NULL) || (size == 0U))
    {
        return SLT_IFB_DEVICE_BAD_ARG;
    }

    status = PAY_SLT_GetRparam(GS_PARAM_UINT8, CSP_NODE_PAY_EXP, BOARD_PARAMETER_TABLE, EXP_I2C_ADDR_ADDRESS,
                               &exp_i2c_addr);
    if (status != CFE_SUCCESS)
    {
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
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    memset(&params, 0, sizeof(params));
    params.RxData = data;
    params.RxSize = size;
    params.Timeout = SLT_IFB_RPARAM_TIMEOUT_MS;
    params.Addr = exp_i2c_addr;

    return CFE_SRL_ApiRead(handle, &params);
}

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

int32 SLT_IFB_GET_BRD_UID(char *Board_uid)
{
    return PAY_SLT_GetRparam(GS_PARAM_STRING, CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, BRD_UID_ADDRESS, Board_uid);
}

int32 SLT_IFB_GET_BRD_REV(uint8 *Board_revision)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT8, CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, BRD_REV_ADDRESS, Board_revision);
}

int32 SLT_IFB_GET_CSP_ADDR(uint8 *MPU_CSP_address)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT8, CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, CSP_ADDR_ADDRESS, MPU_CSP_address);
}

int32 SLT_IFB_GET_CAN_SPEED(uint16 *CAN_bus_speed)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT16, CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, CAN_SPEED_ADDRESS, CAN_bus_speed);
}

int32 SLT_IFB_GET_I2C_ADDR(uint8 *I2C_address)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT8, CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, I2C_ADDR_ADDRESS, I2C_address);
}

int32 SLT_IFB_GET_I2C_SPEED(uint16 *I2C_speed)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT16, CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, I2C_SPEED_ADDRESS, I2C_speed);
}

int32 SLT_IFB_GET_WDT_VAL(uint32 *WDT_value)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT32, CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, WDT_VAL_ADDRESS, WDT_value);
}

int32 SLT_IFB_GET_CSP_RTABLE(char *MPU_CSP_routing_table)
{
    return PAY_SLT_GetRparam(GS_PARAM_STRING, CSP_NODE_PAY_IFB, BOARD_PARAMETER_TABLE, CSP_RTABLE_ADDRESS, MPU_CSP_routing_table);
}

int32 SLT_IFB_GET_SYS_STATUS(int16 *System_status)
{
    return PAY_SLT_GetRparam(GS_PARAM_INT16, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, SYS_STATUS_ADDRESS, System_status);
}

int32 SLT_IFB_GET_SYS_UPTIME(uint32 *System_uptime)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT32, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, SYS_UPTIME_ADDRESS, System_uptime);
}

int32 SLT_IFB_GET_BOOT_CNT(uint16 *System_boot_count)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT16, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, BOOT_CNT_ADDRESS, System_boot_count);
}

int32 SLT_IFB_GET_BOOT_CAUSE(uint16 *Last_boot_cause)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT16, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, BOOT_CAUSE_ADDRESS, Last_boot_cause);
}

int32 SLT_IFB_GET_REBOOT_CAUSE(uint16 *Last_reboot_cause)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT16, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, REBOOT_CAUSE_ADDRESS, Last_reboot_cause);
}

int32 SLT_IFB_GET_WDT_LEFT(uint32 *Time_left_WDT_cause_reboot)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT32, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, WDT_LEFT_ADDRESS, Time_left_WDT_cause_reboot);
}

int32 SLT_IFB_GET_BRD_TEMP(int16 *Board_temperature)
{
    return PAY_SLT_GetRparam(GS_PARAM_INT16, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, BRD_TEMP_ADDRESS, Board_temperature);
}

int32 SLT_IFB_GET_PWR_CURRENT(uint16 *System_power_current)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT16, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, PWR_CURRENT_ADDRESS, System_power_current);
}

int32 SLT_IFB_GET_IMU_DATA(uint16 *IMU_sensor_data)
{
    return PAY_SLT_GetRparam(GS_PARAM_UINT16, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, IMU_DATA_ADDRESS, IMU_sensor_data);
}

int32 SLT_IFB_GET_NTC_DATA(int16 *NTC_sensor_data)
{
    return PAY_SLT_GetRparam(GS_PARAM_INT16, CSP_NODE_PAY_IFB, TELEMETRY_TABLE, NTC_DATA_ADDRESS, NTC_sensor_data);
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
