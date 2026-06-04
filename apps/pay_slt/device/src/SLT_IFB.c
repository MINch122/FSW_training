#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <gs/param/rparam.h>
#include "SLT_IFB.h"
#include "SLT_IFB_conf.h"

typedef int32 (*T_fn)(uint8 Node, uint8 Port, void *TxData, int TxSize, void *RxData, int RxSize);
//typedef int32 (*P_fn)(uint8 Node, uint32 Timeout, unsigned int Size, uint8 Options);
//typedef int32 (*G_fn)(uint8_t Type, uint8_t Node, uint8_t TableId, uint16_t Addr, void *Param);

#ifdef CFE_SRL_H
static T_fn Transaction_fn = CFE_SRL_ApiTransactionCSP;
//static P_fn Ping_fn = CFE_SRL_ApiPingCSP;
//static G_fn Get_fn = CFE_SRL_ApiGetRparamCSP;
#endif

/*
void Trans_Wrapper(T_fn fn){
    Transaction_fn = fn;
}

void Ping_Wrapper(P_fn fn){
    Ping_fn = fn;
}

void Get_Wrapper(G_fn fn){
    Get_fn = fn;
}


void function_init(T_fn fn_T, P_fn fn_P, G_fn fn_G)
{
    Trans_Wrapper(fn_T);
    Ping_Wrapper(fn_P);
    Get_Wrapper(fn_G);
}
*/

//transaction
int32 SLT_IFB_CSP_CMP(void)
{
    int32 status;
    status = Transaction_fn(PAY_IFB_NODE, CSP_CMP_PORT, NULL, 0, NULL, 0);
    return status;
}

int32 SLT_IFB_CSP_PING(void)
{
    int32 status;
    status = CFE_SRL_ApiPingCSP(PAY_IFB_NODE, 1000, 4, CSP_O_CRC32);
    return status;
}

int32 SLT_IFB_CSP_PS(void)
{
    int32 status;
    status = CFE_SRL_ApiTransactionCSP(PAY_IFB_NODE, CSP_PS_PORT, NULL, 0, NULL, 0);
    return status;
}

int32 SLT_IFB_CSP_MEM_FREE(void)
{
    int32 status;
    status = CFE_SRL_ApiTransactionCSP(PAY_IFB_NODE, CSP_MEM_FREE_PORT, NULL, 0, NULL, 0);
    return status;
}

int32 SLT_IFB_CSP_REBOOT(void)
{
    int32 status;
    uint32 magic_word = csp_hton32(CSP_REBOOT_MAGIC);

    status = CFE_SRL_ApiTransactionCSP(PAY_IFB_NODE, CSP_REBOOT_PORT,&magic_word, sizeof(magic_word), NULL, 0);
    return status;
}

int32 SLT_IFB_CSP_BUF_FREE(void)
{
    int32 status;
    status = CFE_SRL_ApiTransactionCSP(PAY_IFB_NODE, CSP_BUF_FREE_PORT, NULL, 0, NULL, 0);
    return status;
}

int32 SLT_IFB_CSP_UPTIME(void)
{
    int32 status;
    status = CFE_SRL_ApiTransactionCSP(PAY_IFB_NODE, CSP_UPTIME_PORT, NULL, 0, NULL, 0);
    return status;
}

int32 SLT_IFB_CSP_GNDWDT(void)
{
    int32 status;
    status = CFE_SRL_ApiTransactionCSP(PAY_IFB_NODE, CSP_GNDWDT_PORT, NULL, 0, NULL, 0);
    return status;
}

/*------------------------------------------------------------------------------------------------------------------------------------*/
//Table 0
int32 SLT_IFB_GET_BRD_UID(char* Board_uid)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_STRING, PAY_IFB_NODE, BOARD_PARAMETER_TABLE, BRD_UID_ADDRESS, Board_uid);
    
    return status;
}

int32 SLT_IFB_GET_BRD_REV(uint8* Board_revision)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT8, PAY_IFB_NODE, BOARD_PARAMETER_TABLE, BRD_REV_ADDRESS, Board_revision);
    return status;
}

int32 SLT_IFB_GET_CSP_ADDR(uint8* MPU_CSP_address)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT8, PAY_IFB_NODE, BOARD_PARAMETER_TABLE, CSP_ADDR_ADDRESS, &MPU_CSP_address);
    return status;
}

int32 SLT_IFB_GET_CAN_SPEED(uint16* CAN_bus_speed) //in kbps
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, PAY_IFB_NODE, BOARD_PARAMETER_TABLE, CAN_SPEED_ADDRESS, &CAN_bus_speed);
    return status;
}

int32 SLT_IFB_GET_I2C_ADDR(uint8* I2C_address)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT8, PAY_IFB_NODE, BOARD_PARAMETER_TABLE, I2C_ADDR_ADDRESS, &I2C_address);
    return status;
}

