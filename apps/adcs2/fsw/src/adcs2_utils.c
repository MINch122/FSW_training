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
 *   This file contains the source code for the Adcs App utility functions
 */

/*
** Include Files:
*/
#include "adcs2_app.h"
#include "adcs2_eventids.h"
#include "adcs2_tbl.h"
#include "adcs2_utils.h"
#include "csp/csp_types.h"

#include "adcs2_cube_error_typedefs.h"
#include "adcs2_cube_typedefs.h"
#include "adcs2_msg.h"
#include "cfe_srl.h"

#include <csp/csp.h>

static Handle handle[TYPEDEF__COMMS_ENDPOINT_MAX];
static TypeDef_TctlmEndpoint endpoint;
static uint8 ADCS2_InterfaceTransportMode = ADCS2_INTERFACE_TRANSPORT_CSP_CAN;
static CFE_SRL_IO_Handle_t *ADCS2_UartHandle = NULL;
static uint8 ADCS2_UartProtocolBuffer[ADCS2_UART_PROTOCOL_BUFFER_SIZE];

void CUBE_EndpointInit(void)
{
	endpoint.nodeType = TYPES_COMMON_FRAMEWORK_TYPES_1__NODE_TYPE_CUBE_COMPUTER;
	endpoint.type = TYPEDEF__COMMS_ENDPOINT_CAN;
	endpoint.proto = TYPEDEF__COMMS_PROTOCOL_CSP;
	endpoint.addr = CAN_ADDR_CC;
	endpoint.timeout = 500u;
	endpoint.passthrough = false;

	/** 
	 * Maybe this member is useless, if `csp_transaction_w_opt()` is used.
	 * Because this function **doesn't care** the sport (source port)
	 */
	endpoint.cspSrcPort = CSP_SRC_PORT;

	/* No need */
	handle->bufferSize = COMMS_BUFFER_SIZE;
	ADCS2_UartHandle = CFE_SRL_ApiGetHandle(CFE_SRL_UART_HANDLE_INDEXER);
}

int32 ADCS2_SetInterfaceTransport(uint8 transport)
{
	if (transport == ADCS2_INTERFACE_TRANSPORT_CSP_CAN)
	{
		ADCS2_InterfaceTransportMode = transport;
		return CFE_SUCCESS;
	}

	if (transport == ADCS2_INTERFACE_TRANSPORT_UART)
	{
		if (ADCS2_UartHandle == NULL)
		{
			ADCS2_UartHandle = CFE_SRL_ApiGetHandle(CFE_SRL_UART_HANDLE_INDEXER);
		}

		if (ADCS2_UartHandle == NULL)
		{
			return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
		}

		ADCS2_InterfaceTransportMode = transport;
		return CFE_SUCCESS;
	}

	return CFE_ES_BAD_ARGUMENT;
}


static uint8_t *cubeObc_connect_buffer(const TctlmCommsMasterSvc_Endpoint *masterEndpoint)
{
	return handle[masterEndpoint->endpoint.type].buffer;
}

static ErrorCode nack2ErrorCode(uint8 byte)
{
	ErrorCode result;

	switch (byte)
	{
		case (uint8)TCTLM__ERROR_OK:
		{
			result = CUBEOBC_ERROR_OK;
		}
		break;

		case (uint8)TCTLM__ERROR_INVALID_ID:
		{
			result = CUBEOBC_ERROR_TCTLM_INVALID_ID;
		}
		break;

		case (uint8)TCTLM__ERROR_INVALID_LENGTH:
		{
			result = CUBEOBC_ERROR_TCTLM_INVALID_LENGTH;
		}
		break;

		case (uint8)TCTLM__ERROR_INVALID_PARAM:
		{
			result = CUBEOBC_ERROR_TCTLM_INVALID_PARAM;
		}
		break;

		case (uint8)TCTLM__ERROR_CRC:
		{
			result = CUBEOBC_ERROR_TCTLM_CRC;
		}
		break;

		case (uint8)TCTLM__ERROR_NOT_IMPLEMENTED:
		{
			result = CUBEOBC_ERROR_TCTLM_NOT_IMPLEMENTED;
		}
		break;

		case (uint8)TCTLM__ERROR_BUSY:
		{
			result = CUBEOBC_ERROR_TCTLM_BUSY;
		}
		break;

		case (uint8)TCTLM__ERROR_SEQUENCE:
		{
			result = CUBEOBC_ERROR_TCTLM_SEQUENCE;
		}
		break;

		case (uint8)TCTLM__ERROR_INTERNAL:
		{
			result = CUBEOBC_ERROR_TCTLM_INTERNAL;
		}
		break;

		case (uint8)TCTLM__ERROR_PASS_TIMEOUT:
		{
			result = CUBEOBC_ERROR_TCTLM_PASS_TOUT;
		}
		break;

		case (uint8)TCTLM__ERROR_PASS_TARGET:
		{
			result = CUBEOBC_ERROR_TCTLM_PASS_TARGET;
		}
		break;

		default:
		{
			result = CUBEOBC_ERROR_UKN_NACK;
		}
		break;
	}

	return result;
}

