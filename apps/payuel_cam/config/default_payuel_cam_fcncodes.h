#ifndef PAYUEL_CAM_FCNCODES_H
#define PAYUEL_CAM_FCNCODES_H

/* cFS Ground Command Codes */
#define PAYUEL_CAM_NOOP_CC                 0
#define PAYUEL_CAM_RESET_COUNTERS_CC       1
#define PAYUEL_CAM_SHOT_CC                 2   /**< 0x41 -> CAM_NODE */
#define PAYUEL_CAM_HEALTH_CHECK_CC         3   /**< 0x42 -> CAM_NODE */
#define PAYUEL_CAM_PROCESS_BINNING_CC      4   /**< 0x43 -> CAM_NODE */
#define PAYUEL_CAM_DOWNLOAD_META_CC        5   /**< 0x44 -> CAM_NODE */
#define PAYUEL_CAM_CHUNK_DOWNLOAD_CC       6   /**< 0x45 -> CAM_NODE */
#define PAYUEL_CAM_SEND_BCN_CC             7   /**< 0x46 -> CAM_NODE */
#define PAYUEL_CAM_DOWNLOAD_IMAGE_CC       8   /**< cFS aggregate cmd: meta + chunk download + CRC32 verify */

#endif /* PAYUEL_CAM_FCNCODES_H */
