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
 *   This file contains the source code for the Strx App.
 */

/*
** Include Files:
*/
#include "strx_app.h"
#include "strx.h"
#include "strx_dispatch.h"
#include "strx_cmds.h"
#include "strx_eventids.h"
#include "strx_msgids.h"
#include "strx_msg.h"
#include "strx_fcncodes.h"
#include <gs/param/rparam.h>


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool STRX_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(STRX_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        STRX_AppData.AppCnt.AppErrCounter ++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* STRX ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void STRX_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    if (CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode) != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(STRX_GETFCN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "STRX: Failed to get command code from message");
        STRX_AppData.AppCnt.AppErrCounter ++;
        return;
    }

    /*
    ** Process STRX app ground commands
    */
    switch (CommandCode)
    {
        case STRX_NOOP_CC:
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoopCmd_t)))
            {
                STRX_NoopCmd((const STRX_NoopCmd_t *)SBBufPtr);
            }
            break;

        case STRX_RESET_COUNTERS_CC:
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_ResetCountersCmd_t)))
            {
                STRX_ResetCountersCmd((const STRX_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

       case STRX_RESET_APP_CMD_COUNTERS_CC:
           if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_ResetAppCmdCountersCmd_t))) 
           {
              STRX_ResetAppCmdCountersCmd((const STRX_ResetAppCmdCountersCmd_t *)SBBufPtr);
           }
           break;

        case STRX_RESET_DEVICE_CMD_COUNTERS_CC:
           if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_ResetDeviceCmdCountersCmd_t))) 
           {
              STRX_ResetDeviceCmdCountersCmd((const STRX_ResetDeviceCmdCountersCmd_t *)SBBufPtr);
           }
           break;

        case STRX_GNDWDT_CLEAR_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_GndwdtClearCmd();
            }
            break;
        }

        case STRX_REBOOT_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RebootCmd();
            }
            break;
        }

        case STRX_RXCONF_SET_BAUD_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_U32ArgsCmd_t)))
            {
                STRX_RXCONF_SetBaudCmd((STRX_U32ArgsCmd_t *) SBBufPtr);
            }
            break;
        }

        case STRX_TXCONF_SET_BAUD_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_U32ArgsCmd_t)))
            {
                STRX_TXCONF_SetBaudCmd((STRX_U32ArgsCmd_t *) SBBufPtr);
            }
            break;
        }

          case STRX_RXCONF_SET_FREQ_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_U32ArgsCmd_t)))
            {
                STRX_RXCONF_SetFreqCmd((STRX_U32ArgsCmd_t *) SBBufPtr);
            }
            break;
        }

          case STRX_TXCONF_SET_FREQ_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_U32ArgsCmd_t)))
            {
                STRX_TXCONF_SetFreqCmd((STRX_U32ArgsCmd_t *) SBBufPtr);
            }
            break;

        }

        case STRX_TLM_SET_KISS_USART_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_8ArgsCmd_t)))
            {
                STRX_TLM_SET_KISS_USARTCmd((STRX_8ArgsCmd_t *) SBBufPtr);
            }
            break;
        }
        case STRX_TLM_SET_GOSH_USART_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_U8ArgsCmd_t)))
            {
                STRX_TLM_SET_GOSH_USARTTCmd((STRX_U8ArgsCmd_t *) SBBufPtr);
            }
            break;
        }

        case STRX_SET_DEFAULT_BAUD_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_SetDefaultBaudCmd();
            }
            break;
        }

        case STRX_RPARAM_SAVE_0_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RparamSave0Cmd();
            }
            break;
        }

        case STRX_RPARAM_SAVE_1_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RparamSave1Cmd();
            }
            break;
        }

        case STRX_RPARAM_SAVE_4_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RparamSave4Cmd();
            }
            break;
        }

        case STRX_RPARAM_SAVE_5_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RparamSave5Cmd();
            }
            break;
        }

        case STRX_RPARAM_SAVE_ALL_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RparamSaveAllCmd();
            }
            break;
        }

        case STRX_CHECK_STATE_PING_CC:
        {

        if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                csp_checkstate_pingCmd();
            }
            break;
        }

        case STRX_RXCONF_GET_BAUD_CC: //Using for Request Telemetry
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RXCONF_GetBaudCmd();
            }
            break;
        }
        case STRX_RXCONF_GET_GUARD_CC:
        {
              if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RXCONF_GetGuardCmd();
            }
            break;
        }

        case STRX_RXCONF_GET_FREQ_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_RXCONF_GetFreqCmd();
            }
            break;
        }

        case STRX_TXCONF_GET_BAUD_CC:
        {
              if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TXCONF_GetBaudCmd();
            }
            break;

        }

          case STRX_TXCONF_GET_FREQ_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TXCONF_GetFreqCmd();
            }
            break;
        }

        case STRX_TLM_GET_TEMP_BRD_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GetTempBrdCmd();
            }
            break;
        }

        case STRX_TLM_GET_LAST_RSSI_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GetLastRssiCmd();
            }
            break;
        }

        case STRX_TLM_GET_LAST_RFERR_CC:
        {
             if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GetLastRferrCmd();
            }
            break;

        }

        case STRX_TLM_GET_BOOT_COUNT_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GetBootCountCmd();
            }
            break;
        }

        case STRX_TLM_GET_BOOT_CAUSE_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GetBootCauseCmd();
            }
            break;
        }

        case STRX_TLM_GET_LAST_CONTACT_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GetLastContactCmd();
            }
            break;

        }

        case STRX_TLM_GET_TOT_TX_BYTES_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GetTotTxBytesCmd();
            }
            break;

        }

        case STRX_TLM_GET_TOT_RX_BYTES_CC:
        {
            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GetTotRxBytesCmd();
            }
            break;
        }
        case STRX_TLM_RXMODE_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {   
                STRX_TLM_GET_RXMODECmd();
            }
            break;
        }
        case STRX_TLM_GET_GNDWDT_CNT_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GET_GND_WDT_CNTCmd();
            }
            break;
        }
        case STRX_TLM_GET_GNDWDT_LEFT_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GET_GND_WDT_LEFTCmd();
            }
            break;

        }
        case STRX_TLM_GET_KISS_USART_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GET_KISS_USARTCmd();
            }
            break;

        }
        case STRX_TLM_GET_GOSH_USART_CC:
        {

            if (STRX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STRX_NoArgsCmd_t)))
            {
                STRX_TLM_GET_GOSH_USARTCmd();
            }
            break;
        }
   
        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(STRX_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);

            memset(STRX_AppData.RptPkt.Report.ReturnValue, 0, sizeof(STRX_AppData.RptPkt.Report.ReturnValue));
            STRX_AppData.RptPkt.Report.CommandCode    = (uint8_t)CommandCode;

            STRX_AppData.RptPkt.Report.ReturnCode = CFE_STATUS_BAD_COMMAND_CODE;
            STRX_AppData.RptPkt.Report.ReturnType = CMD_RETCODE_TYPE_APP;
            STRX_AppData.RptPkt.Report.ReturnDataSize = 0;
            STRX_CountFromReport();

            CFE_MSG_Init(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader),
                            CFE_SB_ValueToMsgId(STRX_RPT_TLM_MID),
                            sizeof(STRX_ReportTlm_t));
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader));
            CFE_SB_TransmitMsg(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader), true);

            break;
    }
    
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the STRX    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void STRX_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
    
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case STRX_CMD_MID:
            STRX_ProcessGroundCommand(SBBufPtr);
            break;

        case STRX_SEND_HK_MID:
            STRX_ReportHousekeeping();
            break;

        case STRX_SEND_BCN_MID:
            STRX_ReportBeacon();
            break;

        default:
            CFE_EVS_SendEvent(STRX_MID_ERR_EID, CFE_EVS_EventType_ERROR, "STRX: invalid command packet,MID = 0x%x",
                              (unsigned int)CFE_SB_MsgIdToValue(MsgId));

            memset(STRX_AppData.RptPkt.Report.ReturnValue, 0, sizeof(STRX_AppData.RptPkt.Report.ReturnValue));
            STRX_AppData.RptPkt.Report.CommandCode    = 0;

            STRX_AppData.RptPkt.Report.ReturnCode = CFE_STATUS_UNKNOWN_MSG_ID;
            STRX_AppData.RptPkt.Report.ReturnType = CMD_RETCODE_TYPE_APP;
            STRX_AppData.RptPkt.Report.ReturnDataSize = 0;
            STRX_CountFromReport();

            CFE_MSG_Init(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader),
                            CFE_SB_ValueToMsgId(STRX_RPT_TLM_MID),
                            sizeof(STRX_ReportTlm_t));
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader));
            CFE_SB_TransmitMsg(CFE_MSG_PTR(STRX_AppData.RptPkt.TelemetryHeader), true);

            break;


            break;
    }
}
