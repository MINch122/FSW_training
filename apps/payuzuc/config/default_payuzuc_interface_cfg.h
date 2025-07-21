/**
 * @file
 *   SAMPLE_APP Application Public Definitions
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
#ifndef PAYUZUC_INTERFACE_CFG_H
#define PAYUZUC_INTERFACE_CFG_H

/**
 * Define Command Packet size
 * **Every command** has same size
 */
#define PAYUZUC_CMD_PKT_SIZE            7


/**
 * Define Tlm Header, Tail size
 */
#define PAYUZUC_TLM_HDR_SIZE            5
#define PAYUZUC_TLM_TAIL_SIZE           1

#define PAYUZUC_HDR_TAIL_SIZE           PAYUZUC_TLM_HDR_SIZE + PAYUZUC_TLM_TAIL_SIZE

/**
 * Image Size
 * This value means **one line (row)** of Image
 */
#define PAYUZUC_IMG_SIZE                640
#define PAYUZUC_THUMBNAIL_IMG_SIZE      64

#define PAYUZUC_IMG_LINE_NUM            480
#define PAYUZUC_THUMBNAIL_IMG_LINE_NUM  48


/* Var `data` means only real payload size */
#define PAYUZUC_GET_TLM_SIZE(data)\
        (PAYUZUC_TLM_HDR_SIZE + PAYUZUC_TLM_TAIL_SIZE + (data))

/*****************************************************************
 * Define Telemetry Packet size
 ****************************************************************/
#define PAYUZUC_PING_TLM_SIZE                   PAYUZUC_GET_TLM_SIZE(4)
#define PAYUZUC_SET_MODE_TLM_SIZE               PAYUZUC_GET_TLM_SIZE(0)
#define PAYUZUC_MEMORY_STATUS_TLM_SIZE          PAYUZUC_GET_TLM_SIZE(20)
#define PAYUZUC_SET_EXPOSURE_TLM_SIZE           PAYUZUC_GET_TLM_SIZE(0)
#define PAYUZUC_CAPTURE_TLM_SIZE                PAYUZUC_GET_TLM_SIZE(2)
#define PAYUZUC_DOWNLOAD_TLM_SIZE               PAYUZUC_GET_TLM_SIZE(PAYUZUC_IMG_SIZE + 2) // +2 is returned line number
#define PAYUZUC_DOWNLOAD_THUMBNAIL_TLM_SIZE     PAYUZUC_GET_TLM_SIZE(PAYUZUC_THUMBNAIL_IMG_SIZE + 2)  // +2 is returned line number
#define PAYUZUC_READ_REGISTER_TLM_SIZE          PAYUZUC_GET_TLM_SIZE(4)
#define PAYUZUC_WRITE_REGISTER_TLM_SIZE         PAYUZUC_GET_TLM_SIZE(0)

#define PAYUZUC_ERROR_TLM_SIZE                  PAYUZUC_GET_TLM_SIZE(3)



/*******************************************
 * Download Table Define
 *******************************************/
#define PAYUZUC_MEMORY_SLOT             6

#define PAYUZUC_DOWNLOAD_NOT_STARTED    0
#define PAYUZUC_DOWNLOAD_ON_GOING       1
#define PAYUZUC_DOWNLOAD_DONE           2

#endif