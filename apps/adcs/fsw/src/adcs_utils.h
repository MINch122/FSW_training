#ifndef ADCS_APP_UTILS_H
#define ADCS_APP_UTILS_H

#include "adcs_msg.h"
#include <math.h>
#include <string.h>

#include "common_types.h"
#include "adcs_msg.h"
#include "adcs_cube_error_typedefs.h"
#include "adcs_cube_typedefs.h"
/**
 * @brief memset variable to all zero's
 */
#define ZERO_VAR(var)							\
		do										\
		{										\
			(void)memset((void *)&(var), 0, sizeof(var));	\
		} while (false)



#define COMMS_BUFFER_SIZE	((uint32)256)

/**
 * @brief Telemetry Start ID
 */
#define V1_TLM_ID_START	((uint8)128u)		/**< Used for distinguishing the TC/TM */

/**
 * CubeADCS CSP configuration
 */
#define CAN_ADDR_CC				((uint8)4u)		/**< CSP Node address of CubeADCS */
#define CSP_SRC_PORT			((uint8)11u)	/**< Source port used for CSP comms with CubeProduct */

#define CSP_HEADER_SIZE			((uint32)2u)	/**< Size of CubeSpace header within CSP packet */
#define CSP_MSG_TYPE_IDX		((uint32)0u)	/**< Index of message type within CSP packet */
#define CSP_TCTLM_ID_IDX		((uint32)1u)	/**< Index of TCTLM ID within CSP packet */
#define CSP_DATA_IDX			((uint32)2u)	/**< Index of TCTLM data within CSP packet */

#define CSP_PORT_TCTLM			((uint8)8u)		/**< CSP port used for TCTLM */
#define CSP_PORT_PASSTHROUGH	((uint8)48u)	/**< CSP port used for passthrough TCTLM */
#define CSP_PORT_EVENT			((uint8)58)		/**< CSP port used for Events ingestion */
#define CSP_UNKNOWN_LEN			((int32)-1)		/**< CSP parameter value which is used in `csp_transaction_w_opt` */


typedef enum CubeADCS_EventClass {
	CLASS_INFORMATION,
	CLASS_MINOR_WARNING,
	CLASS_MAJOR_WARNING,
	CLASS_CRITICAL
} CubeADCS_EventClass_t;

typedef struct HandleStruct {
	uint8 buffer[COMMS_BUFFER_SIZE];/**< Buffer for packing and unpacking */
	uint32 bufferSize;				/**< Buffer size */
	uint32 bufferSizeUsed;			/**< Buffer size used */
	uint32 timeout;					/**< Master rx timeout */
	uint32 busyStart;				/**< Millisecond ticks captured at start of transaction (used for timeout) */
} Handle;

typedef enum V1TctlmCanTransport_TypeEnum {
	V1_TCTLM_CAN_TRANSPORT__TYPE_NONE = 0,				/**< Invalid */
	V1_TCTLM_CAN_TRANSPORT__TYPE_TC = 1,				/**< Telecommand */
	V1_TCTLM_CAN_TRANSPORT__TYPE_TC_EXT = 7,			/**< Telecommand Extended*/
	V1_TCTLM_CAN_TRANSPORT__TYPE_TC_RESP = 2,			/**< Telecommand Reply */
	V1_TCTLM_CAN_TRANSPORT__TYPE_TC_NACK = 3,			/**< Telecommand Request Invalid Reply */
	V1_TCTLM_CAN_TRANSPORT__TYPE_TLM = 4,				/**< Telemetry request */
	V1_TCTLM_CAN_TRANSPORT__TYPE_TLM_RESP = 5,			/**< Telemetry reply */
	V1_TCTLM_CAN_TRANSPORT__TYPE_TLM_RESP_EXT = 8,		/**< Telemetry Extended reply */
	V1_TCTLM_CAN_TRANSPORT__TYPE_TLM_NACK = 6,			/**< Telemetry Request Invalid Reply */
	V1_TCTLM_CAN_TRANSPORT__TYPE_EVENT = 9,				/**< Unsolicited event */
	V1_TCTLM_CAN_TRANSPORT__TYPE_USOL_TLM_FIRST = 10,	/**< Unsolicited telemetry first packet */
	V1_TCTLM_CAN_TRANSPORT__TYPE_USOL_TLM_BODY = 11,	/**< Unsolicited telemetry body packet */
	V1_TCTLM_CAN_TRANSPORT__TYPE_USOL_TLM_LAST = 12,	/**< Unsolicited telemetry last packet */
} V1TctlmCanTransport_Type;

/**
 * @brief Set Endpoint parameter
 */
void CUBE_EndpointInit(void);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                     	 < Telecommand(TC) Functions >                 	     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * */
/**
 * @brief ID 1, Reset command : Perform a soft reset
 * @return DEVICE SUCCESS on success, otherwise DEVICE_ERR_WRITE
 */
