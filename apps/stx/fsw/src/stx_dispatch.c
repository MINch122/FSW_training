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
#include "stx.h"
#include "stx_dispatch.h"
#include "stx_cmds.h"
#include "stx_eventids.h"
#include "stx_msgids.h"
#include "stx_msg.h"
#include "esup.h"
#include <stdio.h>
#include <stdint.h>
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool STX_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(STX_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        STX_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE ground commands                                                     */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

/* basic */
void STX_Basic_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr){
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process SAMPLE app ground commands
    */
    switch (CommandCode)
    {
        /********************************************************************************************************************* */
        case STX_NOOP_CC:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_NoopCmd_t)))
            {
                STX_NoopCmd((const STX_NoopCmd_t *)SBBufPtr);
            }
            break;

        case STX_RESET_COUNTERS_CC:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_ResetCountersCmd_t)))
            {
                STX_ResetCountersCmd((const STX_ResetCountersCmd_t *)SBBufPtr);
            }
            break;

        case STX_PARAM_INIT_CC :
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_ParamInitCmd_t))){
                STX_ParamInitCmd((const STX_ParamInitCmd_t *)SBBufPtr);
            }
            break;

        case STX_MODULE_ID_INIT_CC :
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_ModuleIdInitCmd_t))){
                STX_ModuleIdInitCmd((const STX_ModuleIdInitCmd_t *)SBBufPtr);
            }
            break;

        /********************************************************************************************************************** */
        case STX_SET_SYMBOLRATE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_SYMBOLRAtE_t)))
            {
                STX_SET_SYMBOLRATECmd((const STX_Set_SYMBOLRAtE_t *)SBBufPtr);          
            }
            break;

        case STX_SET_TRANSMITPW:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_TRANSMITPW_t)))
            {
                STX_Set_TRANSMITPWCmd((const STX_Set_TRANSMITPW_t *)SBBufPtr);
            }
            break;


        case STX_SET_CENTERFREQ:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_CENTERFREQ_t)))
            {
                STX_Set_CENTERFREQCmd((const STX_Set_CENTERFREQ_t *)SBBufPtr);
            }
            break;


        case STX_SET_MODCOD:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_MODCOD_t)))
            {

                STX_Set_MODCODCmd((const STX_Set_MODCOD_t *)SBBufPtr);
            }
            break;


        case STX_SET_ROLLOFF:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_ROLLOFF_t)))
            {
               
                STX_Set_ROLLOFFCmd((const STX_Set_ROLLOFF_t *)SBBufPtr);
            }
            break;


        case STX_SET_PILOTSIG:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_PILOTSIG_t)))
            {
                
                STX_Set_PILOTSIGCmd((const STX_Set_PILOTSIG_t *)SBBufPtr);

            }
            break;

        case STX_SET_FECFRAMESZ:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_FECFRAME_t)))
            {
              
                STX_Set_FECFRAMECmd((const STX_Set_FECFRAME_t *)SBBufPtr);
            }
            break;

        case STX_SET_PRETX_DELAY:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_PRETX_DELAY_t)))
            {
               
                STX_Set_PRETX_DELAYCmd((const STX_Set_PRETX_DELAY_t *)SBBufPtr);
            }
            break;

        case STX_SET_ALL_PRAMETERS:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_ALLPRAM_t)))
            {

               STX_Set_ALLPRAMCmd((const STX_Set_ALLPRAM_t *)SBBufPtr);
            }
            break;

        case STX_SET_RS485BAUD:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_RS485_t)))
            {
               STX_Set_RS485Cmd((const STX_Set_RS485_t *)SBBufPtr);
            }
            break;

        case STX_SET_MODULATOR_DATA_INTERFACE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_MODULATION_INTERFACE_t)))
            {
                STX_Set_MODULATION_INTERFACECmd((const STX_Set_MODULATION_INTERFACE_t *)SBBufPtr);
            }
            break;

        /******************************************************************************************************************************* */

        case STX_FILESYS_CC_DIR:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_t)))
            {
                STX_DIRCmd();

            }
            break;

        case STX_FILESYS_CC_DIRNEXT:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_t)))
            {
               STX_DIRNEXTCmd();
            }
            break;

        case STX_FILESYS_CC_DELFILE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_DELFILE_t)))
            {
              STX_DELFILECmd((const STX_DELFILE_t *)SBBufPtr);
            }
            break;

        case STX_FILESYS_CC_DELALLFILE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_t)))
            {
              STX_DELALLFILECmd();
            }
            break;

        case STX_FILESYS_CC_CREATEFILE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_CREATEFILE_t)))
            {
              STX_CREATEFILECmd((const STX_CREATEFILE_t *)SBBufPtr);
            }
            break;

        case STX_FILESYS_CC_WRITEFILE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_WRITEFILE_t)))
            {
                STX_WRITEFILECmd((const STX_WRITEFILE_t *)SBBufPtr);
            }
            break;

        case STX_FILESYS_CC_OPENFILE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_OPENFILE_t)))
            {
                STX_OPENFILECmd((const STX_OPENFILE_t *)SBBufPtr);
            }
            break;

        case STX_FILESYS_CC_READFILE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_READFILE_t)))
            {
                STX_READFILECmd((const STX_READFILE_t *)SBBufPtr);
            }
            break;

        case STX_FILESYS_CC_SENDFILE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_SENDFILE_t)))
            {
                STX_SENDFILECmd((const STX_SENDFILE_t *)SBBufPtr);
            }
            break;
        
        case STX_FILESYS_CC_SENDFILERTI:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_SENDFILE_t)))
            {
                STX_SENDFILE_WITH_ERROR_Cmd((const STX_SENDFILE_t *)SBBufPtr);
            }

        /**************************************************************************************************** */

        case STX_SYSCONF_CC_TRANSMITMODE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_t)))
            {
              STX_SYSCONF_CC_TRANSMITMODECmd();
            }
            break;

        case STX_SYSCONF_CC_IDLEMODE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_t)))
            {
               STX_SYSCONF_CC_IDLEMODECmd();
            }
            break;

        case STX_SYSCONF_CC_SAFESHUTDOWN:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_t)))
            {
                STX_SYSCONF_CC_SAFESHUTDOWNCmd();
            }
            break;

        // case STX_GETRES_CC_GETRES:
        //     if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_t)))
        //     {
        //         const STX_Set_t *cmd = (const STX_Set_t *)SBBufPtr;
        //         status  = ESUP_INSIG;
        //         command = GETRES_CC_GETRES;
        //         type    = CONFIG_TP_MODULATORDTIFC;

        //         ESUP(status, command, type, (void *)&cmd->Payload.data, (uint16_t)cmd->Payload.length);
        //     }
        //     break;
        /* default case already found during FC vs length test */

        case STX_SYSCONF_CC_UPDATEFW:
            // if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Set_t)))
            // {
            //     const STX_Set_t *cmd = (const STX_Set_t *)SBBufPtr;

            //     status  = ESUP_INSIG;
            //     command = SYSCONF_CC_UPDATEFW;
            //     type    = SYSCONF_TP_NA;

            //     ESUP(status, command, type, (void *)&cmd->Payload.data, (uint16_t)cmd->Payload.length);
            // }
            break;

        /************************************************************************************************************* */

        case STX_GET_SYMBOL_RATE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {

                STX_GET_SYMBOL_RATECmd();
            }
            break;

        case STX_GET_TX_POWER:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {

                STX_GET_TX_POWERCmd();

            }
            break;

        case STX_GET_CENTER_FREQ:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {

                STX_GET_CENTER_FREQCmd();

            }
            break;

        case STX_GET_MODCOD:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {
                STX_GET_MODCODCmd();
            }
            break;

        case STX_GET_ROLL_OFF:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {
                STX_GET_ROLL_OFFCmd();
            }
            break;

        case STX_GET_PILOT_SIGNAL:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {
               STX_GET_PILOT_SIGNALCmd();
            }
            break;

        case STX_GET_FEC_FRAME_SIZE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {
               STX_GET_FEC_FRAME_SIZECmd();
            }
            break;

        case STX_GET_PRETX_DELAY:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {
                STX_GET_PRETX_DELAYCmd();
            }
            break;

        case STX_GET_ALL_PRAMETERS:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {
               STX_GET_ALL_PRAMETERSCmd();
            }
            break;

        case STX_GET_REPORT:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {
              STX_GET_REPORTCmd();
            }
            break;

        case STX_GET_MODULATOR_DATA_INTERFACE:
            if (STX_VerifyCmdLength(&SBBufPtr->Msg, sizeof(STX_Get_t))) {
               STX_GET_MODULATOR_DATA_INTERFACECmd();
            }
            break;
            
        default:
            CFE_EVS_SendEvent(STX_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }

    STX_Data.CmdCounter ++;

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet 1that is received on the SAMPLE   */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void STX_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case STX_CMD_MID:
            STX_Basic_ProcessGroundCommand(SBBufPtr);
            break;
        case STX_SEND_HK_MID:
            STX_SendHkCmd();
            break;
        case STX_SEND_BCN_MID:
            STX_SendBCNCmd();
            break;

        default:
            CFE_EVS_SendEvent(STX_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}


