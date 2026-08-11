#ifndef RPT_MSGDEFS_H
#define RPT_MSGDEFS_H

#include "common_types.h"
#include "rpt_fcncodes.h"


typedef struct RPT_HkTlm_Payload{

    /**
     * Operation Data
     */
    uint16 BootCount;

    /**
     * Last successfully stored operation-data backup number
     */
    uint32 Sequence;

    /**
     * Reset Cause
     */
    uint8 ResetCause;

}__attribute__((packed)) RPT_BcnTlm_Payload_t;


typedef struct RPT_Report_Payload {

    /**
     * Start Index of Queue
     */
    uint8 StartIdx;

    /**
     * \# of report number want to get
     */
    uint8 TotalNumber;

    /**
     * Critical report flag
     * `false` : Report queue
     * `true`  : Critical queue
     */
    uint8 IsCritical;

} RPT_Report_Payload_t;



typedef struct RPT_ClearQueue_Payload {

    uint8 IsCritical;
    
} RPT_ClearQueue_Payload_t;

#endif
