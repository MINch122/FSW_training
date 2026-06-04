/**
 * @file
 *   Specification for the PAY UZURO CAM command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in paybee_kisscam_msgdefs.h.
 */
#ifndef paybee_kisscam_MSGSTRUCT_H
#define paybee_kisscam_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "paybee_kisscam_mission_cfg.h"
#include "paybee_kisscam_msgdefs.h"
#include "cfe_msg_hdr.h"

#include "rpt_interface_cfg.h"

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
} paybee_kisscam_NoopCmd_t;

typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} paybee_kisscam_ResetCountersCmd_t;


/**
 * Ping Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    paybee_kisscam_Ping_Payload_t Payload;
} paybee_kisscam_PingCmd_t;


/**
 * Set Mode Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    paybee_kisscam_SetMode_Payload_t Payload;
} paybee_kisscam_SetModeCmd_t;


/**
 * Memory Status Command
 * 
 * No Arg Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
} paybee_kisscam_MemoryStatusCmd_t;


/**
 * Set Exposure Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    paybee_kisscam_SetExposure_Payload_t Payload;
} paybee_kisscam_SetExposureCmd_t;


/**
 * Capture Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    paybee_kisscam_Capture_Payload_t Payload;
} paybee_kisscam_CaptureCmd_t;


/**
 * Download Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    paybee_kisscam_Download_Payload_t Payload;
} paybee_kisscam_DownloadCmd_t;


/**
 * Read Register Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    paybee_kisscam_ReadRegister_Payload_t Payload;
} paybee_kisscam_ReadRegisterCmd_t;


/**
 * Write Register Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    paybee_kisscam_WriteRegister_Payload_t Payload;
} paybee_kisscam_WriteRegisterCmd_t;


/**
 * Download All Command
 */
typedef struct {
    CFE_MSG_CommandHeader_t CommandHeader;
    paybee_kisscam_DownloadAll_Payload_t Payload;
} paybee_kisscam_DownloadAllCmd_t;

/**
 * Mosaic Command
 */
// typedef struct {
//     CFE_MSG_CommandHeader_t CommandHeader;
//     paybee_kisscam_Mosaic_Payload_t Payload;
// } paybee_kisscam_MosaicCmd_t;



/*************************************************************************/
/*
** Type definition (PAY UZURO CAM App housekeeping)
*/
// typedef struct {
//     CFE_MSG_CommandHeader_t CommandHeader;
// } paybee_kisscam_SendHkCmd_t;

// typedef paybee_kisscam_SendHkCmd_t paybee_kisscam_SendBcnCmd_t;

// typedef struct {
//     CFE_MSG_TelemetryHeader_t TelemetryHeader;
//     paybee_kisscam_BcnTlm_Payload_t Payload;
// }__attribute__((packed)) paybee_kisscam_BcnTlm_t;

/**
 * Rerport MSG struct for RPT
 */
typedef struct {
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    RPT_Report_t Report;
} paybee_kisscam_ReportTlm_t;

#endif