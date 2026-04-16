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
 *   This file contains the source code for the Adcs App.
 */

/*
** Include Files:
*/
#include "adcs2_app.h"
#include "adcs2_dispatch.h"
#include "adcs2_cmds.h"
#include "adcs2_msgids.h"
#include "adcs2_msg.h"
#include "adcs2_utils.h"
#include "adcs2_eventids.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length											   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool ADCS2_VerifyCmdLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
	bool			  result	   = true;
	size_t			ActualLength = 0;
	CFE_SB_MsgId_t	MsgId		= CFE_SB_INVALID_MSG_ID;
	CFE_MSG_FcnCode_t FcnCode	  = 0;

	CFE_MSG_GetSize(MsgPtr, &ActualLength);

	/*
	** Verify the command packet length.
	*/
	if (ExpectedLength != ActualLength)
	{
		CFE_MSG_GetMsgId(MsgPtr, &MsgId);
		CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

		CFE_EVS_SendEvent(ADCS2_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
						  "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
						  (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
						  (unsigned int)ExpectedLength);

		result = false;

		ADCS2_AppData.ErrCounter++;

		/* RPT */
		ADCS2_ReportTlm_t *BufPtr = (ADCS2_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(ADCS2_ReportTlm_t));
		if (BufPtr == NULL) goto cleanup;

		if (CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(ADCS2_REPORT_TLM_MID),
		sizeof(ADCS2_ReportTlm_t)) != CFE_SUCCESS) {
			CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
			goto cleanup;
		}
		BufPtr->Report.MsgID = (uint16_t)CFE_SB_MsgIdToValue(MsgId);
		BufPtr->Report.CommandCode = (uint8_t)FcnCode;
		BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
		BufPtr->Report.ReturnCode = CFE_STATUS_WRONG_MSG_LENGTH; // Error code of `Length error`
		BufPtr->Report.ReturnDataSize = 2 * sizeof(uint32_t);
		
		uint32_t Temp32 = (uint32_t)ActualLength;
		memcpy(BufPtr->Report.ReturnValue, &Temp32, sizeof(uint32_t));
		Temp32 = (uint32_t)ExpectedLength;
		memcpy(BufPtr->Report.ReturnValue + sizeof(uint32_t), &Temp32, sizeof(uint32_t));

		CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
		if (CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
			CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
			goto cleanup;
		}
		/* End of RPT */
	}
