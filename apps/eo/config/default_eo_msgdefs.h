#ifndef EO_MSGDEFS_H
#define EO_MSGDEFS_H

#include "common_types.h"
#include "eo_fcncodes.h"


typedef struct EO_HkTlm_Payload{
    uint8 CmdCounter;
    uint8 CmdErrCounter;

    EO_CurrentStep_t PhaseInfo;

}__attribute__((packed)) EO_BcnTlm_Payload_t;


typedef struct EO_Report_Payload {

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

} EO_Report_Payload_t;



typedef struct EO_ClearQueue_Payload {

    uint8 IsCritical;
    
} EO_ClearQueue_Payload_t;

#endif