static void ADCS2_UartRxFlush(void)
{
	uint8 byte;
	CFE_SRL_IO_Param_t Params = {0,};

	if (ADCS2_UartHandle == NULL)
	{
		return;
	}

	Params.RxData = &byte;
	Params.RxSize = sizeof(byte);
	Params.Timeout = ADCS2_UART_FLUSH_TIMEOUT_MS;

	while (CFE_SRL_ApiRead(ADCS2_UartHandle, &Params) == CFE_SUCCESS)
	{
	}
}

static ErrorCode ADCS2_UartWriteFrame(const TctlmCommsMasterSvc_Endpoint *masterEndpoint, const uint8 *data, uint16 dataLen)
{
	CFE_SRL_IO_Param_t Params = {0,};
	uint32 protocolIdx = ADCS2_UART_HEADER_SIZE_PLAIN;

	if ((ADCS2_UART_HEADER_SIZE_PLAIN + ADCS2_UART_FOOTER_SIZE + ((uint32)dataLen * 2u)) > ADCS2_UART_PROTOCOL_BUFFER_SIZE)
	{
		return CUBEOBC_ERROR_SIZEH;
	}

	ADCS2_UartProtocolBuffer[ADCS2_UART_ESCAPE_OFFSET] = ADCS2_UART_ESCAPE;
	ADCS2_UartProtocolBuffer[ADCS2_UART_SOM_OFFSET] = masterEndpoint->endpoint.passthrough ? ADCS2_UART_SOM_NORMAL_PASS : ADCS2_UART_SOM_NORMAL_PLAIN;
	ADCS2_UartProtocolBuffer[ADCS2_UART_ID_OFFSET] = masterEndpoint->id;

	for (uint16 i = 0; i < dataLen; ++i)
	{
		if (data[i] == ADCS2_UART_ESCAPE)
		{
			ADCS2_UartProtocolBuffer[protocolIdx++] = ADCS2_UART_ESCAPE;
		}

		ADCS2_UartProtocolBuffer[protocolIdx++] = data[i];
	}

	ADCS2_UartProtocolBuffer[protocolIdx++] = ADCS2_UART_ESCAPE;
	ADCS2_UartProtocolBuffer[protocolIdx++] = ADCS2_UART_EOM;

	Params.TxData = ADCS2_UartProtocolBuffer;
	Params.TxSize = protocolIdx;

	return (CFE_SRL_ApiWrite(ADCS2_UartHandle, &Params) == CFE_SUCCESS) ? CUBEOBC_ERROR_OK : CUBEOBC_ERROR_WRITE;
}

