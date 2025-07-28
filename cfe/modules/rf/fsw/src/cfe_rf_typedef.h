#ifndef CFE_RF_TYPEDEF_H
#define CFE_RF_TYPEDEF_H

#include "cfe.h"

#include <csp/csp.h>

#define CSP_TIMEOUT(x)              (x)*1000
#define CI_TASK_STACK_SIZE(x)       (x)*4096
#define CI_TASK_PRIORITY(x)         (((x) < 0) ? 0 : ((x) > 255) ? 255 : (x))

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
    
    CFE_RF_DPORT_RPT = 13,
    CFE_RF_DPORT_BCN = 31,
    CFE_RF_DPORT_FTP = 14,

} CFE_RF_Downlink_Port_t;

#endif /* CFE_RF_TYPEDEF_H */