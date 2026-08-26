#include "to_lab_utils.h"

#include "to_lab_app.h"
#include "to_lab_encode.h"
#include "to_lab_eventids.h"
#include "to_lab_msgids.h"
#include "to_lab_perfids.h"
#include "to_lab_version.h"
#include "to_lab_msg.h"
#include "to_lab_tbl.h"

#include "rpt_msgids.h"
#include "rpt_msg.h"
#include "eps_msgids.h"
#include "eps_msg.h"
#include "utrx_msg.h"
#include "ltrx_msg.h"
#include "gpio_msg.h"
#include "adcs_msg.h"
#include "pay_slt_msgids.h"
#include "pay_slt_msg.h"
#include "hk_msgids.h"
#include "fm_msgids.h"
#include "cfe_evs_msgids.h"

#include <stddef.h>
#include <string.h>

static const uint8 TO_LAB_HK_COMBINED_PKT1_RF_PREFIX[] = "BEE1012";
#define TO_LAB_HK_COMBINED_PKT1_RF_PREFIX_SIZE (sizeof(TO_LAB_HK_COMBINED_PKT1_RF_PREFIX) - 1)
#define TO_LAB_RF_MAX_AVAILABLE_BYTES 250
#define TO_LAB_HK_COMBINED_PKT1_PAYLOAD_OFFSET \
    (sizeof(CFE_MSG_TelemetryHeader_t) - sizeof(((CFE_MSG_TelemetryHeader_t *)0)->Spare))

static const uint8 TO_LAB_EPS_PDU_BCN_CHANNELS[EPS_PDU_BCN_USED_CH_COUNT] = EPS_PDU_BCN_USED_CH_LIST;

static const char *TO_LAB_OnOffString(uint8 Value)
{
    return (Value != 0U) ? "ON" : "OFF";
}

static const char *TO_LAB_BitOnOffString(uint8 Value, uint8 Bit)
{
    return ((Value & (uint8)(1U << Bit)) != 0U) ? "ON" : "OFF";
}

static const char *TO_LAB_BitHighLowString(uint8 Value, uint8 Bit)
{
    return ((Value & (uint8)(1U << Bit)) != 0U) ? "HIGH" : "LOW";
}

static const char *TO_LAB_AdcsControlModeName(uint8 Mode)
{
    switch (Mode)
    {
        case 0:
            return " (OFF)";
        case 1:
            return " (DETUMBLE)";
        case 2:
            return " (POINTING)";
        default:
            return "";
    }
}

static void TO_LAB_PrintOutgoing(const char *Path, const CFE_SB_Buffer_t *SBBufPtr, const void *NetBufPtr, size_t NetBufSize, int32 Status, uint16 Port)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_Size_t SourceSize = 0;
    const uint8 *Bytes = (const uint8 *)NetBufPtr;

    (void)CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
    (void)CFE_MSG_GetSize(&SBBufPtr->Msg, &SourceSize);

    TO_LAB_APP_printf("TO_LAB %s OUT: mid=0x%04X src_len=%lu net_len=%lu port=%u status=0x%08lX head=",
                      Path,
                      (unsigned int)CFE_SB_MsgIdToValue(MsgId),
                      (unsigned long)SourceSize,
                      (unsigned long)NetBufSize,
                      (unsigned int)Port,
                      (unsigned long)Status);

    for (size_t i = 0; i < NetBufSize && i < 16; i++)
    {
        TO_LAB_APP_printf("%02X", (unsigned int)Bytes[i]);
    }

    TO_LAB_APP_printf("\n");
}

static void TO_LAB_PrintBytes(const char *Label, const uint8 *Bytes, size_t Size, size_t MaxBytes)
{
    size_t Limit = (Size < MaxBytes) ? Size : MaxBytes;

    TO_LAB_APP_printf("%s", Label);
    for (size_t i = 0; i < Limit; i++)
    {
        TO_LAB_APP_printf("%02X", (unsigned int)Bytes[i]);
    }
    if (Size > Limit)
    {
        TO_LAB_APP_printf("...");
    }
    TO_LAB_APP_printf("\n");
}

static void TO_LAB_PrintU8Array(const char *Label, const uint8 *Values, size_t Count)
{
    TO_LAB_APP_printf("%s[", Label);
    for (size_t i = 0; i < Count; i++)
    {
        TO_LAB_APP_printf("%s%u", (i == 0) ? "" : ",", (unsigned int)Values[i]);
    }
    TO_LAB_APP_printf("]\n");
}

static void TO_LAB_PrintU16Array(const char *Label, const uint16 *Values, size_t Count)
{
    TO_LAB_APP_printf("%s[", Label);
    for (size_t i = 0; i < Count; i++)
    {
        TO_LAB_APP_printf("%s%u", (i == 0) ? "" : ",", (unsigned int)Values[i]);
    }
    TO_LAB_APP_printf("]\n");
}

static void TO_LAB_PrintI16Array(const char *Label, const int16 *Values, size_t Count)
{
    TO_LAB_APP_printf("%s[", Label);
    for (size_t i = 0; i < Count; i++)
    {
        TO_LAB_APP_printf("%s%d", (i == 0) ? "" : ",", (int)Values[i]);
    }
    TO_LAB_APP_printf("]\n");
}

static void TO_LAB_PrintFloatArray(const char *Label, const float *Values, size_t Count)
{
    TO_LAB_APP_printf("%s[", Label);
    for (size_t i = 0; i < Count; i++)
    {
        TO_LAB_APP_printf("%s%.6f", (i == 0) ? "" : ",", (double)Values[i]);
    }
    TO_LAB_APP_printf("]\n");
}