static ErrorCode ADCS2_UartReadFrame(const TctlmCommsMasterSvc_Endpoint *masterEndpoint, uint8 *data, uint16 *dataLen)
{
	bool escaped = false;
	bool som = false;
	bool validRxWindow = false;
	bool nack = false;
	bool passthrough = false;
	bool done = false;
	uint8 tctlmId = 0u;
	uint16 dataIdx = 0u;

	while (done == false)
	{
		uint8 byte = 0u;
		CFE_SRL_IO_Param_t Params = {0,};
		int32 Status;

		Params.RxData = &byte;
		Params.RxSize = sizeof(byte);
		Params.Timeout = validRxWindow ? ADCS2_UART_RX_NEXT_BYTE_TIMEOUT_MS : masterEndpoint->endpoint.timeout;

		Status = CFE_SRL_ApiRead(ADCS2_UartHandle, &Params);
		if (Status != CFE_SUCCESS)
		{
			return (Status == CFE_SRL_TIMEOUT) ? CUBEOBC_ERROR_TOUT : CUBEOBC_ERROR_READ;
		}

		if (escaped == true)
		{
			if ((byte == ADCS2_UART_SOM_ACK_PLAIN) || (byte == ADCS2_UART_SOM_ACK_PASS))
			{
				som = true;
				passthrough = (byte == ADCS2_UART_SOM_ACK_PASS);
			}
			else if ((byte == ADCS2_UART_SOM_NACK_PLAIN) || (byte == ADCS2_UART_SOM_NACK_PASS))
			{
				som = true;
				nack = true;
				passthrough = (byte == ADCS2_UART_SOM_NACK_PASS);
			}
			else if (byte == ADCS2_UART_EOM)
			{
				done = true;
			}
			else if (byte == ADCS2_UART_ESCAPE)
			{
				if (validRxWindow == true)
				{
					if (dataIdx >= *dataLen)
					{
						return CUBEOBC_ERROR_SIZEH;
					}

					data[dataIdx++] = byte;
				}
			}

			escaped = false;
		}
		else if (byte == ADCS2_UART_ESCAPE)
		{
			escaped = true;
		}
		else if (som == true)
		{
			tctlmId = byte;
			validRxWindow = true;
			som = false;
		}
		else if (validRxWindow == true)
		{
			if (dataIdx >= *dataLen)
			{
				return CUBEOBC_ERROR_SIZEH;
			}

			data[dataIdx++] = byte;
		}
	}

	*dataLen = dataIdx;

	if ((validRxWindow == false) || (passthrough != masterEndpoint->endpoint.passthrough))
	{
		return CUBEOBC_ERROR_TCTLM_PROTOCOL;
	}

	if (tctlmId != masterEndpoint->id)
	{
		return CUBEOBC_ERROR_TCTLM_ID;
	}

	if (nack == true)
	{
		if (dataIdx == 0u)
		{
			return CUBEOBC_ERROR_TCTLM_PROTOCOL;
		}

		return nack2ErrorCode(data[0]);
	}

	return CUBEOBC_ERROR_OK;
}


/**
 * @brief SendReceive Function by CSP
 * @param masterEndpoint TctlmCommsMasterSvc_Endpoint Structure pointer
 * @param datalen Data length
 * @return CUBEOBC_ERROR_OK on success, otherwise ErrorCode
 */
