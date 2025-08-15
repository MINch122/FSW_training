/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the Utrx App.
 */

/*
** Include Files:
*/
#include "utrx_app.h"
#include "utrx_app_dispatch.h"
#include "utrx_app_cmds.h"
#include "utrx_app_eventids.h"
#include "utrx_app_msgids.h"
#include "utrx_app_msg.h"
#include "utrx_app_fcncodes.h"
#include "utrx_cmds_ax100.h"
#include <gs/param/rparam.h>

extern UTRX_APP_Data_t UTRX_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool UTRX_APP_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(UTRX_APP_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        UTRX_APP_Data.AppErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* UTRX ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void UTRX_APP_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
   CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_Status_t rc = CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    if (rc != CFE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_GETFCN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX: Failed to get command code from message (rc=%d)", (int32)rc);

       
        memset(UTRX_APP_Data.RptPkt.Report.ReturnValue, 0, sizeof(UTRX_APP_Data.RptPkt.Report.ReturnValue));
        UTRX_APP_Data.RptPkt.Report.ReturnDataSize = 0;
        UTRX_APP_Data.RptPkt.Report.ReturnType = CMD_RETCODE_TYPE_APP;
        UTRX_APP_Data.RptPkt.Report.ReturnCode = rc;
        UTRX_CountFromReport();

          CFE_MSG_Init(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader),
                     CFE_SB_ValueToMsgId(UTRX_APP_RPT_TLM_MID),
                     sizeof(UTRX_ReportTlm_t));
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader), true);
        return;
    }

    /*
    ** Process UTRX app ground commands
    */
    switch (CommandCode)
    {
        case UTRX_APP_NOOP_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_APP_NoopCmd_t)))
            {
                UTRX_APP_NoopCmd((const UTRX_APP_NoopCmd_t *)SBBufPtr);

                
            }
            break;

        case UTRX_APP_RESET_COUNTERS_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_APP_ResetCountersCmd_t)))
            {
                UTRX_APP_ResetCountersCmd((const UTRX_APP_ResetCountersCmd_t *)SBBufPtr);
               

            }
            break;

       case UTRX_APP_RESET_APP_CMD_COUNTERS_CC:
           if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_APP_ResetAppCmdCountersCmd_t))) 
           {
              UTRX_APP_ResetAppCmdCountersCmd((const UTRX_APP_ResetAppCmdCountersCmd_t *)SBBufPtr);
             
           }
           break;

        case UTRX_APP_RESET_DEVICE_CMD_COUNTERS_CC:
           if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_APP_ResetDeviceCmdCountersCmd_t))) 
           {
              UTRX_APP_ResetDeviceCmdCountersCmd((const UTRX_APP_ResetDeviceCmdCountersCmd_t *)SBBufPtr);
             
           }
           break;

///////////////////////////////////////////////////////////////////////////////////////////////
        case UTRX_GNDWDT_CLEAR_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GndwdtClear_t))) {
                UTRX_AX100_GndwdtClearCmd((const UTRX_AX100_GndwdtClear_t *)SBBufPtr);
            }
            break;

        case UTRX_REBOOT_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_UTRX_Reboot_t))) {
                UTRX_AX100_UTRX_RebootCmd((const UTRX_AX100_UTRX_Reboot_t *)SBBufPtr);
            }
            break;

        case UTRX_RXCONF_SET_BAUD_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_RXCONF_SetBaudCmd_t))) {
                UTRX_AX100_RXCONF_SetBaudCmd((const UTRX_AX100_RXCONF_SetBaudCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TXCONF_SET_BAUD_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_TXCONF_SetBaudCmd_t))) {
                UTRX_AX100_TXCONF_SetBaudCmd((const UTRX_AX100_TXCONF_SetBaudCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_RXCONF_SET_FREQ_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_RXCONF_SetFreqCmd_t))) {
                UTRX_AX100_RXCONF_SetFreqCmd((const UTRX_AX100_RXCONF_SetFreqCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TXCONF_SET_FREQ_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_TXCONF_SetFreqCmd_t))) {
                UTRX_AX100_TXCONF_SetFreqCmd((const UTRX_AX100_TXCONF_SetFreqCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_SET_DEFAULT_BAUD_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_SetDefaultBaudCmd_t))) {
                UTRX_AX100_SetDefaultBaudCmd((const UTRX_AX100_SetDefaultBaudCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_RPARAM_SAVE_1_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_RparamSave1Cmd_t))) {
                UTRX_AX100_RparamSave1Cmd((const UTRX_AX100_RparamSave1Cmd_t *)SBBufPtr);
            }
            break;

        case UTRX_RPARAM_SAVE_5_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_RparamSave5Cmd_t))) {
                UTRX_AX100_RparamSave5Cmd((const UTRX_AX100_RparamSave5Cmd_t *)SBBufPtr);
            }
            break;

        case UTRX_RPARAM_SAVE_ALL_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_RparamSaveAllCmd_t))) {
                UTRX_AX100_RparamSaveAllCmd((const UTRX_AX100_RparamSaveAllCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_CHECK_STATE_PING_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_CheckStatePingCmd_t))) {
                UTRX_AX100_CheckStatePingCmd((const UTRX_AX100_CheckStatePingCmd_t *)SBBufPtr);
            }
            break;

        default:
           CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UTRX App: Invalid command code %d", CommandCode);

            memset(UTRX_APP_Data.RptPkt.Report.ReturnValue, 0, sizeof(UTRX_APP_Data.RptPkt.Report.ReturnValue));
            UTRX_APP_Data.RptPkt.Report.CommandCode    = (uint8_t)CommandCode;


            UTRX_APP_Data.RptPkt.Report.ReturnCode = CFE_STATUS_BAD_COMMAND_CODE;
            UTRX_APP_Data.RptPkt.Report.ReturnType = CMD_RETCODE_TYPE_APP;
            UTRX_APP_Data.RptPkt.Report.ReturnDataSize = 0;
            UTRX_CountFromReport();

            
            break;
    }

      CFE_MSG_Init(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader),
                     CFE_SB_ValueToMsgId(UTRX_APP_RPT_TLM_MID),
                     sizeof(UTRX_ReportTlm_t));
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader), true);

    return;
}
/*************************ProcessRequestedTelemetry ***************************/
// MaxBuffer is used to send only the specific data requested by a command.

