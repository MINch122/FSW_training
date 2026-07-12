/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 ************************************************************************/

#include <string.h>
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
#define PAY_SLT_DOWNLOAD_FLUSH_INTERVAL 20U

/* template */
typedef struct
{
    uint8 Node;
    uint8 BoardTable;
    uint8 TlmTable;
    uint16 AddrBrdUid;
    uint16 AddrBrdRev;
    uint16 AddrCspAddrMpu;
    uint16 AddrCspAddrMcu;
    uint16 AddrCanSpeed;
    uint16 AddrI2cAddr;
    uint16 AddrI2cSpeed;
    uint16 AddrRs422Speed;
    uint16 AddrWdtVal;
    uint16 AddrCspRtable;
    uint16 AddrSysStatus;
    uint16 AddrSysUptime;
    uint16 AddrBootCnt;
    uint16 AddrBootCause;
    uint16 AddrRebootCause;
    uint16 AddrWdtLeft;
    uint16 AddrBrdTemp;
    uint16 AddrPwrCurrent;
    uint16 AddrSlfData;
    uint16 AddrBrmData;
    uint16 AddrImuData;
    uint16 AddrNtcData;
    uint16 AddrPwrVolt;
} PAY_SLT_TargetConfig_t;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UPDATE_IFB_CACHE:                                                          */
/*    Update the latest status of the InterFace Board to computer's memory    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static void PAY_SLT_UpdateIfbCache(void) {
    // SLT-IFB data (node 13)
    // [ICD 0x0000] sys_status (INT16, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0000, 1, &SLT_IFB_Data.sys_status);
    // [ICD 0x0002] sys_uptime (UINT32, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0002, 1, &SLT_IFB_Data.sys_uptime);
    // [ICD 0x0006] sys_now (UINT32, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0006, 1, &SLT_IFB_Data.sys_now);
    // [ICD 0x000A] boot_cnt (UINT16, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x000A, 1, &SLT_IFB_Data.boot_cnt);
    // [ICD 0x000C] boot_his (UINT8, 길이 8)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x000C, 8, SLT_IFB_Data.boot_his);
}

CFE_Status_t SLT_IFB_SendHkCmd(const SLT_IFB_SendHkCmd_t *Msg) {
    (void)Msg;
    SLT_IFB_Data.CmdCounter++;

    SLT_IFB_HkTlm_Payload_t *HkPkt = &SLT_IFB_Data.HkTlm.Payload;

    PAY_SLT_UpdateIfbCache();
    
    HkPkt->sys_status = SLT_IFB_Data.sys_status;
    HkPkt->sys_uptime = SLT_IFB_Data.sys_uptime;
    HkPkt->boot_cnt = SLT_IFB_Data.boot_cnt;
    HkPkt->BcnSbEnabled = SLT_IFB_Data.BcnSbEnabled ? 1U : 0U;
    memcpy(HkPkt->boot_his, SLT_IFB_Data.boot_his, sizeof(SLT_IFB_Data.boot_his));

    // [ICD 0x0014] wdt_left (UINT32, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0014, 1, &HkPkt->wdt_left);
    // [ICD 0x002D] att_q (FLOAT, 길이 4)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x002D, 4, HkPkt->att_q);
    // [ICD 0x003D] rot_r (FLOAT, 길이 3)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x003D, 3, HkPkt->rot_r);
    // [ICD 0x0049] lin_acc (FLOAT, 길이 3)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0049, 3, HkPkt->lin_acc);
    // [ICD 0x0055] fld_vec (FLOAT, 길이 3)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0055, 3, HkPkt->fld_vec);

    // [ICD 0x0018] brd_temp (INT16, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0018, 1, &HkPkt->brd_temp);
    // [ICD 0x001A] ntc_data (INT16, 길이 4)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_INT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x001A, 4, HkPkt->ntc_data);
    // [ICD 0x0022] pw_cur (UINT16, 길이 2)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0022, 2, HkPkt->pw_cur);
    // [ICD 0x0026] pw_vol (UINT16, 길이 2)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0026, 2, HkPkt->pw_vol);
    // [ICD 0x0061] sen_rst (UINT16, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0061, 1, &HkPkt->sen_rst);

    // [ICD 0x002A] sen_online (UINT8, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x002A, 1, &HkPkt->sen_online);
    // [ICD 0x002B] sen_qlvl (UINT8, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x002B, 1, &HkPkt->sen_qlvl);
    // [ICD 0x002C] att_ql (UINT8, 길이 1)
    (void)PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x002C, 1, &HkPkt->att_ql);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(SLT_IFB_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(SLT_IFB_Data.HkTlm.TelemetryHeader), true);

    return CFE_SUCCESS;
}

CFE_Status_t PAY_SLT_SendBeaconCmd(const SLT_IFB_SendBcnCmd_t *Msg) {
    (void)Msg;
    
    if (SLT_IFB_Data.BcnSbEnabled)
    {
        
    SLT_IFB_Data.CmdCounter++;
    SLT_IFB_BcnTlm_Payload_t *BcnPkt = &SLT_IFB_Data.BcnTlm.Payload;

    PAY_SLT_UpdateIfbCache();

    BcnPkt->CmdCounter = SLT_IFB_Data.CmdCounter;
    BcnPkt->ErrCounter = SLT_IFB_Data.ErrCounter;

    // PAY-EXP-A7
    // System uptime in sec (U32, Addr: 0x0000)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0000, 1, &BcnPkt->sys_uptime_a7);
    // System current time (U32, Addr: 0x0004)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0004, 1, &BcnPkt->sys_now_a7);
    // System boot count - MPU (U16, Addr: 0x0008)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0008, 1, &BcnPkt->boot_cnt_p);
    // System boot cause code - MPU (U8 Array[8], Addr: 0x000A)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8,  PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x000A, 8, BcnPkt->boot_his_p);
    // System boot count - MCU (U16, Addr: 0x0012)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0012, 1, &BcnPkt->boot_cnt_c);
    // System boot cause code - MCU (U8 Array[8], Addr: 0x0014)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8,  PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0014, 8, BcnPkt->boot_his_c);
    // Time left before WDT causes reboot (U32, Addr: 0x001C)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x001C, 1, &BcnPkt->wdt_left_a7);
    // Board temperature (I16, Addr: 0x0020)
    PAY_SLT_FetchParam_Simple(GS_PARAM_INT16,  PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0020, 1, &BcnPkt->brd_temp_a7);
    // SLF sensor data (U16 Array[4], Addr: 0x0022)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0022, 4, BcnPkt->slf_data);
    // Barometric sensor data (U16 Array[3], Addr: 0x002A)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x002A, 3, BcnPkt->brm_data);
    // IMU sensor data (U16 Array[8], Addr: 0x0030)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0030, 8, BcnPkt->imu_data);
    // NTC sensor data (I16 Array[8], Addr: 0x0040)
    PAY_SLT_FetchParam_Simple(GS_PARAM_INT16,  PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0040, 8, BcnPkt->ntc_data_a7);
    // System power voltage measures (U16 Array[8], Addr: 0x0050)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0050, 8, BcnPkt->pwr_volt);
    // System power current measures (U16 Array[8], Addr: 0x0060)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0060, 8, BcnPkt->pwr_current);
    // System status (I16, Addr: 0x0070)
    PAY_SLT_FetchParam_Simple(GS_PARAM_INT16,  PAY_SLT_EXP_A7_NODE, TABLE_TELEMETRY, 0x0070, 1, &BcnPkt->sys_status_a7);

    // PAY-IFB 
    // System status (I16, Addr: 0x0000)
    BcnPkt->sys_status_ifb = SLT_IFB_Data.sys_status;
    // System uptime in sec (U32, Addr: 0x0002)
    BcnPkt->sys_uptime_ifb = SLT_IFB_Data.sys_uptime;
    // System boot count (U16, Addr: 0x000A)
    BcnPkt->boot_cnt = SLT_IFB_Data.boot_cnt;
    // System boot cause code (U8 Array[8], Addr: 0x000C)
    memcpy(BcnPkt->boot_his, SLT_IFB_Data.boot_his, sizeof(SLT_IFB_Data.boot_his));
    // System current time (Unix epoch, sec) (U32, Addr: 0x0006)
    BcnPkt->sys_now_ifb = SLT_IFB_Data.sys_now;
    // Time left before WDT causes reboot (U32, Addr: 0x0014)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0014, 1, &BcnPkt->wdt_left_ifb);
    // Board temperature (val/10 °C) (I16, Addr: 0x0018)
    PAY_SLT_FetchParam_Simple(GS_PARAM_INT16,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0018, 1, &BcnPkt->brd_temp_ifb);
    // NTC sensor data (val/10 °C) (I16 Array[4], Addr: 0x001A)
    PAY_SLT_FetchParam_Simple(GS_PARAM_INT16,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x001A, 4, BcnPkt->ntc_data_ifb);
    // System power current measures in mA (U16 Array[2], Addr: 0x0022)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0022, 2, BcnPkt->pw_cur);
    // System power voltage measures in mV (U16 Array[2], Addr: 0x0026)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0026, 2, BcnPkt->pw_vol);
    // Sensor data: sen_online (U8, Addr: 0x002A)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x002A, 1, &BcnPkt->sen_online);
    // Sensor data: sen_qlvl (U8, Addr: 0x002B)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x002B, 1, &BcnPkt->sen_qlvl);
    // Sensor data: att_ql (U8, Addr: 0x002C)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT8,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x002C, 1, &BcnPkt->att_ql);
    // Sensor data: att_q (F32 Array[4], Addr: 0x002D)
    PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x002D, 4, BcnPkt->att_q);
    // Sensor data: rot_r (F32 Array[3], Addr: 0x003D)
    PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x003D, 3, BcnPkt->rot_r);
    // Sensor data: lin_acc (F32 Array[3], Addr: 0x0049)
    PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0049, 3, BcnPkt->lin_acc);
    // Sensor data: fld_vec (F32 Array[3], Addr: 0x0055)
    PAY_SLT_FetchParam_Simple(GS_PARAM_FLOAT,  PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0055, 3, BcnPkt->fld_vec);
    // Sensor data: sen_rst (U16, Addr: 0x0061)
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT16, PAY_SLT_IFB_NODE, TABLE_TELEMETRY, 0x0061, 1, &BcnPkt->sen_rst);


    // debug
    // -------------------------------------------------------------------------
    // PAY_SLT_APP_printf: PAY-EXP-A7 Telemetry Data Check
    // -------------------------------------------------------------------------
    PAY_SLT_APP_printf("Bcn: ----- PAY-EXP-A7 Telemetry -----\n");
    // 단일 변수 (부호 없는 정수는 %u)
    PAY_SLT_APP_printf("Bcn: sys_uptime_a7: %u\n", BcnPkt->sys_uptime_a7);
    PAY_SLT_APP_printf("Bcn: sys_now_a7: %u\n", BcnPkt->sys_now_a7);
    PAY_SLT_APP_printf("Bcn: boot_cnt_p: %u\n", BcnPkt->boot_cnt_p);
    // 배열 [8]
    PAY_SLT_APP_printf("Bcn: boot_his_p: [%u, %u, %u, %u, %u, %u, %u, %u]\n", 
            BcnPkt->boot_his_p[0], BcnPkt->boot_his_p[1], BcnPkt->boot_his_p[2], BcnPkt->boot_his_p[3], 
            BcnPkt->boot_his_p[4], BcnPkt->boot_his_p[5], BcnPkt->boot_his_p[6], BcnPkt->boot_his_p[7]);
    PAY_SLT_APP_printf("Bcn: boot_cnt_c: %u\n", BcnPkt->boot_cnt_c);
    // 배열 [8]
    PAY_SLT_APP_printf("Bcn: boot_his_c: [%u, %u, %u, %u, %u, %u, %u, %u]\n", 
            BcnPkt->boot_his_c[0], BcnPkt->boot_his_c[1], BcnPkt->boot_his_c[2], BcnPkt->boot_his_c[3], 
            BcnPkt->boot_his_c[4], BcnPkt->boot_his_c[5], BcnPkt->boot_his_c[6], BcnPkt->boot_his_c[7]);
    PAY_SLT_APP_printf("Bcn: wdt_left_a7: %u\n", BcnPkt->wdt_left_a7);
    // 단일 변수 (부호 있는 정수 I16은 %d)
    PAY_SLT_APP_printf("Bcn: brd_temp_a7: %d\n", BcnPkt->brd_temp_a7);
    // 배열 [4]
    PAY_SLT_APP_printf("Bcn: slf_data: [%u, %u, %u, %u]\n", 
           BcnPkt->slf_data[0], BcnPkt->slf_data[1], BcnPkt->slf_data[2], BcnPkt->slf_data[3]);
    // 배열 [3]
    PAY_SLT_APP_printf("Bcn: brm_data: [%u, %u, %u]\n", 
            BcnPkt->brm_data[0], BcnPkt->brm_data[1], BcnPkt->brm_data[2]);
    // 배열 [8]
    PAY_SLT_APP_printf("Bcn: imu_data: [%u, %u, %u, %u, %u, %u, %u, %u]\n", 
            BcnPkt->imu_data[0], BcnPkt->imu_data[1], BcnPkt->imu_data[2], BcnPkt->imu_data[3], 
            BcnPkt->imu_data[4], BcnPkt->imu_data[5], BcnPkt->imu_data[6], BcnPkt->imu_data[7]);
    // 부호 있는 I16 배열 [8] 이므로 %d 사용
    PAY_SLT_APP_printf("Bcn: ntc_data_a7: [%d, %d, %d, %d, %d, %d, %d, %d]\n", 
            BcnPkt->ntc_data_a7[0], BcnPkt->ntc_data_a7[1], BcnPkt->ntc_data_a7[2], BcnPkt->ntc_data_a7[3], 
            BcnPkt->ntc_data_a7[4], BcnPkt->ntc_data_a7[5], BcnPkt->ntc_data_a7[6], BcnPkt->ntc_data_a7[7]);
    // 배열 [8]
    PAY_SLT_APP_printf("Bcn: pwr_volt: [%u, %u, %u, %u, %u, %u, %u, %u]\n", 
            BcnPkt->pwr_volt[0], BcnPkt->pwr_volt[1], BcnPkt->pwr_volt[2], BcnPkt->pwr_volt[3], 
            BcnPkt->pwr_volt[4], BcnPkt->pwr_volt[5], BcnPkt->pwr_volt[6], BcnPkt->pwr_volt[7]);
    // 배열 [8]
    PAY_SLT_APP_printf("Bcn: pwr_current: [%u, %u, %u, %u, %u, %u, %u, %u]\n", 
            BcnPkt->pwr_current[0], BcnPkt->pwr_current[1], BcnPkt->pwr_current[2], BcnPkt->pwr_current[3], 
            BcnPkt->pwr_current[4], BcnPkt->pwr_current[5], BcnPkt->pwr_current[6], BcnPkt->pwr_current[7]);
    // 단일 변수 (부호 있는 정수 I16)
    PAY_SLT_APP_printf("Bcn: sys_status_a7: %d\n", BcnPkt->sys_status_a7);
    
    PAY_SLT_APP_printf("Bcn: att_q [%f, %f, %f, %f]\n", BcnPkt->att_q[0],BcnPkt->att_q[1],BcnPkt->att_q[2],BcnPkt->att_q[3]);

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(SLT_IFB_Data.BcnTlm.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(SLT_IFB_Data.BcnTlm.TelemetryHeader), true);
    }

    return CFE_SUCCESS;
} 

/* command */

