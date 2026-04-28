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
 * RX (17 B):  [0x20][init_error(1)][boot_count(2)][boot_time(4)]
 *             [shutter_count(2)][alpha_temp(1)][beta_temp(1)]
 *             [imu_temp(1)][tc1(1)][sd_mount_state(1)]
 *             [CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_SendObcBcnCmd(const PAYUEL_OBC_SendObcBcnCmd_t *Msg)
{
    uint8_t  TxData[3];
    uint8_t  RxData[17] = {0};
    int32    rsp_len;
    int32    status = CFE_SUCCESS;
    uint8_t *p;
    uint8_t  rpt_type = RPT_RETTYPE_SUCCESS;

    (void)Msg;
    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("ObcBeacon") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_OBC_ID_OBC_BEACON;
        PAYUEL_OBC_AppendCrc16(TxData, 1);
        OS_printf("TxData[1]=0x%02X, TxData[2]=0x%02X", TxData[1], TxData[2]);

        rsp_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));
            
        PAYUEL_OBC_UnlockHardware("ObcBeacon");

        status = PAYUEL_OBC_ValidateResponse("ObcBeacon", PAYUEL_OBC_ID_OBC_BEACON, RxData, rsp_len,
                                             sizeof(RxData), &rpt_type, NULL);
        if (status == CFE_SUCCESS)
        {
            p = &RxData[1];

            PAYUEL_OBC_Data.obc_bcn.Payload.InitError    = p[0];
            PAYUEL_OBC_Data.obc_bcn.Payload.BootCount    = PAYUEL_OBC_ReadU16BE(&p[1]);
            PAYUEL_OBC_Data.obc_bcn.Payload.BootTime     = PAYUEL_OBC_ReadU32BE(&p[3]);
            PAYUEL_OBC_Data.obc_bcn.Payload.ShutterCount = PAYUEL_OBC_ReadU16BE(&p[7]);
            PAYUEL_OBC_Data.obc_bcn.Payload.AlphaTemp    = (int8_t)p[9];
            PAYUEL_OBC_Data.obc_bcn.Payload.BetaTemp     = (int8_t)p[10];
            PAYUEL_OBC_Data.obc_bcn.Payload.ImuTemp      = (int8_t)p[11];
            PAYUEL_OBC_Data.obc_bcn.Payload.TC1          = (int8_t)p[12];
            PAYUEL_OBC_Data.obc_bcn.Payload.SdMountState = p[13];

            CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUEL_OBC_Data.obc_bcn.TelemetryHeader));
            CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUEL_OBC_Data.obc_bcn.TelemetryHeader), true);

            OS_printf("PAYUEL_OBC OBC BCN: InitErr=%u, BootCnt=%u, BootTime=%u, Shutter=%u, "
                      "Alpha=%d, Beta=%d, IMU=%d, TC1=%d, SD=%u\n",
                      PAYUEL_OBC_Data.obc_bcn.Payload.InitError,
                      PAYUEL_OBC_Data.obc_bcn.Payload.BootCount,
                      PAYUEL_OBC_Data.obc_bcn.Payload.BootTime,
                      PAYUEL_OBC_Data.obc_bcn.Payload.ShutterCount,
                      PAYUEL_OBC_Data.obc_bcn.Payload.AlphaTemp,
                      PAYUEL_OBC_Data.obc_bcn.Payload.BetaTemp,
                      PAYUEL_OBC_Data.obc_bcn.Payload.ImuTemp,
                      PAYUEL_OBC_Data.obc_bcn.Payload.TC1,
                      PAYUEL_OBC_Data.obc_bcn.Payload.SdMountState);
        }
    }

    return status;
}

