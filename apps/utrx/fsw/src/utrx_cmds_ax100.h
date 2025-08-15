#ifndef UTRX_AX100_CMDS_H
#define UTRX_AX100_CMDS_H

#include "utrx_app.h"
#include "utrx_app_dispatch.h"
#include "utrx_app_cmds.h"
#include "utrx_app_eventids.h"
#include "utrx_app_msgids.h"
#include "utrx_app_msg.h"
#include "utrx_app_fcncodes.h"
#include <gs/param/rparam.h>

// Wrapping Set function

void UTRX_AX100_GndwdtClearCmd(const UTRX_AX100_GndwdtClear_t *Msg);
void UTRX_AX100_UTRX_RebootCmd(const UTRX_AX100_UTRX_Reboot_t *Msg);
void UTRX_AX100_RXCONF_SetBaudCmd(const UTRX_AX100_RXCONF_SetBaudCmd_t *Msg);
void UTRX_AX100_TXCONF_SetBaudCmd(const UTRX_AX100_TXCONF_SetBaudCmd_t *Msg);
void UTRX_AX100_RXCONF_SetFreqCmd(const UTRX_AX100_RXCONF_SetFreqCmd_t *Msg);
void UTRX_AX100_TXCONF_SetFreqCmd(const UTRX_AX100_TXCONF_SetFreqCmd_t *Msg);
void UTRX_AX100_SetDefaultBaudCmd(const UTRX_AX100_SetDefaultBaudCmd_t *Msg);
void UTRX_AX100_RparamSave1Cmd(const UTRX_AX100_RparamSave1Cmd_t *Msg);
void UTRX_AX100_RparamSave5Cmd(const UTRX_AX100_RparamSave5Cmd_t *Msg);
void UTRX_AX100_RparamSaveAllCmd(const UTRX_AX100_RparamSaveAllCmd_t *Msg);
void UTRX_AX100_CheckStatePingCmd(const UTRX_AX100_CheckStatePingCmd_t *Msg);

/* ============== GET / TELEMETRY COMMANDS ============== */
/* RXCONF */
void UTRX_AX100_RXCONF_GetBaudCmd  (const UTRX_AX100_GetRxBaudCmd_t  *Msg);
void UTRX_AX100_RXCONF_GetGuardCmd (const UTRX_AX100_GetRxGuardCmd_t *Msg);
void UTRX_AX100_RXCONF_GetFreqCmd  (const UTRX_AX100_GetRxFreqCmd_t  *Msg);

/* TXCONF */
void UTRX_AX100_TXCONF_GetBaudCmd  (const UTRX_AX100_GetTxBaudCmd_t  *Msg);
void UTRX_AX100_TXCONF_GetFreqCmd  (const UTRX_AX100_GetTxFreqCmd_t  *Msg);

/* TLM */
void UTRX_AX100_TLM_GetTempBrdCmd      (const UTRX_AX100_GetTempBrdCmd_t      *Msg);
void UTRX_AX100_TLM_GetLastRssiCmd     (const UTRX_AX100_GetLastRssiCmd_t     *Msg);
void UTRX_AX100_TLM_GetLastRferrCmd    (const UTRX_AX100_GetLastRferrCmd_t    *Msg);
void UTRX_AX100_TLM_GetActiveConfCmd   (const UTRX_AX100_GetActiveConfCmd_t   *Msg);
void UTRX_AX100_TLM_GetBootCountCmd    (const UTRX_AX100_GetBootCountCmd_t    *Msg);
void UTRX_AX100_TLM_GetBootCauseCmd    (const UTRX_AX100_GetBootCauseCmd_t    *Msg);
void UTRX_AX100_TLM_GetLastContactCmd  (const UTRX_AX100_GetLastContactCmd_t  *Msg);
void UTRX_AX100_TLM_GetTotTxBytesCmd   (const UTRX_AX100_GetTotTxBytesCmd_t   *Msg);
void UTRX_AX100_TLM_GetTotRxBytesCmd   (const UTRX_AX100_GetTotRxBytesCmd_t   *Msg);

static inline void UTRX_CountFromReport(void)
{
    CmdErrCounter(&UTRX_APP_Data.CmdCounter,
                  &UTRX_APP_Data.AppErrCounter,
                  &UTRX_APP_Data.DeviceErrCounter,
                  UTRX_APP_Data.RptPkt.Report.ReturnType,
                  UTRX_APP_Data.RptPkt.Report.ReturnCode);
}




// void UTRX_AX100_GetStatusConfigurationCmd(const UTRX_AX100_GetStatusConfigurationCmd_t *Msg);

// Wrapping Get function



#endif /* UTRX_AX100_CMDS_H */