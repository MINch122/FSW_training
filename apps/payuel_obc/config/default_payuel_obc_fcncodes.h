#ifndef PAYUEL_OBC_FCNCODES_H
#define PAYUEL_OBC_FCNCODES_H

/* cFS Ground Command Codes */
#define PAYUEL_OBC_NOOP_CC                 0
#define PAYUEL_OBC_RESET_COUNTERS_CC       1
#define PAYUEL_OBC_SEND_OBC_BCN_CC         2   /**< UEL OBC 비콘 요청       -> CSP 0x20 -> UEL_NODE */
#define PAYUEL_OBC_MOTOR_MODE_CC           3   /**< 모터 모드 제어           -> CSP 0x21 -> UEL_NODE */
#define PAYUEL_OBC_CAM_SHOT_CC             4   /**< 사진 촬영 요청           -> CSP 0x41 -> UEL_NODE */
#define PAYUEL_OBC_DOWNLOAD_META_CC        5   /**< 이미지 다운로드 메타     -> CSP 0x44 -> UEL_NODE */
#define PAYUEL_OBC_CHUNK_DOWNLOAD_CC       6   /**< 이미지 청크 다운로드     -> CSP 0x45 -> UEL_NODE */
#define PAYUEL_OBC_SENSOR_META_CC          7   /**< 센서 다운로드 메타       -> CSP 0x54 -> UEL_NODE */
#define PAYUEL_OBC_SENSOR_CHUNK_CC         8   /**< 센서 청크 다운로드       -> CSP 0x55 -> UEL_NODE */
#define PAYUEL_OBC_DOWNLOAD_IMAGE_CC       9   /**< cFS aggregate cmd: image meta + all chunks + file CRC32 verify */
#define PAYUEL_OBC_DOWNLOAD_SENSOR_CC      10  /**< cFS aggregate cmd: sensor meta + all chunks */

#endif /* PAYUEL_OBC_FCNCODES_H */
