#include "ltrx_child.h"
#include "ltrx_cmds_beacon.h"
#include "ltrx_session.h"
#include "ltrx_eventids.h"

#include "cfe.h"
#include "osapi.h"

#include <stdbool.h>

/* ---- internal child task state ---- */
static CFE_ES_TaskId_t s_ChildTaskId;
static osal_id_t       s_WakeSem = OS_OBJECT_ID_UNDEFINED;
static volatile bool   s_ShutdownRequested = false;     /* parent shutdown flag*/

static void LTRX_ChildMain(void)
{
    /* Child task shut by parent shutdown flag, not CFE_ES_RunLoop */

    LTRX_SessionInit();

    while (!s_ShutdownRequested)
    {
        if (OS_ObjectIdDefined(s_WakeSem))
        {
            (void)OS_BinSemTimedWait(s_WakeSem, 10);
        }
        else
        {
            OS_TaskDelay(10);
        }

        (void)LTRX_BcnProcessOneRx(200);
        LTRX_SessionTick();
        LTRX_Uplink_ForwardToSB();
    }
}

CFE_Status_t LTRX_ChildCreate(void)
{
    CFE_Status_t status;

    s_ShutdownRequested = false;

    if (!OS_ObjectIdDefined(s_WakeSem))
    {
        int32 os_rc = OS_BinSemCreate(&s_WakeSem, "LTRX_CHILD_WAKE", 0, 0);
        if (os_rc != OS_SUCCESS)
        {
            CFE_EVS_SendEvent(LTRX_CHILD_CREATE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "LTRX: failed to create wake semaphore (RC=%ld)",
                              (long)os_rc);
        }
    }

    status = CFE_ES_CreateChildTask(&s_ChildTaskId,
                                   LTRX_CHILD_TASK_NAME,
                                   (CFE_ES_ChildTaskMainFuncPtr_t)LTRX_ChildMain,
                                   NULL,
                                   LTRX_CHILD_TASK_STACK_SIZE,
                                   LTRX_CHILD_TASK_PRIORITY,
                                   0);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LTRX_CHILD_CREATE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LTRX: failed to create child task (RC=0x%08lx)",
                          (unsigned long)status);
    }
    return status;
}

void LTRX_ChildWake(void)
{
    if (OS_ObjectIdDefined(s_WakeSem))
    {
        (void)OS_BinSemGive(s_WakeSem);
    }
}

void LTRX_ChildRequestShutdown(void)
{
    s_ShutdownRequested = true;

    /* check shutdown  right after wake */
    LTRX_ChildWake();

    OS_TaskDelay(500);
}