#ifndef PAYUZUT_INTERNAL_CFG_H
#define PAYUZUT_INTERNAL_CFG_H

#include "common_types.h"


#define PAYUZUT_PIPE_DEPTH  50 /* Depth of the Command Pipe for Application */

#define PAYUZUT_TEMPERATURE_FILE    "./cf/sdcard/payuzut_temperature"
#define PAYUZUT_TEMP_TASK_STACK_SIZE        4096
#define PAYUZUT_TEMP_TASK_STACK_PRIORITY    150
#define PAYUZUT_TEMP_GATHER_TIME    30      // sec
#define PAYUZUT_TEMP_GATHER_TERM    1000   // milli sec

#endif
