#include "payuel_cam.h"
#include "payuel_cam_child.h"
#include "payuel_cam_cmds.h"
#include "payuel_cam_utils.h"
#include "payuel_cam_msgids.h"
#include "payuel_cam_eventids.h"
#include "payuel_cam_version.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/* ===================================================================
 * NOOP
 * =================================================================== */
CFE_Status_t PAYUEL_CAM_NoopCmd(const PAYUEL_CAM_NoopCmd_t *Msg)
{
    (void)Msg;

    PAYUEL_CAM_Data.CmdCounter++;
    CFE_EVS_SendEvent(PAYUEL_CAM_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_CAM: NOOP command %s", PAYUEL_CAM_VERSION);
    return CFE_SUCCESS;
}

/* ===================================================================
 * RESET COUNTERS
 * =================================================================== */
CFE_Status_t PAYUEL_CAM_ResetCountersCmd(const PAYUEL_CAM_ResetCountersCmd_t *Msg)
{
    (void)Msg;

    PAYUEL_CAM_Data.CmdCounter = 0;
    PAYUEL_CAM_Data.ErrCounter = 0;
    CFE_EVS_SendEvent(PAYUEL_CAM_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYUEL_CAM: RESET command");
    return CFE_SUCCESS;
}

/* ===================================================================
 * 0x41 - Shot
 * TX (5 B): [0x41][ImageSlot][CameraNumber][CRC16_H][CRC16_L]
 * RX (4 B): [0x41][Status][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_CAM_ShotCmd(const PAYUEL_CAM_ShotCmd_t *Msg)
{
    uint8_t TxData[5];
    uint8_t RxData[4] = {0};
    int32   rsp_len;
    int32   status = CFE_SUCCESS;
    uint8_t rpt_type = RPT_RETTYPE_SUCCESS;

    PAYUEL_CAM_Data.CmdCounter++;

    if (PAYUEL_CAM_LockHardware("Shot") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_CAM_ID_SHOT;
        TxData[1] = Msg->ImageSlot;
        TxData[2] = Msg->CameraNumber;
        PAYUEL_CAM_AppendCrc16(TxData, 3);

        rsp_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_CAM_NODE, PAYUEL_CAM_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));

        PAYUEL_CAM_UnlockHardware("Shot");

        status = PAYUEL_CAM_ValidateResponse("Shot", PAYUEL_CAM_ID_SHOT, RxData, rsp_len, sizeof(RxData),
                                             &rpt_type, NULL);
        if (status == CFE_SUCCESS && RxData[1] != 0x00U)
        {
            rpt_type = RPT_RETTYPE_HW;
            status   = PAYUEL_CAM_HwStatusError("Shot", RxData[1]);
        }
        else if (status == CFE_SUCCESS)
        {
            PAYUEL_CAM_ClearImageMetaCache();
            OS_printf("PAYUEL_CAM: Shot OK, Status=0x%02X\n", RxData[1]);
        }
    }

    PAYUEL_CAM_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_CAM_CMD_MID), PAYUEL_CAM_SHOT_CC,
                               status, &RxData[1], 1, rpt_type);
    return status;
}

/* ===================================================================
 * 0x42 - Health Check
 * TX (3 B): [0x42][CRC16_H][CRC16_L]
 * RX (4 B): [0x42][Status][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_CAM_HealthCheckCmd(const PAYUEL_CAM_HealthCheckCmd_t *Msg)
{
    uint8_t TxData[3];
    uint8_t RxData[4] = {0};
    int32   rsp_len;
    int32   status = CFE_SUCCESS;
    uint8_t rpt_type = RPT_RETTYPE_SUCCESS;

    (void)Msg;

    PAYUEL_CAM_Data.CmdCounter++;

    if (PAYUEL_CAM_LockHardware("HealthCheck") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_CAM_ID_HEALTH_CHECK;
        PAYUEL_CAM_AppendCrc16(TxData, 1);

        rsp_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_CAM_NODE, PAYUEL_CAM_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));

        PAYUEL_CAM_UnlockHardware("HealthCheck");

        status = PAYUEL_CAM_ValidateResponse("HealthCheck", PAYUEL_CAM_ID_HEALTH_CHECK, RxData, rsp_len,
                                             sizeof(RxData), &rpt_type, NULL);
        if (status == CFE_SUCCESS && RxData[1] != 0x00U)
        {
            rpt_type = RPT_RETTYPE_HW;
            status   = PAYUEL_CAM_HwStatusError("HealthCheck", RxData[1]);
        }
        else if (status == CFE_SUCCESS)
        {
            OS_printf("PAYUEL_CAM: HealthCheck OK, Status=0x%02X (0x00=Normal)\n", RxData[1]);
        }
    }

    PAYUEL_CAM_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_CAM_CMD_MID), PAYUEL_CAM_HEALTH_CHECK_CC,
                               status, &RxData[1], 1, rpt_type);
    return status;
}

/* ===================================================================
 * 0x43 - Process Binning
 * TX (5 B): [0x43][ImageSlot][ImageNumber][CRC16_H][CRC16_L]
 * RX (4 B): [0x43][Status][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_CAM_ProcessBinningCmd(const PAYUEL_CAM_ProcessBinningCmd_t *Msg)
{
    uint8_t TxData[5];
    uint8_t RxData[4] = {0};
    int32   rsp_len;
    int32   status = CFE_SUCCESS;
    uint8_t rpt_type = RPT_RETTYPE_SUCCESS;

    PAYUEL_CAM_Data.CmdCounter++;

    if (PAYUEL_CAM_LockHardware("ProcessBinning") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_CAM_ID_PROCESS_BINNING;
        TxData[1] = Msg->ImageSlot;
        TxData[2] = Msg->ImageNumber;
        PAYUEL_CAM_AppendCrc16(TxData, 3);

        rsp_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_CAM_NODE, PAYUEL_CAM_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));

        PAYUEL_CAM_UnlockHardware("ProcessBinning");

        status = PAYUEL_CAM_ValidateResponse("ProcessBinning", PAYUEL_CAM_ID_PROCESS_BINNING, RxData, rsp_len,
                                             sizeof(RxData), &rpt_type, NULL);
        if (status == CFE_SUCCESS && RxData[1] != 0x00U)
        {
            rpt_type = RPT_RETTYPE_HW;
            status   = PAYUEL_CAM_HwStatusError("ProcessBinning", RxData[1]);
        }
        else if (status == CFE_SUCCESS)
        {
            PAYUEL_CAM_ClearImageMetaCache();
            OS_printf("PAYUEL_CAM: ProcessBinning OK, Slot=%u, Status=0x%02X\n",
                      Msg->ImageSlot, RxData[1]);
        }
    }

    PAYUEL_CAM_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_CAM_CMD_MID), PAYUEL_CAM_PROCESS_BINNING_CC,
                               status, &RxData[1], 1, rpt_type);
    return status;
}

/* ===================================================================
 * 0x44 - Download Meta
 * TX  (5 B): [0x44][ImageSlot][ImageNumber][CRC16_H][CRC16_L]
 * RX (12 B): [0x44][Slot][ImgNum][TotalChunk(2B,BE)][LastChunkSz]
 *            [FileCRC32(4B,BE)][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_CAM_DownloadMetaCmd(const PAYUEL_CAM_DownloadMetaCmd_t *Msg)
{
    CFE_Status_t              status = CFE_SUCCESS;
    uint8_t                   rpt_type = RPT_RETTYPE_SUCCESS;
    PAYUEL_CAM_ImageMetaInfo_t Meta = {0};
    uint8_t                   report_data[9] = {Msg->ImageSlot, Msg->ImageNumber, 0};
    uint8_t                   payload_error[4] = {0};
    const uint8_t            *report_ptr = report_data;
    uint16                    report_len = sizeof(report_data);

    PAYUEL_CAM_Data.CmdCounter++;

    if (PAYUEL_CAM_LockHardware("DownloadMeta") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        status = PAYUEL_CAM_RequestImageMeta(Msg->ImageSlot, Msg->ImageNumber, &Meta, &rpt_type, payload_error);
        PAYUEL_CAM_UnlockHardware("DownloadMeta");

        if (status == CFE_SUCCESS)
        {
            report_data[0] = Meta.ImageSlot;
            report_data[1] = Meta.ImageNumber;
            PAYUEL_CAM_WriteU16BE(&report_data[2], Meta.TotalChunks);
            report_data[4] = Meta.LastChunkSize;
            PAYUEL_CAM_WriteU32BE(&report_data[5], Meta.FileCrc32);

            OS_printf("PAYUEL_CAM: DownloadMeta Slot=%u, ImgNum=%u, TotalChunk=%u, "
                      "LastChunkSz=%u, FileCRC32=0x%08X\n",
                      Meta.ImageSlot, Meta.ImageNumber, Meta.TotalChunks, Meta.LastChunkSize, Meta.FileCrc32);
        }
        else if (payload_error[0] != 0U)
        {
            report_ptr = payload_error;
            report_len = sizeof(payload_error);
        }
    }

    PAYUEL_CAM_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_CAM_CMD_MID), PAYUEL_CAM_DOWNLOAD_META_CC,
                               status, report_ptr, report_len, rpt_type);
    return status;
}

/* ===================================================================
 * 0x45 - Chunk Download
 * TX via CSP CAN (7 B):   [0x45][ImageSlot][ImageNumber][ChunkNum_H][ChunkNum_L]
 *                         [CRC16_H][CRC16_L]
 * RX via CSP CAN (256 B): [0x45][Slot][ImgNum][ChunkNum_H][ChunkNum_L]
 *                         [ImageData/Padding(247B)][CRC32(4B,BE)]
 * CRC32 range:        Cmd..valid data only (padding excluded)
 * =================================================================== */
CFE_Status_t PAYUEL_CAM_ChunkDownloadCmd(const PAYUEL_CAM_ChunkDownloadCmd_t *Msg)
{
    CFE_Status_t     status = CFE_SUCCESS;
    uint8_t          rpt_type = RPT_RETTYPE_SUCCESS;
    uint8_t          ChunkData[PAYUEL_CAM_CHUNK_DATA_SIZE];
    PAYUEL_CAM_ImageMetaInfo_t Meta = {0};
    size_t           valid_data_len;
    char             filename[64];
    int              fd;
    ssize_t          written;
    uint8_t          payload_error[4] = {0};
    const uint8_t   *report_ptr = NULL;
    uint16           report_len = 0U;

    PAYUEL_CAM_Data.CmdCounter++;

    if (PAYUEL_CAM_LockHardware("ChunkDownload") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        if (!PAYUEL_CAM_GetCachedImageMeta(Msg->ImageSlot, Msg->ImageNumber, &Meta))
        {
            status = PAYUEL_CAM_RequestImageMeta(Msg->ImageSlot, Msg->ImageNumber, &Meta, &rpt_type, payload_error);
        }

        if (status == CFE_SUCCESS)
        {
            valid_data_len = PAYUEL_CAM_GetChunkDataLenFromMeta(&Meta, Msg->ChunkNumber);
            status = PAYUEL_CAM_RequestImageChunk(Msg->ImageSlot, Msg->ImageNumber, Msg->ChunkNumber,
                                                  valid_data_len, ChunkData, &rpt_type, payload_error);
        }

        PAYUEL_CAM_UnlockHardware("ChunkDownload");

        if (status == CFE_SUCCESS)
        {
            OS_printf("PAYUEL_CAM: ChunkDownload Slot=%u, ImgNum=%u, Chunk=%u, Bytes=%u\n",
                      Msg->ImageSlot, Msg->ImageNumber, Msg->ChunkNumber, (unsigned int)valid_data_len);

            snprintf(filename, sizeof(filename),
                     "./cf/sdcard/cam_img_%u_%u_chunk%u.bin",
                     Msg->ImageSlot, Msg->ImageNumber, Msg->ChunkNumber);

            fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd < 0)
            {
                status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                rpt_type = RPT_RETTYPE_CFE;
                CFE_EVS_SendEvent(PAYUEL_CAM_FILE_OPEN_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "PAYUEL_CAM: Failed to open %s", filename);
                PAYUEL_CAM_Data.ErrCounter++;
            }
            else
            {
                written = write(fd, ChunkData, valid_data_len);
                close(fd);

                if (written != (ssize_t)valid_data_len)
                {
                    status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                    rpt_type = RPT_RETTYPE_CFE;
                    CFE_EVS_SendEvent(PAYUEL_CAM_FILE_WRITE_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "PAYUEL_CAM: File write incomplete (%ld/%u)",
                                      (long)written, (unsigned int)valid_data_len);
                    PAYUEL_CAM_Data.ErrCounter++;
                }
                else
                {
                    OS_printf("PAYUEL_CAM: Chunk saved -> %s\n", filename);
                }
            }
        }
        else if (payload_error[0] != 0U)
        {
            report_ptr = payload_error;
            report_len = sizeof(payload_error);
        }
    }

    PAYUEL_CAM_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_CAM_CMD_MID), PAYUEL_CAM_CHUNK_DOWNLOAD_CC,
                               status, report_ptr, report_len, rpt_type);
    return status;
}

