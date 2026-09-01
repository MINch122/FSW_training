/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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

#include <string.h>

#include "gps_app.h"
#include "gps_report.h"
#include "gps_eventids.h"
#include "gps_msg.h"

/** A successful outcome is its own RPT bucket, whatever the caller asked for. */
static uint8 GPS_NormalizeReportType(int32 retCode, uint8 retType)
{
    return (retCode == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : retType;
}

void GPS_SendReport(const void* cmd, const void* data, uint16 dataSize, int32 retCode, uint8 retType)
{
    CFE_SB_MsgId_t    cmdMid;
    CFE_MSG_FcnCode_t cmdCode;
    uint16            copySize = 0;

    CFE_MSG_GetMsgId(cmd, &cmdMid);
    CFE_MSG_GetFcnCode(cmd, &cmdCode);

    if (data != NULL && dataSize > 0)
    {
        copySize = (dataSize > sizeof(GPS_AppData.Report.Payload.ReturnValue))
                       ? sizeof(GPS_AppData.Report.Payload.ReturnValue)
                       : dataSize;
    }

    CFE_MSG_Init(CFE_MSG_PTR(GPS_AppData.Report.TelemetryHeader), CFE_SB_ValueToMsgId(GPS_REPORT_TLM_MID),
                 sizeof(GPS_AppData.Report));
    GPS_AppData.Report.Payload.MsgID          = (uint16)CFE_SB_MsgIdToValue(cmdMid);
    GPS_AppData.Report.Payload.CommandCode    = (uint8)cmdCode;
    GPS_AppData.Report.Payload.ReturnType     = GPS_NormalizeReportType(retCode, retType);
    GPS_AppData.Report.Payload.ReturnCode     = retCode;
    GPS_AppData.Report.Payload.ReturnDataSize = copySize;
    memset(GPS_AppData.Report.Payload.ReturnValue, 0, sizeof(GPS_AppData.Report.Payload.ReturnValue));
    if (copySize > 0)
    {
        memcpy(GPS_AppData.Report.Payload.ReturnValue, data, copySize);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(GPS_AppData.Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(GPS_AppData.Report.TelemetryHeader), true);
}
