/**
 * @file
 *   Report (RPT) Init function
 */
#include "rpt_task.h"
#include "common_types.h"
#include "rpt_mission_cfg.h"
#include "rpt_msgids.h"
#include "rpt_utils.h"
#include "rpt_mission_cfg.h"
#include "rpt_eventids.h"
#include "cfe_msgids.h"


void RPT_FIFO_Init(void) {

    RPT_Data.RptQueue.Head = 0;
    RPT_Data.RptQueue.Count = 0;

    RPT_Data.CritQueue.Head = 0;
    RPT_Data.CritQueue.Count = 0;
}

int32 RPT_PriorInit(void) {

    RPT_FIFO_Init();

    return CFE_SUCCESS;
}

CFE_Status_t RPT_TableInit(void) {
    CFE_Status_t Status;
    void *TempTblPtr;

    Status = CFE_TBL_Register(&RPT_Data.SubsTblHandle, "RPT_Subs", (sizeof(RPT_Table_t) * RPT_MAX_TBL_ENTRY),
                        CFE_TBL_OPT_DEFAULT, NULL);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(RPT_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d RPT Can't register table. RC = %d",
                            __LINE__, Status);
    }
    
    if (Status == CFE_SUCCESS) {
        Status = CFE_TBL_Load(RPT_Data.SubsTblHandle, CFE_TBL_SRC_FILE, RPT_TABLE_FILE);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d RPT Can't load table. RC = %d",
                                __LINE__, Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        Status = CFE_TBL_GetAddress((void **)&TempTblPtr, RPT_Data.SubsTblHandle);
        if(Status != CFE_SUCCESS && Status != CFE_TBL_INFO_UPDATED) {
            CFE_EVS_SendEvent(RPT_TBL_ERR_EID, CFE_EVS_EventType_ERROR, "L%d RPT Can't get table addr. RC = %d",
                                __LINE__, Status);
        }
    }
    if (Status == CFE_SUCCESS || Status == CFE_TBL_INFO_UPDATED) {
        RPT_Data.SubsTblPtr = TempTblPtr; /* Save returned address */

        /**
         * Table init success, then create pipe
         */
        Status = CFE_SB_CreatePipe(&RPT_Data.RptPipe, RPT_Data.PipeDepth, RPT_Data.RptPipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RPT: Error creating SB Report Pipe, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    if (Status == CFE_SUCCESS) {
        Status = CFE_SB_CreatePipe(&RPT_Data.CritPipe, RPT_Data.PipeDepth, RPT_Data.CritPipeName);
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendEvent(RPT_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "RPT: Error creating SB Critical Pipe, RC = 0x%08lX", (unsigned long)Status);
        }
    }

    return Status;
}

