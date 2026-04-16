#ifndef ADCS2_APP_UTILS_H
#define ADCS2_APP_UTILS_H

#include "adcs2_msg.h"
#include <math.h>
#include <string.h>

#include "common_types.h"
#include "adcs2_msg.h"
#include "adcs2_cube_error_typedefs.h"
#include "adcs2_cube_typedefs.h"
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


/*
// CubeADCS Log Frame Max Entry Number
*/
#define CUBESPACE_MAX_ENTRY_NUM		11

typedef enum CubeADCS2_EventClass {
	CLASS_INFORMATION,
	CLASS_MINOR_WARNING,
	CLASS_MAJOR_WARNING,
	CLASS_CRITICAL
} CubeADCS2_EventClass_t;

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
int32 ADCS2_Reset(void);


/********************************************************
 * 
 * BEE1000 Actual Set Command Function
 * 
 ********************************************************/
//																									// ID	USE		1	2	3	4	5	6	7	8	9	10
int32 ADCS2_SetCurrentUnixTime(const ADCS2_CurrentUnixTimeCmd_Payload_t *setVal);					// 2			X	X	X
int32 ADCS2_SetPersistConfig(void);																	// 7			O	O	O
int32 ADCS2_SetControlEstimationMode(const ADCS2_ControlEstimationMode_Cmn_Payload_t *setVal);		// 42			O	O	O
int32 ADCS2_SetPowerState(const ADCS2_PowerState_Cmn_Payload_t *setVal);							// 56			O	O	O
int32 ADCS2_SetMountingConfig(const ADCS2_MountingConfig_Cmn_Payload_t *setVal);					// 65

/********************************************************
 * 
 * BEE1000 Actual Get Command Function (Get tlm)
 * 
 ********************************************************/
//																									// ID	USE		1	2	3	4	5	6	7	8	9	10
int32 ADCS2_GetCurrentUnixTime(ADCS2_CurrentUnixTimeTlm_Payload_t *returnVal);						// 133			O	O	O
int32 ADCS2_GetControlEstimationMode(ADCS2_ControlEstimationMode_Cmn_Payload_t *returnVal);			// 150			O	O	O
int32 ADCS2_GetControllerTlm(ADCS2_ControllerTlm_Payload_t *returnVal);								// 172			X	O	X
int32 ADCS2_GetBackupEstTlm(ADCS2_Estimator_Cmn_Payload_t *returnVal);								// 173			O	X	X
int32 ADCS2_GetCalibratedMAGSensor(ADCS2_CalibratedMAGSensorTlm_Payload_t *returnVal);				// 177			X	X	O
int32 ADCS2_GetRawMAGSensor(ADCS2_RawMAGSensorTlm_Paylaod_t *returnVal);							// 180			O	O	X
int32 ADCS2_GetPowerState(ADCS2_PowerState_Cmn_Payload_t *returnVal);								// 183			O	O	O
int32 ADCS2_GetControlMode(ADCS2_ControlModeTlm_Payload_t *returnVal);								// 185			X	X	X
int32 ADCS2_GetMountingConfig(ADCS2_MountingConfig_Cmn_Payload_t *returnVal);						// 193			O	O	O
int32 ADCS2_GetRawCSSSensor(ADCS2_RawCSSSensorTlm_Payload_t *returnVal);							// 203			X	O	X
int32 ADCS2_GetRawGYRSensor(ADCS2_RawGYRSensorTlm_Paylaod_t *returnVal);							// 204			O	O	X
int32 ADCS2_GetCalibratedGYRSensor(ADCS2_CalibratedGYRSensorTlm_Payload_t *returnVal);				// 207			X	X	X
int32 ADCS2_GetMainEstTlm(ADCS2_Estimator_Cmn_Payload_t *returnVal);								// 210			O	O	O

/**
 * Report function
 */
void ADCS2_HandleReport(int32 Status, uint8_t CC, void *ReadData, uint16_t ReadSize);

/**
 * Event Function
 */
// void ADCS2_HandleEvent(const ADCS2_EventEntry_t *Event);
// void ADCS2_ListenEventTask(void);


#endif