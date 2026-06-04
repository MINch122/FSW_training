#ifndef SLT_PAY_H
#define SLT_PAY_H

#include "cfe.h"

int32 SLT_IFB_CSP_CMP(void);
int32 SLT_IFB_CSP_PING(void);
int32 SLT_IFB_CSP_PS(void);
int32 SLT_IFB_CSP_MEM_FREE(void);
int32 SLT_IFB_CSP_REBOOT(void);
int32 SLT_IFB_CSP_BUF_FREE(void);
int32 SLT_IFB_CSP_UPTIME(void);
int32 SLT_IFB_CSP_GNDWDT(void);

int32 SLT_IFB_GET_BRD_UID(char* Board_uid);
int32 SLT_IFB_GET_BRD_REV(uint8* Board_revision);
int32 SLT_IFB_GET_CSP_ADDR(uint8 *MPU_CSP_address);
int32 SLT_IFB_GET_CAN_SPEED(uint16*CAN_bus_speed);
int32 SLT_IFB_GET_I2C_ADDR(uint8 *I2C_address);
int32 SLT_IFB_GET_I2C_SPEED(uint16* I2C_speed);
int32 SLT_IFB_GET_WDT_VAL(uint32* WDT_value);
int32 SLT_IFB_GET_CSP_RTABLE(char* MPU_CSP_routing_table);

int32 SLT_IFB_GET_SYS_STATUS(int16 *System_status);
int32 SLT_IFB_GET_SYS_UPTIME(uint32 *System_uptime);
int32 SLT_IFB_GET_BOOT_CNT(uint16 *System_boot_count);
int32 SLT_IFB_GET_BOOT_CAUSE(uint16 *Last_boot_cause);
int32 SLT_IFB_GET_REBOOT_CAUSE(uint16 *Last_reboot_cause);
int32 SLT_IFB_GET_WDT_LEFT(uint32 *Time_left_WDT_cause_reboot);
int32 SLT_IFB_GET_BRD_TEMP(int16 *Board_temperature);
int32 SLT_IFB_GET_PWR_CURRENT(uint16 *System_power_current);
int32 SLT_IFB_GET_IMU_DATA(uint16* IMU_sensor_data);
int32 SLT_IFB_GET_NTC_DATA(int16* NTC_sensor_data);

int32 SLT_IFB_SAVE_TABLE0(void);
int32 SLT_IFB_SAVE_TABLE1(void);
int32 SLT_IFB_SAVE_TABLE4(void);
int32 SLT_IFB_SAVE_ALL_TABLE(void);

#endif