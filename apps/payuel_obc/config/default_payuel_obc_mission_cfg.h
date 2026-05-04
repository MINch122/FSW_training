#ifndef PAYUEL_OBC_MISSION_CFG_H
#define PAYUEL_OBC_MISSION_CFG_H

#include "payuel_obc_interface_cfg.h"

/* CSP Node Definition */
#define PAYUEL_OBC_NODE      11  /**< UEL OBC (STM32) CSP node (임시) */

/* CSP Port Definition */
#define PAYUEL_OBC_PORT      25  /**< UEL Payload communication port */

/* -------------------------------------------------------------------
 * CSP Command IDs -- UEL OBC ICD (NARA_OBC -> UEL Payload)
 * ------------------------------------------------------------------- */
#define PAYUEL_OBC_ID_OBC_BEACON       0x20  /**< UEL OBC 비콘 (상태 요약) */
#define PAYUEL_OBC_ID_MOTOR_MODE       0x21  /**< 모터 모드 제어 */

#define PAYUEL_OBC_ID_CAM_SHOT         0x41  /**< 사진 촬영 요청 */
#define PAYUEL_OBC_ID_DOWNLOAD_META    0x44  /**< 이미지 다운로드 메타 요청 */
#define PAYUEL_OBC_ID_CHUNK_DOWNLOAD   0x45  /**< 이미지 청크 다운로드 요청 */

#define PAYUEL_OBC_ID_SENSOR_META      0x54  /**< 센서 데이터 다운로드 메타 요청 */
#define PAYUEL_OBC_ID_SENSOR_CHUNK     0x55  /**< 센서 데이터 청크 다운로드 요청 */

/* Image/Sensor chunk data size (excluding header and CRC) */
#define PAYUEL_OBC_CHUNK_DATA_SIZE     247

/* Maximum image chunk response size (header 7B + data 247B + CRC16 2B) */
#define PAYUEL_OBC_CHUNK_RESPONSE_SIZE 256   /**< 0x45 max, 0x55 is one byte shorter */

/* Legacy SPI configuration kept commented for reference.
 * #define PAYUEL_OBC_SPI_CHUNK_SIZE  256
 * #define PAYUEL_OBC_SPI_INTERVAL_US 5000
 */

#endif
