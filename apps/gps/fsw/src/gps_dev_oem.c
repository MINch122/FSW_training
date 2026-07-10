#include "gps_dev_oem.h"
#include "gps_eventids.h"
#include "oem.h"
#include "oem_utils.h"
#include "oem_cb.h"

#include "io_drivers/oem_io_serial_linux.h"

#include <dlfcn.h>

typedef struct {
    const char* name;
    uint16 msgId;
    uint16 msgLen;
    oem_log_callback_t callback;
} GPS_Device_HandlerEntry_t;

static const GPS_Device_HandlerEntry_t defaultHandlers[] =
{
    {.name = "VERSION",
     .msgId = OEM_ID_LOG_VERSION,
     .msgLen = OEM_LOG_HANDLER_MLEN_VARIABLE,
     .callback = oem_callback_VERSION_print
    },

    {.name = "BESTXYZ",
     .msgId = OEM_ID_LOG_BESTXYZ,
     .msgLen = OEM_LOG_HANDLER_MLEN_VARIABLE,
     .callback = oem_callback_BESTXYZ_binfile
    }
};

typedef struct {
    CFE_ES_TaskId_t taskId;
    GPS_DeviceData_Counters_t counters;
} GPS_DeviceData_t;

static GPS_DeviceData_t DeviceData;

void GPS_Device_GetCounters(GPS_DeviceData_Counters_t* hk)
{
    if (hk)
        memcpy(hk, &DeviceData.counters, sizeof(*hk));
}

void GPS_Device_ClearCounters(void)
{
    DeviceData.counters.readErrorCount = 0;
    DeviceData.counters.handlerCritErrCount = 0;
    DeviceData.counters.callbackErrCount = 0;
    DeviceData.counters.strayLogCount = 0;
    DeviceData.counters.responseCount = 0;
    DeviceData.counters.responseErrorCount = 0;
    DeviceData.counters.lastResponseEnum = 0;
    DeviceData.counters.lastResponseMessageId = 0;
}

void GPS_Device_Task(void)
{
    oem_task_context_t ctx;
    int ret;

    CFE_EVS_SendEvent(GPS_DEV_TASK_INIT_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "OEM driver task initialized.");

    while (1) {
        ret = oem_task_read_single_reply(GPS_PORT_INDEX_COM1, 1000, &ctx);

        switch (ctx.task_level) {
        case TASK_MAIN:
            /**
             * Returning from the main level indicates a read error.
             */
            DeviceData.counters.readErrorCount++;
            if (ret == OEM_ERR_IO_IFACE_INDEX)
                return;
            break;

        case TASK_RESPONSE:
            /* The current response handler does not define response-layer errors. */
            DeviceData.counters.responseCount++;
            DeviceData.counters.lastResponseMessageId = ctx.message_id;
            DeviceData.counters.lastResponseEnum = ctx.response_id;
            if (ret != OEM_OK) {
                DeviceData.counters.responseErrorCount++;
                oem_debug_warning("Erroneous response received: ret %d, RID %d\n",
                                  ctx.message_id,
                                  ctx.response_id);
            }
            break;

        case TASK_LOG:
            if (ret == OEM_ERR_NOT_FOUND) {
                /**
                 * We don't know what this log is.
                 */
                oem_debug_warning("Stray log found: MID %d\n",
                                  ctx.message_id);
                DeviceData.counters.strayLogCount++;
            }
            if (ret == OEM_ERR_NOBUF) {
                /**
                 * This is the only case where the handler goes broken.
                 */
                DeviceData.counters.handlerCritErrCount++;
                oem_debug_error("No handler buffer for MID %d: handler marked broken.\n",
                                ctx.message_id);
                
            }
            break;

        case TASK_CALLBACK:
            if (ret == OEM_ERR_NOT_FOUND) {
                /**
                 * This is the only case where the handler goes broken.
                 */
                DeviceData.counters.handlerCritErrCount++;
                oem_debug_error("Null callback for MID %d: handler marked broken.\n",
                                ctx.message_id);
            }
            else if (ret != OEM_OK) {
                DeviceData.counters.callbackErrCount++;
                oem_debug_warning("Callback for MID %d returned with %d.\n",
                             ctx.message_id, ret);
            }
            break;

        default:
            oem_debug_error("invalid tasklv %d from mid %d (isrsp: %d).\n",
                            ctx.task_level,
                            ctx.message_id,
                            ctx.is_response);
            break;
        }
    }

    /* Should never reach here. */
    oem_debug_error("Unexpected driver task termination.\n");
}

