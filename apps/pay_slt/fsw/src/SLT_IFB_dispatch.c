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
 *   This file contains the source code for the Sample App.
 */

/*
** Include Files:
*/
#include "SLT_IFB_app.h"
#include "SLT_IFB_dispatch.h"
#include "SLT_IFB_cmds.h"
#include "SLT_IFB_eventids.h"
#include "SLT_IFB_msgids.h"
#include "SLT_IFB_msg.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool SLT_IFB_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(SLT_IFB_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        SLT_IFB_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void SLT_IFB_ProcessBasicCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process SAMPLE app ground commands
    */
    switch (CommandCode)
    {
        case SLT_IFB_NOOP_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_NoopCmd_t)))
            {
                SLT_IFB_NoopCmd((const SLT_IFB_NoopCmd_t *)SBBufPtr);
            }
            break;

        case SLT_IFB_RESET_COUNTERS_CC:
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_ResetCountersCmd_t)))
            {
                SLT_IFB_ResetCountersCmd((const SLT_IFB_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(SLT_IFB_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

void SLT_IFB_ProcessTransactionCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;
    
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);
    
    switch (CommandCode)
    {
        case SLT_IFB_CSP_CMP_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_TransactionCmd_t)))
            {
                SLT_IFB_CSP_CMP_Cmd();
            }
            break;
        
        case SLT_IFB_CSP_PING_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_TransactionCmd_t)))
            {
                SLT_IFB_CSP_PING_Cmd();
            }    
            break;
        
        case SLT_IFB_CSP_PS_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_TransactionCmd_t)))
            {
                SLT_IFB_CSP_PS_Cmd();
            }
            break;
        
        case SLT_IFB_CSP_MEM_FREE_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_TransactionCmd_t)))
            {
                SLT_IFB_CSP_MEM_FREE_Cmd();
            }
            break;
        
        case SLT_IFB_CSP_REBOOT_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_TransactionCmd_t)))
            {
                SLT_IFB_CSP_REBOOT_Cmd();
            }
            break;
        
        case SLT_IFB_CSP_BUF_FREE_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_TransactionCmd_t)))
            {
                SLT_IFB_CSP_BUF_FREE_Cmd();
            }    
            break;
            
        case SLT_IFB_CSP_UPTIME_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_TransactionCmd_t)))
            {
                SLT_IFB_CSP_UPTIME_Cmd();
            }    
            break;
        
        case SLT_IFB_CSP_GNDWDT_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_TransactionCmd_t)))
            {
                SLT_IFB_CSP_GNDWDT_Cmd();
            }    
            break;
        
    }
}

void SLT_IFB_ProcessGetParamCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;
    
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);
    
    switch (CommandCode)
    {
        case SLT_IFB_GET_BRD_UID_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {   
                char brd_uid[16] = {0,};
                SLT_IFB_GET_BRD_UID_Cmd(brd_uid);
                
            }
            break;
        
        case SLT_IFB_GET_BRD_REV_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {   
                uint8 brd_rev[3] = {0,};
                SLT_IFB_GET_BRD_REV_Cmd(brd_rev);
            }    
            break;
        
        case SLT_IFB_GET_CSP_ADDR_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {   
                uint8 csp_addr_mpu = 0;
                SLT_IFB_GET_CSP_ADDR_Cmd(&csp_addr_mpu);
            }    
            break;

        case SLT_IFB_GET_CAN_SPEED_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                uint16 can_speed = 0;
                SLT_IFB_GET_CAN_SPEED_Cmd(&can_speed);
            }
            break;
        
        case SLT_IFB_GET_I2C_ADDR_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                uint8 i2c_address = 0;
                SLT_IFB_GET_I2C_ADDR_Cmd(&i2c_address);
            }
            break;
        
        case SLT_IFB_GET_WDT_VAL_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                uint32 wdt_value = 0;
                SLT_IFB_GET_WDT_VAL_Cmd(&wdt_value);
            }
            break;
        
        case SLT_IFB_GET_CSP_RTABLE_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                char csp_routing_table[96] = {0, };
                SLT_IFB_GET_CSP_RTABLE_Cmd(csp_routing_table);
            }
            break;
        
        case SLT_IFB_GET_SYS_STATUS_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_SYS_STATUS_Cmd(&SLT_IFB_Data.sys_status);
            }
            break;

        case SLT_IFB_GET_SYS_UPTIME_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_SYS_UPTIME_Cmd(&SLT_IFB_Data.sys_uptime);
            }
            break;

        case SLT_IFB_GET_BOOT_CNT_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_BOOT_CNT_Cmd(&SLT_IFB_Data.boot_cnt);
            }
            break;
        
        case SLT_IFB_GET_BOOT_CAUSE_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_BOOT_CAUSE_Cmd(&SLT_IFB_Data.boot_cause);
            }
            break;

        case SLT_IFB_GET_REBOOT_CAUSE_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_REBOOT_CAUSE_Cmd(&SLT_IFB_Data.reboot_cause);
            }
            break;

        case SLT_IFB_GET_WDT_LEFT_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_WDT_LEFT_Cmd(&SLT_IFB_Data.wdt_left);
            }
            break;
        
        case SLT_IFB_GET_BRD_TEMP_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_BRD_TEMP_Cmd(&SLT_IFB_Data.brd_temp);
            }
            break;
        
        case SLT_IFB_GET_PWR_CURRENT_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_PWR_CURRENT_Cmd(&SLT_IFB_Data.pwr_current);
            }
            break;
        
        case SLT_IFB_GET_IMU_DATA_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_IMU_DATA_Cmd(SLT_IFB_Data.imu_data);
            }
            break;

        case SLT_IFB_GET_NTC_DATA_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_RPARAM_GetCmd_t)))
            {
                SLT_IFB_GET_NTC_DATA_Cmd(SLT_IFB_Data.ntc_data);
            }
            break;
    }
}

void SLT_IFB_ProcessSaveTableCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;
    
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);
    
    switch (CommandCode)
    {
        case SLT_IFB_SAVE_TABLE0_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_SaveCmd_t)))
            {
                SLT_IFB_SAVE_TABLE0_Cmd();
            }
            break;
        
        case SLT_IFB_SAVE_TABLE1_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_SaveCmd_t)))
            {
                SLT_IFB_SAVE_TABLE1_Cmd();
            }
            break;
        
        case SLT_IFB_SAVE_TABLE4_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_SaveCmd_t)))
            {
                SLT_IFB_SAVE_TABLE4_Cmd();
            }
            break;
        
        case SLT_IFB_SAVE_ALL_TABLE_CC :
            if (SLT_IFB_VerifyCmdLength(&SBBufPtr->Msg, sizeof(SLT_IFB_SaveCmd_t)))
            {
                SLT_IFB_SAVE_ALL_TABLE_Cmd();
            }
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SAMPLE    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SAMPLE_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case SLT_IFB_CMD_MID:
            SLT_IFB_ProcessBasicCommand(SBBufPtr);
            break;

        case SLT_IFB_TRANSACTION_MID :
            SLT_IFB_ProcessTransactionCommand(SBBufPtr);
            break;    
        
        case SLT_IFB_GET_RPARAM_MID :
            SLT_IFB_ProcessGetRParamCommand(SBBufPtr);
            break;
            
        case SLT_IFB_SAVE_TABLE_MID :
            SLT_IFB_ProcessSaveTableCommand(SBBufPtr);
            break;
            
        case SLT_IFB_SEND_HK_MID:
            SLT_IFB_SendHkCmd((const SLT_IFB_SendHkCmd_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(SLT_IFB_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
