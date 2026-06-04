/**
 * @file
 *   Specification for the PAY UZURO CAM command and telemetry
 *   message constant definitions.
 *
 *  For PAY UZURO CAM this is only the function/command code definitions
 */
#ifndef paybee_kisscam_MSGDEFS_H
#define paybee_kisscam_MSGDEFS_H

#include "common_types.h"
#include "paybee_kisscam_fcncodes.h"

/************************************************
 * 
 * Command Payload Type Definition
 * 
 ************************************************/
/**
 * Ping Command
 */
typedef struct paybee_kisscam_Ping_Payload {
    /**
     * `0x00` for MCU only
     * `0x01` for MCU & Img sensor
     */
    uint8 PN;
} paybee_kisscam_Ping_Payload_t;


/**
 * Set Mode Command
 */
typedef struct paybee_kisscam_SetMode_Payload {
    /**
     * `0` for sleep
     * `1` for SD
     * `2` for HD
     */
    uint8 MD;
} paybee_kisscam_SetMode_Payload_t;


/**
 * Memory Status Command
 * 
 * No Arg Command
 */


/**
 * Set Exposure Command
 */
typedef struct paybee_kisscam_SetExposure_Payload {
    uint8 EX1;
    uint8 EX2;
} paybee_kisscam_SetExposure_Payload_t;


/**
 * Capture Command
 */
typedef struct paybee_kisscam_Capture_Payload {
    uint8 MEM; /* Memory slot number. In HD mode, this paramter is ignored */
    uint8 TST;
} paybee_kisscam_Capture_Payload_t;


/**
 * Download Command
 * @param MEM Memory Slot: Can be `0` ~ `5`
 * @param PRE Preview Flag : `0` or `1`
 * @param LN1 MSB line number
 * @param LN2 LSB line number
 */
typedef struct paybee_kisscam_Download_Payload {
    uint8 MEM;
    uint8 PRE;
    uint8 LN1;
    uint8 LN2;
} paybee_kisscam_Download_Payload_t;


/**
 * Read Register Command
 */
typedef struct paybee_kisscam_ReadRegister_Payload {
    uint8 AD1;
    uint8 AD2;
} paybee_kisscam_ReadRegister_Payload_t;


/**
 * Write Register Command
 */
typedef struct paybee_kisscam_WriteRegister_Payload {
    uint8 AD1;
    uint8 AD2;
    uint8 RG1;
    uint8 RG2;
} paybee_kisscam_WriteRegister_Payload_t;


/**
 * Download All Command
 * @param MEM Memory Slot: Can be `0` ~ `3`
 * @param PRE Preview Flag : `0` or `1`
 * @param StartLine Starting line number: from `0` to `479`
 * @param LineNum Total number to download : `0` for one line download
 */
typedef struct paybee_kisscam_DownloadAll_Payload {
    uint8 MEM;
    uint8 PRE;

    uint16_t StartLine;
    uint16_t LineNum;
} paybee_kisscam_DownloadAll_Payload_t;


/**
 * MOSAIC Command
 * @param MEM Memory Slot: Recommend `4` or `5`
 */
// typedef struct paybee_kisscam_Mosaic_Payload {
    
//     uint8_t MEM;

// } paybee_kisscam_Mosaic_Payload_t;


/************************************************
 * 
 * Telemetry Payload Type Definition
 * 
 ************************************************/
// typedef struct paybee_kisscam_BcnTlm_Payload {
//     uint8 Mode;
//     /**
//      * `0` : Image Download Not-Started
//      * `1` : Image Download On-going
//      * `2` : Image Download Done
//      */
//     uint8 MemoryState[paybee_kisscam_MEMORY_SLOT];

//     /**
//      * Last Image number of specific memory slot. Used for image naming.
//      * e.g.) If 10 image downloaded on specific memory slot, then `LastImgIdx` become `10`
//      */
//     uint8 LastImgIdx[paybee_kisscam_MEMORY_SLOT];

// } paybee_kisscam_BcnTlm_Payload_t;

#endif