static ErrorCode cubeObc_sendReceive(TctlmCommsMasterSvc_Endpoint *masterEndpoint, uint16 datalen) {
	ErrorCode result = CUBEOBC_ERROR_OK;
	TypeDef_TctlmEndpoint *endpoint = &masterEndpoint->endpoint; // Get the generic endpoint

	uint8 msgType;
	uint8 dstPort;

	if (masterEndpoint->id < V1_TLM_ID_START)
	{
		// Telecommand
		msgType = V1_TCTLM_CAN_TRANSPORT__TYPE_TC;
	}
	else
	{
		// Telemetry
		msgType = V1_TCTLM_CAN_TRANSPORT__TYPE_TLM;
	}

	if (endpoint->passthrough == false)
	{
		dstPort = CSP_PORT_TCTLM;
	}
	else
	{
		dstPort = CSP_PORT_PASSTHROUGH;
	}

	if (ADCS2_InterfaceTransportMode == ADCS2_INTERFACE_TRANSPORT_UART)
	{
		uint16 rxDataLen = COMMS_BUFFER_SIZE;

		if (ADCS2_UartHandle == NULL)
		{
			ADCS2_UartHandle = CFE_SRL_ApiGetHandle(CFE_SRL_UART_HANDLE_INDEXER);
		}

		if (ADCS2_UartHandle == NULL)
		{
			return CUBEOBC_ERROR_READ;
		}

		ADCS2_UartRxFlush();

		result = ADCS2_UartWriteFrame(masterEndpoint,
									  handle[endpoint->type].buffer,
									  (msgType == V1_TCTLM_CAN_TRANSPORT__TYPE_TC) ? datalen : 0u);
		if (result != CUBEOBC_ERROR_OK)
		{
			return result;
		}

		if ((msgType == V1_TCTLM_CAN_TRANSPORT__TYPE_TC) && (masterEndpoint->id == ADCS2_ID_SET_RESET))
		{
			return CUBEOBC_ERROR_OK;
		}

		result = ADCS2_UartReadFrame(masterEndpoint, handle[endpoint->type].buffer, &rxDataLen);
		if (result != CUBEOBC_ERROR_OK)
		{
			return result;
		}

		if ((msgType == V1_TCTLM_CAN_TRANSPORT__TYPE_TLM) && (rxDataLen != datalen))
		{
			return (rxDataLen < datalen) ? CUBEOBC_ERROR_SIZEL : CUBEOBC_ERROR_SIZEH;
		}

		return CUBEOBC_ERROR_OK;
	}

	uint8_t txCspDataBuffer[COMMS_BUFFER_SIZE];
	uint8_t rxCspDataBuffer[COMMS_BUFFER_SIZE];
	int32 res;

	txCspDataBuffer[CSP_MSG_TYPE_IDX] = msgType;
	txCspDataBuffer[CSP_TCTLM_ID_IDX] = masterEndpoint->id;
	memcpy(txCspDataBuffer + CSP_HEADER_SIZE, handle[endpoint->type].buffer, datalen);

	if(msgType == V1_TCTLM_CAN_TRANSPORT__TYPE_TC) {
		/**
		 * If the TC is Reset, there is not ack (i.e. No reply)
		 */
		if(masterEndpoint->id == ADCS2_ID_SET_RESET) {
			res = CFE_SRL_ApiTransactionCSP(endpoint->addr, dstPort,
										txCspDataBuffer, datalen + CSP_HEADER_SIZE,
										NULL, 0);
			if (res == 1) result = CUBEOBC_ERROR_OK;
			return result;
		}
		else {
		/**
		 * In this case, return length shoud be `3`(ACK) or `4`(NACK)
		 */
		res = CFE_SRL_ApiTransactionCSP(endpoint->addr, dstPort,
									txCspDataBuffer, datalen + CSP_HEADER_SIZE,
									rxCspDataBuffer, CSP_UNKNOWN_LEN);
		}
		
	}
	else { // msgType == V1_TCTLM_CAN_TRANSPORT__TYPE_TLM
		res = CFE_SRL_ApiTransactionCSP(endpoint->addr, dstPort,
									txCspDataBuffer, CSP_HEADER_SIZE,
									rxCspDataBuffer, datalen + CSP_HEADER_SIZE);
	}
									
	if (res) {
		result = CUBEOBC_ERROR_OK;
	}
	else
		return result;
	// Check that the response is for the expected TCTLM ID
	if (result == CUBEOBC_ERROR_OK)
	{
		uint8 tctlmId = rxCspDataBuffer[CSP_TCTLM_ID_IDX];

		if (tctlmId != masterEndpoint->id)
		{
			result = CUBEOBC_ERROR_TCTLM_ID;
		}
	}
	// Check the response type
	if (result == CUBEOBC_ERROR_OK)
	{
		V1TctlmCanTransport_Type rxMsgType;

		rxMsgType = (V1TctlmCanTransport_Type)rxCspDataBuffer[CSP_MSG_TYPE_IDX];

		/* Check for Nack */
		if ((rxMsgType == V1_TCTLM_CAN_TRANSPORT__TYPE_TC_NACK) ||
			(rxMsgType == V1_TCTLM_CAN_TRANSPORT__TYPE_TLM_NACK))
		{
			result = nack2ErrorCode(rxCspDataBuffer[CSP_HEADER_SIZE]);
		}
	}

	// Extract the data
	if (result == CUBEOBC_ERROR_OK)
	{
		// Extract TCTLM data from CSP response data
		memcpy(handle[endpoint->type].buffer, &rxCspDataBuffer[CSP_DATA_IDX], datalen);
	}
	return result;
}


int32 ADCS2_SetCommand_Common(uint16 cmdId, const void *setVal, uint16 size)
{
    int32 status;
    TctlmCommsMasterSvc_Endpoint target;
    uint8 *tx_buffer;

    ZERO_VAR(target);
    
    target.id = cmdId;
    memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

    if (size > 0 && setVal != NULL) 
    {
        tx_buffer = cubeObc_connect_buffer(&target);
        if (tx_buffer != NULL) {
            memcpy(tx_buffer, setVal, size);
        }
    }
    
    if((status = cubeObc_sendReceive(&target, size)) != CUBEOBC_ERROR_OK)
    {
        // OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n", status, target.id);
        return status;
    }

    return CFE_SUCCESS;
}

