#ifndef LTRX_SESSION_H
#define LTRX_SESSION_H

#include "cfe.h"
#include "common_types.h"
#include <stdint.h>
#include <stdbool.h>

/* Session state used by session.c */
void LTRX_SessionOnIcdRx(uint8 type_id, uint8 status);
void LTRX_SessionOnIcdRxEx(uint8 type_id, uint8 status, uint32 msg_id);

typedef enum
{
    LTRX_SESS_IDLE = 0,

    /* Downlink (OBC -> Beacon -> GS) states */
    LTRX_SESS_WAIT_BEACON_REQ,     
    LTRX_SESS_DL_IN_PROGRESS,     

    /* Uplink (GS -> Beacon -> OBC) states */
    LTRX_SESS_UL_OFFERED,        
    LTRX_SESS_UL_IN_PROGRESS,       
    LTRX_SESS_UL_COMPLETE,         

    LTRX_SESS_COMPLETE,          
    LTRX_SESS_ABORTED,
    LTRX_SESS_ERROR
} LTRX_SessionState_t;

/* Request types pushed from dispatch -> session */
typedef enum
{
    LTRX_REQ_START_DOWNLINK = 0,
    LTRX_REQ_ABORT,
    LTRX_REQ_RESET,
    LTRX_REQ_QUERY_BEACON_STATUS,
    LTRX_REQ_QUERY_GNSS_INFO
} LTRX_SessionReqType_t;

/* Queue element */
typedef struct
{
    LTRX_SessionReqType_t type;
    uint32_t              arg_u32;
} LTRX_SessionReq_t;

/* Init + periodic tick (called by child task) */
void LTRX_SessionInit(void);
void LTRX_SessionTick(void);

 /* Type6 received, Type8 sent OK */
void LTRX_SessionNotifyUplinkOffered(void);  

/* State query */
LTRX_SessionState_t LTRX_SessionGetState(void);

/* Ground-command wrappers (dispatch calls these) */
CFE_Status_t LTRX_SessionRequestStartDownlink(void);
CFE_Status_t LTRX_SessionRequestAbort(void);
CFE_Status_t LTRX_SessionRequestReset(void);
CFE_Status_t LTRX_SessionRequestQueryBeaconStatus(void);
CFE_Status_t LTRX_SessionRequestQueryGnssInfo(void);

#endif /* LTRX_SESSION_H */