#define TO_LAB_PAY_SLT_ARRAY_COUNT(member) \
    (sizeof(((PAY_SLT_BcnTlm_Payload_t *)0)->member) / sizeof(((PAY_SLT_BcnTlm_Payload_t *)0)->member[0]))

#define TO_LAB_PRINT_PAY_SLT_U8_ARRAY(label, payload_ptr, member)                   \
    do                                                                             \
    {                                                                              \
        uint8 Values[TO_LAB_PAY_SLT_ARRAY_COUNT(member)];                          \
        memcpy(Values, &(payload_ptr)[offsetof(PAY_SLT_BcnTlm_Payload_t, member)], \
               sizeof(Values));                                                    \
        TO_LAB_PrintU8Array((label), Values, TO_LAB_PAY_SLT_ARRAY_COUNT(member));  \
    } while (0)

#define TO_LAB_PRINT_PAY_SLT_U16_ARRAY(label, payload_ptr, member)                  \
    do                                                                             \
    {                                                                              \
        uint16 Values[TO_LAB_PAY_SLT_ARRAY_COUNT(member)];                         \
        memcpy(Values, &(payload_ptr)[offsetof(PAY_SLT_BcnTlm_Payload_t, member)], \
               sizeof(Values));                                                    \
        TO_LAB_PrintU16Array((label), Values, TO_LAB_PAY_SLT_ARRAY_COUNT(member)); \
    } while (0)

#define TO_LAB_PRINT_PAY_SLT_I16_ARRAY(label, payload_ptr, member)                  \
    do                                                                             \
    {                                                                              \
        int16 Values[TO_LAB_PAY_SLT_ARRAY_COUNT(member)];                          \
        memcpy(Values, &(payload_ptr)[offsetof(PAY_SLT_BcnTlm_Payload_t, member)], \
               sizeof(Values));                                                    \
        TO_LAB_PrintI16Array((label), Values, TO_LAB_PAY_SLT_ARRAY_COUNT(member)); \
    } while (0)

#define TO_LAB_PRINT_PAY_SLT_FLOAT_ARRAY(label, payload_ptr, member)                  \
    do                                                                               \
    {                                                                                \
        float Values[TO_LAB_PAY_SLT_ARRAY_COUNT(member)];                            \
        memcpy(Values, &(payload_ptr)[offsetof(PAY_SLT_BcnTlm_Payload_t, member)],   \
               sizeof(Values));                                                      \
        TO_LAB_PrintFloatArray((label), Values, TO_LAB_PAY_SLT_ARRAY_COUNT(member)); \
    } while (0)

static void TO_LAB_PrintReportPayload(const char *Name, const CFE_SB_Buffer_t *SBBufPtr, CFE_MSG_Size_t SourceSize,
                                      bool IsCritical)
{
    const size_t HeaderSize = sizeof(CFE_MSG_TelemetryHeader_t);
    const size_t EntrySize  = IsCritical ? sizeof(RPT_Critical_t) : sizeof(RPT_Report_t);
    const uint8 *PayloadPtr = ((const uint8 *)SBBufPtr) + HeaderSize;
    const RPT_Report_t *Report;
    size_t ReportCount = 0;

    if (SourceSize < (HeaderSize + sizeof(RPT_Report_t)))
    {
        TO_LAB_APP_printf("TO_LAB RF %s parse: packet too short src_len=%lu min=%lu\n", Name,
                          (unsigned long)SourceSize, (unsigned long)(HeaderSize + sizeof(RPT_Report_t)));
        return;
    }

    ReportCount = (SourceSize - HeaderSize) / EntrySize;
    Report      = IsCritical ? &((const RPT_Critical_t *)PayloadPtr)->Report : (const RPT_Report_t *)PayloadPtr;

    TO_LAB_APP_printf("TO_LAB RF %s report: count=%lu report_msg=0x%04X cc=%u type=%u code=0x%08lX data_len=%u\n",
                      Name, (unsigned long)ReportCount, (unsigned int)Report->MsgID,
                      (unsigned int)Report->CommandCode, (unsigned int)Report->ReturnType,
                      (unsigned long)Report->ReturnCode, (unsigned int)Report->ReturnDataSize);
    TO_LAB_PrintBytes("TO_LAB RF report_data_head=", Report->ReturnValue, Report->ReturnDataSize, 16);
}

