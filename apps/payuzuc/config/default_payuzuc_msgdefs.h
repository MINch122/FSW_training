/**
 * @file
 *   Specification for the PAY UZURO CAM command and telemetry
 *   message constant definitions.
 *
 *  For PAY UZURO CAM this is only the function/command code definitions
 */
#ifndef PAYUZUC_MSGDEFS_H
#define PAYUZUC_MSGDEFS_H

#include "common_types.h"
#include "payuzuc_fcncodes.h"

/************************************************
 * 
 * Command Payload Type Definition
 * 
 ************************************************/
/**
 * Ping Command
 */
typedef struct PAYUZUC_Ping_Payload {
    /**
     * `0x00` for MCU only
     * `0x01` for MCU & Img sensor
     */
    uint8 PN;
} PAYUZUC_Ping_Payload_t;


/**
 * Set Mode Command
 */
typedef struct PAYUZUC_SetMode_Payload {
    /**
     * `0` for sleep
     * `1` for SD
     * `2` for HD
     */
    uint8 MD;
} PAYUZUC_SetMode_Payload_t;


/**
 * Memory Status Command
 * 
 * No Arg Command
 */


/**
 * Set Exposure Command
 */
typedef struct PAYUZUC_SetExposure_Payload {
    uint8 EX1;
    uint8 EX2;
} PAYUZUC_SetExposure_Payload_t;


/**
 * Capture Command
 */
typedef struct PAYUZUC_Capture_Payload {
    uint8 MEM; /* Memory slot number. In HD mode, this paramter is ignored */
    uint8 TST;
} PAYUZUC_Capture_Payload_t;


/**
 * Download Command
 */
typedef struct PAYUZUC_Download_Payload {
    uint8 MEM;
    uint8 PRE;
    uint8 LN1;
    uint8 LN2;
} PAYUZUC_Download_Payload_t;


/**
 * Read Register Command
 */
typedef struct PAYUZUC_ReadRegister_Payload {
    uint8 AD1;
    uint8 AD2;
} PAYUZUC_ReadRegister_Payload_t;


/**
 * Write Register Command
 */
typedef struct PAYUZUC_WriteRegister_Payload {
    uint8 AD1;
    uint8 AD2;
    uint8 RG1;
    uint8 RG2;
} PAYUZUC_WriteRegister_Payload_t;


/**
 * Download All Command
 */
typedef struct PAYUZUC_DownloadAll_Payload {
    uint8 MEM;
    uint8 PRE;
} PAYUZUC_DownloadAll_Payload_t;


/************************************************
 * 
 * Telemetry Payload Type Definition
 * 
 ************************************************/
typedef struct PAYUZUC_HkTlm_Payload {
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
} PAYUZUC_HkTlm_Payload_t;

#endif