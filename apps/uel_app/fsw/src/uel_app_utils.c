/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 *   This file contains the source code for the UEL utility functions
 */

/*
** Include Files:
*/
#include "osapi.h"
#include "uel_app.h"
#include "uel_app_eventids.h"
#include "uel_app_tbl.h"
#include "uel_app_utils.h"
#include "uel_app_mission_cfg.h"
#include "osapi.h"

#include <fcntl.h>        // open, O_CREAT/O_TRUNC/O_WRONLY
#include <unistd.h>       // write, close, ssize_t
#include <sys/types.h>    // ssize_t (일부 환경)
#include <stddef.h>       // size_t
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>


void UEL_APP_DownloadTask(void) {
    int32 Status;

    /* Configure Args */
    OS_MutSemTake(UEL_APP_Data.MutexId);
    uint8_t ImgSlot = UEL_APP_Data.ImgSlot;
    uint8_t ImgNumber = UEL_APP_Data.ImgNumber;
    uint16_t TotChunk = UEL_APP_Data.TotChunk;          /* From PI */
    uint16_t LastChunkSz = UEL_APP_Data.LastChunkSize;  /* From PI */
    uint16_t StartChunk = UEL_APP_Data.StartChunk;  /* From GS */
    uint16_t EndChunk = UEL_APP_Data.EndChunk;      /* From GS */
    OS_MutSemGive(UEL_APP_Data.MutexId);

    /* Validate Args */
    if (!TotChunk) return; // Image not ready
    if (StartChunk > TotChunk) return;
    if (EndChunk > TotChunk) return;
    if (StartChunk > EndChunk) return;

    /* Open file */
    osal_id_t FD = OS_OBJECT_ID_UNDEFINED;
    char Path[64] = {0,};
    snprintf(Path, sizeof(Path), "%s_%u_%u", UEL_APP_IMG_PATH, ImgSlot, ImgNumber);
    Status = OS_OpenCreate(&FD, Path, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE, OS_WRITE_ONLY);
    if (Status != OS_SUCCESS) return;

    /* Download */
    uint8_t TxData[5];
    uint8_t RxData[256];
    for (uint16_t i = 0; i <= EndChunk - StartChunk; i ++) {
        uint16_t Now = StartChunk + i;
        /* Configure Tx Packet */
        TxData[0] = UEL_APP_ID_GetCamImage;  
        TxData[1] = ImgSlot;
        TxData[2] = ImgNumber;
        TxData[3] = (uint8_t)(Now & 0xFF);        // Little Endian LSB
        TxData[4] = (uint8_t)((Now >> 8) & 0xFF); // Little Endian MSB

        Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_UEL_PI, UEL_APP_PORT,
                                            TxData, sizeof(TxData),
                                            RxData, sizeof(RxData));

        if (!Status) goto cleanup; /* Transaction fail.*/
        if (Status != sizeof(RxData)) goto cleanup; /* Transaction fail.*/

        /* If Transaction Success, Handle the data */
        uint8_t *ImgData = RxData + 5;

        /* If last chunk, */
        if (Now == TotChunk - 1) {
            Status = OS_write(FD, ImgData, LastChunkSz); /* Confirm that `LastChunkSz` include header?! */
            if (Status < 0 || Status != LastChunkSz) goto cleanup; // write fail
        }
        else {
            Status = OS_write(FD, ImgData, UEL_APP_IMG_DATA_SIZE);
            if (Status < 0 || Status != UEL_APP_IMG_DATA_SIZE) goto cleanup; // write fail
        }
        memset(RxData, 0, sizeof(RxData));

        OS_TaskDelay(50); /* Prevent Hogging */
    }

    CFE_EVS_SendEvent(UEL_APP_DOWNLOAD_DONE_INF_EID, CFE_EVS_EventType_INFORMATION,
                        "UEL Image Download Done.");
cleanup:
    /* Close file */
    OS_close(FD);
}
