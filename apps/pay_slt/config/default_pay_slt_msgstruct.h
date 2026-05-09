/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#ifndef SLT_IFB_MSGSTRUCT_H
#define SLT_IFB_MSGSTRUCT_H

#include "pay_slt_mission_cfg.h"
#include "pay_slt_msgdefs.h"
#include "cfe_msg_hdr.h"
#include "rpt_interface_cfg.h"

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
} PAY_SLT_NoArgsCmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint8 Arg;
} PAY_SLT_U8Cmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16 Arg;
} PAY_SLT_U16Cmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint32 Arg;
} PAY_SLT_U32Cmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_CommandHeader_t CommandHeader;
    char Arg[PAY_SLT_RTABLE_STR_SIZE];
} PAY_SLT_StrCmd_t;

typedef PAY_SLT_NoArgsCmd_t SLT_IFB_NoopCmd_t;
typedef PAY_SLT_NoArgsCmd_t SLT_IFB_ResetCountersCmd_t;
typedef PAY_SLT_NoArgsCmd_t SLT_IFB_SendHkCmd_t;
typedef PAY_SLT_NoArgsCmd_t SLT_IFB_SendBcnCmd_t;

typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_CMP_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_PING_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_PS_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_MEM_FREE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_REBOOT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_BUF_FREE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_UPTIME_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_CSP_GNDWDT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_BRD_UID_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_BRD_REV_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_CSP_ADDR_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_CAN_SPEED_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_I2C_ADDR_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_I2C_SPEED_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_WDT_VAL_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_CSP_RTABLE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_SYS_STATUS_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_SYS_UPTIME_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_BOOT_CNT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_BOOT_CAUSE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_REBOOT_CAUSE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_WDT_LEFT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_BRD_TEMP_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_PWR_CURRENT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_IMU_DATA_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_GET_NTC_DATA_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_SAVE_TABLE0_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_SAVE_TABLE1_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_SAVE_TABLE4_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_IFB_SAVE_ALL_TABLE_Cmd_t;
typedef PAY_SLT_U8Cmd_t PAY_SLT_IFB_SET_CSP_ADDR_Cmd_t;
typedef PAY_SLT_U16Cmd_t PAY_SLT_IFB_SET_CAN_SPEED_Cmd_t;
typedef PAY_SLT_U8Cmd_t PAY_SLT_IFB_SET_I2C_ADDR_Cmd_t;
typedef PAY_SLT_U16Cmd_t PAY_SLT_IFB_SET_I2C_SPEED_Cmd_t;
typedef PAY_SLT_U32Cmd_t PAY_SLT_IFB_SET_WDT_VAL_Cmd_t;
typedef PAY_SLT_StrCmd_t PAY_SLT_IFB_SET_CSP_RTABLE_Cmd_t;

typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_CSP_CMP_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_CSP_PING_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_CSP_PS_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_CSP_MEM_FREE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_CSP_REBOOT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_CSP_BUF_FREE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_CSP_UPTIME_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_CSP_GNDWDT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_BRD_UID_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_BRD_REV_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_CSP_ADDR_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_CAN_SPEED_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_I2C_ADDR_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_I2C_SPEED_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_WDT_VAL_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_CSP_RTABLE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_SYS_STATUS_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_SYS_UPTIME_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_BOOT_CNT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_BOOT_CAUSE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_REBOOT_CAUSE_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_WDT_LEFT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_BRD_TEMP_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_PWR_CURRENT_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_IMU_DATA_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_GET_NTC_DATA_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_SAVE_TABLE0_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_SAVE_TABLE1_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_SAVE_TABLE4_Cmd_t;
typedef PAY_SLT_NoArgsCmd_t PAY_SLT_EXP_SAVE_ALL_TABLE_Cmd_t;
typedef PAY_SLT_U8Cmd_t PAY_SLT_EXP_SET_CSP_ADDR_Cmd_t;
typedef PAY_SLT_U16Cmd_t PAY_SLT_EXP_SET_CAN_SPEED_Cmd_t;
typedef PAY_SLT_U8Cmd_t PAY_SLT_EXP_SET_I2C_ADDR_Cmd_t;
typedef PAY_SLT_U16Cmd_t PAY_SLT_EXP_SET_I2C_SPEED_Cmd_t;
typedef PAY_SLT_U32Cmd_t PAY_SLT_EXP_SET_WDT_VAL_Cmd_t;
typedef PAY_SLT_StrCmd_t PAY_SLT_EXP_SET_CSP_RTABLE_Cmd_t;
typedef PAY_SLT_U32Cmd_t PAY_SLT_EXP_I2C_READ_CHUNK_Cmd_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SLT_IFB_HkTlm_Payload_t Payload;
} SLT_IFB_HkTlm_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    SLT_IFB_BcnTlm_Payload_t Payload;
} SLT_IFB_BcnTlm_t;

typedef struct __attribute__((__packed__))
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} SLT_IFB_RPT_t;

#endif
