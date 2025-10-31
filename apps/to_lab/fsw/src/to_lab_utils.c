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
#include "ftp_msgids.h"
#include "hk_msgids.h"

void TO_LAB_ForwardTelemetryRF(void) {
    CFE_Status_t     Status;
    CFE_SB_Buffer_t *SBBufPtr;
    const void      *NetBufPtr;
    size_t           NetBufSize;
    uint32_t         PktCount = 0;

    CFE_SB_MsgId_t   MsgId = CFE_SB_INVALID_MSG_ID;
    uint8_t          Port = CFE_RF_DPORT_BCN;
    uint8_t          EmissionMode = TO_S_ONLY_EMISSION;
    bool             EmitS = true;
    bool             EmitU = false;

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
                case (CFE_SB_MsgId_Atom_t)FTP_FILE_MID:
                    Port = CFE_RF_DPORT_FTP;
                    break;

                case (CFE_SB_MsgId_Atom_t)RPT_REPORT_TLM_MID:
                case (CFE_SB_MsgId_Atom_t)RPT_CRITICAL_TLM_MID:
                    Port = CFE_RF_DPORT_RPT;
                    break;

                default:
                    Port = CFE_RF_DPORT_BCN;
                    break;
            }

            /* Determine the Emission Mode */
            OS_MutSemTake(TO_LAB_Global.MutexId);
            EmissionMode = TO_LAB_Global.EmissionMode;
            OS_MutSemGive(TO_LAB_Global.MutexId);
            switch (EmissionMode) {
                case TO_NO_EMISSION:
                    EmitS = false; EmitU = false;
                    break;

                case TO_S_ONLY_EMISSION:
                    EmitS = true; EmitU = false;
                    break;

                case TO_U_ONLY_EMISSION:
                    EmitS = false; EmitU = true;
                    break;
                    
                case TO_DUAL_EMISSION:
                    EmitS = true; EmitU = true;
                    break;
                    
                default: /* Equivalent to `TO_S_ONLY_EMISSION` */
                    EmitS = true; EmitU = false;
                    break;
            }
            
            /* Do Actual Transmission */
            if (EmitS) {
                Status = CFE_RF_TelemetryEmit((void *)NetBufPtr, NetBufSize, Port); /* Eliminate `const` attr by (void *) casting */ 
                if (Status == 1) Status = CFE_SUCCESS;
                else CFE_EVS_SendErr(TO_LAB_TLMOUTSTOP_ERR_EID, "%s: RF emit error. RC=0x%08X\n", __func__, Status);

                OS_printf("%s: S Transmission Status : %d\n", __func__, Status);
            }
            if (EmitU) {
                Status = CFE_RF_TelemetryEmit2((void *)NetBufPtr, NetBufSize, Port); /* Eliminate `const` attr by (void *) casting */ 
                if (Status == 1) Status = CFE_SUCCESS;
                else CFE_EVS_SendErr(TO_LAB_TLMOUTSTOP_ERR_EID, "%s: RF emit error. RC=0x%08X\n", __func__, Status);
                
                OS_printf("%s: UHF Transmission Status : %d\n", __func__, Status);
            }

            PktCount ++;

            if (PktCount >= TO_LAB_MAX_TLM_PKTS) OS_TaskDelay(5); /* Prevent Hogging */
        }

    }
    CFE_EVS_SendCrit(TO_LAB_END_CHILD_CRIT_EID, "TO child Task finished. Should be restarted.");
}


void TO_HandleReport(int32 Status, uint8 CC, const void *Data, size_t DataSize) {
    TO_LAB_Global.ReportTlm.Payload.MsgID = TO_LAB_CMD_MID;
    TO_LAB_Global.ReportTlm.Payload.CommandCode = CC;
    TO_LAB_Global.ReportTlm.Payload.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_APP;
    TO_LAB_Global.ReportTlm.Payload.ReturnCode = Status;
    TO_LAB_Global.ReportTlm.Payload.ReturnDataSize = (uint16)DataSize;
    memcpy(TO_LAB_Global.ReportTlm.Payload.ReturnValue, Data,
            (DataSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : DataSize);

    /* Transmit to SB */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(TO_LAB_Global.ReportTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(TO_LAB_Global.ReportTlm.TelemetryHeader), true);

}