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

void TO_LAB_ForwardTelemetryRF(void) {
    CFE_Status_t     Status;
    CFE_SB_Buffer_t *SBBufPtr;
    const void      *NetBufPtr;
    size_t           NetBufSize;
    uint32_t         BCN_PktCount = 0;
    uint8_t         beacon_delay_pattern[] = {2,5,10,20};   // BEE  
    // uint8_t        beacon_delay_patter[] = (10, 15, 18, 20); // UYS
    CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;
    uint8_t          Port = CFE_RF_DPORT_BCN;

    /* Debug */
    OS_printf("%s: TO child start.\n", __func__);
    
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
            switch (CFE_SB_MsgIdToValue(MsgId)) {
                case (CFE_SB_MsgId_Atom_t)RPT_REPORT_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)RPT_CRITICAL_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)EPS_REPORT_MID:
                    Port = CFE_RF_DPORT_RPT;
                    break;
                case (CFE_SB_MsgId_Atom_t)HK_COMBINED_PKT1_MID:
                    Port = CFE_RF_DPORT_BCN;
                    uint32_t beacon_slot = BCN_PktCount % 20;
                    BCN_PktCount++;

                    if (beacon_slot != beacon_delay_pattern[0] &&
                        beacon_slot != beacon_delay_pattern[1] &&
                        beacon_slot != beacon_delay_pattern[2] &&
                        beacon_slot != beacon_delay_pattern[3])
                    {
                        continue;
                    }
                    break;

                default:
                    Port = CFE_RF_DPORT_BCN;
                    break;
            }

            /* Determine the Emission Mode */
            Status = CFE_RF_TelemetryEmit((void *)NetBufPtr, NetBufSize, Port); /* Eliminate `const` attr by (void *) casting */
            TO_LAB_PrintOutgoing("RF", SBBufPtr, NetBufPtr, NetBufSize, Status, Port);
            OS_printf("%s: U Transmission Status : %d\n", __func__, Status);
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