int32 ADCS_Reset(void);


/********************************************************
 * 
 * COSMIC Actual Set Command Function
 * 
 ********************************************************/
int32 ADCS_SetCurrentUnixTime(const ADCS_CurrentUnixTimeCmd_Payload_t *setVal);
int32 ADCS_SetControlEstimationMode(const ADCS_ControlEstimationModeCmd_Payload_t *setVal);
int32 ADCS_SetReferenceLLHTarget(const ADCS_ReferenceLLHTargetCmd_Payload_t *setVal);
int32 ADCS_SetOrbitMode(const ADCS_OrbitModeCmd_Payload_t *setVal);
int32 ADCS_SetReferenceRPYValues(const ADCS_ReferenceRPYvaluesCmd_Payload_t *setVal);
int32 ADCS_SetSatOrbitParamConfig(const ADCS_SatOrbitParamConfigCmd_Payload_t *setVal);

int32 ADCS_SetPersistConfig(void);
int32 ADCS_SetPowerState(const ADCS_PowerStateCmd_Payload_t *setVal);
int32 ADCS_SetRunMode(const ADCS_RunModeCmd_Payload_t *setVal);
int32 ADCS_SetSatelliteConfig(const ADCS_SatConfigCmd_Payload_t *setVal);
int32 ADCS_SetControllerConfig(const ADCS_ControllerConfig_Payload_t *setVal);
int32 ADCS_SetDefaultModeConfig(const ADCS_DefaultModeConfigCmd_Payload_t *setVal);
int32 ADCS_SetMountingConfig(const ADCS_MountingConfigCmd_Payload_t *setVal);
int32 ADCS_SetUnsolicitEventMsgSetup(const ADCS_UnsolicitEventMsgSetupCmd_InternalPayload_t *setVal);

/********************************************************
 * 
 * COSMIC Actual Get Command Function (Get tlm)
 * 
 ********************************************************/
int32 ADCS_GetCurrentUnixTime(ADCS_CurrentUnixTimeTlm_Payload_t *returnVal);
int32 ADCS_GetControlEstimationMode(ADCS_ControlEstimationModeTlm_Payload_t *returnVal);
int32 ADCS_GetReferenceLLHTarget(ADCS_ReferenceLLHTargetTlm_Payload_t *returnVal);
int32 ADCS_GetOrbitMode(ADCS_OrbitModeTlm_Payload_t *returnVal);
int32 ADCS_GetRawCubeSenseSun(ADCS_RawCubeSenseSunTlm_Payload_t *returnVal);
int32 ADCS_GetPowerState(ADCS_PowerStateTlm_Payload_t *returnVal);
int32 ADCS_GetControlMode(ADCS_ControlModeTlm_Payload_t *returnVal);
int32 ADCS_GetSatOrbitParamConfig(ADCS_SatOrbitParamConfigTlm_Payload_t *returnVal);
int32 ADCS_GetRawCSSSensor(ADCS_RawCSSSensorTlm_Payload_t *returnVal);
int32 ADCS_GetRawGYRSensor(ADCS_RawGYRSensorTlm_Paylaod_t *returnVal);
int32 ADCS_GetCalibratedGYRSensor(ADCS_CalibratedGYRSensorTlm_Payload_t *returnVal);

int32 ADCS_GetPersistConfigDiagnostic(ADCS_PersistConfigDiagnosticTlm_Payload_t *returnVal);
int32 ADCS_GetCommunicationStatus(ADCS_CommunicationStatusTlm_Payload_t *returnVal);
int32 ADCS_GetRunMode(ADCS_RunModeTlm_Payload_t *returnVal);
int32 ADCS_GetSatelliteConfig(ADCS_SatelliteConfigTlm_Payload_t *returnVal);
int32 ADCS_GetControllerConfig(ADCS_ControllerConfigTlm_Payload_t *returnVal);
int32 ADCS_GetDefaultModeConfig(ADCS_DefaultModeConfigTlm_Payload_t *returnVal);
int32 ADCS_GetMountingConfig(ADCS_MountingConfigTlm_Payload_t *returnVal);
int32 ADCS_GetOperationalState(ADCS_OperationalStateTlm_Payload_t *returnVal);
int32 ADCS_GetUnsolicitEventMsgSetup(ADCS_UnsolicitEventMsgSetupTlm_Payload_t *returnVal);


/**
 * Report function
 */
void ADCS_HandleReport(int32 Status, uint8_t CC, void *ReadData, uint16_t ReadSize);

/**
 * Event Function
 */
void ADCS_HandleEvent(const ADCS_EventEntry_t *Event);
void ADCS_ListenEventTask(void);

#endif