static void TO_LAB_PrintPaySltBcnPayload(const char *Path, const CFE_SB_Buffer_t *SBBufPtr,
                                         CFE_MSG_Size_t SourceSize)
{
    const size_t HeaderSize = sizeof(CFE_MSG_TelemetryHeader_t);
    const uint8 *Bytes = (const uint8 *)SBBufPtr;
    const uint8 *PayloadPtr = &Bytes[HeaderSize];
    PAY_SLT_BcnTlm_Payload_t Bcn;

    if (SourceSize < (HeaderSize + sizeof(Bcn)))
    {
        TO_LAB_APP_printf("TO_LAB %s PAY_SLT_BCN parse: packet too short src_len=%lu min=%lu\n",
                          Path, (unsigned long)SourceSize, (unsigned long)(HeaderSize + sizeof(Bcn)));
        return;
    }

    memcpy(&Bcn, PayloadPtr, sizeof(Bcn));

    TO_LAB_APP_printf("\n============[ TO_LAB %s PAY-SLT BCN ]============\n", Path);
    TO_LAB_APP_printf("[CNT] cmd=%u err=%u\n", (unsigned int)Bcn.CmdCounter, (unsigned int)Bcn.ErrCounter);
    TO_LAB_APP_printf("[PAY-EXP-A7] uptime=%lu now=%lu boot_p=%u boot_c=%u brd_temp=%d sys_status=[%d,%d]\n",
                      (unsigned long)Bcn.sys_uptime_a7, (unsigned long)Bcn.sys_now_a7,
                      (unsigned int)Bcn.boot_cnt_p, (unsigned int)Bcn.boot_cnt_c,
                      (int)Bcn.brd_temp_a7, (int)Bcn.sys_status_a7[0], (int)Bcn.sys_status_a7[1]);
    TO_LAB_PRINT_PAY_SLT_U8_ARRAY("       boot_his_p=", PayloadPtr, boot_his_p);
    TO_LAB_PRINT_PAY_SLT_U8_ARRAY("       boot_his_c=", PayloadPtr, boot_his_c);
    TO_LAB_PRINT_PAY_SLT_I16_ARRAY("       slf_data=", PayloadPtr, slf_data);
    TO_LAB_PRINT_PAY_SLT_I16_ARRAY("       brm_data=", PayloadPtr, brm_data);
    TO_LAB_PRINT_PAY_SLT_I16_ARRAY("       imu_data=", PayloadPtr, imu_data);
    TO_LAB_PRINT_PAY_SLT_I16_ARRAY("       ntc_data_a7=", PayloadPtr, ntc_data_a7);
    TO_LAB_PRINT_PAY_SLT_U16_ARRAY("       pwr_volt=", PayloadPtr, pwr_volt);
    TO_LAB_PRINT_PAY_SLT_U16_ARRAY("       pwr_current=", PayloadPtr, pwr_current);

    TO_LAB_APP_printf("[PAY-IFB] uptime=%lu now=%lu boot=%u sys_status=%d wdt_left=%lu brd_temp=%d sen_rst=%u\n",
                      (unsigned long)Bcn.sys_uptime_ifb, (unsigned long)Bcn.sys_now_ifb,
                      (unsigned int)Bcn.boot_cnt, (int)Bcn.sys_status_ifb,
                      (unsigned long)Bcn.wdt_left_ifb, (int)Bcn.brd_temp_ifb,
                      (unsigned int)Bcn.sen_rst);
    TO_LAB_APP_printf("          sen_online=%u sen_qlvl=%u att_ql=%u\n",
                      (unsigned int)Bcn.sen_online, (unsigned int)Bcn.sen_qlvl,
                      (unsigned int)Bcn.att_ql);
    TO_LAB_PRINT_PAY_SLT_U8_ARRAY("       boot_his=", PayloadPtr, boot_his);
    TO_LAB_PRINT_PAY_SLT_I16_ARRAY("       ntc_data_ifb=", PayloadPtr, ntc_data_ifb);
    TO_LAB_PRINT_PAY_SLT_U16_ARRAY("       pw_cur=", PayloadPtr, pw_cur);
    TO_LAB_PRINT_PAY_SLT_U16_ARRAY("       pw_vol=", PayloadPtr, pw_vol);
    TO_LAB_PRINT_PAY_SLT_FLOAT_ARRAY("       att_q=", PayloadPtr, att_q);
    TO_LAB_PRINT_PAY_SLT_FLOAT_ARRAY("       rot_r=", PayloadPtr, rot_r);
    TO_LAB_PRINT_PAY_SLT_FLOAT_ARRAY("       lin_acc=", PayloadPtr, lin_acc);
    TO_LAB_PRINT_PAY_SLT_FLOAT_ARRAY("       fld_vec=", PayloadPtr, fld_vec);
    TO_LAB_APP_printf("===========================================\n");
}


