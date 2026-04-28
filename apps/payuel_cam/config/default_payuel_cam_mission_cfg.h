#ifndef PAYUEL_CAM_MISSION_CFG_H
#define PAYUEL_CAM_MISSION_CFG_H

#include "payuel_cam_interface_cfg.h"

/* CSP Node Definition */
#define PAYUEL_CAM_NODE      13  /**< CAM Interface Board (RPi CM4) CSP node */

/* CSP Port Definition */
#define PAYUEL_CAM_PORT      25  /**< CAM Payload communication port */

/* -------------------------------------------------------------------
 * CSP Command IDs -- CAM Interface Board ICD
 * ------------------------------------------------------------------- */
#define PAYUEL_CAM_ID_SHOT            0x41 /**< CAPTURE (RAW 생성) */
#define PAYUEL_CAM_ID_HEALTH_CHECK    0x42 /**< HEALTHCHECK */
#define PAYUEL_CAM_ID_PROCESS_BINNING 0x43 /**< POSTPROCESS (사진 후처리) */
#define PAYUEL_CAM_ID_DOWNLOAD_META   0x44 /**< DOWNLOAD META */
#define PAYUEL_CAM_ID_CHUNK_DOWNLOAD  0x45 /**< DOWNLOAD CHUNK */
#define PAYUEL_CAM_ID_BEACON          0x46 /**< BEACON (상태 조회) */

/* Image chunk data size (excluding header and CRC) */
#define PAYUEL_CAM_CHUNK_DATA_SIZE   247

/* Maximum response size for 0x45 chunk download over CSP/CAN */
#define PAYUEL_CAM_CHUNK_RESPONSE_SIZE 256

/* Legacy SPI path kept commented for reference.
 * #define PAYUEL_CAM_SPI_CHUNK_SIZE  256
 * #define PAYUEL_CAM_SPI_INTERVAL_US 5000
 */

#endif