int32 SLT_IFB_GET_I2C_SPEED(uint16* I2C_speed)  //in kbps
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, PAY_IFB_NODE, BOARD_PARAMETER_TABLE, I2C_SPEED_ADDRESS, &I2C_speed);
    return status;
}

int32 SLT_IFB_GET_WDT_VAL(uint32* WDT_value)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, PAY_IFB_NODE, BOARD_PARAMETER_TABLE, WDT_VAL_ADDRESS, &WDT_value);
    return status;
}

int32 SLT_IFB_GET_CSP_RTABLE(char* MPU_CSP_routing_table)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_STRING, PAY_IFB_NODE, BOARD_PARAMETER_TABLE, CSP_RTABLE_ADDRESS, MPU_CSP_routing_table);
    return status;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------*/
//Table 4
int32 SLT_IFB_GET_SYS_STATUS(int16* System_status)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT16, PAY_IFB_NODE, TELEMETRY_TABLE, SYS_STATUS_ADDRESS, &System_status);
    return status;
}

int32 SLT_IFB_GET_SYS_UPTIME(uint32* System_uptime)  //in sec
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, PAY_IFB_NODE, TELEMETRY_TABLE, SYS_UPTIME_ADDRESS, &System_uptime);
    return status;
}

int32 SLT_IFB_GET_BOOT_CNT(uint16* System_boot_count)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, PAY_IFB_NODE, TELEMETRY_TABLE, BOOT_CNT_ADDRESS, &System_boot_count);
    return status;
}

int32 SLT_IFB_GET_BOOT_CAUSE(uint16* Last_boot_cause)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, PAY_IFB_NODE, TELEMETRY_TABLE, BOOT_CAUSE_ADDRESS, &Last_boot_cause);
    return status;
}

int32 SLT_IFB_GET_REBOOT_CAUSE(uint16* Last_reboot_cause)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, PAY_IFB_NODE, TELEMETRY_TABLE, REBOOT_CAUSE_ADDRESS, &Last_reboot_cause);
    return status;
}

int32 SLT_IFB_GET_WDT_LEFT(uint32* Time_left_WDT_cause_reboot)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT32, PAY_IFB_NODE, TELEMETRY_TABLE, WDT_LEFT_ADDRESS, &Time_left_WDT_cause_reboot);
    return status;
}

int32 SLT_IFB_GET_BRD_TEMP(int16* Board_temperature)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT16, PAY_IFB_NODE, TELEMETRY_TABLE, BRD_TEMP_ADDRESS, &Board_temperature);
    return status;
}

int32 SLT_IFB_GET_PWR_CURRENT(uint16* System_power_current) //in mA
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, PAY_IFB_NODE, TELEMETRY_TABLE, PWR_CURRENT_ADDRESS, &System_power_current);
    return status;
}

int32 SLT_IFB_GET_IMU_DATA(uint16* IMU_sensor_data)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_UINT16, PAY_IFB_NODE, TELEMETRY_TABLE, IMU_DATA_ADDRESS, IMU_sensor_data);
    return status;
}

int32 SLT_IFB_GET_NTC_DATA(int16* NTC_sensor_data)
{
    int32 status;
    status = CFE_SRL_ApiGetRparamCSP(GS_PARAM_INT16, PAY_IFB_NODE, TELEMETRY_TABLE, NTC_DATA_ADDRESS, NTC_sensor_data);
    return status;
}

/*------------------------------------------------------------------------------------------------------------------------------------------------*/
//Table save
int32 SLT_IFB_SAVE_TABLE0(void)
{
    int32 status;
    status = CFE_SRL_ApiRparamSaveCSP(PAY_IFB_NODE, 1000, BOARD_PARAMETER_TABLE, BOARD_PARAMETER_TABLE);
    return status;
}

int32 SLT_IFB_SAVE_TABLE1(void)
{
    int32 status;
    status = CFE_SRL_ApiRparamSaveCSP(PAY_IFB_NODE, 1000, CONFIGURATION_PARAMETER_TABLE, CONFIGURATION_PARAMETER_TABLE);
    return status;
}

int32 SLT_IFB_SAVE_TABLE4(void)
{
    int32 status;
    status = CFE_SRL_ApiRparamSaveCSP(PAY_IFB_NODE, 1000, TELEMETRY_TABLE, TELEMETRY_TABLE);
    return status;
}

int32 SLT_IFB_SAVE_ALL_TABLE(void)
{
    int32 status = 0;
    status |= SLT_IFB_SAVE_TABLE0();
    if (status != 0){
        return 1;
    }
    status |= SLT_IFB_SAVE_TABLE1();
    if (status != 0){
        return 2;
    }
    status |= SLT_IFB_SAVE_TABLE4();
    if (status != 0){
        return 5;
    }
    return status;
}