static void TO_LAB_PrintHkCombinedPkt1Hardcoded(const char *Path, const CFE_SB_Buffer_t *SBBufPtr,
                                                CFE_MSG_Size_t SourceSize)
{
    const uint8 *Bytes = (const uint8 *)SBBufPtr;
    RPT_BcnTlm_Payload_t      Rpt;
    UTRX_BcnTlm_Payload_t     Utrx;
    LTRX_BcnTlm_Payload_t     Ltrx;
    EPS_BcnTlm_Full_Payload_t Eps;
    GPIO_BcnTlm_Payload_t     Gpio;
    ADCS_BcnTlm_Payload_t     Adcs;

    enum
    {
        BCN_RPT_OFFSET  = TO_LAB_HK_COMBINED_PKT1_PAYLOAD_OFFSET,
        BCN_UTRX_OFFSET = BCN_RPT_OFFSET + sizeof(RPT_BcnTlm_Payload_t),
        BCN_LTRX_OFFSET = BCN_UTRX_OFFSET + sizeof(UTRX_BcnTlm_Payload_t),
        BCN_EPS_OFFSET  = BCN_LTRX_OFFSET + sizeof(LTRX_BcnTlm_Payload_t),
        BCN_GPIO_OFFSET = BCN_EPS_OFFSET + sizeof(EPS_BcnTlm_Full_Payload_t),
        BCN_ADCS_OFFSET = BCN_GPIO_OFFSET + sizeof(GPIO_BcnTlm_Payload_t),
        BCN_TOTAL_SIZE  = BCN_ADCS_OFFSET + sizeof(ADCS_BcnTlm_Payload_t)
    };

    if (SBBufPtr == NULL)
    {
        return;
    }

    if (SourceSize < BCN_TOTAL_SIZE)
    {
        TO_LAB_APP_printf("TO_LAB %s HKPKT1 parse: packet too short src_len=%lu min=%lu\n",
                          Path, (unsigned long)SourceSize, (unsigned long)BCN_TOTAL_SIZE);
        return;
    }

    TO_LAB_APP_printf("TO_LAB %s HKPKT1 layout: src_len=%lu payload_offset=%u payload_len=%lu total_struct_len=%u\n",
                      Path, (unsigned long)SourceSize, (unsigned int)BCN_RPT_OFFSET,
                      (unsigned long)(SourceSize - BCN_RPT_OFFSET),
                      (unsigned int)(BCN_TOTAL_SIZE - BCN_RPT_OFFSET));
    TO_LAB_APP_printf("TO_LAB %s HKPKT1 offsets: RPT=%u UTRX=%u LTRX=%u EPS=%u GPIO=%u ADCS=%u\n",
                      Path, (unsigned int)BCN_RPT_OFFSET, (unsigned int)BCN_UTRX_OFFSET,
                      (unsigned int)BCN_LTRX_OFFSET, (unsigned int)BCN_EPS_OFFSET,
                      (unsigned int)BCN_GPIO_OFFSET, (unsigned int)BCN_ADCS_OFFSET);

    if (SourceSize > BCN_TOTAL_SIZE)
    {
        TO_LAB_APP_printf("TO_LAB %s HKPKT1 parse: extra_bytes=%lu\n",
                          Path, (unsigned long)(SourceSize - BCN_TOTAL_SIZE));
    }

    memcpy(&Rpt, &Bytes[BCN_RPT_OFFSET], sizeof(Rpt));
    memcpy(&Utrx, &Bytes[BCN_UTRX_OFFSET], sizeof(Utrx));
    memcpy(&Ltrx, &Bytes[BCN_LTRX_OFFSET], sizeof(Ltrx));
    memcpy(&Eps, &Bytes[BCN_EPS_OFFSET], sizeof(Eps));
    memcpy(&Gpio, &Bytes[BCN_GPIO_OFFSET], sizeof(Gpio));
    memcpy(&Adcs, &Bytes[BCN_ADCS_OFFSET], sizeof(Adcs));

    TO_LAB_APP_printf("\n============[ TO_LAB %s HK COMBINED BCN ]============\n", Path);
    TO_LAB_APP_printf("[RPT]  boot_count=%u | seq=%lu | reset_cause=0x%02X\n",
                      (unsigned int)Rpt.BootCount, (unsigned long)Rpt.Sequence,
                      (unsigned int)Rpt.ResetCause);
    TO_LAB_APP_printf("[UTRX] active=%u | boot_count=%u\n",
                      (unsigned int)Utrx.ActiveConf, (unsigned int)Utrx.BootCount);
    TO_LAB_APP_printf("       boot_cause=0x%08lX | temp=%d\n",
                      (unsigned long)Utrx.BootCause, (int)Utrx.TempBrd);
    TO_LAB_APP_printf("[LTRX] temp=%d | conn_quality=%u | batt_capacity=%u\n",
                      (int)Ltrx.Temperature, (unsigned int)Ltrx.ConnectionQuality,
                      (unsigned int)Ltrx.BatteryCapacity);
    TO_LAB_APP_printf("------------------[ EPS ]------------------\n");
    TO_LAB_APP_printf("[PMU]  bootcause=0x%08lX reset=%u boot=%u\n",
                      (unsigned long)Eps.PMU.bootcause, (unsigned int)Eps.PMU.resetcause,
                      (unsigned int)Eps.PMU.bootcount);
    TO_LAB_APP_printf("       out_en=[%s,%s,%s,%s,%s,%s]\n",
                      TO_LAB_OnOffString(Eps.PMU.out_en[0]), TO_LAB_OnOffString(Eps.PMU.out_en[1]),
                      TO_LAB_OnOffString(Eps.PMU.out_en[2]), TO_LAB_OnOffString(Eps.PMU.out_en[3]),
                      TO_LAB_OnOffString(Eps.PMU.out_en[4]), TO_LAB_OnOffString(Eps.PMU.out_en[5]));
    TO_LAB_APP_printf("       temp=[%d,%d] batt_mode=%u sm=0x%02X\n",
                      (int)Eps.PMU.temp[0], (int)Eps.PMU.temp[1],
                      (unsigned int)Eps.PMU.batt_mode, (unsigned int)Eps.PMU.sm_en_mask);
    TO_LAB_APP_printf("       sm_en: SM0=%s SM1=%s SM2=%s SM3=%s SM4=%s SM5=%s SM6=%s SM7=%s\n",
                      TO_LAB_BitOnOffString(Eps.PMU.sm_en_mask, 0),
                      TO_LAB_BitOnOffString(Eps.PMU.sm_en_mask, 1),
                      TO_LAB_BitOnOffString(Eps.PMU.sm_en_mask, 2),
                      TO_LAB_BitOnOffString(Eps.PMU.sm_en_mask, 3),
                      TO_LAB_BitOnOffString(Eps.PMU.sm_en_mask, 4),
                      TO_LAB_BitOnOffString(Eps.PMU.sm_en_mask, 5),
                      TO_LAB_BitOnOffString(Eps.PMU.sm_en_mask, 6),
                      TO_LAB_BitOnOffString(Eps.PMU.sm_en_mask, 7));
    TO_LAB_APP_printf("       batt: i=%d mA  v=%u mV\n",
                      (int)Eps.PMU.batt_i, (unsigned int)Eps.PMU.batt_v);
    TO_LAB_APP_printf("       wdt: gnd_cnt=%u  gnd_left=%lu s  bus_cnt=%u  bus_left=%lu s\n",
                      (unsigned int)Eps.PMU.gnd_wdt_cnt, (unsigned long)Eps.PMU.gnd_wdt_left,
                      (unsigned int)Eps.PMU.bus_wdt_cnt, (unsigned long)Eps.PMU.bus_wdt_left);
    TO_LAB_APP_printf("[PDU]  ch | I(mA) | EN\n");
    for (size_t i = 0; i < EPS_PDU_BCN_USED_CH_COUNT; i++)
    {
        TO_LAB_APP_printf("       %02u | %6d | %3s\n", (unsigned int)TO_LAB_EPS_PDU_BCN_CHANNELS[i],
                          (int)Eps.PDU.out_i[i], TO_LAB_OnOffString(Eps.PDU.out_en[i]));
    }
    TO_LAB_APP_printf("[ACU1] in_i(mA)=[%d,%d,%d,%d,%d,%d]\n",
                      (int)Eps.ACU[0].input_i[0], (int)Eps.ACU[0].input_i[1],
                      (int)Eps.ACU[0].input_i[2], (int)Eps.ACU[0].input_i[3],
                      (int)Eps.ACU[0].input_i[4], (int)Eps.ACU[0].input_i[5]);
    TO_LAB_APP_printf("       in_v(mV)=[%u,%u,%u,%u,%u,%u] mppt=%u\n",
                      (unsigned int)Eps.ACU[0].input_v[0], (unsigned int)Eps.ACU[0].input_v[1],
                      (unsigned int)Eps.ACU[0].input_v[2], (unsigned int)Eps.ACU[0].input_v[3],
                      (unsigned int)Eps.ACU[0].input_v[4], (unsigned int)Eps.ACU[0].input_v[5],
                      (unsigned int)Eps.ACU[0].mppt_mode);
    TO_LAB_APP_printf("[ACU2] in_i(mA)=[%d,%d,%d,%d,%d,%d]\n",
                      (int)Eps.ACU[1].input_i[0], (int)Eps.ACU[1].input_i[1],
                      (int)Eps.ACU[1].input_i[2], (int)Eps.ACU[1].input_i[3],
                      (int)Eps.ACU[1].input_i[4], (int)Eps.ACU[1].input_i[5]);
    TO_LAB_APP_printf("       in_v(mV)=[%u,%u,%u,%u,%u,%u] mppt=%u\n",
                      (unsigned int)Eps.ACU[1].input_v[0], (unsigned int)Eps.ACU[1].input_v[1],
                      (unsigned int)Eps.ACU[1].input_v[2], (unsigned int)Eps.ACU[1].input_v[3],
                      (unsigned int)Eps.ACU[1].input_v[4], (unsigned int)Eps.ACU[1].input_v[5],
                      (unsigned int)Eps.ACU[1].mppt_mode);
    TO_LAB_APP_printf("[BP8]  boot=%u cause=0x%04X reset=%u\n",
                      (unsigned int)Eps.BP8.bootcount, (unsigned int)Eps.BP8.bootcause,
                      (unsigned int)Eps.BP8.resetcause);
    TO_LAB_APP_printf("       soc=%.1f %%  temp=%.1f degC\n",
                      (double)Eps.BP8.soc * 100.0, (double)Eps.BP8.bat_avr_temp);
    TO_LAB_APP_printf("       vbat=%u mV  current=%.3f A  heater_i=%u mA\n",
                      (unsigned int)Eps.BP8.vbat, (double)Eps.BP8.current,
                      (unsigned int)Eps.BP8.heater_i);
    TO_LAB_APP_printf("-------------------------------------------\n");
    TO_LAB_APP_printf("[GPIO] state=0x%02X padding=%u deployed=%u | bits: B0=%s B1=%s B2=%s B3=%s B4=%s B5=%s B6=%s B7=%s\n",
                      (unsigned int)Gpio.GpioState, (unsigned int)Gpio.Padding, (unsigned int)Gpio.isDeployed,
                      TO_LAB_BitHighLowString(Gpio.GpioState, 0), TO_LAB_BitHighLowString(Gpio.GpioState, 1),
                      TO_LAB_BitHighLowString(Gpio.GpioState, 2), TO_LAB_BitHighLowString(Gpio.GpioState, 3),
                      TO_LAB_BitHighLowString(Gpio.GpioState, 4), TO_LAB_BitHighLowString(Gpio.GpioState, 5),
                      TO_LAB_BitHighLowString(Gpio.GpioState, 6), TO_LAB_BitHighLowString(Gpio.GpioState, 7));
    TO_LAB_APP_printf("[ADCS] power: RWL0=%s RWL1=%s RWL2=%s MAG0=%s GYR0=%s FSS0=%s HSS0=%s | mode=%u%s\n",
                      TO_LAB_BitOnOffString(Adcs.PowerState, 6), TO_LAB_BitOnOffString(Adcs.PowerState, 5),
                      TO_LAB_BitOnOffString(Adcs.PowerState, 4), TO_LAB_BitOnOffString(Adcs.PowerState, 3),
                      TO_LAB_BitOnOffString(Adcs.PowerState, 2), TO_LAB_BitOnOffString(Adcs.PowerState, 1),
                      TO_LAB_BitOnOffString(Adcs.PowerState, 0), (unsigned int)Adcs.ControlMode,
                      TO_LAB_AdcsControlModeName(Adcs.ControlMode));
    TO_LAB_APP_printf("       gyr(deg/s)=[%.6f, %.6f, %.6f]\n",
                      (double)Adcs.GYR0CalibratedRateXComponent,
                      (double)Adcs.GYR0CalibratedRateYComponent,
                      (double)Adcs.GYR0CalibratedRateZComponent);
    TO_LAB_APP_printf("       css: CSS0=%u CSS1=%u CSS2=%u CSS3=%u CSS4=%u CSS5=%u\n",
                      (unsigned int)Adcs.CSS[0], (unsigned int)Adcs.CSS[1], (unsigned int)Adcs.CSS[2],
                      (unsigned int)Adcs.CSS[3], (unsigned int)Adcs.CSS[4], (unsigned int)Adcs.CSS[5]);
    TO_LAB_APP_printf("===========================================\n");
}

