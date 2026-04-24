#ifndef RPT_TBLDEFS_H
#define RPT_TBLDEFS_H


#include "common_types.h"
#include "rpt_mission_cfg.h"
#include "cfe_sb_api_typedefs.h"


/**
 * Table Constant definition
 */
/* UsedState */
#define RPT_DISABLED    0
#define RPT_ENABLED     1

/* IsCritical */
#define RPT_NOT_CRITICAL    0
#define RPT_CRITICAL        1




/**
 * RPT Table structure
 */
typedef struct {

    /**
     * MID to subscribe.
     */
    CFE_SB_MsgId_t MessageID;

    uint8 IsCritical;

} RPT_TableEntry_t;

#endif