int GPS_Device_Init(void)
{
    int status;

    /**
     * Clear the device data counter fields.
     */
    GPS_Device_ClearCounters();

    /**
     * Log handler mutex init.
     */
    oem_log_init();

    status = oem_io_driver_serial_init(GPS_PORT_INDEX_COM1,
                                      "/dev/ttyS4",
                                      115200);

    status = oem_io_init_interface(GPS_PORT_INDEX_COM1,
                                   0, /* use OEM_IO_DEFAULT_READBUF_SIZE */
                                   oem_io_driver_serial_write,
                                   oem_io_driver_serial_read);
    if (status != OEM_OK) {
        CFE_EVS_SendEvent(GPS_DEV_HANDLER_INIT_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "GPS: handler init err");
        return status;
    }

    oem_cmd_UNLOGALL(GPS_PORT_INDEX_COM1, OEM_PORT_ALL_PORTS, true);
    OS_TaskDelay(500);
    oem_cmd_UNLOGALL(GPS_PORT_INDEX_COM1, OEM_PORT_THIS, true);

    /**
     * Register default log handlers.
     */
    for (int i = 0; i < sizeof(defaultHandlers)/sizeof(defaultHandlers[0]); ++i) {
        const GPS_Device_HandlerEntry_t* entry = &defaultHandlers[i];
        status = oem_log_handler_register(entry->name, entry->msgId, entry->msgLen);
        if (status != OEM_OK) {
            CFE_EVS_SendEvent(GPS_DEV_HANDLER_INIT_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                             "GPS: Handler register err: MID %d", entry->msgId);
            return status;
        }
        if (entry->callback) {
            status = oem_log_add_callback(entry->msgId, entry->callback);
            if (status != OEM_OK) {
                CFE_EVS_SendEvent(GPS_DEV_HANDLER_INIT_ERR_EID,
                                CFE_EVS_EventType_ERROR,
                                "GPS: Handler callback add err: MID %d", entry->msgId);
                return status;
            }
        }
        oem_log_handler_activate(entry->msgId);
    }

    /**
     * Create log receiving thread.
     */
    status = CFE_ES_CreateChildTask(&DeviceData.taskId,
                                    "OEM_TASK",
                                    GPS_Device_Task,
                                    NULL,
                                    GPS_DEVICE_TASK_STACK_SIZE,
                                    GPS_DEVICE_TASK_PRIORITY,
                                    0);
    if (status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(GPS_DRIVER_TASK_INIT_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "GPS: driver task init error: %X",
                          status);
    }

    return status;
}

int GPS_Device_LoadFunctionSymbol(const char* filename,
                                  const char* functionName,
                                  int (**pfunc)(void*),
                                  int options,
                                  char* err) {
    void* dlhandle;
    void* symbol;
    char* msg;
    size_t msglen;

    if (!functionName) {
        CFE_EVS_SendEvent(GPS_DEV_DL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: dl load: null functionName");
        return GPS_DEV_ERR_NULL;
    }

    dlerror();
    dlhandle = dlopen(filename, options);
    if (dlhandle == NULL) {
        msg = dlerror();
        msglen = strlen(msg);
        CFE_EVS_SendEvent(GPS_DEV_DL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: dl load: %s", msg);
        if (err)
            memcpy(err, msg, msglen > 64 ? 64 : msglen);  // TODO; fix this hard-coded strlen.
        return GPS_DEV_ERR_MODULE_LOAD;
    }

    dlerror();
    symbol = dlsym(dlhandle, functionName);
    if (symbol == NULL) {
        msg = dlerror();
        msglen = strlen(msg);
        CFE_EVS_SendEvent(GPS_DEV_DL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: sym load: %s", msg);
        if (err)
            memcpy(err, msg, msglen > 64 ? 64 : msglen);  // TODO; fix this hard-coded strlen.
        dlclose(dlhandle);
        return GPS_DEV_ERR_SYMBOL_LOAD;
    }

    *pfunc = *((oem_log_callback_t*) &symbol);

    dlclose(dlhandle);
    return GPS_DEV_SUCCESS;
}