/* No-op command */
CFE_Status_t SLT_IFB_NoopCmd(const SLT_IFB_NoopCmd_t *Msg)
{
    (void)Msg;
    (void)PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_NOOP_CC, false, NULL, 0);
    CFE_EVS_SendEvent(SLT_IFB_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "PAY_SLT: NOOP command %s",
                      SLT_IFB_VERSION);
    return CFE_SUCCESS;
}

/* Reset Counters */
CFE_Status_t SLT_IFB_ResetCountersCmd(const SLT_IFB_ResetCountersCmd_t *Msg)
{
    (void)Msg;
    SLT_IFB_Data.CmdCounter = 0;
    SLT_IFB_Data.ErrCounter = 0;
    SLT_IFB_Data.AppErrCounter = 0;
    SLT_IFB_Data.DeviceErrCounter = 0;
    (void)PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_RESET_COUNTERS_CC, false, NULL, 0);
    return CFE_SUCCESS;
}

CFE_Status_t PAY_SLT_SetBcnEnabledCmd(const PAY_SLT_SetBcnEnabledCmd_t *Msg)
{
    uint8 enabled;

    if (Msg->Arg > 1U)
    {
        enabled = SLT_IFB_Data.BcnSbEnabled ? 1U : 0U;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: SET_BCN_ENABLED rejected, Arg must be 0 or 1");
        return PAY_SLT_HandleReport(CFE_ES_BAD_ARGUMENT, PAY_SLT_SET_BCN_ENABLED_CC, false,
                                    &enabled, sizeof(enabled));
    }

    SLT_IFB_Data.BcnSbEnabled = (Msg->Arg != 0U);
    enabled = SLT_IFB_Data.BcnSbEnabled ? 1U : 0U;

    CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAY_SLT: SB BCN %s", SLT_IFB_Data.BcnSbEnabled ? "enabled" : "disabled");

    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_SET_BCN_ENABLED_CC, false, &enabled, sizeof(enabled));
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
    SLT_IFB_Data.CmdCounter ++;

    PAY_SLT_ParSet_Payload_t Payload = Msg->Payload;

    int32 Status = PAY_SLT_SetParam(Payload.node, Payload.table, Payload.addr, Payload.type, &Payload.value);

    return PAY_SLT_HandleReport(Status, PAY_SLT_PAR_SET_CC, true, NULL, 0);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* 새로 만들어 보아요                                                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Get Parameter                                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_ParGetCmd(const PAY_SLT_ParGetCmd_t *Msg) {
    SLT_IFB_Data.CmdCounter ++;

    static PAY_SLT_Params_t ReqPayload;
    memset(&ReqPayload, 0, sizeof(PAY_SLT_Params_t));

    ReqPayload.node = Msg->Payload.node;
    ReqPayload.table = Msg->Payload.table;
    ReqPayload.addr = Msg->Payload.addr;
    ReqPayload.type = Msg->Payload.type;
    ReqPayload.len = Msg->Payload.len;


    PAY_SLT_APP_printf("Before\n");
    PAY_SLT_APP_printf("type: %u\n", ReqPayload.type);
    PAY_SLT_APP_printf("len: %u\n", ReqPayload.len);

    int32 Status = PAY_SLT_FetchParam(&ReqPayload);

    PAY_SLT_APP_printf("After\n");
    PAY_SLT_APP_printf("type: %u\n", ReqPayload.type);
    PAY_SLT_APP_printf("len: %u\n", ReqPayload.len);

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
                }
                else {
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
                }
                else {
                    snprintf(ValBuf, sizeof(ValBuf), "%u", ReqPayload.param.u16[0]);
                }
                break;
            case GS_PARAM_UINT32:
                snprintf(ValBuf, sizeof(ValBuf), "%u", (unsigned int)ReqPayload.param.u32[0]);
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
                }
                else {
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
                }
                else {
                    snprintf(ValBuf, sizeof(ValBuf), "%f", ReqPayload.param.flt[0]);
                }
                break;

            case GS_PARAM_STRING:
                // null 삽입
                if (ReqPayload.len < sizeof(ReqPayload.param.str))
                {
                    ReqPayload.param.str[ReqPayload.len] = '\0';
                }
                else
                {
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
        SLT_IFB_Data.ErrCounter++;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "PAR_GET: CSP transaction failed: %d", (int)Status);
    }

    // 지상으로 쏠 때는 ReqPayload 잘라서 보냄
    uint16 elements_size = 0;
    switch (ReqPayload.type) {
        case GS_PARAM_UINT8:   elements_size = 1;  break;
        case GS_PARAM_UINT16:  elements_size = 2;  break;
        case GS_PARAM_UINT32:  elements_size = 4;  break;
        case GS_PARAM_INT16:   elements_size = 2;  break;
        case GS_PARAM_STRING:  elements_size = 1;  break;
        default:                   elements_size = 0;  break;
    }

    uint16 total_size = 8 + ReqPayload.len * elements_size; // 헤더 8 byte(node, table, addr, type, len, padding) + 요소 개수 * 개당 byte

    PAY_SLT_HandleReport(Status, PAY_SLT_PAR_GET_CC, true, &ReqPayload, total_size);

    return CFE_SUCCESS;

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Get Full Table:                                                            */
/*   Get all params of the table                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_GetFullTableCmd(const PAY_SLT_GetFullTableCmd_t *Msg) {
    SLT_IFB_Data.CmdCounter ++;

    gs_param_table_instance_t tinst = {0};
    gs_error_t err = PAY_SLT_GetFullTable(Msg->Payload.node,
                                               Msg->Payload.table,
                                               &tinst, 1000); // 1000ms timeout
    if (err != GS_OK) {
        SLT_IFB_Data.ErrCounter++;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAY_SLT: RParam Get Full Table command failed, err=%d", err);
        PAY_SLT_APP_printf("[PAY-SLT] RParam Get Full Table FAILED: node=%u table=%u err=%d\n", Msg->Payload.node, Msg->Payload.table, err);
        return PAY_SLT_HandleReport(CFE_STATUS_EXTERNAL_RESOURCE_FAIL, PAY_SLT_GET_FULL_TABLE_CC, false, NULL, 0);
    }

    PAY_SLT_PrintParamTable(Msg->Payload.node, Msg->Payload.table, &tinst);
    gs_param_table_free(&tinst);

    return PAY_SLT_HandleReport(CFE_SUCCESS, PAY_SLT_GET_FULL_TABLE_CC, false, NULL, 0);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* File scan                                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAY_SLT_ScanFilesCmd(const PAY_SLT_ScanFilesCmd_t *Msg) {
    SLT_IFB_Data.CmdCounter ++;

    uint32 scan_val =1;

    uint8 node = Msg->Payload.node;

    if (node != 11 && node != 12) {     // PAY-EXP의 node가 아니먄(11, 12) 에러 처리
        SLT_IFB_Data.ErrCounter++;
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Node should be 11 or 12. Received node: %d", node);
        return CFE_SUCCESS;
    }

    // [Table3 0x0000] ft_scan set 1
    // 일단 node 11만 넣도록 했음. 나중에 수정 필요
    int32 Status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0000, GS_PARAM_UINT32, &scan_val);

    if (Status >= 0) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "Payload File Scan Success. scanned node: %d", node);
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Payload File Scan Failed. Status: 0x%06X", Status);
        return PAY_SLT_HandleReport(Status, PAY_SLT_SCAN_FILES_CC, true, NULL, 0);
    }

    OS_TaskDelay(1000); // scan & table update 대기 1초

    PAY_SLT_ScanFileRpl_t  Reply;

    // ft_snap_id: uint32, table3, 0x0004
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0004, 1, &Reply.snap_id);
    // ft_file_ccount: uint32, table3, 0x0008
    int32 Status1 = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0008, 1, &Reply.file_count);

    if (Status >= 0 && Status1 >= 0) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "File Scan: snap_id=%u, file count=%u", Reply.snap_id, Reply.file_count);
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Scan Params Read failed. Status: 0x%06X(snap_id), 0x%06X(file_count)", Status, Status1);
    }

    int32 final_status = (Status >= 0 && Status1 >= 0) ? CFE_SUCCESS : -1;

    return PAY_SLT_HandleReport(final_status, PAY_SLT_SCAN_FILES_CC, true, &Reply, sizeof(Reply));
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Scan File Download                                                         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

// download file index
static uint32 PAY_SLT_TargetFileIndex = 0;
static uint32 PAY_SLT_DownloadStartChunk = 0;
static uint32 PAY_SLT_DownloadTaskID;


void PAY_SLT_DownloadChildTask(void) {
    int32 Status;
    uint32 file_npart = 0;
    uint32 part_ready = 0;
    uint32 start_chunk = PAY_SLT_DownloadStartChunk;
    int32  file_fd = -1;
    uint8 chunk_buffer[1024]; // default: 1024바이트 조각 버퍼. 일단 크게 받고 이후에 chunk_size만큼 잘라쓰기

    // 0. Chunk size(ft_chunk_size, addr 0x005C)
    uint32 chunk_size = 0;
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x005C, 1, &chunk_size);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: Get RParam(ft_chunk_size) failed. Status: 0x%06X", Status);
        goto TASK_EXIT;
    }
    // chunk size overflow 방진
    if (Status != CFE_SUCCESS || chunk_size > sizeof(chunk_buffer)) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid chunk size (%d) or Fetch failed.", chunk_size);
        goto TASK_EXIT;
    }

    // 1. Load file at <index> (ft_file_load, addr 0x000C)
    Status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x000C, GS_PARAM_UINT32, &PAY_SLT_TargetFileIndex);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: Set RParam(ft_file_load) failed. Status: 0x%06X", Status);
        goto TASK_EXIT;
    }

    // 탑재체가 파일을 로드할 시간을 잠깐
    OS_TaskDelay(100); 

    // 2. 총 조각 수(ft_file_npart, addr 0x0054) 읽기 (FetchParam 사용)
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0054, 1, &file_npart);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: Get RParam(ft_file_npart) failed. Status: 0x%06X", Status);
        goto TASK_EXIT;
    }

    if (start_chunk >= file_npart) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "DownloadChildTask: start_chunk %u is out of range (file_npart=%u)",
                          (unsigned int)start_chunk, (unsigned int)file_npart);
        goto TASK_EXIT;
    }

    // 3. OBC 로컬 파일 시스템에 빈 파일 열기 (OSAL API)
    // 파일명: /cf/payload_data.bin? ft_file_name(str, 0x0010)?
    char ft_file_name[64] = {0};
    Status = PAY_SLT_FetchParam_Simple(GS_PARAM_STRING, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0010, 64, ft_file_name);
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "/cf/%s", ft_file_name); // /cf/<filename> 경로에 파일 생성

    // debug
    PAY_SLT_APP_printf("[PAY-SLT] DownloadChildTask: Target file path: %s\n", filepath);

    uint32 open_flags = OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE;
    if (start_chunk != 0U) {
        open_flags = OS_FILE_FLAG_CREATE | OS_FILE_FLAG_APPEND;
    }

    Status = OS_OpenCreate(&file_fd, filepath, open_flags, OS_READ_WRITE);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "DownloadChildTask: OpenCreate File failed. Status: 0x%06X", Status);
        goto TASK_EXIT;
    }

    // 전체 파일 CRC 누적용 초기값
    uint32 total_crc = 0xFFFFFFFF;

    // 4. 조각(Chunk) 무한 다운로드 루프
    for (uint32 i = start_chunk; i < file_npart; i++) 
    {
        uint8 retry_count = 0; // 다운로드 반복 횟수

CHUNK_RETRY:
        if (retry_count > 3) {
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Max retries reached at chunk %d. Aborting.", i);
                goto FILE_CLEANUP; // 3번 연속 실패하면 다운로드 중단
        }

        // 4-1. Load part/chunk at <index> (ft_part_load, addr 0x0060)
        Status = PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0060, GS_PARAM_UINT32, &i);
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
            // ft_part_ready(0x006C): 0 = loading, 1 = ready, 2 = error
            Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x006C, 1, &part_ready);
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

        if (part_ready == 2) {
            retry_count ++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Part ready Error at chunk %d. Download Retry", i);
            goto CHUNK_RETRY; // error 발생시 다운로드 재시도
        }

        // 4-3. I2C 데이터 읽기 (이 부분은 하드웨어 통신 API 사용)
        uint32 data_buffer_addr = 0x00000000; // ICD: Memory address is start from 0x00000000
        Status = PAY_SLT_ReadExpI2CChunk(SLT_IFB_Data.I2c1Handle, data_buffer_addr, chunk_buffer, chunk_size);

        PAY_SLT_APP_printf("Chunk Data (Chunk %d): ", i);
        for (uint32 j = 0; j < chunk_size; j++) {
            PAY_SLT_APP_printf("%02X ", chunk_buffer[j]);
        }
        PAY_SLT_APP_printf("\n");

        if (Status != CFE_SUCCESS) {
            retry_count ++;
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "I2C Read Error at chunk %d. Download Retry", i);
            goto CHUNK_RETRY; // I2C 통신 끊기면 다운로드 재시도
        }

        OS_TaskDelay(100); // I2C 통신 안정화 대기 3초

        // 4-4. CRC check - chunk data

        uint32 chunk_crc = PAY_SLT_CalculateCRC32(chunk_buffer, chunk_size);
        // ft_part_crc32, addr 0x0068
        uint32 ft_part_crc32 = 0;
        Status = PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0068, 1, &ft_part_crc32);
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
        Status = OS_write(file_fd, chunk_buffer, chunk_size);
        if (Status < 0) {
            CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DownloadChildTask: File write failed at chunk %u. Status: 0x%06X", (unsigned int)i, Status);
            goto FILE_CLEANUP;
        }

        // 4-6. 전체 CRC 변수에 현재 청크 데이터 누적 (청크 검증이 통과된 찐 데이터만 기록)
        total_crc = PAY_SLT_UpdateCRC32(total_crc, chunk_buffer, chunk_size);

        // 4-7. 20청크마다 파일을 닫기 & 다시 열기: append 상태를 유지하면서 중간 저장을 보장
        if (((i + 1U) % PAY_SLT_DOWNLOAD_FLUSH_INTERVAL) == 0U || ((i + 1U) == file_npart)) {
            if (file_fd >= 0) {
                OS_close(file_fd);
                file_fd = -1;
            }

            Status = OS_OpenCreate(&file_fd, filepath, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_APPEND, OS_READ_WRITE);
            if (Status != CFE_SUCCESS) {
                CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "DownloadChildTask: Reopen file for append failed at chunk %u. Status: 0x%06X",
                                  (unsigned int)i, Status);
                goto FILE_CLEANUP;
            }
        }

        // 터미널에서 진행률 확인
        if (i % 10 == 0) PAY_SLT_APP_printf("Downloading... %d / %d\n", i, file_npart);
    }

    // 5. CRC check - full data
    total_crc ^= 0xFFFFFFFF; // 다운로드가 끝났으므로 최종 반전(XOR Out)

    // ft_file_crc32, addr 0x0058
    uint32 ft_file_crc32 = 0;
    PAY_SLT_FetchParam_Simple(GS_PARAM_UINT32, PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0058, 1, &ft_file_crc32);

    if (total_crc != ft_file_crc32) {
        CFE_EVS_SendEvent(PAY_SLT_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Total CRC Mismatch. Calc: 0x%08X, Payload: 0x%08X", total_crc, ft_file_crc32);
        // 옵션: CRC가 틀렸으므로 방금 받은 파일을 삭제(OS_remove)하는 로직 추가?
    } else {
        CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "Total CRC Match Success! File securely downloaded.");
    }

FILE_CLEANUP:
    // 6. 파일 닫기
    if (file_fd >= 0) {
        OS_close(file_fd);
        file_fd = -1;
    }

    // 7. Release loaded file (ft_release, addr 0x0070)
    uint32 release_val = 1;
    PAY_SLT_SetParam(PAY_SLT_EXP_A7_NODE, TABLE_DATA_CONTROL_PARAM, 0x0070, GS_PARAM_UINT32, &release_val);

    CFE_EVS_SendEvent(PAY_SLT_CMD_INF_EID, CFE_EVS_EventType_INFORMATION, "Payload Data Download Complete.");

    // 스레드 종료
TASK_EXIT:
    CFE_ES_ExitChildTask();
}

CFE_Status_t PAY_SLT_DownloadFileCmd(const PAY_SLT_DownloadFileCmd_t *Msg) {
    SLT_IFB_Data.CmdCounter ++;

    PAY_SLT_TargetFileIndex = Msg->Payload.file_index;
    PAY_SLT_DownloadStartChunk = Msg->Payload.start_chunk;

    int32 Status = CFE_ES_CreateChildTask(
        &PAY_SLT_DownloadTaskID,
        "PAY_DL_TASK",            // 스레드 이름
        PAY_SLT_DownloadChildTask,// 실행할 스레드 함수 이름
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

    return PAY_SLT_HandleReport(Status, PAY_SLT_DOWNLOAD_FILE_CC, true, NULL, 0);
}
