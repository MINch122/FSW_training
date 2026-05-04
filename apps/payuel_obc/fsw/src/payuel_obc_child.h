#ifndef PAYUEL_OBC_CHILD_H
#define PAYUEL_OBC_CHILD_H

#include "cfe_error.h"

#include <stdbool.h>
#include <stdint.h>

/* Create the semaphore/mutex/task set used for long full-file downloads. */
CFE_Status_t PAYUEL_OBC_ChildInit(void);
/* Queue an image full-download request for the child task and return immediately to the caller. */
CFE_Status_t PAYUEL_OBC_QueueDownloadImage(uint8_t CameraID, uint8_t ImageNumber, uint8_t *ReturnTypeOut);
/* Queue a sensor-data full-download request for the child task and return immediately to the caller. */
CFE_Status_t PAYUEL_OBC_QueueDownloadSensor(uint8_t DataSlot, uint8_t DataNumber, uint8_t *ReturnTypeOut);
/* Report whether the child mailbox is occupied or the child is already processing a request. */
bool         PAYUEL_OBC_ChildIsBusy(void);

#endif /* PAYUEL_OBC_CHILD_H */
