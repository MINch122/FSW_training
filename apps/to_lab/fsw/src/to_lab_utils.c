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
#include "eps_msgids.h"
#include "hk_msgids.h"
#include "fm_msgids.h"
#include "cfe_evs_msgids.h"

#include <string.h>

static const uint8 TO_LAB_HK_COMBINED_PKT1_RF_PREFIX[] = "BEE1012";
#define TO_LAB_HK_COMBINED_PKT1_RF_PREFIX_SIZE (sizeof(TO_LAB_HK_COMBINED_PKT1_RF_PREFIX) - 1)
#define TO_LAB_RF_MAX_AVAILABLE_BYTES 250

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


static void TO_LAB_PrintHardcodedBcnField(const char *Name, const uint8 *Bytes, size_t PacketSize, size_t Offset,
                                          size_t FieldSize)
{
    if ((Offset + FieldSize) > PacketSize)
    {
        TO_LAB_APP_printf("TO_LAB RF BCN %-24s: out_of_range offset=%lu len=%lu packet_len=%lu\n", Name,
                          (unsigned long)Offset, (unsigned long)FieldSize, (unsigned long)PacketSize);
        return;
    }

    TO_LAB_APP_printf("TO_LAB RF BCN %-24s:", Name);
    for (size_t i = 0; i < FieldSize; i++)
    {
        TO_LAB_APP_printf(" %02X", (unsigned int)Bytes[Offset + i]);
    }
    TO_LAB_APP_printf("\n");
}

