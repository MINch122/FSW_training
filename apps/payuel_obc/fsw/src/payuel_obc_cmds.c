#include "payuel_obc.h"
#include "payuel_obc_child.h"
#include "payuel_obc_cmds.h"
#include "payuel_obc_utils.h"
#include "payuel_obc_msgids.h"
#include "payuel_obc_eventids.h"
#include "payuel_obc_version.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/* ===================================================================
 * NOOP
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_NoopCmd(const PAYUEL_OBC_NoopCmd_t *Msg)
{
    (void)Msg;

    PAYUEL_OBC_Data.CmdCounter++;
    CFE_EVS_SendEvent(PAYUEL_OBC_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_OBC: NOOP command %s", PAYUEL_OBC_VERSION);
    return CFE_SUCCESS;
}

/* ===================================================================
 * RESET COUNTERS
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_ResetCountersCmd(const PAYUEL_OBC_ResetCountersCmd_t *Msg)
{
    (void)Msg;

    PAYUEL_OBC_Data.CmdCounter = 0;
    PAYUEL_OBC_Data.ErrCounter = 0;
    CFE_EVS_SendEvent(PAYUEL_OBC_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_OBC: RESET command");
    return CFE_SUCCESS;
}

/* ===================================================================
 * 0x20 - UEL OBC 비콘 요청
 * TX  (3 B):  [0x20][CRC16_H][CRC16_L]
 * RX (25 B):  [0x20][init_error(1)][boot_count(2)][boot_time_a(4)]
 *             [boot_count_b(2)][boot_time_b(4)][shutter_count_a(2)]
 *             [shutter_count_b(2)][alpha_temp(1)][beta_temp(1)]
 *             [imu_temp(1)][tc1(1)][sd_mount_state(1)]
 *             [CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_SendObcBcnCmd(const PAYUEL_OBC_SendObcBcnCmd_t *Msg)
{
    uint8_t  TxData[3];
    uint8_t  RxData[25] = {0};
    int32    rx_len;
    int32    status = CFE_SUCCESS;
    uint8_t *payload;

    (void)Msg;
    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("ObcBeacon") != CFE_SUCCESS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    TxData[0] = PAYUEL_OBC_ID_OBC_BEACON;
    PAYUEL_OBC_AppendCrc16(TxData, 1);
    OS_printf("TxData[1]=0x%02X, TxData[2]=0x%02X", TxData[1], TxData[2]);

    rx_len = CFE_SRL_ApiTransactionCSP(PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
                                       TxData, sizeof(TxData), RxData, sizeof(RxData));
    PAYUEL_OBC_UnlockHardware("ObcBeacon");

    status = PAYUEL_OBC_ValidateResponse("ObcBeacon", PAYUEL_OBC_ID_OBC_BEACON, RxData, rx_len,
                                         sizeof(RxData), NULL, NULL);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    payload = &RxData[1];
    PAYUEL_OBC_Data.obc_bcn.Payload.InitError      = payload[0];
    PAYUEL_OBC_Data.obc_bcn.Payload.BootCount_a    = PAYUEL_OBC_ReadU16BE(&payload[1]);
    PAYUEL_OBC_Data.obc_bcn.Payload.BootTime_a     = PAYUEL_OBC_ReadU32BE(&payload[3]);
    PAYUEL_OBC_Data.obc_bcn.Payload.BootCount_b    = PAYUEL_OBC_ReadU16BE(&payload[7]);
    PAYUEL_OBC_Data.obc_bcn.Payload.BootTime_b     = PAYUEL_OBC_ReadU32BE(&payload[9]);
    PAYUEL_OBC_Data.obc_bcn.Payload.ShutterCount_a = PAYUEL_OBC_ReadU16BE(&payload[13]);
    PAYUEL_OBC_Data.obc_bcn.Payload.ShutterCount_b = PAYUEL_OBC_ReadU16BE(&payload[15]);
    PAYUEL_OBC_Data.obc_bcn.Payload.AlphaTemp      = (int8_t)payload[17];
    PAYUEL_OBC_Data.obc_bcn.Payload.BetaTemp       = (int8_t)payload[18];
    PAYUEL_OBC_Data.obc_bcn.Payload.ImuTemp        = (int8_t)payload[19];
    PAYUEL_OBC_Data.obc_bcn.Payload.TC1            = (int8_t)payload[20];
    PAYUEL_OBC_Data.obc_bcn.Payload.SdMountState   = payload[21];

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUEL_OBC_Data.obc_bcn.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUEL_OBC_Data.obc_bcn.TelemetryHeader), true);

    OS_printf("PAYUEL_OBC: ObcBeacon init_error=%u, boot_count_a=%u, boot_time_a=%u, "
              "boot_count_b=%u, boot_time_b=%u, shutter_count_a=%u, shutter_count_b=%u, "
              "alpha_temp=%d, beta_temp=%d, imu_temp=%d, tc1=%d, sd_mount_state=%u\n",
              PAYUEL_OBC_Data.obc_bcn.Payload.InitError,
              PAYUEL_OBC_Data.obc_bcn.Payload.BootCount_a,
              PAYUEL_OBC_Data.obc_bcn.Payload.BootTime_a,
              PAYUEL_OBC_Data.obc_bcn.Payload.BootCount_b,
              PAYUEL_OBC_Data.obc_bcn.Payload.BootTime_b,
              PAYUEL_OBC_Data.obc_bcn.Payload.ShutterCount_a,
              PAYUEL_OBC_Data.obc_bcn.Payload.ShutterCount_b,
              PAYUEL_OBC_Data.obc_bcn.Payload.AlphaTemp,
              PAYUEL_OBC_Data.obc_bcn.Payload.BetaTemp,
              PAYUEL_OBC_Data.obc_bcn.Payload.ImuTemp,
              PAYUEL_OBC_Data.obc_bcn.Payload.TC1,
              PAYUEL_OBC_Data.obc_bcn.Payload.SdMountState);

    return status;
}

/* ===================================================================
 * 0x21 - 모터 모드 제어
 * TX  (6 B): [0x21][L_Speed(int8)][R_Speed(int8)][Duration]
 *            [CRC16_H][CRC16_L]
 * RX (15 B): [0x21][la_mA(2)][lb_mA(2)][lc_mA(2)]
 *            [lu_mA(2)][lv_mA(2)][lw_mA(2)][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_MotorModeCmd(const PAYUEL_OBC_MotorModeCmd_t *Msg)
{
    uint8_t  TxData[6];
    uint8_t  RxData[15] = {0};
    int32    rx_len;
    int32    status = CFE_SUCCESS;
    uint8_t  return_type = RPT_RETTYPE_SUCCESS;
    uint8_t *p;
    uint16_t la;
    uint16_t lb;
    uint16_t lc;
    uint16_t lu;
    uint16_t lv;
    uint16_t lw;
    uint8_t  payload_error[4] = {0};
    const uint8_t *report_ptr = &RxData[1];
    uint16   report_len = 12U;

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("MotorMode") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_OBC_ID_MOTOR_MODE;
        TxData[1] = (uint8_t)Msg->L_Speed;
        TxData[2] = (uint8_t)Msg->R_Speed;
        TxData[3] = Msg->Duration;
        PAYUEL_OBC_AppendCrc16(TxData, 4);

        rx_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));

        PAYUEL_OBC_UnlockHardware("MotorMode");

        status = PAYUEL_OBC_ValidateResponse("MotorMode", PAYUEL_OBC_ID_MOTOR_MODE, RxData, rx_len,
                                             sizeof(RxData), &return_type, payload_error);
        if (status == CFE_SUCCESS)
        {
            p  = &RxData[1];
            la = PAYUEL_OBC_ReadU16BE(&p[0]);
            lb = PAYUEL_OBC_ReadU16BE(&p[2]);
            lc = PAYUEL_OBC_ReadU16BE(&p[4]);
            lu = PAYUEL_OBC_ReadU16BE(&p[6]);
            lv = PAYUEL_OBC_ReadU16BE(&p[8]);
            lw = PAYUEL_OBC_ReadU16BE(&p[10]);

            OS_printf("PAYUEL_OBC: Motor la=%u lb=%u lc=%u lu=%u lv=%u lw=%u mA\n",
                      la, lb, lc, lu, lv, lw);
        }
        else if (payload_error[0] != 0U)
        {
            report_ptr = payload_error;
            report_len = sizeof(payload_error);
        }
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_MOTOR_MODE_CC,
                               status, report_ptr, report_len, return_type);
    return status;
}

/* ===================================================================
 * 0x41 - 사진 촬영 요청 (UEL OBC)
 * TX (5 B): [0x41][CameraID][ImageNumber][CRC16_H][CRC16_L]
 * RX (4 B): [0x41][Status][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_CamShotCmd(const PAYUEL_OBC_CamShotCmd_t *Msg)
{
    uint8_t TxData[5];
    uint8_t RxData[4] = {0};
    int32   rx_len;
    int32   status = CFE_SUCCESS;
    uint8_t return_type = RPT_RETTYPE_SUCCESS;

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("CamShot") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_OBC_ID_CAM_SHOT;
        TxData[1] = Msg->CameraID;
        TxData[2] = Msg->ImageNumber;
        PAYUEL_OBC_AppendCrc16(TxData, 3);

        rx_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));

        PAYUEL_OBC_UnlockHardware("CamShot");

        status = PAYUEL_OBC_ValidateResponse("CamShot", PAYUEL_OBC_ID_CAM_SHOT, RxData, rx_len,
                                             sizeof(RxData), &return_type, NULL);
        if (status == CFE_SUCCESS && RxData[1] != 0x00U)
        {
            return_type = RPT_RETTYPE_HW;
            status   = PAYUEL_OBC_RejectHwStatus("CamShot", RxData[1]);
        }
        else if (status == CFE_SUCCESS)
        {
            PAYUEL_OBC_ClearImageMetaCache();
            OS_printf("PAYUEL_OBC: CamShot OK, Status=0x%02X (image meta cache cleared)\n", RxData[1]);
        }
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_CAM_SHOT_CC,
                               status, &RxData[1], 1, return_type);
    return status;
}

/* ===================================================================
 * 0x44 - 이미지 다운로드 메타 요청 (UEL OBC)
 * TX  (5 B): [0x44][Camera ID][ImageNumber][CRC16_H][CRC16_L]
 * RX (18 B): [0x44][Status][CameraID][ImageValid][ImageIndex]
 *            [ImageSize(4B,BE)][ImageFileCRC32(4B,BE)][LastChunkSz]
 *            [ChunkCount(2B,BE)][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_DownloadMetaCmd(const PAYUEL_OBC_DownloadMetaCmd_t *Msg)
{
    CFE_Status_t              status = CFE_SUCCESS;
    uint8_t                   return_type = RPT_RETTYPE_SUCCESS;
    PAYUEL_OBC_ImageMetaInfo_t Meta = {0};
    uint8_t                   report_data[9] = {Msg->CameraID, Msg->ImageNumber, 0};
    uint8_t                   payload_error[4] = {0};
    const uint8_t            *report_ptr = report_data;
    uint16                    report_len = sizeof(report_data);

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("DownloadMeta") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        status = PAYUEL_OBC_RequestImageMeta(Msg->CameraID, Msg->ImageNumber, &Meta, &return_type, payload_error);
        PAYUEL_OBC_UnlockHardware("DownloadMeta");

        if (status == CFE_SUCCESS)
        {
            report_data[0] = Meta.CameraID;
            report_data[1] = Meta.ImageIndex;
            PAYUEL_OBC_WriteU16BE(&report_data[2], Meta.ChunkCount);
            report_data[4] = Meta.LastChunkSize;
            PAYUEL_OBC_WriteU32BE(&report_data[5], Meta.ImageFileCRC32);

            OS_printf("PAYUEL_OBC: DownloadMeta CameraID=0x%02X, ImageIndex=%u, "
                      "ImageValid=%u, ImageSize=%u, ChunkCount=%u, LastChunkSz=%u, "
                      "ImageFileCRC32=0x%08X\n",
                      Meta.CameraID, Meta.ImageIndex, Meta.ImageValid, Meta.ImageSize,
                      Meta.ChunkCount, Meta.LastChunkSize, Meta.ImageFileCRC32);
        }
        else if (payload_error[0] != 0U)
        {
            report_ptr = payload_error;
            report_len = sizeof(payload_error);
        }
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_DOWNLOAD_META_CC,
                               status, report_ptr, report_len, return_type);
    return status;
}

/* ===================================================================
 * 0x45 - 이미지 청크 다운로드 요청 (UEL OBC)
 * TX via CSP CAN (7 B):   [0x45][CameraID][ImageNumber][ChunkNum_H][ChunkNum_L]
 *                         [CRC16_H][CRC16_L]
 * RX via CSP CAN:         [0x45][Status][CameraID][ImgNum_H][ImgNum_L]
 *                         [ChunkNum_H][ChunkNum_L][ImageData][CRC16_H][CRC16_L]
 *                         length = 7 + valid data length + 2, max 256 B
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_ChunkDownloadCmd(const PAYUEL_OBC_ChunkDownloadCmd_t *Msg)
{
    CFE_Status_t              status = CFE_SUCCESS;
    uint8_t                   return_type = RPT_RETTYPE_SUCCESS;
    uint8_t                   ChunkData[PAYUEL_OBC_CHUNK_DATA_SIZE];
    PAYUEL_OBC_ImageMetaInfo_t Meta = {0};
    size_t                    valid_data_len = 0U;
    char                      filename[64];
    int                       fd;
    ssize_t                   written;
    uint8_t                   payload_error[4] = {0};
    const uint8_t            *report_ptr = NULL;
    uint16                    report_len = 0U;

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("ChunkDownload") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        if (!PAYUEL_OBC_GetCachedImageMeta(Msg->CameraID, Msg->ImageNumber, &Meta))
        {
            OS_printf("PAYUEL_OBC: ChunkDownload cache miss, fetching meta for camera=0x%02X image=%u\n",
                      Msg->CameraID, Msg->ImageNumber);
            status = PAYUEL_OBC_RequestImageMeta(Msg->CameraID, Msg->ImageNumber, &Meta, &return_type, payload_error);
        }

        if (status == CFE_SUCCESS)
        {
            valid_data_len = PAYUEL_OBC_GetChunkDataLenFromImageMeta(&Meta, Msg->ChunkNumber);
            status = PAYUEL_OBC_RequestImageChunk(Msg->CameraID, Msg->ImageNumber, Msg->ChunkNumber,
                                                  valid_data_len, ChunkData, &return_type, payload_error);
        }

        PAYUEL_OBC_UnlockHardware("ChunkDownload");
    }

    if (status == CFE_SUCCESS)
    {
        OS_printf("PAYUEL_OBC: ChunkDownload CameraID=0x%02X, ImgNum=%u, Chunk=%u, Bytes=%u\n",
                  Msg->CameraID, Msg->ImageNumber, Msg->ChunkNumber, (unsigned int)valid_data_len);

        snprintf(filename, sizeof(filename),
                 "./cf/sdcard/uel_img_%02X_%u_chunk%u.bin",
                 Msg->CameraID, Msg->ImageNumber, Msg->ChunkNumber);

        fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd < 0)
        {
            status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            return_type = RPT_RETTYPE_CFE;
            CFE_EVS_SendEvent(PAYUEL_OBC_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Failed to open %s", filename);
            PAYUEL_OBC_Data.ErrCounter++;
        }
        else
        {
            written = write(fd, ChunkData, valid_data_len);
            close(fd);

            if (written != (ssize_t)valid_data_len)
            {
                status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                return_type = RPT_RETTYPE_CFE;
                CFE_EVS_SendEvent(PAYUEL_OBC_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "PAYUEL_OBC: File write incomplete (%ld/%u)",
                                  (long)written, (unsigned int)valid_data_len);
                PAYUEL_OBC_Data.ErrCounter++;
            }
            else
            {
                OS_printf("PAYUEL_OBC: Chunk saved -> %s\n", filename);
            }
        }
    }
    else if (payload_error[0] != 0U)
    {
        report_ptr = payload_error;
        report_len = sizeof(payload_error);
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_CHUNK_DOWNLOAD_CC,
                               status, report_ptr, report_len, return_type);
    return status;
}

/* ===================================================================
 * 0x54 - 센서 데이터 다운로드 메타 요청
 * TX (5 B): [0x54][DataSlot][DataNumber][CRC16_H][CRC16_L]
 * RX (18 B): [0x54][Status][DataSlot][BinValid][DataIndex]
 *            [BinSize(4B,BE)][BinFileCRC32(4B,BE)][LastChunkSz]
 *            [ChunkCount(2B,BE)][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_SensorMetaCmd(const PAYUEL_OBC_SensorMetaCmd_t *Msg)
{
    CFE_Status_t               status = CFE_SUCCESS;
    uint8_t                    return_type = RPT_RETTYPE_SUCCESS;
    PAYUEL_OBC_SensorMetaInfo_t Meta = {0};
    uint8_t                    report_data[9] = {Msg->DataSlot, Msg->DataNumber, 0};
    uint8_t                    payload_error[4] = {0};
    const uint8_t             *report_ptr = report_data;
    uint16                     report_len = sizeof(report_data);

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("SensorMeta") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        status = PAYUEL_OBC_RequestSensorMeta(Msg->DataSlot, Msg->DataNumber, &Meta, &return_type, payload_error);
        PAYUEL_OBC_UnlockHardware("SensorMeta");

        if (status == CFE_SUCCESS)
        {
            report_data[0] = Meta.DataSlot;
            report_data[1] = Meta.DataIndex;
            PAYUEL_OBC_WriteU16BE(&report_data[2], Meta.ChunkCount);
            report_data[4] = Meta.LastChunkSize;
            PAYUEL_OBC_WriteU32BE(&report_data[5], Meta.BinFileCRC32);

            OS_printf("PAYUEL_OBC: SensorMeta Slot=%u, DataNum=%u, BinValid=%u, BinSize=%u, "
                      "ChunkCount=%u, LastChunkSz=%u, BinFileCRC32=0x%08X\n",
                      Meta.DataSlot, Meta.DataIndex, Meta.BinValid, Meta.BinSize,
                      Meta.ChunkCount, Meta.LastChunkSize, Meta.BinFileCRC32);
        }
        else if (payload_error[0] != 0U)
        {
            report_ptr = payload_error;
            report_len = sizeof(payload_error);
        }
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_SENSOR_META_CC,
                               status, report_ptr, report_len, return_type);
    return status;
}

/* ===================================================================
 * 0x55 - 센서 데이터 청크 다운로드 요청
 * TX via CSP CAN (7 B):   [0x55][DataSlot][DataNumber][ChunkNum_H][ChunkNum_L]
 *                         [CRC16_H][CRC16_L]
 * RX via CSP CAN:         [0x55][Status][DataSlot][DataNumber][ChunkNum_H][ChunkNum_L]
 *                         [SensorData(valid bytes)][CRC16_H][CRC16_L]
 *                         length = 6 + valid data length + 2, max 255 B
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_SensorChunkCmd(const PAYUEL_OBC_SensorChunkCmd_t *Msg)
{
    CFE_Status_t               status = CFE_SUCCESS;
    uint8_t                    return_type = RPT_RETTYPE_SUCCESS;
    uint8_t                    ChunkData[PAYUEL_OBC_CHUNK_DATA_SIZE];
    PAYUEL_OBC_SensorMetaInfo_t Meta = {0};
    size_t                     valid_data_len = 0U;
    char                       filename[64];
    int                        fd;
    ssize_t                    written;
    uint8_t                    payload_error[4] = {0};
    const uint8_t             *report_ptr = NULL;
    uint16                     report_len = 0U;

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("SensorChunk") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        return_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        if (!PAYUEL_OBC_GetCachedSensorMeta(Msg->DataSlot, Msg->DataNumber, &Meta))
        {
            OS_printf("PAYUEL_OBC: SensorChunk cache miss, fetching meta for slot=%u data=%u\n",
                      Msg->DataSlot, Msg->DataNumber);
            status = PAYUEL_OBC_RequestSensorMeta(Msg->DataSlot, Msg->DataNumber, &Meta, &return_type, payload_error);
        }

        if (status == CFE_SUCCESS)
        {
            valid_data_len = PAYUEL_OBC_GetChunkDataLenFromSensorMeta(&Meta, Msg->ChunkNumber);
            status = PAYUEL_OBC_RequestSensorChunk(Msg->DataSlot, Msg->DataNumber, Msg->ChunkNumber,
                                                   valid_data_len, ChunkData, &return_type, payload_error);
        }

        PAYUEL_OBC_UnlockHardware("SensorChunk");
    }

    if (status == CFE_SUCCESS)
    {
        OS_printf("PAYUEL_OBC: SensorChunk Slot=%u, DataNum=%u, Chunk=%u, Bytes=%u\n",
                  Msg->DataSlot, Msg->DataNumber, Msg->ChunkNumber, (unsigned int)valid_data_len);

        snprintf(filename, sizeof(filename),
                 "./cf/sdcard/sens_%u_%u_chunk%u.bin",
                 Msg->DataSlot, Msg->DataNumber, Msg->ChunkNumber);

        fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd < 0)
        {
            status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            return_type = RPT_RETTYPE_CFE;
            CFE_EVS_SendEvent(PAYUEL_OBC_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYUEL_OBC: Failed to open %s", filename);
            PAYUEL_OBC_Data.ErrCounter++;
        }
        else
        {
            written = write(fd, ChunkData, valid_data_len);
            close(fd);

            if (written != (ssize_t)valid_data_len)
            {
                status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                return_type = RPT_RETTYPE_CFE;
                CFE_EVS_SendEvent(PAYUEL_OBC_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "PAYUEL_OBC: Sensor file write incomplete (%ld/%u)",
                                  (long)written, (unsigned int)valid_data_len);
                PAYUEL_OBC_Data.ErrCounter++;
            }
            else
            {
                OS_printf("PAYUEL_OBC: SensorChunk saved -> %s\n", filename);
            }
        }
    }
    else if (payload_error[0] != 0U)
    {
        report_ptr = payload_error;
        report_len = sizeof(payload_error);
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_SENSOR_CHUNK_CC,
                               status, report_ptr, report_len, return_type);
    return status;
}

CFE_Status_t PAYUEL_OBC_DownloadImageCmd(const PAYUEL_OBC_DownloadImageCmd_t *Msg)
{
    CFE_Status_t status;
    uint8_t      return_type = RPT_RETTYPE_SUCCESS;
    uint8_t      report_data[2] = {Msg->CameraID, Msg->ImageNumber};

    PAYUEL_OBC_Data.CmdCounter++;

    status = PAYUEL_OBC_QueueDownloadImage(Msg->CameraID, Msg->ImageNumber, &return_type);
    if (status != CFE_SUCCESS)
    {
        PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_DOWNLOAD_IMAGE_CC,
                                   status, report_data, sizeof(report_data), return_type);
    }

    return status;
}

CFE_Status_t PAYUEL_OBC_DownloadSensorCmd(const PAYUEL_OBC_DownloadSensorCmd_t *Msg)
{
    CFE_Status_t status;
    uint8_t      return_type = RPT_RETTYPE_SUCCESS;
    uint8_t      report_data[2] = {Msg->DataSlot, Msg->DataNumber};

    PAYUEL_OBC_Data.CmdCounter++;

    status = PAYUEL_OBC_QueueDownloadSensor(Msg->DataSlot, Msg->DataNumber, &return_type);
    if (status != CFE_SUCCESS)
    {
        PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_DOWNLOAD_SENSOR_CC,
                                   status, report_data, sizeof(report_data), return_type);
    }

    return status;
}