cleanup:
	return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ADCS ground commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void ADCS2_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr)
{
	CFE_MSG_FcnCode_t CommandCode = 0;

	CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

	/*
	** Process ADCS app ground commands
	*/
	switch (CommandCode)
	{
		case ADCS2_NOOP_CC:
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_NoopCmd_t)))
			{
				ADCS2_NoopCmd((const ADCS2_NoopCmd_t *)SBBufPtr);
			}
			break;

		case ADCS2_RESET_COUNTERS_CC:
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_ResetCountersCmd_t)))
			{
				ADCS2_ResetCountersCmd((const ADCS2_ResetCountersCmd_t *)SBBufPtr);
			}
			break;
		
		/*
		* ADCS Telecommand
		*/
		case ADCS2_SET_RESET_CC:
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_ResetCmd_t)))
			{
				ADCS2_SetReset();
			}
			break;

		case ADCS2_SET_CURRENT_UNIX_TIME_CC:
			// ID 2
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_CurrentUnixTimeCmd_t))) {
				ADCS2_SetCurrentUnixTimeCmd((const ADCS2_CurrentUnixTimeCmd_t *)SBBufPtr);
			}
			break;

        case ADCS2_SET_PERSIST_CONFIG_CC:
            // ID 7
            if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_PersistConfigCmd_t))) {
                ADCS2_SetPersistConfigCmd((const ADCS2_PersistConfigCmd_t *)SBBufPtr);
            }
            break;
        
		case ADCS2_SET_CONTROL_ESTIMATION_MODE_CC:
			// ID 42
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_ControlEstimationModeCmd_t))) {
				ADCS2_SetControlEstimationModeCmd((const ADCS2_ControlEstimationModeCmd_t *)SBBufPtr);
			}
			break;

		case ADCS2_SET_POWER_STATE_CC:
			// ID 56
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_PowerStateCmd_t))) {
				ADCS2_SetPowerStateCmd((const ADCS2_PowerStateCmd_t *)SBBufPtr);
			}
			break;

		case ADCS2_SET_MOUNTING_CONFIG_CC:
			// ID 65
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_MountingConfigCmd_t))) {
				ADCS2_SetMountingConfigCmd((const ADCS2_MountingConfigCmd_t *)SBBufPtr);
			}
			break;


		/* 
		* ADCS Telemetry
		*/		
		case ADCS2_GET_CURRENT_UNIX_TIME_CC:
			// ID 133
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_GetCurrentUnixTimeCmd_t))) {
				ADCS2_GetCurrentUnixTimeCmd();
			}
			break;

        case ADCS2_GET_CONTROL_ESTIMATION_MODE_CC:
            // ID 150
            if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_GetControlEstimationModeCmd_t))) {
                ADCS2_GetControlEstimationModeCmd();
            }
            break;

        case ADCS2_GET_RAW_MAG_SENSOR_CC:
            // ID 180
            if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_GetRawMAGSensorCmd_t))) {
                ADCS2_GetRawMAGSensorCmd();
            }
            break;

        case ADCS2_GET_POWER_STATE_CC:
            // ID 183
            if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_GetPowerStateCmd_t))) {
                ADCS2_GetPowerStateCmd();
            }
            break;
        
        case ADCS2_GET_MOUNTING_CONFIG_CC:
            // ID 193
            if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_GetMountingConfigCmd_t))) {
                ADCS2_GetMountingConfigCmd();
            }
            break;

        case ADCS2_GET_RAW_GYR_SENSOR_CC:
            // ID 204
            if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_GetRawGYRSensorCmd_t))) {
                ADCS2_GetRawGYRSensorCmd();
            }
            break;

		/*
		* ADCS Commissioning Sequence
		*/
		case ADCS2_COMM_01_CC:
			// Determine initial angular rates
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm01Cmd_t))) {
				ADCS2_Comm01Cmd((const ADCS2_Comm01Cmd_t *)SBBufPtr);
			}
			break;

		case ADCS2_COMM_02_CC:
			// Initial safe detumbling
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm02Cmd_t))) {
				ADCS2_Comm02Cmd((const ADCS2_Comm02Cmd_t *)SBBufPtr);
			}
			break;

		case ADCS2_COMM_03_CC:
			// Magnetometer calibration
			if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm03Cmd_t))) {
				ADCS2_Comm03Cmd((const ADCS2_Comm03Cmd_t *)SBBufPtr);
			}
			break;

		// case ADCS2_COMM_04_CC:
		// 	// Magnetometer calibration and deployment
		// 	if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm04Cmd_t))) {
		// 		ADCS2_Comm04Cmd((const ADCS2_Comm04Cmd_t *)SBBufPtr);
		// 	}
		// 	break;

		// case ADCS2_COMM_05_CC:
		// 	// EKF commissioning
		// 	if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm05Cmd_t))) {
		// 		ADCS2_Comm05Cmd((const ADCS2_Comm05Cmd_t *)SBBufPtr);
		// 	}
		// 	break;

		// case ADCS2_COMM_06_CC:
		// 	// CSS commissioning
		// 	if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm06Cmd_t))) {
		// 		ADCS2_Comm06Cmd((const ADCS2_Comm06Cmd_t *)SBBufPtr);
		// 	}
		// 	break;

		// case ADCS2_COMM_07_CC:
		// 	// FSS commissioning
		// 	if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm07Cmd_t))) {
		// 		ADCS2_Comm07Cmd((const ADCS2_Comm07Cmd_t *)SBBufPtr);
		// 	}
		// 	break;

		// case ADCS2_COMM_08_CC:
		// 	// 3-axis Reaction wheel commissioning
		// 	if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm08Cmd_t))) {
		// 		ADCS2_Comm08Cmd((const ADCS2_Comm08Cmd_t *)SBBufPtr);
		// 	}
		// 	break;

		// case ADCS2_COMM_09_CC:
		// 	// 3-axis Reaction wheel control modes
		// 	if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm09Cmd_t))) {
		// 		ADCS2_Comm09Cmd((const ADCS2_Comm09Cmd_t *)SBBufPtr);
		// 	}
		// 	break;

		// case ADCS2_COMM_10_CC:
		// 	// HSS commissioning
		// 	if (ADCS2_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ADCS2_Comm10Cmd_t))) {
		// 		ADCS2_Comm10Cmd((const ADCS2_Comm10Cmd_t *)SBBufPtr);
		// 	}
		// 	break;


		
		/* default case already found during FC vs length test */
		default:
			CFE_EVS_SendEvent(ADCS2_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
							  CommandCode);


			/* RPT */
			ADCS2_ReportTlm_t *BufPtr = (ADCS2_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(ADCS2_ReportTlm_t));
			if (BufPtr == NULL) break;
			if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(ADCS2_REPORT_TLM_MID), sizeof(ADCS2_ReportTlm_t) != CFE_SUCCESS)) {
				CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
				break;
			}
			BufPtr->Report.MsgID = ADCS2_CMD_MID;
			BufPtr->Report.CommandCode = (uint8_t)CommandCode;
			BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
			BufPtr->Report.ReturnCode = CFE_STATUS_BAD_COMMAND_CODE;
			BufPtr->Report.ReturnDataSize = 0;
			CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
			if(CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
				CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
				break;
			}
			/* End of RPT */
			break;
	}
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:																  */
/*	 This routine will process any packet that is received on the ADCS2	      */
/*	 command pipe.                                                            */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void ADCS2_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
	CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

	CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

	switch (CFE_SB_MsgIdToValue(MsgId))
	{
		case ADCS2_CMD_MID:
			ADCS2_ProcessGroundCommand(SBBufPtr);
			break;

		default:
			CFE_EVS_SendEvent(ADCS2_MID_ERR_EID, CFE_EVS_EventType_ERROR,
							  "ADCS: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));


			 /* RPT */
			ADCS2_ReportTlm_t *BufPtr = (ADCS2_ReportTlm_t *)CFE_SB_AllocateMessageBuffer(sizeof(ADCS2_ReportTlm_t));
			if (BufPtr == NULL) break;
			if(CFE_MSG_Init(CFE_MSG_PTR(BufPtr->TelemetryHeader), CFE_SB_ValueToMsgId(ADCS2_REPORT_TLM_MID), sizeof(ADCS2_ReportTlm_t) != CFE_SUCCESS)) {
				CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
				break;
			}
			BufPtr->Report.MsgID = ADCS2_CMD_MID;
			BufPtr->Report.CommandCode = 0;
			BufPtr->Report.ReturnType = RPT_RETTYPE_APP;
			BufPtr->Report.ReturnCode = CFE_STATUS_UNKNOWN_MSG_ID;
			BufPtr->Report.ReturnDataSize = sizeof(CFE_SB_MsgId_Atom_t);
			memcpy(BufPtr->Report.ReturnValue, &MsgId.Value, sizeof(CFE_SB_MsgId_Atom_t));
			CFE_SB_TimeStampMsg(CFE_MSG_PTR(BufPtr->TelemetryHeader));
			if(CFE_SB_TransmitBuffer((CFE_SB_Buffer_t *)BufPtr, true) != CFE_SUCCESS) {
				CFE_SB_ReleaseMessageBuffer((CFE_SB_Buffer_t *)BufPtr);
				break;
			}
			/* End of RPT */
			break;
	}
}