/* ===================================================================
 * 0x46 - CAM I/F Beacon
 * TX  (3 B):  [0x46][CRC16_H][CRC16_L]
 * RX (15 B):  [0x46][PI_Boot_State(1)][PI_Boot_Count(2)]
 *             [CAM_Detect_State(1)][CAM_Shutter_Count(2)]
 *             [CPU_Temp_x100(2)][Free_Disk_MB(4)][CRC16_H][CRC16_L]
 * =================================================================== */
CFE_Status_t PAYUEL_CAM_SendBcnCmd(const PAYUEL_CAM_SendBcnCmd_t *Msg)
{
    uint8_t  TxData[3];
    uint8_t  RxData[15] = {0};
    int32    rsp_len;
    int32    status = CFE_SUCCESS;
    uint8_t *p;
    uint8_t  rpt_type = RPT_RETTYPE_SUCCESS;

    (void)Msg;
    PAYUEL_CAM_Data.CmdCounter++;

    if (PAYUEL_CAM_LockHardware("Beacon") != CFE_SUCCESS)
    {
        status   = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        rpt_type = RPT_RETTYPE_OSAL;
    }
    else
    {
        TxData[0] = PAYUEL_CAM_ID_BEACON;
        PAYUEL_CAM_AppendCrc16(TxData, 1);

        rsp_len = CFE_SRL_ApiTransactionCSP(
            PAYUEL_CAM_NODE, PAYUEL_CAM_PORT,
            TxData, sizeof(TxData),
            RxData, sizeof(RxData));

        PAYUEL_CAM_UnlockHardware("Beacon");

        status = PAYUEL_CAM_ValidateResponse("Beacon", PAYUEL_CAM_ID_BEACON, RxData, rsp_len,
                                             sizeof(RxData), &rpt_type, NULL);
        if (status == CFE_SUCCESS)
        {
            p = &RxData[1];

            PAYUEL_CAM_Data.bcn.Payload.PI_Boot_State     = p[0];
            PAYUEL_CAM_Data.bcn.Payload.PI_Boot_Count     = PAYUEL_CAM_ReadU16BE(&p[1]);
            PAYUEL_CAM_Data.bcn.Payload.CAM_Detect_State  = p[3];
            PAYUEL_CAM_Data.bcn.Payload.CAM_Shutter_Count = PAYUEL_CAM_ReadU16BE(&p[4]);
            PAYUEL_CAM_Data.bcn.Payload.CPU_Temp_x100     = PAYUEL_CAM_ReadI16BE(&p[6]);
            PAYUEL_CAM_Data.bcn.Payload.Free_Disk_MB      = PAYUEL_CAM_ReadU32BE(&p[8]);

            CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUEL_CAM_Data.bcn.TelemetryHeader));
            CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUEL_CAM_Data.bcn.TelemetryHeader), true);

            OS_printf("PAYUEL_CAM BCN: PI_State=%d, PI_Count=%d, CAM_State=0x%02X, "
                      "Shutter=%d, CPU_Temp=%.2f C, Free=%u MB\n",
                      PAYUEL_CAM_Data.bcn.Payload.PI_Boot_State,
                      PAYUEL_CAM_Data.bcn.Payload.PI_Boot_Count,
                      PAYUEL_CAM_Data.bcn.Payload.CAM_Detect_State,
                      PAYUEL_CAM_Data.bcn.Payload.CAM_Shutter_Count,
                      PAYUEL_CAM_Data.bcn.Payload.CPU_Temp_x100 / 100.0f,
                      PAYUEL_CAM_Data.bcn.Payload.Free_Disk_MB);
        }
    }

    return status;
}

CFE_Status_t PAYUEL_CAM_DownloadImageCmd(const PAYUEL_CAM_DownloadImageCmd_t *Msg)
{
    CFE_Status_t status;
    uint8_t      rpt_type = RPT_RETTYPE_SUCCESS;
    uint8_t      report_data[2] = {Msg->ImageSlot, Msg->ImageNumber};

    PAYUEL_CAM_Data.CmdCounter++;

    status = PAYUEL_CAM_QueueDownloadImage(Msg->ImageSlot, Msg->ImageNumber, &rpt_type);
    if (status != CFE_SUCCESS)
    {
        PAYUEL_CAM_ReportCmdStatus(CFE_SB_ValueToMsgId(PAYUEL_CAM_CMD_MID), PAYUEL_CAM_DOWNLOAD_IMAGE_CC,
                                   status, report_data, sizeof(report_data), rpt_type);
    }

    return status;
}
