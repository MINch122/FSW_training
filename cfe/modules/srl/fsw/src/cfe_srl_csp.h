#ifndef CFE_SRL_CSP_H
#define CFE_SRL_CSP_H

#include "cfe.h"

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
typedef enum {
    CSP_NODE_EPS = 2,   // EPS Dock p31u Node
    CSP_NODE_OBC = 3,
    CSP_NODE_UTRX = 5,
    CSP_NODE_STRX = 6,
    CSP_NODE_GS_KISS = 8,
    CSP_NODE_GSTRX = 20,
    // CSP_NODE_ADCS = 24
} CFE_SRL_CSP_Node_t;

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
int CFE_SRL_InitCSP(void);

int CFE_SRL_TransactionCSP(uint8_t Node, uint8_t Port, void *TxData, int TxSize, void *RxData, int RxSize);
int CFE_SRL_GetRparamCSP(gs_param_type_t Type, uint8_t Node, gs_param_table_id_t TableId, uint16_t Addr, void *Param);
int CFE_SRL_SetRparamCSP(gs_param_type_t Type, uint8_t Node, gs_param_table_id_t TableId, uint16_t Addr, void *Param);
#endif /* CFE_SRL_CSP_H */