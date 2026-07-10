

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "oem.h"
#include "io_drivers/oem_io_serial_linux.h"
#include "oem_cb.h"

typedef struct {
    uint32_t taskId;
    uint32_t readErrorCount;
    uint32_t strayLogCount;
    uint32_t responseCount;
    uint32_t responseErrorCount;
    uint32_t lastResponseEnum;
    uint16_t lastResponseMessageId;
} GPS_DeviceData_t;

static GPS_DeviceData_t DeviceData;

void* read_task(void* args)
{
    oem_task_context_t ctx;
    int ret;

    (void) args;

    printf("test read task created\n");

    while (1) {
        ret = oem_task_read_single_reply(0, 1000, &ctx);

        switch (ctx.task_level) {
        case TASK_MAIN:
            /**
             * Returning from the main level indicates a read error.
             */
            DeviceData.readErrorCount++;
            break;

        case TASK_RESPONSE:
            DeviceData.responseCount++;
            DeviceData.lastResponseMessageId = ctx.message_id;
            DeviceData.lastResponseEnum = ctx.response_id;
            if (ret == OEM_OK) {
                printf("oem response received: MID %d, RID %d\n",
                        ctx.message_id,
                        ctx.response_id);
            }
            else {
                printf("  Erroneous response received: ret %d, RID %d\n",
                        ret, ctx.response_id);
                DeviceData.responseErrorCount++;
            }
            break;

        case TASK_LOG:
            if (ret == OEM_ERR_NOT_FOUND) {
                /**
                 * We don't know what this log is.
                 */
                printf("  Stray log found: MID %d.\n",
                        ctx.message_id);
                DeviceData.strayLogCount++;
            }
            else if (ret == OEM_ERR_NOBUF) {
                /**
                 * This is the only case where the handler goes broken.
                 */
                printf("  No msg buf for mid %d: handler marked broken.\n",
                       ctx.message_id);
            }
            else if (ret != OEM_OK) {
                printf("  Log handler returned with %d: MID %d.\n",
                       ret, ctx.message_id);
            }
            break;

        case TASK_CALLBACK:
            if (ret == OEM_ERR_NOT_FOUND) {
                /**
                 * This is the only case where the handler goes broken.
                 */
                printf("  Null callback for mid %d: handler marked broken.\n",
                       ctx.message_id);
            }
            else if (ret != OEM_OK) {
                printf("  Callback returned with %d: MID %d.\n",
                       ret, ctx.message_id);
            }
            break;

        default:
            printf("  invalid tasklv %d from MID %d (RID: %d).\n",
                   ctx.task_level,
                   ctx.message_id,
                   ctx.response_id);
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

    oem_log_init();
    oem_io_driver_serial_init(0, "/dev/ttyS0", 115200);
    ret = oem_io_init_interface(0, oem_io_driver_serial_write, oem_io_driver_serial_read);
    if (ret != OEM_OK) {
        printf("failed to initialize serial ports: returned %d\n",
                ret);
        exit(1);
    }

    ret = oem_log_handler_register("VERSION", OEM_ID_LOG_VERSION, 0);
    if (ret != OEM_OK) {
        printf("oem_log_handler_register error: returned %d\n",
                ret);
        exit(1);
    }

    ret = oem_log_add_callback(OEM_ID_LOG_VERSION, oem_callback_VERSION_print);
    if (ret != OEM_OK) {
        printf("oem_log_add_callback error: returned %d\n",
                ret);
        exit(1);
    }

    ret = oem_log_handler_register("LOGLIST", OEM_ID_LOG_LOGLIST, 0);
    if (ret != OEM_OK) {
        printf("oem_log_handler_register error: returned %d\n",
                ret);
        exit(1);
    }

    ret = oem_log_handler_register("BESTXYZ", OEM_ID_LOG_BESTXYZ, 0);
    if (ret != OEM_OK) {
        printf("oem_log_handler_register error: returned %d\n",
                ret);
        exit(1);
    }

    ret = oem_log_handler_register("BESTXYZ", OEM_ID_LOG_BESTXYZ, 0);
    if (ret != OEM_OK) {
        printf("oem_log_handler_register error: returned %d\n",
                ret);
        exit(1);
    }

    ret = oem_log_handler_register("BESTXYZ", OEM_ID_LOG_BESTXYZ, 0);
    if (ret != OEM_OK) {
        printf("oem_log_handler_register error: returned %d\n",
                ret);
        exit(1);
    }

    ret = oem_log_handler_register("HWMONITOR", OEM_ID_LOG_HWMONITOR, 0);
    if (ret != OEM_OK) {
        printf("oem_log_handler_register error: returned %d\n",
                ret);
        exit(1);
    }

    ret = oem_log_add_callback(OEM_ID_LOG_HWMONITOR, oem_callback_HWMONITOR_print);
    if (ret != OEM_OK) {
        printf("oem_log_add_callback error: returned %d\n",
                ret);
        exit(1);
    }

    oem_log_handler_activate_all();

    // oem_log_disable_cs_verification(OEM_ID_LOG_VERSION)
    // oem_log_disable_cs_verification(OEM_ID_LOG_LOGLIST);

    pthread_create(&tid, NULL, read_task, NULL);
}

int main(void)
{
    task_init();
    printf("initialization complete\n");

    oem_cmd_UNLOGALL(0, OEM_PORT_COM1, true);

    sleep(2);
    printf("sending VERSION request...\n");
    oem_cmd_LOG_once(0, OEM_ID_LOG_VERSION, OEM_PORT_THIS);
    oem_cmd_LOG_ontime(0, OEM_ID_LOG_HWMONITOR, OEM_PORT_THIS, 2, 0.5);
    oem_cmd_LOG_once(0, OEM_ID_LOG_VERSION, OEM_PORT_THIS);

    oem_cmd_LOG_ontime(0, OEM_ID_LOG_BESTXYZ, OEM_PORT_THIS, 1, 0.2);

    sleep(1);
    oem_cmd_LOG_once(0, OEM_ID_LOG_LOGLIST, OEM_PORT_THIS);

    sleep(1);
    oem_cmd_LOG_once(0, OEM_ID_LOG_HWMONITOR, OEM_PORT_THIS);
    oem_cmd_LOG_once(0, OEM_ID_LOG_HWMONITOR, OEM_PORT_THIS);

    while (1)
        sleep(100);
    return 0;
}
