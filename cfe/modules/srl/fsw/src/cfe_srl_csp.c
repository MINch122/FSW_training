#include "cfe_srl_csp.h"

const char *SatName = "COSMIC"; // Revise name according to specific misison

/**
 * Indexed by CSP Node number.
 */
CFE_SRL_CSP_Node_Config_t *NodeConfig[CFE_SRL_CSP_MAX_DEVICE_NUM] = {0,};

int CFE_SRL_RouteInitCSP(void) {
    int Status;

    csp_iface_t *InterfaceCAN = NULL;
    // csp_iface_t *InterfaceI2C = NULL;

    Status = csp_route_start_task(CSP_TASK_STACK_SIZE(1), GS_THREAD_PRIORITY_HIGH);
    if (Status != CSP_ERR_NONE) {
        CFE_ES_WriteToSysLog("%s: csp_route_start_task failed! CSP RC=%d\n", __func__, Status);
        return CFE_SRL_CSP_ROUTE_START_ERR;
    }

    /**
     * CSP CAN Initialization
     * @param bitrate meaningless
     */
    Status = csp_can_socketcan_open_and_add_interface(CSP_CAN_DEV_NAME, "CSP CAN", 1000000, false, &InterfaceCAN);
    if (Status != CSP_ERR_NONE) {
        CFE_ES_WriteToSysLog("%s: CSP Socket CAN Init failed! RC = %d", __func__, Status);
        return CFE_SRL_CSP_CAN_INIT_ERR;
    }
    /**
     * CSP I2C Initialization
     */
    // Status = gs_csp_i2c_init2(0, 0, "p31u", false, &InterfaceI2C);
    // if (Status != GS_OK) return CFE_SRL_CSP_I2C_INIT_ERR;

    /**
     * CSP Routing Table Set
     */
    /* CAN */
    Status = CFE_SRL_RtableCSP(InterfaceCAN);
    if (Status != CFE_SUCCESS) return CFE_SRL_CSP_RTABLE_SET_ERR;
    // Status = csp_rtable_set(CSP_NODE_UTRX, CSP_ID_HOST_SIZE, InterfaceCAN, CSP_NO_VIA_ADDRESS);
    // if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

    // Status = csp_rtable_set(CSP_NODE_STRX, CSP_ID_HOST_SIZE, InterfaceCAN, CSP_NO_VIA_ADDRESS);
    // if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

    // Status = csp_rtable_set(CSP_NODE_GS_KISS, CSP_ID_HOST_SIZE, InterfaceCAN, CSP_NODE_UTRX);
    // if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

    // Status = csp_rtable_set(CSP_NODE_GSTRX, CSP_ID_HOST_SIZE, InterfaceCAN, CSP_NODE_UTRX);
    // if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;
    /* I2C */
    // Status = csp_rtable_set(CSP_NODE_EPS, CSP_ID_HOST_SIZE, InterfaceI2C, CSP_NO_VIA_ADDRESS);
    // if (Status != CSP_ERR_NONE) return CFE_SRL_CSP_RTABLE_SET_ERR;

    return CFE_SUCCESS;
}

int CFE_SRL_NodeConfigCSP(uint8_t Node, uint8_t Priority, uint32_t Timeout, uint32_t Options) {
    if (NodeConfig[Node] == NULL) {
        NodeConfig[Node] = (CFE_SRL_CSP_Node_Config_t *)malloc(sizeof(CFE_SRL_CSP_Node_Config_t));
        memset(NodeConfig[Node], 0, sizeof(CFE_SRL_CSP_Node_Config_t));
    }

    NodeConfig[Node]->Priority = Priority;
    NodeConfig[Node]->Timeout = Timeout;
    NodeConfig[Node]->Options = Options;

    return CFE_SUCCESS;
}

int CFE_SRL_GetNodeConfigCSP(uint8_t Node, CFE_SRL_CSP_Node_Config_t **Config) {
    if (Config == NULL) return CFE_SRL_BAD_ARGUMENT;
    if (NodeConfig[Node] == NULL) return CFE_SRL_NOT_FOUND;

    *Config = NodeConfig[Node];
    return CFE_SUCCESS;
}

/**
 * CSP Integrated Initialization function
 * Called by Early Init of CFE SRL module
 */
int CFE_SRL_InitCSP(void) {
    int Status;

    // csp_conf_t CspConfig = {
    //     .address            =   CSP_NODE_OBC, // Revise to Real OBC Node
    //     .hostname           =   "OBC",
    //     .model              =   SatName,
    //     .revision           =   "mozart",
    //     .conn_max           =   10, // Revise if needed. If external CSP device is too many.
    //     .conn_queue_length  =   10, // Connection queue's packet number.
    //     .fifo_length        =   25, // Router queue's packet number.
    //     .port_max_bind      =   32,
    //     .rdp_max_window     =   20,
    //     .buffers            =   10,
    //     .buffer_data_size   =   256,
    //     .conn_dfl_so        =   CSP_O_NONE
    // };
    csp_conf_t CspConfig;
    CFE_SRL_ConfigHost(&CspConfig);
    
    Status = csp_init(&CspConfig);
    if (Status != CSP_ERR_NONE) {
        CFE_ES_WriteToSysLog("%s: csp_init failed! CSP RC=%d\n",__func__, Status);
        return CFE_SRL_CSP_INIT_ERR;
    }

    /**
     * Init CSP CAN and Register Routing Configuration
     */
    Status = CFE_SRL_RouteInitCSP();
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: CSP Route Init failed! CSP RC=%d\n", __func__, Status);
        return CFE_SRL_CSP_ROUTE_INIT_ERR;
    }


    /**
     * Register Node Configuration to each Node.
     * If Node appended(or revised), insert(or revise) the function.
     */
    CFE_SRL_AllNodeConfigCSP();
    // CFE_SRL_NodeConfigCSP(CSP_NODE_EPS, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
    // CFE_SRL_NodeConfigCSP(CSP_NODE_UTRX, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);
    // CFE_SRL_NodeConfigCSP(CSP_NODE_STRX, CSP_PRIO_NORM, CSP_TIMEOUT(1), CSP_O_CRC32);

    // CFE_SRL_NodeConfigCSP(CSP_NODE_GS_KISS, CSP_PRIO_HIGH, CSP_TIMEOUT(3), CSP_O_CRC32);
    // CFE_SRL_NodeConfigCSP(CSP_NODE_GSTRX, CSP_PRIO_HIGH, CSP_TIMEOUT(3), CSP_O_CRC32);

    return CFE_SUCCESS;
}

