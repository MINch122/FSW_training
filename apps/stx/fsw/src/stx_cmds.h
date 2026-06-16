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
 * @file
 *   This file contains the prototypes for the Sample App Ground Command-handling functions
 */

#ifndef STX_CMDS_H
#define STX_CMDS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "stx_msg.h"

void STX_SendHkCmd(void);
void STX_SendBCNCmd(void);
CFE_Status_t STX_ResetCountersCmd(const STX_ResetCountersCmd_t *Msg);
CFE_Status_t STX_NoopCmd(const STX_NoopCmd_t *Msg);
CFE_Status_t STX_SetModuleIdCmd(const STX_SetModuleIdCmd_t *Msg);

/* SET COMMAND */
void STX_SET_SYMBOLRATECmd(const STX_Set_SYMBOLRAtE_t *Msg);
void STX_Set_TRANSMITPWCmd(const STX_Set_TRANSMITPW_t *Msg);
void STX_Set_CENTERFREQCmd(const STX_Set_CENTERFREQ_t *Msg);
void STX_Set_MODCODCmd(const STX_Set_MODCOD_t *Msg);
void STX_Set_ROLLOFFCmd(const STX_Set_ROLLOFF_t *Msg);
void STX_Set_PILOTSIGCmd(const STX_Set_PILOTSIG_t *Msg);
void STX_Set_FECFRAMECmd(const STX_Set_FECFRAME_t *Msg);
void STX_Set_PRETX_DELAYCmd(const STX_Set_PRETX_DELAY_t *Msg);
void STX_Set_ALLPRAMCmd(const STX_Set_ALLPRAM_t *Msg);
void STX_Set_RS485Cmd(const STX_Set_RS485_t *Msg);
void STX_Set_MODULATION_INTERFACECmd(const STX_Set_MODULATION_INTERFACE_t *Msg);

/* FILE COMMAND */
void STX_DIRCmd(void);
void STX_DIRNEXTCmd(void);
void STX_DELFILECmd(const STX_DELFILE_t *Msg);
void STX_DELALLFILECmd(void);
void STX_CREATEFILECmd(const STX_CREATEFILE_t *Msg);
void STX_WRITEFILECmd(const STX_WRITEFILE_t *Msg);
void STX_OPENFILECmd(const STX_OPENFILE_t *Msg);
void STX_READFILECmd(const STX_READFILE_t *Msg);
void STX_SENDFILECmd(const STX_SENDFILE_t *Msg);
void STX_SENDFILE_WITH_ERROR_Cmd(const STX_SENDFILE_t * Msg);

/* MODE COMMAND */  
void STX_SYSCONF_CC_TRANSMITMODECmd(void);
void STX_SYSCONF_CC_IDLEMODECmd(void);
void STX_SYSCONF_CC_SAFESHUTDOWNCmd(void);

/* Get command*/
void STX_GET_SYMBOL_RATECmd(void);
void STX_GET_TX_POWERCmd(void);
void STX_GET_CENTER_FREQCmd(void);
void STX_GET_MODCODCmd(void);
void STX_GET_ROLL_OFFCmd(void);
void STX_GET_PILOT_SIGNALCmd(void);
void STX_GET_FEC_FRAME_SIZECmd(void);
void STX_GET_PRETX_DELAYCmd(void);
void STX_GET_ALL_PRAMETERSCmd(void);
void STX_GET_REPORTCmd(void);
void STX_GET_MODULATOR_DATA_INTERFACECmd(void);


void STX_Param_init(void);

#endif /* STX_CMDS_H */
