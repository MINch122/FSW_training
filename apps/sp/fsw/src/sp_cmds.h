#ifndef SP_CMDS_H
#define SP_CMDS_H

#include "cfe_error.h"
#include "sp_msg.h"

CFE_Status_t SP_SendHkCmd(const SP_SendHkCmd_t *Msg);
CFE_Status_t SP_SendBcnCmd(const SP_SendBcnCmd_t *Msg);
CFE_Status_t SP_NoopCmd(const SP_NoopCmd_t *Msg);
CFE_Status_t SP_ResetCounterCmd(const SP_ResetCountersCmd_t *Msg);
CFE_Status_t SP_GetHkCmd(const SP_GetHkCmd_t *Msg);
CFE_Status_t SP_DeployCmd(const SP_DeployCmd_t *Msg);
CFE_Status_t SP_StopBurnCmd(const SP_StopBurnCmd_t *Msg);
CFE_Status_t SP_AutoDeployCmd(const SP_AutoDeployCmd_t *Msg);
/**
 * AR6 I2C address commissioning commands.
 *
 * Background:
 *   The OBC is the I2C master. When SP_ScanAr6Cmd is called, the OBC probes
 *   every address from 1 to 127 on the I2C1 bus by sending GSSB transactions.
 *   An AR6 board at a given address replies with ACK; empty addresses return NAK.
 *   No prior knowledge of the slave address is required for scanning.
 *
 * Commissioning procedure (one-time, hardware setup):
 *
 *   Step 1. Connect ONLY ONE AR6 board to the OBC I2C1 bus.
 *
 *   Step 2. Send SP_SCAN_AR6_CC (no parameters).
 *           The OBC scans addresses 1-127 by itself and emits an EVS event for
 *           each address that responds:
 *             "SP: GSSB device found at I2C addr 0xXX"
 *           Note the reported address — this is the factory default address.
 *
 *   Step 3. Send SP_SET_AR6_ADDR_CC with:
 *             CurrentAddr = <address found in Step 2>
 *             NewAddr     = 0x11  (target address for DSP1 Board A, per config)
 *           The OBC sends a GSSB "set address" frame to CurrentAddr, then a
 *           "commit" frame to NewAddr, which writes the new address to the
 *           AR6's internal NVM (flash). The board now permanently uses NewAddr.
 *
 *   Step 4. Disconnect this board, connect the next AR6 board.
 *           Repeat Steps 2-3 for the remaining three boards:
 *             DSP1 Board B → NewAddr = 0x12
 *             DSP2 Board A → NewAddr = 0x13
 *             DSP2 Board B → NewAddr = 0x14
 *
 *   Step 5. Connect all four boards. Send SP_SCAN_AR6_CC to verify:
 *           Expected result: four events at 0x11, 0x12, 0x13, 0x14.
 *           All operational commands (DEPLOY, GET_HK, etc.) will now work.
 */
CFE_Status_t SP_ScanAr6Cmd(const SP_ScanAr6Cmd_t *Msg);
CFE_Status_t SP_SetAr6AddrCmd(const SP_SetAr6AddrCmd_t *Msg);

#endif