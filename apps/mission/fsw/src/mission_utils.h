#ifndef MISSION_UTILS_H
#define MISSION_UTILS_H

#include "common_types.h"
#include "gpio_msgids.h"
#include "gpio_msg.h"
#include "to_lab_msgids.h"
#include "to_lab_msg.h"

CFE_Status_t MISSION_LEOP_Lock(void);
void         MISSION_LEOP_Unlock(void);
CFE_Status_t MISSION_LEOP_SaveState(void);
void         MISSION_LEOP_Process(void);
void         MISSION_LEOP_Task(void);

#endif
