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
#include "adcs_app.h"
#include "adcs_eventids.h"
#include "adcs_tbl.h"
#include "adcs_utils.h"
#include "csp/csp_types.h"

#include "adcs_cube_error_typedefs.h"
#include "adcs_cube_typedefs.h"
#include "adcs_msg.h"


static Handle handle[TYPEDEF__COMMS_ENDPOINT_MAX];
// static uint8 cspDataBuffer[COMMS_BUFFER_SIZE];
static TypeDef_TctlmEndpoint endpoint;

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

	uint8_t txCspDataBuffer[COMMS_BUFFER_SIZE];
	uint8_t rxCspDataBuffer[COMMS_BUFFER_SIZE];

	// Add header data to CSP packet
	txCspDataBuffer[CSP_MSG_TYPE_IDX] = msgType;
	txCspDataBuffer[CSP_TCTLM_ID_IDX] = masterEndpoint->id;

	/**
	 * Copy the data buffer
	 */
	memcpy(txCspDataBuffer + CSP_HEADER_SIZE, handle[endpoint->type].buffer, datalen);
	OS_printf("TxData: ");
	for (uint8_t i=0; i< datalen + CSP_HEADER_SIZE; i++) {
		OS_printf("0x%02X\t", txCspDataBuffer[i]);
	}
	OS_printf("\n");

	int32 res;
	if(msgType == V1_TCTLM_CAN_TRANSPORT__TYPE_TC) {
		/**
		 * If the TC is Reset, there is not ack (i.e. No reply)
		 */
		if(masterEndpoint->id == ADCS_ID_SET_RESET) {
			res = CFE_SRL_ApiTransactionCSP(endpoint->addr, dstPort,
										txCspDataBuffer, datalen + CSP_HEADER_SIZE,
										NULL, 0);
			if (res == 1) result = CUBEOBC_ERROR_OK;
			goto cleanup;
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
cleanup:
	return result;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                     << Telecommand(TC) Functions >>                       */
/*								ID: 0 ~ 127								     */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * */
int32 ADCS_Reset(void)
{
	// ID 1
    int32_t status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8_t *tx_buffer;
	uint16_t bufferSizeUsed = 1;
	
	ZERO_VAR(target);
	
	target.id = ADCS_ID_SET_RESET;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	tx_buffer = cubeObc_connect_buffer(&target);

	uint8 resetType = 66;		/**< Performs reset immediately - hard reset*/
	memcpy(tx_buffer, &resetType, bufferSizeUsed);

	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK)
	{
		OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n",status, target.id);
		return status;
	}

	return CFE_SUCCESS;
}

/********************************************************
 * 
 * COSMIC Actual Set Command Function
 * 
 ********************************************************/
int32 ADCS_SetCurrentUnixTime(ADCS_CurrentUnixTimeCmd_Payload_t *setVal)
{	// ID 2
	
    int32_t status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8_t *tx_buffer;
	uint16_t bufferSizeUsed;
	
	ZERO_VAR(target);
	
	target.id = ADCS_ID_SET_CURRENT_UNIX_TIME;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	tx_buffer = cubeObc_connect_buffer(&target);

	//setVal->unixTimeSeconds = 	1731686180; // 예시 : 24-11-15-15-56-38 // CNDH에서 주는 값 그대로 input으로 받아야 함
	//memcpy(&rx_buffer[0], &setVal->unixTimeSeconds, sizeof(uint32_t)); 

	bufferSizeUsed = sizeof(ADCS_CurrentUnixTimeCmd_Payload_t);
	memcpy(tx_buffer, setVal, bufferSizeUsed);

	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK)
	{
		OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n",status, target.id);
		return status;
	}

	return CFE_SUCCESS;
}


int32 ADCS_SetControlEstimationMode(ADCS_ControlEstimationModeCmd_Payload_t *setVal)
{	// ID 42
	
    int32_t status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8_t *tx_buffer;
	uint16_t bufferSizeUsed;
	
	ZERO_VAR(target);
	
	target.id = ADCS_ID_SET_CONTROL_ESTIMATION_MODE;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	tx_buffer = cubeObc_connect_buffer(&target);

	//setVal->unixTimeSeconds = 	1731686180; // 예시 : 24-11-15-15-56-38 // CNDH에서 주는 값 그대로 input으로 받아야 함
	//memcpy(&rx_buffer[0], &setVal->unixTimeSeconds, sizeof(uint32_t)); 

	bufferSizeUsed = sizeof(ADCS_ControlEstimationModeCmd_Payload_t);
	memcpy(tx_buffer, setVal, bufferSizeUsed);

	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK)
	{
		OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n",status, target.id);
		return status;
	}

	return CFE_SUCCESS;
}

int32 ADCS_SetReferenceLLHTarget(ADCS_ReferenceLLHTargetCmd_Payload_t *setVal)
{	// ID 48
	
    int32_t status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8_t *tx_buffer;
	uint16_t bufferSizeUsed;
	
	ZERO_VAR(target);
	
	target.id = ADCS_ID_SET_REFERENCE_LLH_TARGET;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	tx_buffer = cubeObc_connect_buffer(&target);

	//setVal->unixTimeSeconds = 	1731686180; // 예시 : 24-11-15-15-56-38 // CNDH에서 주는 값 그대로 input으로 받아야 함
	//memcpy(&rx_buffer[0], &setVal->unixTimeSeconds, sizeof(uint32_t)); 

	bufferSizeUsed = sizeof(ADCS_ReferenceLLHTargetCmd_Payload_t);
	memcpy(tx_buffer, setVal, bufferSizeUsed);

	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK)
	{
		OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n",status, target.id);
		return status;
	}

	return CFE_SUCCESS;
}