static void TO_LAB_PrintRfPayloadDetail(CFE_SB_MsgId_t MsgId, const CFE_SB_Buffer_t *SBBufPtr, const void *NetBufPtr,
                                        size_t NetBufSize, uint16 Port, uint32 BeaconSlot, bool HasBeaconSlot)
{
    CFE_MSG_Size_t SourceSize = 0;
    uint32         MidValue;

    (void)CFE_MSG_GetSize(&SBBufPtr->Msg, &SourceSize);
    MidValue = CFE_SB_MsgIdToValue(MsgId);

    TO_LAB_APP_printf("TO_LAB RF PRE-EMIT: mid=0x%04lX src_len=%lu net_len=%lu port=%u",
                      (unsigned long)MidValue, (unsigned long)SourceSize, (unsigned long)NetBufSize,
                      (unsigned int)Port);
    if (HasBeaconSlot)
    {
        TO_LAB_APP_printf(" beacon_slot=%lu", (unsigned long)BeaconSlot);
    }
    TO_LAB_APP_printf("\n");
    TO_LAB_PrintBytes("TO_LAB RF net_head=", (const uint8 *)NetBufPtr, NetBufSize, 32);

    switch (MidValue)
    {
        case (CFE_SB_MsgId_Atom_t)RPT_REPORT_TLM_MID:
            TO_LAB_PrintReportPayload("RPT_REPORT", SBBufPtr, SourceSize, false);
            break;
        case (CFE_SB_MsgId_Atom_t)RPT_CRITICAL_TLM_MID:
            TO_LAB_PrintReportPayload("RPT_CRITICAL", SBBufPtr, SourceSize, true);
            break;
        case (CFE_SB_MsgId_Atom_t)EPS_REPORT_MID:
            TO_LAB_PrintReportPayload("EPS_REPORT", SBBufPtr, SourceSize, false);
            break;
        case (CFE_SB_MsgId_Atom_t)HK_COMBINED_PKT1_MID:
            TO_LAB_APP_printf("TO_LAB RF HK_COMBINED_PKT1: prefix=%s prefix_len=%lu original_len=%lu final_len=%lu\n",
                              TO_LAB_HK_COMBINED_PKT1_RF_PREFIX,
                              (unsigned long)TO_LAB_HK_COMBINED_PKT1_RF_PREFIX_SIZE,
                              (unsigned long)SourceSize, (unsigned long)NetBufSize);
            TO_LAB_PrintHkCombinedPkt1Hardcoded("RF", SBBufPtr, SourceSize);
            break;
        case (CFE_SB_MsgId_Atom_t)PAY_SLT_BCN_TLM_MID:
            TO_LAB_PrintPaySltBcnPayload("RF", SBBufPtr, SourceSize);
            break;
        default:
            TO_LAB_APP_printf("TO_LAB RF DEFAULT: unparsed payload\n");
            break;
    }
}

