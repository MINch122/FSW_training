#ifndef CFE_SRL_CSP_H
#define CFE_SRL_CSP_H

#include "cfe.h"

#include "common_types.h"
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <csp/arch/csp_thread.h>
#include <csp/drivers/usart.h>
#include <csp/drivers/can_socketcan.h>
#include <csp/interfaces/csp_if_zmqhub.h>
#include <gs/ftp/client.h>
#include <gs/csp/csp.h>
#include <gs/csp/router.h>
#include <gs/param/rparam.h>

#include <gs/csp/drivers/i2c/i2c.h>
#include "cfe_srl_extern_typedefs.h"
#include "cfe_srl_internal_cfg.h"
#include "cfe_srl_mission_cfg.h"
#include <gs/param/types.h>

#define CSP_TASK_STACK_SIZE(x)      (x)*1024

#define CSP_CAN_DEV_NAME            "can0"

#define CSP_TIMEOUT(x)              (x)*1000

/**
 * BASE5TH CSP Nodes
 * 
 * SANT is GomSpace but, doesn't need CSP
 * Consider CubeSpace's ADCS Solution.
 */

/**
 * Module internal CSP node configuration sturcture
 */
typedef struct {
    uint8_t Priority;
    uint32_t Timeout;
    uint32_t Options;
} CFE_SRL_CSP_Node_Config_t;



/**
 * Declaration of CSP function
 * Do NOT Call these function directly
 */
int CFE_SRL_RouteInitCSP(void);
int CFE_SRL_NodeConfigCSP(uint8_t Node, uint8_t Priority, uint32_t Timeout, uint32_t Options);
int CFE_SRL_GetNodeConfigCSP(uint8_t Node, CFE_SRL_CSP_Node_Config_t **Config);
int32 CFE_SRL_InitCSP(void);

int CFE_SRL_TransactionCSP(uint8_t Node, uint8_t Port, void *TxData, int TxSize, void *RxData, int RxSize);
int CFE_SRL_GetRparamCSP(uint8_t Type, uint8_t Node, gs_param_table_id_t TableId, uint16_t Addr, void *Param);
int CFE_SRL_SetRparamCSP(uint8_t Type, uint8_t Node, gs_param_table_id_t TableId, uint16_t Addr, void *Param);
int CFE_SRL_PingCSP(uint8 Node, uint32 Timeout, unsigned int Size, uint8 Options);


int CFE_SRL_RtableCSP(csp_iface_t *Iface);
int CFE_SRL_AllNodeConfigCSP(void);
void CFE_SRL_ConfigHost(csp_conf_t *Conf);
int CFE_SRL_ChangeVia(uint8_t Via);
void CFE_SRL_PrintRtable(void);

int CFE_SRL_RparamSaveCSP(uint8 Node, uint32 Timeout, uint8 TableId, uint8 To);

csp_socket_t *CFE_SRL_SocketCSP(uint32_t Option);
int CFE_SRL_BindCSP(csp_socket_t *Socket, uint8_t Port);
int CFE_SRL_ListenCSP(csp_socket_t *Socket, size_t BackLog);
csp_conn_t *CFE_SRL_AcceptCSP(csp_socket_t *Socket, uint32_t Timeout);
csp_packet_t *CFE_SRL_ReadCSP(csp_conn_t *Connection, uint32_t Timeout);
int CFE_SRL_ConndPort(csp_conn_t *Connection);
void CFE_SRL_BufferFreeCSP(csp_packet_t *Packet);

#endif /* CFE_SRL_CSP_H */