int32 ADCS_SetOrbitMode(ADCS_OrbitModeCmd_Payload_t *setVal)
{	// ID 51
	
    int32_t status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8_t *tx_buffer;
	uint16_t bufferSizeUsed;
	
	ZERO_VAR(target);
	
	target.id = ADCS_ID_SET_ORBIT_MODE;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	tx_buffer = cubeObc_connect_buffer(&target);

	//setVal->unixTimeSeconds = 	1731686180; // 예시 : 24-11-15-15-56-38 // CNDH에서 주는 값 그대로 input으로 받아야 함
	//memcpy(&rx_buffer[0], &setVal->unixTimeSeconds, sizeof(uint32_t)); 

	bufferSizeUsed = sizeof(ADCS_OrbitModeCmd_Payload_t);
	memcpy(tx_buffer, setVal, bufferSizeUsed);

	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK)
	{
		OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n",status, target.id);
		return status;
	}

	return CFE_SUCCESS;
}


int32 ADCS_SetReferenceRPYValues(ADCS_ReferenceRPYvaluesCmd_Payload_t *setVal)
{	// ID 54
	
    int32_t status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8_t *tx_buffer;
	uint16_t bufferSizeUsed;
	
	ZERO_VAR(target);
	
	target.id = ADCS_ID_SET_REFERENCE_RPY_VALUES;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	tx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_ReferenceRPYvaluesCmd_Payload_t);
	memcpy(tx_buffer, setVal, bufferSizeUsed);

	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK)
	{
		OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n",status, target.id);
		return status;
	}

	return CFE_SUCCESS;
}

int32 ADCS_SetSatOrbitParamConfig(ADCS_SatOrbitParamConfigCmd_Payload_t *setVal)
{	// ID 68
	
    int32_t status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8_t *tx_buffer;
	uint16_t bufferSizeUsed;
	
	ZERO_VAR(target);
	
	target.id = ADCS_ID_SET_SAT_ORBIT_PARAM_CONFIG;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	tx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_SatOrbitParamConfigCmd_Payload_t);
	memcpy(tx_buffer, setVal, bufferSizeUsed);

	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK)
	{
		OS_printf("ADCS CAN Write Error (Error code : %d, ID: %d)\n",status, target.id);
		return status;
	}

	return CFE_SUCCESS;
}


/********************************************************
 * 
 * COSMIC Actual Get Command Function (Get tlm)
 * 
 ********************************************************/
int32 ADCS_GetCurrentUnixTime(ADCS_CurrentUnixTimeTlm_Payload_t *returnVal)
{	// ID 133

    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_CURRENT_UNIX_TIME_TELEMETRY;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_CurrentUnixTimeTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetControlEstimationMode(ADCS_ControlEstimationModeTlm_Payload_t *returnVal)
{	// ID 150
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_CONTROL_ESTIMATION_MODE;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_ControlEstimationModeTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetReferenceLLHTarget(ADCS_ReferenceLLHTargetTlm_Payload_t *returnVal)
{	// ID 157
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_REFERENCE_LLH_TARGET;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_ReferenceLLHTargetTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetOrbitMode(ADCS_OrbitModeTlm_Payload_t *returnVal)
{	// ID 162
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_ORBIT_MODE;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_OrbitModeTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetRawCubeSenseSun(ADCS_RawCubeSenseSunTlm_Payload_t *returnVal)
{	// ID 170
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_RAW_CUBESENSE_SUN;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_RawCubeSenseSunTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetPowerState(ADCS_PowerStateTlm_Payload_t *returnVal)
{	// ID 183
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_POWERSTATE;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_PowerStateTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}

int32 ADCS_GetControlMode(ADCS_ControlModeTlm_Payload_t *returnVal)
{	// ID 185
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_CONTROL_MODE;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_ControlModeTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetSatOrbitParamConfig(ADCS_SatOrbitParamConfigTlm_Payload_t *returnVal)
{	// ID 196
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_SAT_ORBIT_PARAM_CONFIG;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_SatOrbitParamConfigTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetRawCSSSensor(ADCS_RawCSSSensorTlm_Payload_t *returnVal)
{	// ID 203
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_RAW_CSS_SENSOR;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_RawCSSSensorTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetRawGYRSensor(ADCS_RawGYRSensorTlm_Paylaod_t *returnVal)
{	// ID 204
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_RAW_GYR_SENSOR;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_RawGYRSensorTlm_Paylaod_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}


int32 ADCS_GetCalibratedGYRSensor(ADCS_CalibratedGYRSensorTlm_Payload_t *returnVal)
{	// ID 207
	
    int32 status;
	TctlmCommsMasterSvc_Endpoint target;
	uint8 *rx_buffer;
	uint16 bufferSizeUsed;

	ZERO_VAR(target);
	
	target.id = ADCS_ID_GET_CALIBRATED_GYR_SENSOR;
	memcpy((uint8_t *) &target.endpoint, (uint8_t *) &endpoint, sizeof(TypeDef_TctlmEndpoint));

	rx_buffer = cubeObc_connect_buffer(&target);

	bufferSizeUsed = sizeof(ADCS_CalibratedGYRSensorTlm_Payload_t);
	if((status = cubeObc_sendReceive(&target, bufferSizeUsed)) != CUBEOBC_ERROR_OK) {
		OS_printf("ADCS CAN Read Error (Error code : %d, ID: %d)\n", status, target.id);
		return status;
	}
	memcpy(returnVal, rx_buffer, bufferSizeUsed);

	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(s) : %d\n", returnVal->unixTimeSeconds);
	// CFE_EVS_SendEve, CFE_EVS_INFORMATION, "CUBESENSE - Current Unix Time(ns) : %d\n", returnVal->unixTimeNanoSeconds);

	return CFE_SUCCESS;
}