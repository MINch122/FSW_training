/**
 * @file
 *
 * Define LTRX Event IDs
 */
#ifndef LTRX_EVENTIDS_H
#define LTRX_EVENTIDS_H

/* Base / Initialization */
#define LTRX_RESERVED_EID          0
#define LTRX_INIT_INF_EID          1   
#define LTRX_INIT_ERR_EID          2  

/* Software Bus / Pipe */
#define LTRX_CR_PIPE_ERR_EID       3   
#define LTRX_SUB_STATUS_ERR_EID    4   
#define LTRX_SUB_HK_ERR_EID        5   
#define LTRX_SUB_CMD_ERR_EID       6   
#define LTRX_PIPE_ERR_EID          7   
#define LTRX_MID_ERR_EID           8   
#define LTRX_SUB_BCN_ERR_EID       28

/* Command Processing */
#define LTRX_CC_ERR_EID            9   
#define LTRX_CMD_LEN_ERR_EID       10  
#define LTRX_CMD_INF_EID           11  
#define LTRX_CMD_ERR_EID           12  

#define LTRX_NOOP_INF_EID               13
#define LTRX_RESET_INF_EID              14
#define LTRX_RESET_APP_COUNTER_EID      15
#define LTRX_RESET_DEVICE_COUNTER_EID   16

/* Telemetry / Allocation */
#define LTRX_ALLOC_ERR_EID         17  /* SB buffer allocation failure */
#define LTRX_HK_TX_ERR_EID         18  /* HK transmit failure */
#define LTRX_STATUS_TX_ERR_EID     19  /* Status transmit failure */

/* Child Task */
#define LTRX_CHILD_CREATE_ERR_EID  20  /* Child task create failure */
#define LTRX_CHILD_WAKE_ERR_EID    21  /* Child wake/semaphore error */

/* Session / Protocol */
#define LTRX_SESS_ERR_EID          22  /* Session error */
#define LTRX_SESS_TIMEOUT_EID      23  /* Session timeout */
#define LTRX_SESS_STATE_INF_EID    24  /* Session state info */

/* Beacon / ICD Layer */
#define LTRX_BCN_TX_ERR_EID        25  /* Beacon transmit failure */
#define LTRX_BCN_RX_ERR_EID        26  /* Beacon receive parse failure */
#define LTRX_BCN_PROTO_ERR_EID     27  /* Beacon protocol violation */

/* Bus Beacon / Downlink data */
#define LTRX_SUB_BUS_BCN_ERR_EID  30  /* Bus Beacon subscribe failure */
#define LTRX_BUS_BCN_INF_EID      31  /* Bus Beacon received & staged for downlink */
 
/* Uplink forward to SB */
#define LTRX_UL_FWD_INF_EID       32  /* Uplink message forwarded to SB */
#define LTRX_UL_FWD_ERR_EID       33  /* Uplink forward failed */

/* Downstream gating */
#define LTRX_DOWNSTREAM_ENABLE_INF_EID   34  /* Downstream enabled */
#define LTRX_DOWNSTREAM_DISABLE_INF_EID  35  /* Downstream disabled */
#define LTRX_BUS_BCN_PERIOD_INF_EID      36  /* Bus Beacon period updated */

/* Transport wrapper (CSP) errors */
#define LTRX_TX_ERR_EID            40
#define LTRX_RX_ERR_EID            41

#endif /* LTRX_EVENTIDS_H */
