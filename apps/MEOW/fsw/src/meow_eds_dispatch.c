#include "meow.h"
#include "meow_dispatch.h"
#include "meow_eventids.h"

/* EDS-based dispatch is not implemented. The non-EDS meow_dispatch.c is the
 * authoritative path; this file only exists to satisfy the EDS build variant. */

void MEOW_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_EVS_SendEvent(MEOW_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                      "MEOW: EDS dispatch not implemented");
}