CFE_Status_t RPT_OpsDataInit(void) {
    CFE_Status_t Status = CFE_SUCCESS;
    
    RPT_Data.OpsDataHandle = RPT_OpenOpsFile();
    if (RPT_Data.OpsDataHandle == OS_OBJECT_ID_UNDEFINED) {
        CFE_EVS_SendErr(RPT_DATA_OPEN_ERR_EID, "Open Operation data file failed.");
        Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }
    
    if(Status == CFE_SUCCESS) {
        Status = RPT_ReadFromFile(RPT_Data.OpsDataHandle, &RPT_Data.OpsData, sizeof(RPT_OperationData_t));
        if (Status < CFE_SUCCESS) {
            CFE_EVS_SendErr(RPT_DATA_READ_ERR_EID, "Read Operation data file failed.");
        }
        else if (RPT_Data.OpsData.BootCount == 0 || Status == 0) {
            /* If First Boot, Store time epoch */
            CFE_TIME_SysTime_t Epoch = CFE_TIME_GetTime();
            RPT_Data.OpsData.EpochSec = Epoch.Seconds;
            RPT_Data.OpsData.EpochSubsec = Epoch.Subseconds;
            Status = CFE_SUCCESS;

            RPT_APP_printf("Epoch - Sec: %u || Subsec: %u\n", RPT_Data.OpsData.EpochSec,
                           RPT_Data.OpsData.EpochSubsec);
        }
        else if (RPT_Data.OpsData.BootCount != 0 || Status == sizeof(RPT_OperationData_t)) Status = CFE_SUCCESS;
        else Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }
    
    if (Status == CFE_SUCCESS && RPT_Data.OpsData.BootCount) { // If First Boot, skip this
        /**
         * CRC Check
         */
        uint32 CRC = RPT_CalculateCRC(&RPT_Data.OpsData, (sizeof(RPT_OperationData_t) - sizeof(CFE_MSG_Checksum_t)));
        if (RPT_Data.OpsData.CRC != CRC) {
            /* If CRC is not matched, clear all data */
            CFE_EVS_SendErr(RPT_DATA_CRC_INVALID_ERR_EID, "Operation data CRC not matched.");
            memset(&RPT_Data.OpsData, 0, sizeof(RPT_OperationData_t));
        }
        else {
            /**
             * If CRC well matched, and if contained the time data, 
             * Set Spacecraft time
             */
            if (RPT_Data.OpsData.TimeSec != 0 || RPT_Data.OpsData.TimeSubsec != 0) {
                RPT_SetTimeCmd_t Cmd;
                CFE_MSG_Init(CFE_MSG_PTR(Cmd.CommandHeader), CFE_SB_ValueToMsgId(CFE_TIME_CMD_MID), sizeof(RPT_SetTimeCmd_t));
                CFE_MSG_SetFcnCode(CFE_MSG_PTR(Cmd.CommandHeader), 7);
                Cmd.Payload.Seconds = RPT_Data.OpsData.TimeSec;
                Cmd.Payload.MicroSeconds = CFE_TIME_Sub2MicroSecs(RPT_Data.OpsData.TimeSubsec);
                CFE_SB_TransmitMsg(CFE_MSG_PTR(Cmd.CommandHeader), true);
            }
            
            // Debugging
            CFE_EVS_SendInfo(RPT_DATA_CRC_VALID_INF_EID, "Operation Data CRC matched.");
        }
    }

    if (Status == CFE_SUCCESS) {
        /**
         * If successfully read ops data, then increase the boot count.
         */
        RPT_Data.OpsData.BootCount ++;
        CFE_ES_WriteToSysLog("%s: Boot count : %u\n", __func__, RPT_Data.OpsData.BootCount);

        /**
         * Store ResetCause
         */
        RPT_Data.ResetType = CFE_ES_GetResetType(&RPT_Data.ResetSubType);
        CFE_ES_WriteToSysLog("%s: Reset Type: %d || Reset SubType: %u\n", __func__, RPT_Data.ResetType, RPT_Data.ResetSubType);
        RPT_Data.OpsData.ResetCause = RPT_CalculateResetCause((uint8)RPT_Data.ResetType, (uint8)RPT_Data.ResetSubType);
        CFE_ES_WriteToSysLog("%s: Reset Cause : 0x%02X\n", __func__, RPT_Data.OpsData.ResetCause);
        
        RPT_Data.OpsData.CRC = RPT_CalculateCRC(&RPT_Data.OpsData, (sizeof(RPT_OperationData_t) - sizeof(uint32_t)));
        Status = RPT_WriteToFile(RPT_Data.OpsDataHandle, &RPT_Data.OpsData, sizeof(RPT_OperationData_t));
        if (Status != CFE_SUCCESS) {
            CFE_EVS_SendErr(RPT_DATA_WRITE_ERR_EID, "RPT Operation data write error.\n");
            Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }
    }

    return Status;
}

CFE_Status_t RPT_CriticalQInit(void) {
    CFE_Status_t Status;

    RPT_Data.CritDataHandle = RPT_OpenCriticalFile();
    if (RPT_Data.CritDataHandle == OS_OBJECT_ID_UNDEFINED) {
        CFE_EVS_SendErr(RPT_DATA_OPEN_ERR_EID, "RPT Critical Q Open failed.");
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    Status = RPT_ReadFromFile(RPT_Data.CritDataHandle, &RPT_Data.CritQueue, sizeof(RPT_CriticalQueue_t));
    if (Status < 0) {
        CFE_EVS_SendErr(RPT_DATA_READ_ERR_EID, "RPT critical Q Read fail.");
    }
    else if (RPT_Data.OpsData.BootCount == 0 || Status == 0) Status = CFE_SUCCESS;
    else if (RPT_Data.OpsData.BootCount != 0 || Status == sizeof(RPT_CriticalQueue_t)) Status = CFE_SUCCESS;

    if (Status == CFE_SUCCESS) {
        /**
         * CRC Check
         */
        uint32 CRC = RPT_CalculateCRC(&RPT_Data.CritQueue, (sizeof(RPT_CriticalQueue_t) - sizeof(uint32_t)));
        if (RPT_Data.CritQueue.CRC == CRC) {
            CFE_EVS_SendInfo(RPT_DATA_CRC_VALID_INF_EID, "Critical Q CRC well matched.\n");
            Status = CFE_SUCCESS;
        }
        else {
            CFE_EVS_SendErr(RPT_DATA_CRC_INVALID_ERR_EID, "Critical Q CRC not matched. Clear Critical Queue.\n");
            memset(&RPT_Data.CritQueue, 0, sizeof(RPT_CriticalQueue_t));
            Status = CFE_SUCCESS;
        }
    }

    return Status;
}