/**
 * CSP Transaction function. Wrapped to API function
 * Do NOT call this function directly
 */
int CFE_SRL_TransactionCSP(uint8_t Node, uint8_t Port, void *TxData, int TxSize, void *RxData, int RxSize) {
    // If there is no reply, Put `RxSize = 0`
    int Status;
    if (NodeConfig[Node] == NULL || TxData == NULL || RxData == NULL) return CFE_SRL_BAD_ARGUMENT;

    Status = csp_transaction_w_opts(NodeConfig[Node]->Priority, Node, Port, NodeConfig[Node]->Timeout, TxData, TxSize, RxData, RxSize, NodeConfig[Node]->Options);
    if (Status == 0) return CFE_SRL_TRANSACTION_ERR;

    // 1 or `reply size` on success
    return Status;
}

/**
 * CSP rparam function
 * These functions wrapped to API function
 * Do NOT call this function directly
 */
int CFE_SRL_GetRparamCSP(uint8_t Type, uint8_t Node, gs_param_table_id_t TableId, uint16_t Addr, void *Param) {
    int Status;
    if (Param == NULL) return CFE_SRL_BAD_ARGUMENT;
    
    switch(Type) {
        case GS_PARAM_UINT8:    
            Status = gs_rparam_get_uint8(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (uint8_t *)Param);
            break;
        case GS_PARAM_INT8:
            Status = gs_rparam_get_int8(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (int8_t *)Param);
            break;
        case GS_PARAM_UINT16:
            Status = gs_rparam_get_uint16(Node, TableId, Addr,GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (uint16_t *)Param);
            break;
        case GS_PARAM_INT16:
            Status = gs_rparam_get_int16(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (int16_t *)Param);
            break;
        case GS_PARAM_UINT32:
            Status = gs_rparam_get_uint32(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (uint32_t *)Param);
            break;
        case GS_PARAM_INT32:
            Status = gs_rparam_get_int32(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (int32_t *)Param);
            break;
        case GS_PARAM_UINT64:
            Status = gs_rparam_get_uint64(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (uint64_t *)Param);
            break;
        case GS_PARAM_INT64:
            Status = gs_rparam_get_int64(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (int64_t *)Param);
            break;
        case GS_PARAM_FLOAT:
            Status = gs_rparam_get_float(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (float *)Param);
            break;
        case GS_PARAM_DOUBLE:
            Status = gs_rparam_get_double(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, (double *)Param);
            break;
        default:
            Status = CFE_SRL_TYPE_UNSUPPORTED;
            break;
    }

    if (Status != GS_OK) {
        CFE_ES_WriteToSysLog("%s: Get param failed! CSP RC=%d", __func__, Status);
        return CFE_SRL_GET_RPARAM_ERR;
    }
    return CFE_SUCCESS;
}

int CFE_SRL_SetRparamCSP(uint8_t Type, uint8_t Node, gs_param_table_id_t TableId, uint16_t Addr, void *Param) {
    int Status;

    if (Param == NULL) return CFE_SRL_BAD_ARGUMENT;

    switch(Type) {
        case GS_PARAM_UINT8:
            Status = gs_rparam_set_uint8(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(uint8_t *)Param);
            break;
        case GS_PARAM_INT8:
            Status = gs_rparam_set_int8(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(int8_t *)Param);
            break;
        case GS_PARAM_UINT16:
            Status = gs_rparam_set_uint16(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(uint16_t *)Param);
            break;
        case GS_PARAM_INT16:
            Status = gs_rparam_set_int16(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(int16_t *)Param);
            break;
        case GS_PARAM_UINT32:
            Status = gs_rparam_set_uint32(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(uint32_t *)Param);
            break;
        case GS_PARAM_INT32:
            Status = gs_rparam_set_int32(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(int32_t *)Param);
            break;
        case GS_PARAM_UINT64:
            Status = gs_rparam_set_uint64(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(uint64_t *)Param);
            break;
        case GS_PARAM_INT64:
            Status = gs_rparam_set_int64(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(int64_t *)Param);
            break;
        case GS_PARAM_FLOAT:
            Status = gs_rparam_set_float(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(float *)Param);
            break;
        case GS_PARAM_DOUBLE:
            Status = gs_rparam_set_double(Node, TableId, Addr, GS_RPARAM_MAGIC_CHECKSUM, NodeConfig[Node]->Timeout, *(double *)Param);
            break;
        default:
            Status = CFE_SRL_TYPE_UNSUPPORTED;
            break;
    }

    if (Status != GS_OK) {
        CFE_ES_WriteToSysLog("%s: Set param failed! CSP RC=%d", __func__, Status);
        return CFE_SRL_SET_RPARAM_ERR;
    }
    return CFE_SUCCESS;
}

int CFE_SRL_PingCSP(uint8 Node, uint32 Timeout, unsigned int Size, uint8 Options) {
    return csp_ping(Node, Timeout, Size, Options);
}