/* ===================================================================
 * 0x21 - 모터 모드 제어
 * TX  (7 B): [0x21][L_Speed(int8)][R_Speed(int8)][Duration][Instance]
 *            [CRC16_H][CRC16_L]
 * RX (15 B): [0x21][la_mA(2)][lb_mA(2)][lc_mA(2)]
 *            [lu_mA(2)][lv_mA(2)][lw_mA(2)][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_MotorModeCmd(const PAYUEL_OBC_MotorModeCmd_t *Msg)
{
    uint8_t  TxData[7];
    uint8_t  RxData[15] = {0};
    int32    rsp_len;
    int32    status = CFE_SUCCESS;
    uint8_t  rpt_type = RPT_RETTYPE_SUCCESS;
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
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_OBC_ID_MOTOR_MODE;
        TxData[1] = (uint8_t)Msg->L_Speed;
        TxData[2] = (uint8_t)Msg->R_Speed;
        TxData[3] = Msg->Duration;
        TxData[4] = Msg->Instance;
        PAYUEL_OBC_AppendCrc16(TxData, 5);

        rsp_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));

        PAYUEL_OBC_UnlockHardware("MotorMode");

        status = PAYUEL_OBC_ValidateResponse("MotorMode", PAYUEL_OBC_ID_MOTOR_MODE, RxData, rsp_len,
                                             sizeof(RxData), &rpt_type, payload_error);
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
                               status, report_ptr, report_len, rpt_type);
    return status;
}

/* ===================================================================
 * 0x41 - 사진 촬영 요청 (UEL OBC)
 * TX (5 B): [0x41][ImageSlot][CameraNumber][CRC16_H][CRC16_L]
 * RX (4 B): [0x41][Status][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_CamShotCmd(const PAYUEL_OBC_CamShotCmd_t *Msg)
{
    uint8_t TxData[5];
    uint8_t RxData[4] = {0};
    int32   rsp_len;
    int32   status = CFE_SUCCESS;
    uint8_t rpt_type = RPT_RETTYPE_SUCCESS;

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("CamShot") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_OBC_ID_CAM_SHOT;
        TxData[1] = Msg->ImageSlot;
        TxData[2] = Msg->CameraNumber;
        PAYUEL_OBC_AppendCrc16(TxData, 3);

        rsp_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_OBC_NODE, PAYUEL_OBC_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));

        PAYUEL_OBC_UnlockHardware("CamShot");

        status = PAYUEL_OBC_ValidateResponse("CamShot", PAYUEL_OBC_ID_CAM_SHOT, RxData, rsp_len,
                                             sizeof(RxData), &rpt_type, NULL);
        if (status == CFE_SUCCESS && RxData[1] != 0x00U)
        {
            rpt_type = RPT_RETTYPE_HW;
            status   = PAYUEL_OBC_HwStatusError("CamShot", RxData[1]);
        }
        else if (status == CFE_SUCCESS)
        {
            PAYUEL_OBC_ClearImageMetaCache();
            OS_printf("PAYUEL_OBC: CamShot OK, Status=0x%02X (image meta cache cleared)\n", RxData[1]);
        }
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_CAM_SHOT_CC,
                               status, &RxData[1], 1, rpt_type);
    return status;
}

/* ===================================================================
 * 0x44 - 이미지 다운로드 메타 요청 (UEL OBC)
 * TX  (5 B): [0x44][ImageSlot][ImageNumber][CRC16_H][CRC16_L]
 * RX (12 B): [0x44][Slot][ImgNum][TotalChunk(2B,BE)][LastChunkSz]
 *            [FileCRC32(4B,BE)][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_DownloadMetaCmd(const PAYUEL_OBC_DownloadMetaCmd_t *Msg)
{
    CFE_Status_t              status = CFE_SUCCESS;
    uint8_t                   rpt_type = RPT_RETTYPE_SUCCESS;
    PAYUEL_OBC_ImageMetaInfo_t Meta = {0};
    uint8_t                   report_data[9] = {Msg->ImageSlot, Msg->ImageNumber, 0};
    uint8_t                   payload_error[4] = {0};
    const uint8_t            *report_ptr = report_data;
    uint16                    report_len = sizeof(report_data);

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("DownloadMeta") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        status = PAYUEL_OBC_RequestImageMeta(Msg->ImageSlot, Msg->ImageNumber, &Meta, &rpt_type, payload_error);
        PAYUEL_OBC_UnlockHardware("DownloadMeta");

        if (status == CFE_SUCCESS)
        {
            report_data[0] = Meta.ImageSlot;
            report_data[1] = Meta.ImageNumber;
            PAYUEL_OBC_WriteU16BE(&report_data[2], Meta.TotalChunks);
            report_data[4] = Meta.LastChunkSize;
            PAYUEL_OBC_WriteU32BE(&report_data[5], Meta.FileCrc32);

            OS_printf("PAYUEL_OBC: DownloadMeta Slot=%u, ImgNum=%u, TotalChunk=%u, "
                      "LastChunkSz=%u, FileCRC32=0x%08X\n",
                      Meta.ImageSlot, Meta.ImageNumber, Meta.TotalChunks, Meta.LastChunkSize, Meta.FileCrc32);
        }
        else if (payload_error[0] != 0U)
        {
            report_ptr = payload_error;
            report_len = sizeof(payload_error);
        }
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_DOWNLOAD_META_CC,
                               status, report_ptr, report_len, rpt_type);
    return status;
}

/* ===================================================================
 * 0x45 - 이미지 청크 다운로드 요청 (UEL OBC)
 * TX via CSP CAN (7 B):   [0x45][ImageSlot][ImageNumber][ChunkNum_H][ChunkNum_L]
 *                         [CRC16_H][CRC16_L]
 * RX via CSP CAN:         [0x45][Slot][ImgNum][ChunkNum_H][ChunkNum_L]
 *                         [ImageData(valid bytes)][CRC32(4B,BE)]
 *                         length = 5 + valid data length + 4, max 256 B
 * CRC32 range:            Cmd..valid data only
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_ChunkDownloadCmd(const PAYUEL_OBC_ChunkDownloadCmd_t *Msg)
{
    CFE_Status_t              status = CFE_SUCCESS;
    uint8_t                   rpt_type = RPT_RETTYPE_SUCCESS;
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
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        if (!PAYUEL_OBC_GetCachedImageMeta(Msg->ImageSlot, Msg->ImageNumber, &Meta))
        {
            OS_printf("PAYUEL_OBC: ChunkDownload cache miss, fetching meta for slot=%u image=%u\n",
                      Msg->ImageSlot, Msg->ImageNumber);
            status = PAYUEL_OBC_RequestImageMeta(Msg->ImageSlot, Msg->ImageNumber, &Meta, &rpt_type, payload_error);
        }

        if (status == CFE_SUCCESS)
        {
            valid_data_len = PAYUEL_OBC_GetChunkDataLenFromImageMeta(&Meta, Msg->ChunkNumber);
            status = PAYUEL_OBC_RequestImageChunk(Msg->ImageSlot, Msg->ImageNumber, Msg->ChunkNumber,
                                                  valid_data_len, ChunkData, &rpt_type, payload_error);
        }

        PAYUEL_OBC_UnlockHardware("ChunkDownload");
    }

    if (status == CFE_SUCCESS)
    {
        OS_printf("PAYUEL_OBC: ChunkDownload Slot=%u, ImgNum=%u, Chunk=%u, Bytes=%u\n",
                  Msg->ImageSlot, Msg->ImageNumber, Msg->ChunkNumber, (unsigned int)valid_data_len);

        snprintf(filename, sizeof(filename),
                 "./cf/sdcard/uel_img_%u_%u_chunk%u.bin",
                 Msg->ImageSlot, Msg->ImageNumber, Msg->ChunkNumber);

        fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd < 0)
        {
            status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
            rpt_type = RPT_RETTYPE_CFE;
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
                rpt_type = RPT_RETTYPE_CFE;
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
                               status, report_ptr, report_len, rpt_type);
    return status;
}

/* ===================================================================
 * 0x54 - 센서 데이터 다운로드 메타 요청
 * TX (5 B): [0x54][DataSlot][DataNumber][CRC16_H][CRC16_L]
 * RX (8 B): [0x54][DataSlot][DataNumber][TotalChunk(2B,BE)]
 *           [LastChunkSize(1)][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_SensorMetaCmd(const PAYUEL_OBC_SensorMetaCmd_t *Msg)
{
    CFE_Status_t               status = CFE_SUCCESS;
    uint8_t                    rpt_type = RPT_RETTYPE_SUCCESS;
    PAYUEL_OBC_SensorMetaInfo_t Meta = {0};
    uint8_t                    report_data[5] = {Msg->DataSlot, Msg->DataNumber, 0};
    uint8_t                    payload_error[4] = {0};
    const uint8_t             *report_ptr = report_data;
    uint16                     report_len = sizeof(report_data);

    PAYUEL_OBC_Data.CmdCounter++;

    if (PAYUEL_OBC_LockHardware("SensorMeta") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        status = PAYUEL_OBC_RequestSensorMeta(Msg->DataSlot, Msg->DataNumber, &Meta, &rpt_type, payload_error);
        PAYUEL_OBC_UnlockHardware("SensorMeta");

        if (status == CFE_SUCCESS)
        {
            report_data[0] = Meta.DataSlot;
            report_data[1] = Meta.DataNumber;
            PAYUEL_OBC_WriteU16BE(&report_data[2], Meta.TotalChunks);
            report_data[4] = Meta.LastChunkSize;

            OS_printf("PAYUEL_OBC: SensorMeta Slot=%u, DataNum=%u, TotalChunk=%u, LastChunkSz=%u\n",
                      Meta.DataSlot, Meta.DataNumber, Meta.TotalChunks, Meta.LastChunkSize);
        }
        else if (payload_error[0] != 0U)
        {
            report_ptr = payload_error;
            report_len = sizeof(payload_error);
        }
    }

    PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_SENSOR_META_CC,
                               status, report_ptr, report_len, rpt_type);
    return status;
}

/* ===================================================================
 * 0x55 - 센서 데이터 청크 다운로드 요청
 * TX via CSP CAN (7 B):   [0x55][DataSlot][DataNumber][ChunkNum_H][ChunkNum_L]
 *                         [CRC16_H][CRC16_L]
 * RX via CSP CAN:         [0x55][DataSlot][DataNumber][ChunkNum_H][ChunkNum_L]
 *                         [SensorData(valid bytes)][CRC32(4B,BE)]
 *                         length = 5 + valid data length + 4, max 256 B
 * CRC32 range:            Cmd..valid data only
 * =================================================================== */
