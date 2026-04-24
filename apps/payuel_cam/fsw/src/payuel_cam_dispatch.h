#ifndef PAYUEL_CAM_DISPATCH_H
#define PAYUEL_CAM_DISPATCH_H

#include "cfe.h"
#include "payuel_cam_msg.h"

void PAYUEL_CAM_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);
void PAYUEL_CAM_ProcessGroundCommand(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* PAYUEL_CAM_DISPATCH_H */
