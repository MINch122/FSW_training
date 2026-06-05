#ifndef LTRX_CHILD_H
#define LTRX_CHILD_H

#include "cfe.h"
#include <stdbool.h>

#ifndef LTRX_CHILD_TASK_PRIORITY
#define LTRX_CHILD_TASK_PRIORITY  120
#endif

#ifndef LTRX_CHILD_TASK_STACK_SIZE
#define LTRX_CHILD_TASK_STACK_SIZE  (32 * 1024)
#endif

#ifndef LTRX_CHILD_TASK_NAME
#define LTRX_CHILD_TASK_NAME "LTRX_CHILD"
#endif

CFE_Status_t LTRX_ChildCreate(void);
void         LTRX_ChildWake(void);
void         LTRX_ChildRequestShutdown(void);

#endif /* LTRX_CHILD_H */