CFE_Status_t PAYUEL_OBC_SensorChunkCmd(const PAYUEL_OBC_SensorChunkCmd_t *Msg)
{
    CFE_Status_t               status = CFE_SUCCESS;
    uint8_t                    rpt_type = RPT_RETTYPE_SUCCESS;
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
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        if (!PAYUEL_OBC_GetCachedSensorMeta(Msg->DataSlot, Msg->DataNumber, &Meta))
        {
            OS_printf("PAYUEL_OBC: SensorChunk cache miss, fetching meta for slot=%u data=%u\n",
                      Msg->DataSlot, Msg->DataNumber);
            status = PAYUEL_OBC_RequestSensorMeta(Msg->DataSlot, Msg->DataNumber, &Meta, &rpt_type, payload_error);
        }

        if (status == CFE_SUCCESS)
        {
            valid_data_len = PAYUEL_OBC_GetChunkDataLenFromSensorMeta(&Meta, Msg->ChunkNumber);
            status = PAYUEL_OBC_RequestSensorChunk(Msg->DataSlot, Msg->DataNumber, Msg->ChunkNumber,
                                                   valid_data_len, ChunkData, &rpt_type, payload_error);
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
            rpt_type = RPT_RETTYPE_CFE;
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
                rpt_type = RPT_RETTYPE_CFE;
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
                               status, report_ptr, report_len, rpt_type);
    return status;
}

