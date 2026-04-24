#ifndef PAYUEL_CAM_CHILD_H
#define PAYUEL_CAM_CHILD_H

#include "cfe_error.h"

#include <stdbool.h>
#include <stdint.h>

/* Create the semaphore/mutex/task set used for the long full-image download job. */
CFE_Status_t PAYUEL_CAM_ChildInit(void);
/* Queue one full-image download request for the child task and return immediately to the caller. */
CFE_Status_t PAYUEL_CAM_QueueDownloadImage(uint8_t ImageSlot, uint8_t ImageNumber, uint8_t *ReturnTypeOut);
/* Report whether the child mailbox is occupied or the child is already processing a request. */
bool         PAYUEL_CAM_ChildIsBusy(void);

#endif /* PAYUEL_CAM_CHILD_H */
