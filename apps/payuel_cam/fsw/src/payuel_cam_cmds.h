#ifndef PAYUEL_CAM_CMDS_H
#define PAYUEL_CAM_CMDS_H

#include "cfe_error.h"
#include "payuel_cam_msg.h"

/*
 * No-op command used to verify that the PAYUEL CAM app command pipe is alive.
 * Arguments: none.
 * Example: send the command with no payload fields.
 */
CFE_Status_t PAYUEL_CAM_NoopCmd(const PAYUEL_CAM_NoopCmd_t *Msg);
/*
 * Reset the app command/error counters after a test sequence.
 * Arguments: none.
 * Example: send the command with no payload fields before a fresh test run.
 */
CFE_Status_t PAYUEL_CAM_ResetCountersCmd(const PAYUEL_CAM_ResetCountersCmd_t *Msg);
/*
 * 0x41 Camera shot command.
 * Fill ImageSlot with the destination slot to store the new image.
 * Fill CameraNumber with the camera index defined by the payload ICD.
 * Example: ImageSlot=0, CameraNumber=1 captures a new image from camera 1 into slot 0.
 */
CFE_Status_t PAYUEL_CAM_ShotCmd(const PAYUEL_CAM_ShotCmd_t *Msg);
/*
 * 0x42 Health check command.
 * Arguments: none.
 * Example: send the command with no payload fields to request a health response/beacon.
 */
CFE_Status_t PAYUEL_CAM_HealthCheckCmd(const PAYUEL_CAM_HealthCheckCmd_t *Msg);
/*
 * 0x43 Process/binning command for an existing captured image.
 * Fill ImageSlot with the slot that already contains the source image.
 * Fill ImageNumber with the image index returned by the shot/meta flow.
 * Example: ImageSlot=0, ImageNumber=3 processes image 3 stored in slot 0.
 */
CFE_Status_t PAYUEL_CAM_ProcessBinningCmd(const PAYUEL_CAM_ProcessBinningCmd_t *Msg);
/*
 * 0x44 Download-meta command.
 * Fill ImageSlot and ImageNumber for the image whose metadata you want to query.
 * Use this before a manual 0x45 chunk download so total chunks and last-chunk size are known.
 * Example: ImageSlot=0, ImageNumber=3 requests metadata for image 3 in slot 0.
 */
CFE_Status_t PAYUEL_CAM_DownloadMetaCmd(const PAYUEL_CAM_DownloadMetaCmd_t *Msg);
/*
 * 0x45 Single chunk download command.
 * Fill ImageSlot and ImageNumber to identify the image, then ChunkNumber for the chunk index.
 * ChunkNumber is zero-based and should be less than the total chunk count from 0x44 metadata.
 * Example: ImageSlot=0, ImageNumber=3, ChunkNumber=0 downloads the first chunk of that image.
 */
CFE_Status_t PAYUEL_CAM_ChunkDownloadCmd(const PAYUEL_CAM_ChunkDownloadCmd_t *Msg);
/*
 * cFS aggregate image download command.
 * Fill ImageSlot and ImageNumber for the image to download end-to-end.
 * The child task will run 0x44, download all 0x45 chunks, assemble the file, and verify File CRC32.
 * Example: ImageSlot=0, ImageNumber=3 downloads the full image 3 from slot 0 in one command.
 */
CFE_Status_t PAYUEL_CAM_DownloadImageCmd(const PAYUEL_CAM_DownloadImageCmd_t *Msg);
/*
 * 0x46 Beacon command.
 * Arguments: none.
 * Example: send the command with no payload fields to request the PAYUEL CAM beacon packet.
 */
CFE_Status_t PAYUEL_CAM_SendBcnCmd(const PAYUEL_CAM_SendBcnCmd_t *Msg);

#endif /* PAYUEL_CAM_CMDS_H */
