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
#ifndef paybee_kisscam_INTERNAL_CFG_H
#define paybee_kisscam_INTERNAL_CFG_H


#include "common_types.h"


#define paybee_kisscam_PIPE_DEPTH  50 /* Depth of the Command Pipe for Application */

/* Download Child Task Configuration */
// #define paybee_kisscam_CHILD_TASK_NAME         "paybee_kisscam_ChildTask"
// #define paybee_kisscam_CHILD_STACK_SIZE(n)     (4096 *(n))
// #define paybee_kisscam_CHILD_PRIORITY          90


/**
 * Define Packet Start, End Byte
 */
#define paybee_kisscam_PKT_START_BYTE          0x40
#define paybee_kisscam_PKT_TERMINATE_BYTE      0x0D

/**
 * Define Command Code of KissCAM
 */
#define paybee_kisscam_PING_CMD_CODE           0x50 /* `P` */
#define paybee_kisscam_SET_MODE_CMD_CODE       0x4D /* `M` */
#define paybee_kisscam_MEMORY_STATUS_CMD_CODE  0x53 /* `S` */
#define paybee_kisscam_SET_EXPOSURE_CMD_CODE   0x45 /* `E` */
#define paybee_kisscam_CAPTURE_CMD_CODE        0x43 /* `C` */
#define paybee_kisscam_DOWNLOAD_CMD_CODE       0x44 /* `D` */
#define paybee_kisscam_READ_REGISTER_CMD_CODE  0x52 /* `R` */
#define paybee_kisscam_WRITE_REGISTER_CMD_CODE 0x57 /* `W` */
// #define paybee_kisscam_MOSAIC_CMD_CODE         0x51 /* `Q` */

/**
 * Define Param number
 */
#define paybee_kisscam_PING_PARAM_SIZE           1
#define paybee_kisscam_SET_MODE_PARAM_SIZE       1
#define paybee_kisscam_MEMORY_STATUS_PARAM_SIZE  0
#define paybee_kisscam_SET_EXPOSURE_PARAM_SIZE   2
#define paybee_kisscam_CAPTURE_PARAM_SIZE        2
#define paybee_kisscam_DOWNLOAD_PARAM_SIZE       4
#define paybee_kisscam_READ_REGISTER_PARAM_SIZE  2
#define paybee_kisscam_WRITE_REGISTER_PARAM_SIZE 4
// #define paybee_kisscam_MOSAIC_PARAM_SIZE         1

/**
 * Define Tlm Error flag
 */
#define paybee_kisscam_TLM_ERR_FLAG                0xFF

/**
 * Download Image flag
 * Only used for Download command
 */
#define paybee_kisscam_DOWNLOAD_ORIGINAL_FLAG      0x00
#define paybee_kisscam_DOWNLOAD_THUMBNAIL_FLAG     0x01


/**
 * paybee_kisscam File Path Definition
 */
#define paybee_kisscam_TBL_PATH            "./cf/sdcard/paybee_kisscam.tbl"
#define paybee_kisscam_IMG_PATH            "./cf/sdcard/Kiss"

/* paybee_kisscam Mutex */
// #define paybee_kisscam_MUTEX_NAME          "paybee_kisscam_MUTEX"

typedef struct {
    uint8 StartByte; /* Always `0x40` */
    uint8 Command;
    uint8 Params[4]; /* Big endian */
    uint8 EndByte; /* Always `0x0D` */
} paybee_kisscam_Cmd_s;

typedef union paybee_kisscam_Cmd
{
    paybee_kisscam_Cmd_s Packet;
    uint8 Bytes[7];
} paybee_kisscam_Cmd_t;


#endif