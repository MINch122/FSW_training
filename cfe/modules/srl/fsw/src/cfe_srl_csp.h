#ifndef CFE_SRL_CSP_H
#define CFE_SRL_CSP_H

// #include "cfe.h"

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

#include "cfe_srl_basic.h"
#include <gs/param/types.h>

#define CSP_TASK_STACK_SIZE(x)      (x)*1024

#define CSP_CAN_DEV_NAME            "can0"

#define CSP_TIMEOUT(x)              (x)*1000

/**
 * COSMIC CSP Nodes
 * 
 * SANT is GomSpace but, doesn't need CSP
 * Consider CubeSpace's ADCS Solution.
 */
// typedef enum {
//     CSP_NODE_EPS = 2,   // EPS Dock p31u Node
//     CSP_NODE_OBC = 3,
//     CSP_NODE_UTRX = 5,
//     CSP_NODE_STRX = 6,
//     CSP_NODE_GS_KISS = 8,
//     CSP_NODE_GSTRX = 20,
//     // CSP_NODE_ADCS = 24
// } CFE_SRL_CSP_Node_t;

// typedef struct {
//     uint8_t Priority;
//     uint32_t Timeout;
//     uint32_t Options;
// } CFE_SRL_CSP_Node_Config_t;



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
#endif /* CFE_SRL_CSP_H */