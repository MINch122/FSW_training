/**
 * @file
 *   Specification for the BATT command and telemetry message data types.
 */
#ifndef BATT_MSGSTRUCT_H
#define BATT_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/

#include "batt_mission_cfg.h"
#include "batt_msgdefs.h"
#include "cfe_msg_hdr.h"

/*************************************************************************/

/*
** Command message structures
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} BATT_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} BATT_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} BATT_GetHkCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t    CommandHeader; /**< \brief Command header */
    BATT_SetHeater_Payload_t   Payload;       /**< \brief Heater payload */
} BATT_SetHeaterCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} BATT_ResetFaultCmd_t;

/*************************************************************************/
/*
** Housekeeping request
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} BATT_SendHkCmd_t;

/*************************************************************************/
/*
** Telemetry message structures
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry header */
    BATT_HkTlm_Payload_t     Payload;          /**< \brief Telemetry payload */
} BATT_HkTlm_t;

#endif /* BATT_MSGSTRUCT_H */