void TO_LAB_ForwardTelemetryRF(void) {
    CFE_Status_t     Status;
    CFE_SB_Buffer_t *SBBufPtr;
    const void      *NetBufPtr;
    size_t           NetBufSize;
    uint8            HkCombinedPkt1RfBuf[TO_LAB_RF_MAX_AVAILABLE_BYTES];
    uint8_t         beacon_delay_pattern[] = {2,5,10,20};   // BEE  
    CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;
    uint8_t          Port = CFE_RF_DPORT_BCN;

    /* Debug */
    TO_LAB_APP_printf("%s: TO child start.\n", __func__);
    
    for (;;) {
        Status  = CFE_SB_ReceiveBuffer(&SBBufPtr, TO_LAB_Global.Tlm_pipe, CFE_SB_PEND_FOREVER);
        if (!TO_LAB_Global.downlink_on) continue;

        if (Status == CFE_SUCCESS) { // If Tlm Message Received, 
            Status = TO_LAB_EncodeOutputMessage(SBBufPtr, &NetBufPtr, &NetBufSize);
        }

        if (Status != CFE_SUCCESS) { // If Encode fails,
            CFE_EVS_SendEvent(TO_LAB_ENCODE_ERR_EID, CFE_EVS_EventType_ERROR, "Error packing output: %d\n",
                                      (int)Status);
            continue;
        }
        else { // Else, transmit the telemetry
            /* Find out the Msgid */

            CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

            /* Determine the Destination Port */
            uint32 BeaconSlot = 0;
            bool   HasBeaconSlot = false;
            switch (CFE_SB_MsgIdToValue(MsgId)) {
                case (CFE_SB_MsgId_Atom_t)RPT_REPORT_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)RPT_CRITICAL_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)EPS_REPORT_MID:
                    Port = CFE_RF_DPORT_RPT;
                    break;
                case (CFE_SB_MsgId_Atom_t)CFE_EVS_HK_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)CFE_EVS_LONG_EVENT_MSG_MID:
                case (CFE_SB_MsgId_Atom_t)CFE_EVS_SHORT_EVENT_MSG_MID:
                    Port = CFE_RF_DPORT_EVS;
                    break;
                case (CFE_SB_MsgId_Atom_t)FM_HK_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)FM_FILE_INFO_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)FM_DIR_LIST_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)FM_OPEN_FILES_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)FM_FREE_SPACE_TLM_MID:
                    Port = CFE_RF_DPORT_FM;
                    break;
                case (CFE_SB_MsgId_Atom_t)HK_COMBINED_PKT1_MID:
                    Port = CFE_RF_DPORT_BCN;

                    Status = OS_MutSemTake(TO_LAB_Global.MutexId);
                    if (Status != OS_SUCCESS)
                    {
                        CFE_EVS_SendEvent(TO_LAB_BCN_RESET_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "TO: Failed to lock RF BCN counter, RC=0x%08X",
                                          (unsigned int)Status);
                        continue;
                    }

                    BeaconSlot = TO_LAB_Global.BCN_PktCount % 20;
                    HasBeaconSlot = true;
                    TO_LAB_Global.BCN_PktCount++;

                    Status = OS_MutSemGive(TO_LAB_Global.MutexId);
                    if (Status != OS_SUCCESS)
                    {
                        CFE_EVS_SendEvent(TO_LAB_BCN_RESET_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "TO: Failed to unlock RF BCN counter, RC=0x%08X",
                                          (unsigned int)Status);
                        continue;
                    }

                    if (BeaconSlot != beacon_delay_pattern[0] &&
                        BeaconSlot != beacon_delay_pattern[1] &&
                        BeaconSlot != beacon_delay_pattern[2] &&
                        BeaconSlot != beacon_delay_pattern[3])
                    {
                        continue;
                    }
                    if ((NetBufSize + TO_LAB_HK_COMBINED_PKT1_RF_PREFIX_SIZE) > sizeof(HkCombinedPkt1RfBuf))
                    {
                        CFE_EVS_SendErr(TO_LAB_TLMOUTSTOP_ERR_EID,
                                        "%s: HK combined RF packet too large. size=%lu prefix=%lu max=%lu\n",
                                        __func__, (unsigned long)NetBufSize,
                                        (unsigned long)TO_LAB_HK_COMBINED_PKT1_RF_PREFIX_SIZE,
                                        (unsigned long)sizeof(HkCombinedPkt1RfBuf));
                        continue;
                    }
                    memcpy(HkCombinedPkt1RfBuf, TO_LAB_HK_COMBINED_PKT1_RF_PREFIX,
                           TO_LAB_HK_COMBINED_PKT1_RF_PREFIX_SIZE);
                    memcpy(&HkCombinedPkt1RfBuf[TO_LAB_HK_COMBINED_PKT1_RF_PREFIX_SIZE], NetBufPtr, NetBufSize);
                    NetBufPtr  = HkCombinedPkt1RfBuf;
                    NetBufSize += TO_LAB_HK_COMBINED_PKT1_RF_PREFIX_SIZE;
                    break;

                case (CFE_SB_MsgId_Atom_t)PAY_SLT_BCN_TLM_MID:
                    Port = CFE_RF_DPORT_BCN;
                    break;

                default:
                    Port = CFE_RF_DPORT_BCN;
                    break;
            }

            TO_LAB_PrintRfPayloadDetail(MsgId, SBBufPtr, NetBufPtr, NetBufSize, Port, BeaconSlot, HasBeaconSlot);

            /* Determine the Emission Mode */
            Status = CFE_RF_TelemetryEmit((void *)NetBufPtr, NetBufSize, Port); /* Eliminate `const` attr by (void *) casting */
            TO_LAB_PrintOutgoing("RF", SBBufPtr, NetBufPtr, NetBufSize, Status, Port);
            TO_LAB_APP_printf("%s: RF Transmission Status : %d\n", __func__, Status);
            if (Status != CFE_SUCCESS) {
                CFE_EVS_SendErr(TO_LAB_TLMOUTSTOP_ERR_EID, "%s: RF emit error. RC=0x%08X\n", __func__, Status);
            }

            OS_TaskDelay(10);

        }

    }
    CFE_EVS_SendCrit(TO_LAB_END_CHILD_CRIT_EID, "TO child Task finished. Should be restarted.");
}