CFE_Status_t PAYUEL_OBC_DownloadImageCmd(const PAYUEL_OBC_DownloadImageCmd_t *Msg)
{
    CFE_Status_t status;
    uint8_t      rpt_type = RPT_RETTYPE_SUCCESS;
    uint8_t      report_data[2] = {Msg->ImageSlot, Msg->ImageNumber};

    PAYUEL_OBC_Data.CmdCounter++;

    status = PAYUEL_OBC_QueueDownloadImage(Msg->ImageSlot, Msg->ImageNumber, &rpt_type);
    if (status != CFE_SUCCESS)
    {
        PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_DOWNLOAD_IMAGE_CC,
                                   status, report_data, sizeof(report_data), rpt_type);
    }

    return status;
}

CFE_Status_t PAYUEL_OBC_DownloadSensorCmd(const PAYUEL_OBC_DownloadSensorCmd_t *Msg)
{
    CFE_Status_t status;
    uint8_t      rpt_type = RPT_RETTYPE_SUCCESS;
    uint8_t      report_data[2] = {Msg->DataSlot, Msg->DataNumber};

    PAYUEL_OBC_Data.CmdCounter++;

    status = PAYUEL_OBC_QueueDownloadSensor(Msg->DataSlot, Msg->DataNumber, &rpt_type);
    if (status != CFE_SUCCESS)
    {
        PAYUEL_OBC_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_OBC_CMD_MID), PAYUEL_OBC_DOWNLOAD_SENSOR_CC,
                                   status, report_data, sizeof(report_data), rpt_type);
    }

    return status;
}
