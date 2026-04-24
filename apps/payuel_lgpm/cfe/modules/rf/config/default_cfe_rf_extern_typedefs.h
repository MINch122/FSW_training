#ifndef CFE_RF_EXTERN_TYPEDEFS_H
#define CFE_RF_EXTERN_TYPEDEFS_H


#include "common_types.h"
#include "cfe_mission_cfg.h"
#include "cfe_msg_hdr.h"

/**
 * @brief CI Task Telemetry Message
 * @note This Tlm Msg only contain the Tlm hdr. Used for update last GS contact time
 */
typedef struct {

    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    
} CFE_RF_ContactTimeTlm_t;


/************************************
 * 
 * Uplink Port Definition
 * OBC should bind this port
 * GS should trasmit to this port
 * 
 ***********************************/
typedef enum {

    CFE_RF_UPORT_PING = 1,
    CFE_RF_UPORT_TC = 13,
    CFE_RF_UPORT_FTP = 14,
    
} CFE_RF_Uplink_Port_t;


/*************************************
 * 
 * Downlink Port Definition
 * GS should bind this port
 * OBC should transmit to this port
 * 
 *************************************/
typedef enum {
    
    CFE_RF_DPORT_TRX = 13,
    CFE_RF_DPORT_RPT = 24,
    CFE_RF_DPORT_BCN = 23,

} CFE_RF_Downlink_Port_t;


#endif