int32 ADCS2_GetTelemetry_Common(uint16 tlmId, void *returnVal, uint16 bufferSize)
{
    int32 status;
    TctlmCommsMasterSvc_Endpoint target;
    uint8 *rx_buffer;

    ZERO_VAR(target);
    
    target.id = tlmId;
    memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

    rx_buffer = cubeObc_connect_buffer(&target);

    
    if((status = cubeObc_sendReceive(&target, bufferSize)) != CUBEOBC_ERROR_OK) {
        // OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
        return status;
    }
    
    if (returnVal != NULL && rx_buffer != NULL) {
        memcpy(returnVal, rx_buffer, bufferSize);
    }
	else
	{
		return -1;
	}

    return CFE_SUCCESS;
}


/********************************************************
 * 
 * COSMIC Actual Set Command Function
 * 
 ********************************************************/
/* TC Functions, ID: 0 ~ 127 */
int32 ADCS2_Reset(void)
{
	// ID 1
	int32_t status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8_t *tx_buffer;
	uint16_t bufferSizeUsed = 1;
	
	ZERO_VAR(target);
	
	target.id = ADCS2_ID_SET_RESET;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	tx_buffer = cubeObc_connect_buffer(&target);

	uint8 resetType = 66;		/**< Performs reset immediately - hard reset*/
	memcpy(tx_buffer, &resetType, bufferSizeUsed);

	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK)
	{
		// OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n",status, target.id);
		return status;
	}

	return CFE_SUCCESS;
}

int32 ADCS2_SetCurrentUnixTime(const ADCS2_CurrentUnixTimeCmd_Payload_t *setVal)
{	// ID 2
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_CURRENT_UNIX_TIME, setVal, sizeof(*setVal));	
}

int32 ADCS2_SetPersistConfig(void)
{	// ID 7
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_PERSIST_CONFIG, NULL, 0);
}

int32 ADCS2_SetControlEstimationMode(const ADCS2_ControlEstimationMode_Cmn_Payload_t *setVal)
{	// ID 42
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_CONTROL_ESTIMATION_MODE, setVal, sizeof(*setVal));
}

int32 ADCS2_SetOrbitMode(const ADCS2_OrbitMode_Cmn_Payload_t *setVal)
{	// ID 51
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_ORBIT_MODE, setVal, sizeof(*setVal));
}

int32 ADCS_SetReferenceRPYValues(const ADCS2_ReferenceRPYvaluesCmd_Payload_t *setVal)
{	// ID 54
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_REFERENCE_RPY_VALUES, setVal, sizeof(*setVal));
}

int32 ADCS2_SetPowerState(const ADCS2_PowerState_Cmn_Payload_t *setVal)
{	// ID 56
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_POWER_STATE, setVal, sizeof(*setVal));
}

int32 ADCS2_SetMountingConfig(const ADCS2_MountingConfig_Cmn_Payload_t *setVal)
{	// ID 65
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_MOUNTING_CONFIG, setVal, sizeof(*setVal));
}

int32 ADCS2_SetEstimatorConfig(const ADCS2_EstimatorConfig_Cmn_Payload_t *setVal)
{	// ID 67
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_ESTIMATOR_CONFIG, setVal, sizeof(*setVal));
}

int32 ADCS2_SetSatOrbitParamConfig(const ADCS2_SatOrbitParamConfig_Cmn_Payload_t *setVal)
{	// ID 68
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_SAT_ORBIT_PARAM_CONFIG, setVal, sizeof(*setVal));
}

int32 ADCS2_SetOpenLoopCmdHxyzRW(const ADCS2_OpenLoopCmdHxyzRWCmd_Payload_t *setVal)
{	// ID 76
	return ADCS2_SetCommand_Common(ADCS2_ID_SET_OPENLOOP_CMD_HXYZ_RW, setVal, sizeof(*setVal));
}

