

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "oem.h"
#include "arch/oem_arch.h"
#include "oem_log_cb.h"

typedef struct {
    uint32_t taskId;
    uint32_t readErrorCount;
    uint32_t strayLogCount;
    uint32_t responseCount;
    uint32_t responseErrorCount;
    uint32_t lastResponseEnum;
    uint16_t lastResponseMessageId;
} GRX_DeviceData_t;

static GRX_DeviceData_t DeviceData;

void* read_task(void* args)
{
    oem_task_context_t ctx;
    int ret;

    (void) args;

    printf("test read task created\n");

    while (1) {
        ret = OEM_Task_ReadTaskSingleRun(0, &ctx);

        switch (ctx.taskLevel) {
        case TASK_MAIN:
            /**
             * Returning from the main level indicates a read error.
             */
            DeviceData.readErrorCount++;
            break;

        case TASK_RESPONSE:
            DeviceData.responseCount++;
            DeviceData.lastResponseMessageId = ctx.mid;
            DeviceData.lastResponseEnum = ctx.respId;
            if (ret == OEM_OK) {
                printf("oem response received: MID %d, RID %d\n",
                        ctx.mid,
                        ctx.respId);
            }
            else {
                printf("  Errornous response received: ret %d, RID %d\n",
                        ret, ctx.respId);
                DeviceData.responseErrorCount++;
            }
            break;

        case TASK_LOG:
            if (ret == OEM_ERR_NOTFOUND) {
                /**
                 * We don't know what this log is.
                 */
                printf("  Stray log found: MID %d.\n",
                        ctx.mid);
                DeviceData.strayLogCount++;
            }
            else if (ret == OEM_ERR_NOBUF) {
                /**
                 * This is the only case where the handler goes broken.
                 */
                printf("  No msg buf for mid %d: handler marked broken.\n",
                       ctx.mid);
            }
            else if (ret != OEM_OK) {
                printf("  Log handler returned with %d: MID %d.\n",
                       ret, ctx.mid);
            }
            break;

        case TASK_CALLBACK:
            if (ret == OEM_ERR_NOTFOUND) {
                /**
                 * This is the only case where the handler goes broken.
                 */
                printf("  Null callback for mid %d: handler marked broken.\n",
                       ctx.mid);
            }
            else if (ret != OEM_OK) {
                printf("  Callback returned with %d: MID %d.\n",
                       ret, ctx.mid);
            }
            break;

        default:
            printf("  invalid tasklv %d from MID %d (RID: %d).\n",
                   ctx.taskLevel,
                   ctx.mid,
                   ctx.isResponse);
            break;
        }
    }

    /* Should never reach here. */
    return NULL;
}

void task_init(void)
{
    int ret;
    pthread_t tid;

    OEM_Log_HandlerInit();
    OEM_IO_SerialInit();
    ret = OEM_IO_PortInit(0, OEM_IO_WriteCallback, OEM_IO_ReadCallback);
    if (ret != OEM_OK) {
        printf("failed to initialize serial ports: returned %d\n",
                ret);
        exit(1);
    }

    ret = OEM_Log_RegisterHandler("VERSION", OEM_ID_LOG_VERSION, 0);
    if (ret != OEM_OK) {
        printf("OEM_Log_RegisterHandler error: returned %d\n",
                ret);
        exit(1);
    }

    ret = OEM_Log_AddCallback(OEM_ID_LOG_VERSION, OEM_Log_Callback_VERSION);
    if (ret != OEM_OK) {
        printf("OEM_Log_AddCallback error: returned %d\n",
                ret);
        exit(1);
    }

    ret = OEM_Log_RegisterHandler("LOGLIST", OEM_ID_LOG_LOGLIST, 0);
    if (ret != OEM_OK) {
        printf("OEM_Log_RegisterHandler error: returned %d\n",
                ret);
        exit(1);
    }

    ret = OEM_Log_RegisterHandler("BESTXYZ", OEM_ID_LOG_BESTXYZ, 0);
    if (ret != OEM_OK) {
        printf("OEM_Log_RegisterHandler error: returned %d\n",
                ret);
        exit(1);
    }

    ret = OEM_Log_AddCallback(OEM_ID_LOG_LOGLIST, OEM_Log_Callback_LOGLIST);
    if (ret != OEM_OK) {
        printf("OEM_Log_AddCallback error: returned %d\n",
                ret);
        exit(1);
    }

    ret = OEM_Log_RegisterHandler("HWMONITOR", OEM_ID_LOG_HWMONITOR, 0);
    if (ret != OEM_OK) {
        printf("OEM_Log_RegisterHandler error: returned %d\n",
                ret);
        exit(1);
    }

    ret = OEM_Log_AddCallback(OEM_ID_LOG_HWMONITOR, OEM_Log_Callback_HWMONITOR);
    if (ret != OEM_OK) {
        printf("OEM_Log_AddCallback error: returned %d\n",
                ret);
        exit(1);
    }

    OEM_Log_HandlerActivateAll();

    // OEM_Log_DisableCsVerification(OEM_ID_LOG_VERSION)
    // OEM_Log_DisableCsVerification(OEM_ID_LOG_LOGLIST);

    pthread_create(&tid, NULL, read_task, NULL);
}

int main(void)
{
    task_init();
    printf("initialization complete\n");

    OEM_Cmd_UNLOGALL(0, OEM_PORT_COM1, true);

    sleep(2);
    printf("sending VERSION request...\n");
    OEM_Cmd_LogOnce(0, OEM_ID_LOG_VERSION, OEM_PORT_THIS);
    OEM_Cmd_LogOnTime(0, OEM_ID_LOG_HWMONITOR, OEM_PORT_THIS, 2, 0.5);
    OEM_Cmd_LogOnce(0, OEM_ID_LOG_VERSION, OEM_PORT_THIS);

    OEM_Cmd_LogOnTime(0, OEM_ID_LOG_BESTXYZ, OEM_PORT_THIS, 1, 0.2);

    sleep(1);
    OEM_Cmd_LogOnce(0, OEM_ID_LOG_LOGLIST, OEM_PORT_THIS);

    sleep(1);
    OEM_Cmd_LogOnce(0, OEM_ID_LOG_HWMONITOR, OEM_PORT_THIS);
    OEM_Cmd_LogOnce(0, OEM_ID_LOG_HWMONITOR, OEM_PORT_THIS);

    while (1)
        sleep(100);
    return 0;
}
