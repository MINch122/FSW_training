/**
 * @file
 *   Specification for the FTP command and telemetry
 *   message constant definitions.
 *
 *  For SLT FTP this is only the function/command code definitions
 */
#ifndef FTP_MSGDEFS_H
#define FTP_MSGDEFS_H

#include "common_types.h"
#include "ftp_fcncodes.h"
#include "ftp_internal_cfg.h"

/************************************************
 * 
 * Command Payload Type Definition
 * 
 ************************************************/
typedef struct FTP_SendFileCmd_Payload {
    /**
     * \brief File name should be include the whole path of file
     * \note This value must include a terminating NUL character
     * \note Also, single file name shoul not exceed the value `OS_MAX_FILE_NAME`
     */
    char FileName[64];

    /* Start Byte of specific file */
    uint32_t StartByte;

    /* End Byte of specific file */
    uint32_t EndByte;

    /* Time limitation for file transfer in minutes */
    uint8_t TimeLimit;

    uint8_t Padding[3];

} FTP_SendFileCmd_Payload_t;
/************************************************
 * 
 * Telemetry Payload Type Definition
 * 
 ************************************************/
typedef struct FTP_HkTlm_Payload {
    uint8 CommandCounter;
    uint8 CommandErrorCounter;

    /**
     * Else ....
     */
    // .....
} FTP_HkTlm_Payload_t;

typedef struct FTP_BcnTlm_Payload {
    uint8 CommandCounter;
    uint8 CommandErrorCounter;

    uint16_t TransferCnt;       /* <\brief Total transfered chunk count */
    uint16_t TransferErrCnt;    /* <\brief Total error count */
} FTP_BcnTlm_Payload_t;

typedef struct FTP_File_Payload {
    uint8_t Bytes[FTP_MAX_CHUNK_SIZE];
} FTP_File_Payload_t;

#endif