void TO_LAB_ForwardTelemetryUDP(void)
{
    OS_SockAddr_t    d_addr;
    int32            OsStatus;
    CFE_Status_t     CfeStatus;
    CFE_SB_Buffer_t *SBBufPtr;
    const void      *NetBufPtr;
    size_t           NetBufSize;
    CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;

    uint32_t         BCN_PktCount = 0;
    //uint8_t         beacon_delay_pattern[] = {2,5,10,20};   // BEE  
    uint8_t        beacon_delay_pattern[] = {10, 15, 18, 20}; // UYS

    OS_printf("%s: TO child start.\n", __func__);

    OS_SocketAddrInit(&d_addr, OS_SocketDomain_INET);
    OS_SocketAddrSetPort(&d_addr, TO_LAB_TLM_PORT);
    OsStatus = 0;

    for(; ; ){
 
        CfeStatus = CFE_SB_ReceiveBuffer(&SBBufPtr, TO_LAB_Global.Tlm_pipe, TO_LAB_TLM_PIPE_TIMEOUT);

        if ((CfeStatus == CFE_SUCCESS) && (TO_LAB_Global.suppress_sendto == false))
        {
            OsStatus = OS_SUCCESS;

            if (TO_LAB_Global.downlink_on == true)
            {
                CFE_ES_PerfLogEntry(TO_LAB_SOCKET_SEND_PERF_ID);

                CfeStatus = TO_LAB_EncodeOutputMessage(SBBufPtr, &NetBufPtr, &NetBufSize);

                if (CfeStatus != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(TO_LAB_ENCODE_ERR_EID, CFE_EVS_EventType_ERROR, "Error packing output: %d\n",
                                      (int)CfeStatus);
                }
                else
                {

                    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
                    if (CFE_SB_MsgIdToValue(MsgId) == (CFE_SB_MsgId_Atom_t)HK_COMBINED_PKT1_MID)
                    {
                        CFE_MSG_Size_t SourceSize = 0;
                        uint32_t beacon_slot = BCN_PktCount % 20;
                        BCN_PktCount++;

                        if (beacon_slot != beacon_delay_pattern[0] &&
                            beacon_slot != beacon_delay_pattern[1] &&
                            beacon_slot != beacon_delay_pattern[2] &&
                            beacon_slot != beacon_delay_pattern[3])
                        {
                            continue;
                        }

                        (void)CFE_MSG_GetSize(&SBBufPtr->Msg, &SourceSize);
                        TO_LAB_APP_printf("TO_LAB UDP HK_COMBINED_PKT1: src_len=%lu net_len=%lu slot=%lu dest=%s:%u\n",
                                          (unsigned long)SourceSize, (unsigned long)NetBufSize,
                                          (unsigned long)beacon_slot, TO_LAB_Global.tlm_dest_IP,
                                          (unsigned int)TO_LAB_TLM_PORT);
                        TO_LAB_PrintHkCombinedPkt1Hardcoded("UDP", SBBufPtr, SourceSize);
                        
                    }
                    else if (CFE_SB_MsgIdToValue(MsgId) == (CFE_SB_MsgId_Atom_t)PAY_SLT_BCN_TLM_MID)
                    {
                        CFE_MSG_Size_t SourceSize = 0;

                        (void)CFE_MSG_GetSize(&SBBufPtr->Msg, &SourceSize);
                        TO_LAB_APP_printf("TO_LAB UDP PAY_SLT_BCN: src_len=%lu net_len=%lu dest=%s:%u\n",
                                          (unsigned long)SourceSize, (unsigned long)NetBufSize,
                                          TO_LAB_Global.tlm_dest_IP, (unsigned int)TO_LAB_TLM_PORT);
                        TO_LAB_PrintPaySltBcnPayload("UDP", SBBufPtr, SourceSize);
                    }
                    
                    OsStatus = OS_SocketAddrFromString(&d_addr, TO_LAB_Global.tlm_dest_IP);
                    if (OsStatus == OS_SUCCESS)
                    {
                        OsStatus = OS_SocketAddrSetPort(&d_addr, TO_LAB_TLM_PORT);
                    }
                    if (OsStatus == OS_SUCCESS)
                    {
                        OsStatus = OS_SocketSendTo(TO_LAB_Global.TLMsockid, NetBufPtr, NetBufSize, &d_addr);
                        TO_LAB_PrintOutgoing("UDP", SBBufPtr, NetBufPtr, NetBufSize, OsStatus, TO_LAB_TLM_PORT);
                    }
                }

                CFE_ES_PerfLogExit(TO_LAB_SOCKET_SEND_PERF_ID);
            }

            if (OsStatus < 0)
            {
                CFE_EVS_SendEvent(TO_LAB_TLMOUTSTOP_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "L%d TO sendto error %d. Tlm output suppressed\n", __LINE__, (int)OsStatus);
                TO_LAB_Global.suppress_sendto = true;
            }
        }
        /* If CFE_SB_status != CFE_SUCCESS, then no packet was received from CFE_SB_ReceiveBuffer() */
        OS_TaskDelay(10);
    }
    OS_printf("%s: TO child terminated.\n", __func__);
}

void TO_HandleReport(int32 Status, uint8 CC, const void *Data, size_t DataSize) {
    size_t CopySize = (DataSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : DataSize;

    TO_LAB_Global.ReportTlm.Payload.MsgID = TO_LAB_CMD_MID;
    TO_LAB_Global.ReportTlm.Payload.CommandCode = CC;
    TO_LAB_Global.ReportTlm.Payload.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_APP;
    TO_LAB_Global.ReportTlm.Payload.ReturnCode = Status;
    TO_LAB_Global.ReportTlm.Payload.ReturnDataSize = (uint16)CopySize;
    if (Data != NULL && CopySize > 0)
    {
        memcpy(TO_LAB_Global.ReportTlm.Payload.ReturnValue, Data, CopySize);
    }

    /* Transmit to SB */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(TO_LAB_Global.ReportTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(TO_LAB_Global.ReportTlm.TelemetryHeader), true);

}
