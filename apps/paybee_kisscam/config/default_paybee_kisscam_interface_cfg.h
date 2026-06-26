/**
 * @file
 *   PAYUZUZ Application Public Definitions
 *
 * This provides default values for configurable items that affect
 * the interface(s) of this module.  This includes the CMD/TLM message
 * interface, tables definitions, and any other data products that
 * serve to exchange information with other entities.
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef paybee_kisscam_INTERFACE_CFG_H
#define paybee_kisscam_INTERFACE_CFG_H

/**
 * Define Command Packet size
 * **Every command** has same size
 */
#define paybee_kisscam_CMD_PKT_SIZE            7


/**
 * Define Tlm Header, Tail size
 */
#define paybee_kisscam_TLM_HDR_SIZE            5
#define paybee_kisscam_TLM_TAIL_SIZE           1

#define paybee_kisscam_HDR_TAIL_SIZE           paybee_kisscam_TLM_HDR_SIZE + paybee_kisscam_TLM_TAIL_SIZE

/**
 * Image Size
 * This value means **one line (row)** of Image
 */
#define paybee_kisscam_IMG_SIZE                640
#define paybee_kisscam_THUMBNAIL_IMG_SIZE      64

#define paybee_kisscam_IMG_LINE_NUM            480
#define paybee_kisscam_THUMBNAIL_IMG_LINE_NUM  48


/* Var `data` means only real payload size */
#define paybee_kisscam_GET_TLM_SIZE(data)\
        (paybee_kisscam_TLM_HDR_SIZE + paybee_kisscam_TLM_TAIL_SIZE + (data))

/*****************************************************************
 * Define Telemetry Packet size
 ****************************************************************/
#define paybee_kisscam_PING_TLM_SIZE                   paybee_kisscam_GET_TLM_SIZE(4)
#define paybee_kisscam_SET_MODE_TLM_SIZE               paybee_kisscam_GET_TLM_SIZE(0)
#define paybee_kisscam_MEMORY_STATUS_TLM_SIZE          paybee_kisscam_GET_TLM_SIZE(20)
#define paybee_kisscam_SET_EXPOSURE_TLM_SIZE           paybee_kisscam_GET_TLM_SIZE(0)
#define paybee_kisscam_CAPTURE_TLM_SIZE                paybee_kisscam_GET_TLM_SIZE(0)
#define paybee_kisscam_DOWNLOAD_TLM_SIZE               paybee_kisscam_GET_TLM_SIZE(paybee_kisscam_IMG_SIZE + 2) // +2 is returned line number
#define paybee_kisscam_DOWNLOAD_THUMBNAIL_TLM_SIZE     paybee_kisscam_GET_TLM_SIZE(paybee_kisscam_THUMBNAIL_IMG_SIZE + 2)  // +2 is returned line number
#define paybee_kisscam_READ_REGISTER_TLM_SIZE          paybee_kisscam_GET_TLM_SIZE(4)
#define paybee_kisscam_WRITE_REGISTER_TLM_SIZE         paybee_kisscam_GET_TLM_SIZE(0)
#define paybee_kisscam_MOSAIC_TLM_SIZE                 paybee_kisscam_GET_TLM_SIZE(0)

#define paybee_kisscam_ERROR_TLM_SIZE                  paybee_kisscam_GET_TLM_SIZE(3)



/*******************************************
 * Download Table Define
 *******************************************/
#define paybee_kisscam_MEMORY_SLOT             4

#define paybee_kisscam_DOWNLOAD_NOT_STARTED    0
#define paybee_kisscam_DOWNLOAD_ON_GOING       1
#define paybee_kisscam_DOWNLOAD_DONE           2

#endif