/********************************************************
 * 
 * COSMIC Actual Get Command Function (Get tlm)
 * 
 ********************************************************/

 int32 ADCS2_GetCurrentUnixTime(ADCS2_CurrentUnixTimeTlm_Payload_t *returnVal)
{	// ID 133
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CURRENT_UNIX_TIME, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetControlEstimationMode(ADCS2_ControlEstimationMode_Cmn_Payload_t *returnVal)
{	// ID 150
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CONTROL_ESTIMATION_MODE, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetRawCubeSenseSun(ADCS2_RawCubeSenseSunTlm_Payload_t *returnVal)
{	// ID 170
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_RAW_CUBESENSE_SUN, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetControllerTlm(ADCS2_ControllerTlm_Payload_t *returnVal)
{	// ID 172
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CONTROLLER_TLM, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetBackupEstTlm(ADCS2_Estimator_Cmn_Payload_t *returnVal)
{	// ID 173
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_BACKUP_ESTIMATOR_TLM, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetModelsTlm(ADCS2_ModelsTlm_Payload_t *returnVal)
{	// ID 174
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_MODELS_TLM, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetCalibratedHSSSensor(ADCS2_CalibratedHSSSensorTlm_Payload_t *returnVal)
{	// ID 176
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CALIBRATED_HSS_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetCalibratedMAGSensor(ADCS2_CalibratedMAGSensorTlm_Payload_t *returnVal)
{	// ID 177
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CALIBRATED_MAG_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetCalibratedFSSSensor(ADCS2_CalibratedFSSSensorTlm_Payload_t *returnVal)
{	// ID 178
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CALIBRATED_FSS_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetRawCubeSenseEarth(ADCS2_RawCubeSenseEarthTlm_Payload_t *returnVal)
{	// ID 179
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_RAW_CUBESENSE_EARTH, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetRawMAGSensor(ADCS2_RawMAGSensorTlm_Paylaod_t *returnVal)
{	// ID 180
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_RAW_MAG_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetPowerState(ADCS2_PowerState_Cmn_Payload_t *returnVal)
{	// ID 183
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_POWERSTATE, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetControlMode(ADCS2_ControlModeTlm_Payload_t *returnVal)
{	// ID 185
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CONTROL_MODE, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetMountingConfig(ADCS2_MountingConfig_Cmn_Payload_t *returnVal)
{	// ID 193
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_MOUNTING_CONFIG, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetEstimatorConfig(ADCS2_EstimatorConfig_Cmn_Payload_t *returnVal)
{	// ID 195
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_ESTIMATOR_CONFIG, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetSatOrbitParamConfig(ADCS2_SatOrbitParamConfig_Cmn_Payload_t *returnVal)
{	// ID 196
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_SAT_ORBIT_PARAM_CONFIG, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetRawCSSSensor(ADCS2_RawCSSSensorTlm_Payload_t *returnVal)
{	// ID 203
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_RAW_CSS_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetRawGYRSensor(ADCS2_RawGYRSensorTlm_Payload_t *returnVal)
{	// ID 204
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_RAW_GYR_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetRawRWLSensor(ADCS2_RawRWLSensorTlm_Payload_t *returnVal)
{	// ID 205
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_RAW_RWL_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetCalibratedCSSSensor(ADCS2_CalibratedCSSSensorTlm_Payload_t *returnVal)
{	// ID 206
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CALIBRATED_CSS_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetCalibratedGYRSensor(ADCS2_CalibratedGYRSensorTlm_Payload_t *returnVal)
{	// ID 207
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CALIBRATED_GYR_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetCalibratedRWLSensor(ADCS2_CalibratedRWLSensorTlm_Payload_t *returnVal)
{	// ID 209
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_CALIBRATED_RWL_SENSOR, returnVal, sizeof(*returnVal));
}

int32 ADCS2_GetMainEstTlm(ADCS2_Estimator_Cmn_Payload_t *returnVal)
{	// ID 210
	return ADCS2_GetTelemetry_Common(ADCS2_ID_GET_MAIN_ESTIMATOR_TLM, returnVal, sizeof(*returnVal));
}

/***********************************************
 * 
 * Report util function
 * 
 **********************************************/
void ADCS2_HandleReport(int32 Status, uint8_t CC, void *ReadData, uint16_t ReadSize) {
	CFE_SB_Buffer_t *BufPtr = CFE_SB_AllocateMessageBuffer(sizeof(ADCS2_ReportTlm_t));
	if (BufPtr == NULL) return;

	ADCS2_ReportTlm_t *Report = (ADCS2_ReportTlm_t *)BufPtr;
	if (CFE_MSG_Init(CFE_MSG_PTR(Report->TelemetryHeader), CFE_SB_ValueToMsgId(ADCS2_REPORT_TLM_MID), sizeof(ADCS2_ReportTlm_t)) != CFE_SUCCESS) {
		CFE_SB_ReleaseMessageBuffer(BufPtr);
		return;
	}

	Report->Report.MsgID = ADCS2_CMD_MID;
	Report->Report.CommandCode = CC;
	Report->Report.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_HW;
	Report->Report.ReturnCode = Status; // `adcs2_cube_error_typedefs.h`
	uint16_t CopySize = (ReadSize > RPT_RET_VALUE_BUF_SIZE) ? RPT_RET_VALUE_BUF_SIZE : ReadSize;
	Report->Report.ReturnDataSize = CopySize;
	if (ReadSize && ReadData) {
		memcpy(Report->Report.ReturnValue, ReadData,
				CopySize);
	}

	CFE_SB_TimeStampMsg((CFE_MSG_PTR(Report->TelemetryHeader)));
	if (CFE_SB_TransmitBuffer(BufPtr, true) != CFE_SUCCESS) {
		CFE_SB_ReleaseMessageBuffer(BufPtr);
		return;
	}
	return;
}


/**************************************
 * CubeADCS EVS listen Task function
 **************************************/
// void ADCS2_HandleEvent(const ADCS2_EventEntry_t *Event) {
// 	switch (Event->Identifier.EventClass)
// 	{
// 	case CLASS_CRITICAL:
// 		// OS_printf("CRRITICAL EventType : %u || EventSource : %u || EventClass : %u\n",
// 				Event->Identifier.EventType, Event->Identifier.EventSource,
// 				Event->Identifier.EventClass);
// 		break;
// 	case CLASS_MAJOR_WARNING:
// 		// OS_printf("MAJOR EventType : %u || EventSource : %u || EventClass : %u\n",
// 				Event->Identifier.EventType, Event->Identifier.EventSource,
// 				Event->Identifier.EventClass);
// 		break;
// 	case CLASS_MINOR_WARNING:
// 		// OS_printf("MINOR EventType : %u || EventSource : %u || EventClass : %u\n",
// 				Event->Identifier.EventType, Event->Identifier.EventSource,
// 				Event->Identifier.EventClass);
// 		break;
// 	case CLASS_INFORMATION:
// 		// OS_printf("INFO EventType : %u || EventSource : %u || EventClass : %u\n",
// 				Event->Identifier.EventType, Event->Identifier.EventSource,
// 				Event->Identifier.EventClass);
		
// 		switch (Event->Identifier.EventType)
// 		{
// 		case 142: // Eclipse/sunlight transition occurred
// 			// OS_printf("Event Data : ");
// 			for (uint8_t i=0; i < 8; i++) {
// 				// OS_printf("0x%02X\t", Event->EventData[i]);
// 			}
// 			// OS_printf("\n");
// 			break;
// 		case 139:
			
// 		default:
// 			break;
// 		}

// 		break;
// 	/* End of CLASS_INFORMATION */

// 	default:
// 		break;
// 	}
// }

// void ADCS2_ListenEventTask(void) {
// 	int Status;
// 	csp_socket_t *Sock;
// 	csp_conn_t *Conn = NULL;
// 	csp_packet_t *Packet = NULL;

// 	Sock = csp_socket(CSP_O_NONE);
// 	if (Sock == NULL) {
// 		CFE_ES_WriteToSysLog("%s: csp_socket failed! NO RC\n", __func__);
// 		return;  // Revise to `csp_socket failed`
// 	}

// 	Status = csp_bind(Sock, CSP_PORT_EVENT);
// 	if (Status != CSP_ERR_NONE) {
// 		CFE_ES_WriteToSysLog("%s: csp_bind failed at Port: %d RC=%d\n", __func__, 58, Status);
// 		return;
// 	}

// 	Status = csp_listen(Sock, 5);
// 	if (Status != CSP_ERR_NONE) {
// 		CFE_ES_WriteToSysLog("%s: csp_listen failed! RC=%d\n", __func__, Status);
// 		return;
// 	}

// 	for (;;) {
// 		Conn = csp_accept(Sock, 10000);
// 		if (Conn == NULL) {
// 			continue;
// 		}
// 		while ((Packet = csp_read(Conn, 1000)) != NULL) {
// 			int Port = csp_conn_dport(Conn);
// 			switch (Port)
// 			{
// 			case CSP_PORT_EVENT:
// 				// OS_printf("ADCS Event Comming.\n");
// 				ADCS2_HandleEvent((const ADCS2_EventEntry_t *)Packet);

// 				/* Free buffer & Remove dangled pointer */
// 				csp_buffer_free(Packet);
// 				Packet = NULL;
// 				break;
			
// 			default:
// 				break;
// 			}
// 		}
// 		csp_close(Conn);
// 	}
	
// }
