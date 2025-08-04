/**
 * @file
 *   PAY UZURO CAM Application Private Config Definitions
 *
 * This provides default values for configurable items that are internal
 * to this module and do NOT affect the interface(s) of this module.  Changes
 * to items in this file only affect the local module and will be transparent
 * to external entities that are using the public interface(s).
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef PAYUZUC_INTERNAL_CFG_H
#define PAYUZUC_INTERNAL_CFG_H


#include "common_types.h"


#define PAYUZUC_PIPE_DEPTH  50 /* Depth of the Command Pipe for Application */


/**
 * Define Packet Start, End Byte
 */
#define PAYUZUC_PKT_START_BYTE          0x40
#define PAYUZUC_PKT_TERMINATE_BYTE      0x0D

/**
 * Define Command Code of KissCAM
 */
#define PAYUZUC_PING_CMD_CODE           0x50 /* `P` */
#define PAYUZUC_SET_MODE_CMD_CODE       0x4D /* `M` */
#define PAYUZUC_MEMORY_STATUS_CMD_CODE  0x53 /* `S` */
#define PAYUZUC_SET_EXPOSURE_CMD_CODE   0x45 /* `E` */
#define PAYUZUC_CAPTURE_CMD_CODE        0x43 /* `C` */
#define PAYUZUC_DOWNLOAD_CMD_CODE       0x44 /* `D` */
#define PAYUZUC_READ_REGISTER_CMD_CODE  0x52 /* `R` */
#define PAYUZUC_WRITE_REGISTER_CMD_CODE 0x57 /* `W` */
#define PAYUZUC_MOSAIC_CMD_CODE         0x51 /* `Q` */

/**
 * Define Param number
 */
#define PAYUZUC_PING_PARAM_SIZE           1
#define PAYUZUC_SET_MODE_PARAM_SIZE       1
#define PAYUZUC_MEMORY_STATUS_PARAM_SIZE  0
#define PAYUZUC_SET_EXPOSURE_PARAM_SIZE   2
#define PAYUZUC_CAPTURE_PARAM_SIZE        2
#define PAYUZUC_DOWNLOAD_PARAM_SIZE       4
#define PAYUZUC_READ_REGISTER_PARAM_SIZE  2
#define PAYUZUC_WRITE_REGISTER_PARAM_SIZE 4
#define PAYUZUC_MOSAIC_PARAM_SIZE         1

/**
 * Define Tlm Error flag
 */
#define PAYUZUC_TLM_ERR_FLAG                0xFF

/**
 * Download Image flag
 * Only used for Download command
 */
#define PAYUZUC_DOWNLOAD_ORIGINAL_FLAG      0x00
#define PAYUZUC_DOWNLOAD_THUMBNAIL_FLAG     0x01


/**
 * PAYUZUC File Path Definition
 */
#define PAYUZUC_TBL_PATH            "./cf/sdcard/PAYUZUC.tbl"
#define PAYUZUC_IMG_PATH            "./cf/sdcard/Kiss"

typedef struct {
    uint8 StartByte; /* Always `0x40` */
    uint8 Command;
    uint8 Params[4]; /* Big endian */
    uint8 EndByte; /* Always `0x0D` */
} PAYUZUC_Cmd_s;

typedef union PAYUZUC_Cmd
{
    PAYUZUC_Cmd_s Packet;
    uint8 Bytes[7];
} PAYUZUC_Cmd_t;


#endif