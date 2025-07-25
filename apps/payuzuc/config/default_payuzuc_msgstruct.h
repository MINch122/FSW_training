/**
 * @file
 *   Specification for the PAY UZURO CAM command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in payuzuc_msgdefs.h.
 */
#ifndef PAYUZUC_MSGSTRUCT_H
#define PAYUZUC_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "payuzuc_mission_cfg.h"
#include "payuzuc_msgdefs.h"
#include "cfe_msg_hdr.h"

/*************************************************************************/

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUC_NoopCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUC_ResetCountersCmd_t;


/**
 * Ping Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_Ping_Payload_t Payload;
} PAYUZUC_PingCmd_t;


/**
 * Set Mode Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_SetMode_Payload_t Payload;
} PAYUZUC_SetModeCmd_t;


/**
 * Memory Status Command
 * 
 * No Arg Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUC_MemoryStatusCmd_t;


/**
 * Set Exposure Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_SetExposure_Payload_t Payload;
} PAYUZUC_SetExposureCmd_t;


/**
 * Capture Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_Capture_Payload_t Payload;
} PAYUZUC_CaptureCmd_t;


/**
 * Download Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_Download_Payload_t Payload;
} PAYUZUC_DownloadCmd_t;


/**
 * Read Register Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_ReadRegister_Payload_t Payload;
} PAYUZUC_ReadRegisterCmd_t;


/**
 * Write Register Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_WriteRegister_Payload_t Payload;
} PAYUZUC_WriteRegisterCmd_t;


/**
 * Download All Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_DownloadAll_Payload_t Payload;
} PAYUZUC_DownloadAllCmd_t;

/**
 * Mosaic Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    PAYUZUC_Mosaic_Payload_t Payload;
} PAYUZUC_MosaicCmd_t;



/*************************************************************************/
/*
** Type definition (PAY UZURO CAM App housekeeping)
*/
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} PAYUZUC_SendHkCmd_t;

typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    PAYUZUC_HkTlm_Payload_t Payload;
} PAYUZUC_HkTlm_t;

/**
 * Image MSG struct
 */
typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint8_t Payload[640];
} PAYUZUC_ImgTlm_t;

#endif