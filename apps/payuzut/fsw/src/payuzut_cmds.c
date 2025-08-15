/**
 * \file
 *   This file contains the source code for the PAY UZURO CAM Ground Command-handling functions
 */

/*
** Include Files:
*/
#include "payuzut_task.h"
#include "payuzut_cmds.h"
#include "payuzut_msgids.h"
#include "payuzut_eventids.h"
#include "payuzut_msg.h"

#include "payuzut_internal_cfg.h"
#include "payuzut_interface_cfg.h"
#include "payuzut_utils.h"
#include "cfe.h"
#include <unistd.h>


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUZUT_SendHkCmd(const PAYUZUT_SendHkCmd_t *Msg) {
    PAYUZUT_Data.HkTlm.Payload.CommandCounter = PAYUZUT_Data.CmdCounter;
    PAYUZUT_Data.HkTlm.Payload.CommandErrorCounter = PAYUZUT_Data.ErrCounter;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUZUT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUZUT_Data.HkTlm.TelemetryHeader), true);

    // CFE_EVS_SendEvent(PAYUZUT_SEND_HK_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUT Send HK Cmd Received.");

    return CFE_SUCCESS;
}

CFE_Status_t PAYUZUT_SendBcnCmd(const PAYUZUT_SendBcnCmd_t *Msg) {

    PAYUZUT_GetADCValue(PAYUZUT_ADC_SLAVE_ADDR_1, &PAYUZUT_Data.BcnTlm.Payload.ADC1);
    PAYUZUT_GetADCValue(PAYUZUT_ADC_SLAVE_ADDR_2, &PAYUZUT_Data.BcnTlm.Payload.ADC2);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUZUT_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUZUT_Data.BcnTlm.TelemetryHeader), true);

    memset(&PAYUZUT_Data.BcnTlm.Payload, 0, sizeof(PAYUZUT_Data.BcnTlm.Payload));

    // CFE_EVS_SendEvent(PAYUZUT_SEND_HK_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUT Send HK Cmd Received.");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUT NOOP commands                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_NoopCmd(const PAYUZUT_NoopCmd_t *Msg) {

    PAYUZUT_Data.CmdCounter++;

    uint8_t Cnts[2] = {PAYUZUT_Data.CmdCounter, PAYUZUT_Data.ErrCounter};

    PAYUZUT_ReportTlm_t Report = {0,};
    CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_REPORT_TLM_MID),
                    sizeof(PAYUZUT_ReportTlm_t));
    
    Report.Report.MsgID = PAYUZUT_CMD_MID;
    Report.Report.CommandCode = PAYUZUT_NOOP_CC;
    Report.Report.ReturnType = RPT_RETTYPE_SUCCESS;
    Report.Report.ReturnCode = CFE_SUCCESS;
    Report.Report.ReturnDataSize = sizeof(Cnts);
    memcpy(Report.Report.ReturnValue, Cnts, sizeof(Cnts));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);

    CFE_EVS_SendEvent(PAYUZUT_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUT Noop Command Received");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
CFE_Status_t PAYUZUT_ResetCountersCmd(const PAYUZUT_ResetCountersCmd_t *Msg) {
    PAYUZUT_Data.CmdCounter = 0;
    PAYUZUT_Data.ErrCounter = 0;
    PAYUZUT_Data.DeviceErrCounter = 0;

    CFE_EVS_SendEvent(PAYUZUT_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYUZUT Reset Counters Command Received");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUT Get Temperature commands                                           */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_GetTempCmd(const PAYUZUT_GetTempCmd_t *Msg) {
    int32 Status1, Status2;

    uint16_t ADC[2] = {0,};

    Status1 = PAYUZUT_GetADCValue(PAYUZUT_ADC_SLAVE_ADDR_1, &ADC[0]);
    if (Status1 != CFE_SUCCESS) PAYUZUT_Data.ErrCounter ++;
    Status2 = PAYUZUT_GetADCValue(PAYUZUT_ADC_SLAVE_ADDR_2, &ADC[1]);
    if (Status2 != CFE_SUCCESS) PAYUZUT_Data.ErrCounter ++;

    PAYUZUT_ReportTlm_t Report = {0,};
    CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_REPORT_TLM_MID),
                    sizeof(PAYUZUT_ReportTlm_t));
    
    Report.Report.MsgID = PAYUZUT_CMD_MID;
    Report.Report.CommandCode = PAYUZUT_GET_TEMP_CC;
    Report.Report.ReturnType = (Status1 == CFE_SUCCESS && Status2 == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    if (Report.Report.ReturnType == RPT_RETTYPE_SUCCESS) Report.Report.ReturnCode = CFE_SUCCESS;
    else if (Status1 != CFE_SUCCESS) Report.Report.ReturnCode = Status1;
    else Report.Report.ReturnCode = Status2;
    Report.Report.ReturnDataSize = sizeof(ADC);
    memcpy(Report.Report.ReturnValue, ADC, sizeof(ADC));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);

    OS_printf("ADC1 : %u || ADC2 : %u\n", ADC[0], ADC[1]);
    
    
    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUT Thruster ON commands                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_ThrusterOnCmd(const PAYUZUT_ThrusterOnCmd_t *Msg) {
    int32 Status = 0;

    Status = CFE_SRL_ApiGpioSet(PAYUZUT_Data.GpioHandle, true);
    if (Status != CFE_SUCCESS) PAYUZUT_Data.ErrCounter ++;
    
    PAYUZUT_ReportTlm_t Report = {0, };
    CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_REPORT_TLM_MID),
                    sizeof(PAYUZUT_ReportTlm_t));
    
    Report.Report.MsgID = PAYUZUT_CMD_MID;
    Report.Report.CommandCode = PAYUZUT_THRUSTER_ON_CC;
    Report.Report.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    Report.Report.ReturnCode = Status;
    Report.Report.ReturnDataSize = 0;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);

    CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "PAYUZUT: Thruster On Cmd.\n");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUT Thruster OFF commands                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_ThrusterOffCmd(const PAYUZUT_ThrusterOffCmd_t *Msg) {
    int32 Status = 0;

    Status = CFE_SRL_ApiGpioSet(PAYUZUT_Data.GpioHandle, false);
    if (Status != CFE_SUCCESS) PAYUZUT_Data.ErrCounter ++;

    PAYUZUT_ReportTlm_t Report = {0, };
    CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_REPORT_TLM_MID),
                    sizeof(PAYUZUT_ReportTlm_t));
    
    Report.Report.MsgID = PAYUZUT_CMD_MID;
    Report.Report.CommandCode = PAYUZUT_THRUSTER_ON_CC;
    Report.Report.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    Report.Report.ReturnCode = Status;
    Report.Report.ReturnDataSize = 0;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);

    CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "PAYUZUT: Thruster Off Cmd.\n");

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUT Cumulate Temperature Command                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_CumulateTempCmd(const PAYUZUT_CumulateTempCmd_t *Msg) {

    CFE_ES_CreateChildTask(&PAYUZUT_Data.TaskId, "Temperature Task", 
                        PAYUZUT_CumulateTempTask, CFE_ES_TASK_STACK_ALLOCATE,
                        PAYUZUT_TEMP_TASK_STACK_SIZE, PAYUZUT_TEMP_TASK_STACK_PRIORITY, 0);

    CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "PAYUZUT: Cumulate Temp Task Start.\n");
    
    return CFE_SUCCESS;
}