static void TO_LAB_PrintHkCombinedPkt1Hardcoded(const CFE_SB_Buffer_t *SBBufPtr, CFE_MSG_Size_t SourceSize)
{
    const uint8 *Bytes = (const uint8 *)SBBufPtr;

    enum
    {
        BCN_RPT_OFFSET  = 12,
        BCN_UTRX_OFFSET = 19,
        BCN_LTRX_OFFSET = 28,
        BCN_EPS_OFFSET  = 33,
        BCN_GPIO_OFFSET = 177,
        BCN_ADCS_OFFSET = 180
    };

    TO_LAB_APP_printf("TO_LAB RF BCN hardcoded parse: src_len=%lu\n", (unsigned long)SourceSize);

    TO_LAB_PrintHardcodedBcnField("RPT.BootCount", Bytes, SourceSize, BCN_RPT_OFFSET + 0, 2);
    TO_LAB_PrintHardcodedBcnField("RPT.Sequence", Bytes, SourceSize, BCN_RPT_OFFSET + 2, 4);
    TO_LAB_PrintHardcodedBcnField("RPT.ResetCause", Bytes, SourceSize, BCN_RPT_OFFSET + 6, 1);

    TO_LAB_PrintHardcodedBcnField("UTRX.ActiveConf", Bytes, SourceSize, BCN_UTRX_OFFSET + 0, 1);
    TO_LAB_PrintHardcodedBcnField("UTRX.BootCount", Bytes, SourceSize, BCN_UTRX_OFFSET + 1, 2);
    TO_LAB_PrintHardcodedBcnField("UTRX.BootCause", Bytes, SourceSize, BCN_UTRX_OFFSET + 3, 4);
    TO_LAB_PrintHardcodedBcnField("UTRX.TempBrd", Bytes, SourceSize, BCN_UTRX_OFFSET + 7, 2);

    TO_LAB_PrintHardcodedBcnField("LTRX.Temperature", Bytes, SourceSize, BCN_LTRX_OFFSET + 0, 2);
    TO_LAB_PrintHardcodedBcnField("LTRX.ConnectionQuality", Bytes, SourceSize, BCN_LTRX_OFFSET + 2, 1);
    TO_LAB_PrintHardcodedBcnField("LTRX.BatteryCapacity", Bytes, SourceSize, BCN_LTRX_OFFSET + 3, 2);

    TO_LAB_PrintHardcodedBcnField("EPS.PMU.bootcause", Bytes, SourceSize, BCN_EPS_OFFSET + 0, 4);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.resetcause", Bytes, SourceSize, BCN_EPS_OFFSET + 4, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.bootcount", Bytes, SourceSize, BCN_EPS_OFFSET + 6, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.out_en", Bytes, SourceSize, BCN_EPS_OFFSET + 8, 6);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.temp", Bytes, SourceSize, BCN_EPS_OFFSET + 14, 4);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.batt_mode", Bytes, SourceSize, BCN_EPS_OFFSET + 18, 1);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.batt_i", Bytes, SourceSize, BCN_EPS_OFFSET + 19, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.batt_v", Bytes, SourceSize, BCN_EPS_OFFSET + 21, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.sm_en_mask", Bytes, SourceSize, BCN_EPS_OFFSET + 23, 1);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.gnd_wdt_cnt", Bytes, SourceSize, BCN_EPS_OFFSET + 24, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.bus_wdt_cnt", Bytes, SourceSize, BCN_EPS_OFFSET + 26, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.gnd_wdt_left", Bytes, SourceSize, BCN_EPS_OFFSET + 28, 4);
    TO_LAB_PrintHardcodedBcnField("EPS.PMU.bus_wdt_left", Bytes, SourceSize, BCN_EPS_OFFSET + 32, 4);

    TO_LAB_PrintHardcodedBcnField("EPS.PDU.out_i", Bytes, SourceSize, BCN_EPS_OFFSET + 36, 24);
    TO_LAB_PrintHardcodedBcnField("EPS.PDU.out_en", Bytes, SourceSize, BCN_EPS_OFFSET + 60, 12);

    TO_LAB_PrintHardcodedBcnField("EPS.ACU0.input_i", Bytes, SourceSize, BCN_EPS_OFFSET + 72, 12);
    TO_LAB_PrintHardcodedBcnField("EPS.ACU0.input_v", Bytes, SourceSize, BCN_EPS_OFFSET + 84, 12);
    TO_LAB_PrintHardcodedBcnField("EPS.ACU0.mppt_mode", Bytes, SourceSize, BCN_EPS_OFFSET + 96, 1);
    TO_LAB_PrintHardcodedBcnField("EPS.ACU1.input_i", Bytes, SourceSize, BCN_EPS_OFFSET + 97, 12);
    TO_LAB_PrintHardcodedBcnField("EPS.ACU1.input_v", Bytes, SourceSize, BCN_EPS_OFFSET + 109, 12);
    TO_LAB_PrintHardcodedBcnField("EPS.ACU1.mppt_mode", Bytes, SourceSize, BCN_EPS_OFFSET + 121, 1);

    TO_LAB_PrintHardcodedBcnField("EPS.BP8.bootcount", Bytes, SourceSize, BCN_EPS_OFFSET + 122, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.BP8.bootcause", Bytes, SourceSize, BCN_EPS_OFFSET + 124, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.BP8.resetcause", Bytes, SourceSize, BCN_EPS_OFFSET + 126, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.BP8.soc", Bytes, SourceSize, BCN_EPS_OFFSET + 128, 4);
    TO_LAB_PrintHardcodedBcnField("EPS.BP8.bat_avr_temp", Bytes, SourceSize, BCN_EPS_OFFSET + 132, 4);
    TO_LAB_PrintHardcodedBcnField("EPS.BP8.vbat", Bytes, SourceSize, BCN_EPS_OFFSET + 136, 2);
    TO_LAB_PrintHardcodedBcnField("EPS.BP8.current", Bytes, SourceSize, BCN_EPS_OFFSET + 138, 4);
    TO_LAB_PrintHardcodedBcnField("EPS.BP8.heater_i", Bytes, SourceSize, BCN_EPS_OFFSET + 142, 2);

    TO_LAB_PrintHardcodedBcnField("GPIO.GpioState", Bytes, SourceSize, BCN_GPIO_OFFSET + 0, 1);
    TO_LAB_PrintHardcodedBcnField("GPIO.Padding", Bytes, SourceSize, BCN_GPIO_OFFSET + 1, 1);
    TO_LAB_PrintHardcodedBcnField("GPIO.isDeployed", Bytes, SourceSize, BCN_GPIO_OFFSET + 2, 1);

    TO_LAB_PrintHardcodedBcnField("ADCS.PowerState", Bytes, SourceSize, BCN_ADCS_OFFSET + 0, 1);
    TO_LAB_PrintHardcodedBcnField("ADCS.ControlMode", Bytes, SourceSize, BCN_ADCS_OFFSET + 1, 1);
    TO_LAB_PrintHardcodedBcnField("ADCS.GYR0CalRateX", Bytes, SourceSize, BCN_ADCS_OFFSET + 2, 4);
    TO_LAB_PrintHardcodedBcnField("ADCS.GYR0CalRateY", Bytes, SourceSize, BCN_ADCS_OFFSET + 6, 4);
    TO_LAB_PrintHardcodedBcnField("ADCS.GYR0CalRateZ", Bytes, SourceSize, BCN_ADCS_OFFSET + 10, 4);
    TO_LAB_PrintHardcodedBcnField("ADCS.CSS", Bytes, SourceSize, BCN_ADCS_OFFSET + 14, 6);
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
            TO_LAB_PrintHkCombinedPkt1Hardcoded(SBBufPtr, SourceSize);
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
    uint32_t         BCN_PktCount = 0;
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
                    BeaconSlot = BCN_PktCount % 20;
                    HasBeaconSlot = true;
                    BCN_PktCount++;

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
                        uint32_t beacon_slot = BCN_PktCount % 20;
                        BCN_PktCount++;

                        if (beacon_slot != beacon_delay_pattern[0] &&
                            beacon_slot != beacon_delay_pattern[1] &&
                            beacon_slot != beacon_delay_pattern[2] &&
                            beacon_slot != beacon_delay_pattern[3])
                        {
                            continue;
                        }
                        
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
