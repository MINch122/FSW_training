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

    CFE_SRL_IO_Param_t Params = {0,};

    uint8_t TxBuf[23] = {0,};

    /**
     * !!!!!!!!!!!!!!!!Revise the parameters!!!!!!!!!!!!!!!!!1
     */
    Params.TxData = TxBuf;
    Params.TxSize = sizeof(TxBuf);
    Params.RxData = &PAYUZUT_Data.BcnTlm.Payload.Temperature;
    Params.RxSize = sizeof(PAYUZUT_Data.BcnTlm.Payload.Temperature);
    Params.Addr = 0x23;

    CFE_SRL_ApiRead(PAYUZUT_Data.Handle, &Params);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYUZUT_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYUZUT_Data.BcnTlm.TelemetryHeader), true);

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
    int32 Status;
    CFE_SRL_IO_Param_t Params = {0,};

    uint8_t TxBuf[23] = {0,};
    uint8_t RxBuf[4]  = {0,};

    /**
     * !!!!!!!!!!!!!!!!Revise the parameters!!!!!!!!!!!!!!!!!1
     */
    Params.TxData = TxBuf;
    Params.TxSize = sizeof(TxBuf);
    Params.RxData = RxBuf;
    Params.RxSize = sizeof(RxBuf);
    Params.Addr = 0x23;

    Status = CFE_SRL_ApiRead(PAYUZUT_Data.Handle, &Params);
    if (Status != CFE_SUCCESS) PAYUZUT_Data.ErrCounter ++;

    PAYUZUT_ReportTlm_t Report = {0,};
    CFE_MSG_Init(CFE_MSG_PTR(Report.TelemetryHeader), CFE_SB_ValueToMsgId(PAYUZUT_REPORT_TLM_MID),
                    sizeof(PAYUZUT_ReportTlm_t));
    
    Report.Report.MsgID = PAYUZUT_CMD_MID;
    Report.Report.CommandCode = PAYUZUT_GET_TEMP_CC;
    Report.Report.ReturnType = (Status == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    Report.Report.ReturnCode = Status;
    Report.Report.ReturnDataSize = sizeof(RxBuf);
    memcpy(Report.Report.ReturnValue, RxBuf, sizeof(RxBuf));

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Report.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Report.TelemetryHeader), true);
    
    
    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUT Thruster ON commands                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_ThrusterOnCmd(const PAYUZUT_ThrusterOnCmd_t *Msg) {
    int32 Status;

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

    return CFE_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* PAYUZUT Thruster OFF commands                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYUZUT_ThrusterOffCmd(const PAYUZUT_ThrusterOffCmd_t *Msg) {
    int32 Status;

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

    return CFE_SUCCESS;
}
