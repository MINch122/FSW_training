#ifndef PAYUEL_OBC_CMDS_H
#define PAYUEL_OBC_CMDS_H

#include "cfe_error.h"
#include "payuel_obc_msg.h"

/*
 * No-op command used to verify that the PAYUEL OBC app command pipe is alive.
 * Arguments: none.
 * Example: send the command with no payload fields.
 */
CFE_Status_t PAYUEL_OBC_NoopCmd(const PAYUEL_OBC_NoopCmd_t *Msg);
/*
 * Reset the app command/error counters after a test sequence.
 * Arguments: none.
 * Example: send the command with no payload fields before a fresh test run.
 */
CFE_Status_t PAYUEL_OBC_ResetCountersCmd(const PAYUEL_OBC_ResetCountersCmd_t *Msg);
/*
 * 0x20 UEL OBC beacon command.
 * Arguments: none.
 * Example: send the command with no payload fields to request the latest UEL OBC beacon.
 */
CFE_Status_t PAYUEL_OBC_SendObcBcnCmd(const PAYUEL_OBC_SendObcBcnCmd_t *Msg);
/*
 * 0x21 Motor mode command.
 * Fill L_Speed and R_Speed with signed motor speeds, Duration with run time, and Instance with target instance.
 * Use negative values for reverse direction if that is defined by the ICD.
 * Example: L_Speed=10, R_Speed=10, Duration=5, Instance=0 drives both motors for 5 units on instance 0.
 */
CFE_Status_t PAYUEL_OBC_MotorModeCmd(const PAYUEL_OBC_MotorModeCmd_t *Msg);
/*
 * 0x41 Camera shot command through the UEL OBC path.
 * Fill ImageSlot with the destination slot and CameraNumber with the camera index.
 * Example: ImageSlot=0, CameraNumber=1 captures a new image from camera 1 into slot 0.
 */
CFE_Status_t PAYUEL_OBC_CamShotCmd(const PAYUEL_OBC_CamShotCmd_t *Msg);
/*
 * 0x44 Image download-meta command.
 * Fill ImageSlot and ImageNumber for the image whose metadata you want to query.
 * Use this before a manual 0x45 chunk download so total chunks and last-chunk size are known.
 * Example: ImageSlot=0, ImageNumber=2 requests metadata for image 2 in slot 0.
 */
CFE_Status_t PAYUEL_OBC_DownloadMetaCmd(const PAYUEL_OBC_DownloadMetaCmd_t *Msg);
/*
 * 0x45 Single image chunk download command.
 * Fill ImageSlot and ImageNumber to identify the image, then ChunkNumber for the zero-based chunk index.
 * ChunkNumber should be less than the total chunk count returned by 0x44 metadata.
 * Example: ImageSlot=0, ImageNumber=2, ChunkNumber=0 downloads the first image chunk.
 */
CFE_Status_t PAYUEL_OBC_ChunkDownloadCmd(const PAYUEL_OBC_ChunkDownloadCmd_t *Msg);
/*
 * 0x54 Sensor-data metadata command.
 * Fill DataSlot and DataNumber for the sensor dataset whose metadata you want to query.
 * Use this before a manual 0x55 sensor chunk download so total chunks and final size are known.
 * Example: DataSlot=1, DataNumber=4 requests metadata for sensor dataset 4 in slot 1.
 */
CFE_Status_t PAYUEL_OBC_SensorMetaCmd(const PAYUEL_OBC_SensorMetaCmd_t *Msg);
/*
 * 0x55 Single sensor-data chunk download command.
 * Fill DataSlot and DataNumber to identify the dataset, then ChunkNumber for the zero-based chunk index.
 * ChunkNumber should be less than the total chunk count returned by 0x54 metadata.
 * Example: DataSlot=1, DataNumber=4, ChunkNumber=0 downloads the first sensor-data chunk.
 */
CFE_Status_t PAYUEL_OBC_SensorChunkCmd(const PAYUEL_OBC_SensorChunkCmd_t *Msg);
/*
 * cFS aggregate image download command.
 * Fill ImageSlot and ImageNumber for the image to download end-to-end.
 * The child task will run 0x44, download all 0x45 chunks, assemble the file, and verify Image File CRC32.
 * Example: ImageSlot=0, ImageNumber=2 downloads the full image 2 from slot 0 in one command.
 */
CFE_Status_t PAYUEL_OBC_DownloadImageCmd(const PAYUEL_OBC_DownloadImageCmd_t *Msg);
/*
 * cFS aggregate sensor download command.
 * Fill DataSlot and DataNumber for the sensor dataset to download end-to-end.
 * The child task will run 0x54, download all 0x55 chunks, and assemble the complete sensor file.
 * Example: DataSlot=1, DataNumber=4 downloads the full sensor dataset 4 from slot 1 in one command.
 */
CFE_Status_t PAYUEL_OBC_DownloadSensorCmd(const PAYUEL_OBC_DownloadSensorCmd_t *Msg);

#endif /* PAYUEL_OBC_CMDS_H */