void UTRX_APP_ProcessRequestedTelemetry(const CFE_SB_Buffer_t *SBBufPtr){
     CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_Status_t rc = CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    if (rc != CFE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_GETFCN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX: Failed to get command code from message (rc=%d)", (int32)rc);

        /* 실패 경로: 데이터 비우고 앱 에러로 기록 */
        memset(UTRX_APP_Data.RptPkt.Report.ReturnValue, 0, sizeof(UTRX_APP_Data.RptPkt.Report.ReturnValue));
        UTRX_APP_Data.RptPkt.Report.ReturnDataSize = 0;
        UTRX_APP_Data.RptPkt.Report.ReturnType = CMD_RETCODE_TYPE_APP;
        UTRX_APP_Data.RptPkt.Report.ReturnCode = rc;
        UTRX_CountFromReport();

        CFE_MSG_Init(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader),
                     CFE_SB_ValueToMsgId(UTRX_APP_RPT_TLM_MID),
                     sizeof(UTRX_ReportTlm_t));
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader), true);
        return;
    }



    switch (CommandCode)
    {
        // case UTRX_GET_STATUS_CONFIGURATION_CC:
        // {
        //     if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetStatusConfigurationCmd_t)))
        //     {
        //         UTRX_Tlm_t *conf_buf = (UTRX_Tlm_t *)&ReportMsg->Data.MaxBuffer;
        //         status = UTRX_GetStatusConfiguration(conf_buf);
        //         ReportMsg->Report.DataSize = sizeof(UTRX_Tlm_t);
        //         OS_printf("[UTRX] active_conf: %u\n", conf_buf->active_conf);
        //         OS_printf("[UTRX] rx_guard   : %u\n", conf_buf->rx_guard);
        //         OS_printf("[UTRX] rx_baudrate: %u\n", conf_buf->rx_baudrate);
        //         OS_printf("[UTRX] tx_baudrate: %u\n", conf_buf->tx_baudrate);

                
        //         statusType = CMD_RETCODE_TYPE_DRIVER;
        //     }
        //     break;

        // }

       case UTRX_RXCONF_GET_BAUD_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetRxBaudCmd_t))) {
                UTRX_AX100_RXCONF_GetBaudCmd((const UTRX_AX100_GetRxBaudCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_RXCONF_GET_GUARD_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetRxGuardCmd_t))) {
                UTRX_AX100_RXCONF_GetGuardCmd((const UTRX_AX100_GetRxGuardCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_RXCONF_GET_FREQ_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetRxFreqCmd_t))) {
                UTRX_AX100_RXCONF_GetFreqCmd((const UTRX_AX100_GetRxFreqCmd_t *)SBBufPtr);
            }
            break;

        /* ---------------- TXCONF ---------------- */
        case UTRX_TXCONF_GET_BAUD_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetTxBaudCmd_t))) {
                UTRX_AX100_TXCONF_GetBaudCmd((const UTRX_AX100_GetTxBaudCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TXCONF_GET_FREQ_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetTxFreqCmd_t))) {
                UTRX_AX100_TXCONF_GetFreqCmd((const UTRX_AX100_GetTxFreqCmd_t *)SBBufPtr);
            }
            break;

        /* ---------------- TLM ---------------- */
        case UTRX_TLM_GET_TEMP_BRD_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetTempBrdCmd_t))) {
                UTRX_AX100_TLM_GetTempBrdCmd((const UTRX_AX100_GetTempBrdCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TLM_GET_LAST_RSSI_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetLastRssiCmd_t))) {
                UTRX_AX100_TLM_GetLastRssiCmd((const UTRX_AX100_GetLastRssiCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TLM_GET_LAST_RFERR_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetLastRferrCmd_t))) {
                UTRX_AX100_TLM_GetLastRferrCmd((const UTRX_AX100_GetLastRferrCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TLM_GET_ACTIVE_CONF_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetActiveConfCmd_t))) {
                UTRX_AX100_TLM_GetActiveConfCmd((const UTRX_AX100_GetActiveConfCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TLM_GET_BOOT_COUNT_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetBootCountCmd_t))) {
                UTRX_AX100_TLM_GetBootCountCmd((const UTRX_AX100_GetBootCountCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TLM_GET_BOOT_CAUSE_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetBootCauseCmd_t))) {
                UTRX_AX100_TLM_GetBootCauseCmd((const UTRX_AX100_GetBootCauseCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TLM_GET_LAST_CONTACT_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetLastContactCmd_t))) {
                UTRX_AX100_TLM_GetLastContactCmd((const UTRX_AX100_GetLastContactCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TLM_GET_TOT_TX_BYTES_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetTotTxBytesCmd_t))) {
                UTRX_AX100_TLM_GetTotTxBytesCmd((const UTRX_AX100_GetTotTxBytesCmd_t *)SBBufPtr);
            }
            break;

        case UTRX_TLM_GET_TOT_RX_BYTES_CC:
            if (UTRX_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(UTRX_AX100_GetTotRxBytesCmd_t))) {
                UTRX_AX100_TLM_GetTotRxBytesCmd((const UTRX_AX100_GetTotRxBytesCmd_t *)SBBufPtr);
            }
            break;

        default:
            CFE_EVS_SendEvent(UTRX_APP_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                              "UTRX App: Invalid command code %d", CommandCode);

            memset(UTRX_APP_Data.RptPkt.Report.ReturnValue, 0, sizeof(UTRX_APP_Data.RptPkt.Report.ReturnValue));
            UTRX_APP_Data.RptPkt.Report.CommandCode    = (uint8_t)CommandCode;


            UTRX_APP_Data.RptPkt.Report.ReturnCode = CFE_STATUS_BAD_COMMAND_CODE;
            UTRX_APP_Data.RptPkt.Report.ReturnType = CMD_RETCODE_TYPE_APP;
            UTRX_APP_Data.RptPkt.Report.ReturnDataSize = 0;
            UTRX_CountFromReport();
            break;


    }

     CFE_MSG_Init(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader),
                     CFE_SB_ValueToMsgId(UTRX_APP_RPT_TLM_MID),
                     sizeof(UTRX_ReportTlm_t));
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(UTRX_APP_Data.RptPkt.TelemetryHeader), true);

    return;

        
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the UTRX    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void UTRX_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_Status_t   rc    = CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    if (rc != CFE_SUCCESS) {
        CFE_EVS_SendEvent(UTRX_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                          "UTRX: Failed to get MsgId (rc=%d)", (int32)rc);

        
        memset(UTRX_APP_Data.RptPkt.Report.ReturnValue, 0, sizeof(UTRX_APP_Data.RptPkt.Report.ReturnValue));
        UTRX_APP_Data.RptPkt.Report.ReturnDataSize = 0;
        UTRX_APP_Data.RptPkt.Report.MsgID      = CFE_SB_MsgIdToValue(MsgId); 
        UTRX_APP_Data.RptPkt.Report.CommandCode= 0;                          
        UTRX_APP_Data.RptPkt.Report.ReturnType = CMD_RETCODE_TYPE_APP;
        UTRX_APP_Data.RptPkt.Report.ReturnCode = rc;
        UTRX_CountFromReport();
        return;
    }

    uint32 mid = CFE_SB_MsgIdToValue(MsgId);

    UTRX_APP_Data.RptPkt.Report.MsgID = mid;


    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case UTRX_APP_CMD_MID:
        CFE_EVS_SendEvent(UTRX_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR, "UTRX: Call ProcessGround Telemetery MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            UTRX_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case UTRX_APP_OIF_MID:
        CFE_EVS_SendEvent(UTRX_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR, "UTRX: Call ProcessRequested Telemetry MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
                              
            UTRX_APP_ProcessRequestedTelemetry(SBBufPtr);
            break;

        case UTRX_APP_SEND_HK_MID:

        CFE_EVS_SendEvent(UTRX_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR, "UTRX: Call Send HK MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            UTRX_ReportHousekeeping();
            break;

        case UTRX_APP_SEND_BCN_MID:

        CFE_EVS_SendEvent(UTRX_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR, "UTRX: Call Sendbcn Mid = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
       
            UTRX_ReportBeacon();
            break;
        

        default:
            CFE_EVS_SendEvent(UTRX_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR, "UTRX: invalid command packet,MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
