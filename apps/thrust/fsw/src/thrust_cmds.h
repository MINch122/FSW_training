#ifndef THRUST_CMDS_H
#define THRUST_CMDS_H

#include "cfe.h"           // CFE_Status_t, CFE_SUCCESS 등
#include "thrust_msg.h"    // 각 Cmd 메시지 구조체

/* -------------------------------------------------------
 * 결과 보고 (모든 명령 공통)
 * ------------------------------------------------------- */
void THRUST_HandleReportForMid(uint16_t msgId, int32 status, uint8_t cc, const void *data, uint16_t dataSize);
void THRUST_HandleReport(int32 status, uint8_t cc, const void *data, uint16_t dataSize);

/* -------------------------------------------------------
 * 기본 명령
 * ------------------------------------------------------- */
CFE_Status_t THRUST_NoopCmd(const THRUST_NoopCmd_t *Msg);
CFE_Status_t THRUST_ResetCountersCmd(const THRUST_ResetCountersCmd_t *Msg);

/* -------------------------------------------------------
 * HK 요청 명령
 * ------------------------------------------------------- */
CFE_Status_t THRUST_SendHkCmd(const THRUST_SendHkCmd_t *Msg);
CFE_Status_t THRUST_SendScheduledHkCmd(const THRUST_SendHkCmd_t *Msg);
CFE_Status_t THRUST_ReqStatusCmd(const THRUST_ReqStatusCmd_t *Msg);

/* -------------------------------------------------------
 * 지상국 명령 (ICD 표 2)
 * ------------------------------------------------------- */
CFE_Status_t THRUST_PingCmd(const THRUST_PingCmd_t *Msg);
CFE_Status_t THRUST_ResetModuleCmd(const THRUST_ResetModuleCmd_t *Msg);
CFE_Status_t THRUST_SetModeCmd(const THRUST_SetModeCmd_t *Msg);
CFE_Status_t THRUST_ArmCmd(const THRUST_ArmCmd_t *Msg);
CFE_Status_t THRUST_DisarmCmd(const THRUST_DisarmCmd_t *Msg);
CFE_Status_t THRUST_MainThrusterFireCmd(const THRUST_MainFireCmd_t *Msg);
CFE_Status_t THRUST_MainAbortCmd(const THRUST_MainAbortCmd_t *Msg);
CFE_Status_t THRUST_CGPulseCmd(const THRUST_CGPulseCmd_t *Msg);
CFE_Status_t THRUST_CGAbortCmd(const THRUST_CGAbortCmd_t *Msg);
CFE_Status_t THRUST_ReqFaultLogCmd(const THRUST_ReqFaultLogCmd_t *Msg);
CFE_Status_t THRUST_ClearFaultCmd(const THRUST_ClearFaultCmd_t *Msg);

#endif /* THRUST_CMDS_H */