void PAYUZUT_CumulateTempTask(void) {
    int32 Status1 = 0, Status2 = 0;
    uint8_t Iteration = 0, Success = 0;

    int FD = PAYUZUT_OpenFile(0);
    int FD2 = PAYUZUT_OpenFile(1);
    if (FD < 0 || FD2 < 0) {
        if (FD >= 0) PAYUZUT_CloseFile(FD);
        if (FD2 >= 0) PAYUZUT_CloseFile(FD2);
        return;
    }

    do {
        OS_printf("Iteration : %u || Success : %u\n", Iteration, Success);
        uint16_t Temp1 = 0xFFFF, Temp2 = 0xFFFF;

        Status1 = PAYUZUT_ConfigADC(PAYUZUT_ADC_SLAVE_ADDR_1);
        Status2 = PAYUZUT_ConfigADC(PAYUZUT_ADC_SLAVE_ADDR_2);

        if (Status1 == CFE_SUCCESS || Status2 == CFE_SUCCESS)  {
            OS_TaskDelay(PAYUZUT_ADC_POLL_MSEC);
            if (Status1 == CFE_SUCCESS) {
                Status1 = PAYUZUT_ReadADC(PAYUZUT_ADC_SLAVE_ADDR_1, &Temp1);
            }
            if (Status2 == CFE_SUCCESS) {
                Status2 = PAYUZUT_ReadADC(PAYUZUT_ADC_SLAVE_ADDR_2, &Temp2);
            }
        }
        else {
            Iteration ++;
            continue;
        }
        
        PAYUZUT_WriteToFile(FD, &Temp1, sizeof(Temp1));
        PAYUZUT_WriteToFile(FD2, &Temp2, sizeof(Temp2));

        if (Status1 == CFE_SUCCESS || Status2 == CFE_SUCCESS) Success ++;
        Iteration ++;

        OS_TaskDelay(PAYUZUT_TEMP_GATHER_TERM - PAYUZUT_ADC_POLL_MSEC);

    } while (Iteration < 255 && Success < PAYUZUT_TEMP_GATHER_TIME);

    PAYUZUT_CloseFile(FD);
    PAYUZUT_CloseFile(FD2);

    CFE_EVS_SendEvent(488, CFE_EVS_EventType_INFORMATION, "PAYUZUT temperature gathering Done.\n");
}
