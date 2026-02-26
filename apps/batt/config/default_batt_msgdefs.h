/**
 * @file
 *   Specification for the BATT command and telemetry
 *   message constant definitions.
 *
 *   Defines payload structures for NanoPower BP8 battery pack commands and telemetry.
 */
#ifndef BATT_MSGDEFS_H
#define BATT_MSGDEFS_H

#include "common_types.h"
#include "batt_fcncodes.h"

/**
 * \brief Set Heater command payload
 *
 * Activates the BP8 manual heater timer for the specified duration.
 * Valid range: 1-600 seconds. Set to 0 to stop.
 */
typedef struct BATT_SetHeater_Payload
{
    uint16 Duration; /**< Heater duration in seconds (1-600, 0=stop) */
    uint16 spare;
} BATT_SetHeater_Payload_t;

/**
 * \brief Housekeeping telemetry payload
 *
 * Contains BP8 battery pack telemetry data retrieved via rparam.
 */
typedef struct BATT_HkTlm_Payload
{
    /* App counters */
    uint8  CommandCounter;
    uint8  CommandErrorCounter;
    uint8  spare[2];

    /* BP8 Telemetry (Table 3.7) */
    uint32 Uptime;        /**< Unit uptime in seconds */
    uint16 BootCount;     /**< Number of boots */
    uint16 BootCause;     /**< Boot cause code */
    uint16 ResetCause;    /**< Reset cause code */
    uint16 Vbat;          /**< Battery voltage in mV */
    float  Soc;           /**< State of charge (0.0=empty, 1.0=full) */
    float  Current;       /**< Current in/out of battery pack in A */
    uint16 InCurrent;     /**< Current into battery pack in mA */
    uint16 OutCurrent;    /**< Current out of battery pack in mA */
    uint16 HeaterCurrent; /**< Heater current in mA */
    int16  IntTemp;       /**< Internal MCU temperature in ddegC */
    float  BatAvrTemp;    /**< Average battery temp in degC */
    int16  BatTemp[4];    /**< Battery sensor temps 1-4 in ddegC */
    uint16 OVoltCount;    /**< Over-voltage event count */
    uint8  BatFault;      /**< Battery cell fault flag */
    uint8  spare2;
} BATT_HkTlm_Payload_t;

#endif
