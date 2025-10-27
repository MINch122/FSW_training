#include "gps_dev_oem.h"
#include "gps_eventids.h"
#include "oem.h"
#include "oem_utils.h"
#include "oem_log_cb.h"

#include "arch/oem_arch.h"

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
     .callback = OEM_Log_Callback_VERSION
    },

    {.name = "BESTXYZ",
     .msgId = OEM_ID_LOG_BESTXYZ,
     .msgLen = OEM_LOG_HANDLER_MLEN_VARIABLE,
     .callback = NULL
    },

    {.name = "HWMONITOR",
     .msgId = OEM_ID_LOG_HWMONITOR,
     .msgLen = OEM_LOG_HANDLER_MLEN_VARIABLE,
     .callback = OEM_Log_Callback_HWMONITOR
    }
};

typedef struct {
    CFE_ES_TaskId_t taskId;
    GRX_DeviceData_Counters_t counters;
} GRX_DeviceData_t;

static GRX_DeviceData_t DeviceData;

void GPS_Device_GetCounters(GRX_DeviceData_Counters_t* hk)
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
        ret = OEM_Task_ReadTaskSingleRun(GPS_PORT_INDEX_COM1, &ctx);

        switch (ctx.taskLevel) {
        case TASK_MAIN:
            /**
             * Returning from the main level indicates a read error.
             */
            DeviceData.counters.readErrorCount++;
            break;

        case TASK_RESPONSE:
            DeviceData.counters.responseCount++;
            DeviceData.counters.lastResponseMessageId = ctx.mid;
            DeviceData.counters.lastResponseEnum = ctx.respId;
            if (ret != OEM_OK) {
                DeviceData.counters.responseErrorCount++;
                DebugWarning("Errornous response received: ret %d, RID %d\n",
                             ctx.mid,
                             ctx.respId);
            }
            break;

        case TASK_LOG:
            if (ret == OEM_ERR_NOTFOUND) {
                /**
                 * We don't know what this log is.
                 */
                DebugWarning("Stray log found: MID %d\n",
                             ctx.mid);
                DeviceData.counters.strayLogCount++;
            }
            if (ret == OEM_ERR_NOBUF) {
                /**
                 * This is the only case where the handler goes broken.
                 */
                DeviceData.counters.handlerCritErrCount++;
                DebugError("No handler buffer for MID %d: handler marked broken.\n",
                           ctx.mid);
                
            }
            break;

        case TASK_CALLBACK:
            if (ret == OEM_ERR_NOTFOUND) {
                /**
                 * This is the only case where the handler goes broken.
                 */
                DeviceData.counters.handlerCritErrCount++;
                DebugError("Null callback for MID %d: handler marked broken.\n",
                           ctx.mid);
            }
            else if (ret != OEM_OK) {
                DeviceData.counters.callbackErrCount++;
                DebugWarning("Callback for MID %d returned with %d.\n",
                             ctx.mid, ret);
            }
            break;

        default:
            DebugError("invalid tasklv %d from mid %d (isrsp: %d).\n",
                       ctx.taskLevel,
                       ctx.mid,
                       ctx.isResponse);
            break;
        }
    }

    /* Should never reach here. */
    DebugError("Unexpected driver task termination.\n");
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
    OEM_Log_HandlerInit();

    status = OEM_IO_SerialInit();
    if (status != OEM_OK) {

    }

    status = OEM_IO_PortInit(GPS_PORT_INDEX_COM1,
                             OEM_IO_WriteCallback,
                             OEM_IO_ReadCallback);
    if (status != OEM_OK) {
        CFE_EVS_SendEvent(GPS_DEV_HANDLER_INIT_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "GPS: handler init err");
        return status;
    }

    /**
     * Register default log handlers.
     */
    for (int i = 0; i < sizeof(defaultHandlers)/sizeof(defaultHandlers[0]); ++i) {
        const GPS_Device_HandlerEntry_t* entry = &defaultHandlers[i];
        status = OEM_Log_RegisterHandler(entry->name, entry->msgId, entry->msgLen);
        if (status != OEM_OK) {
            CFE_EVS_SendEvent(GPS_DEV_HANDLER_INIT_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                             "GPS: Handler register err: MID %d", entry->msgId);
            return status;
        }
        if (entry->callback) {
            status = OEM_Log_AddCallback(entry->msgId, entry->callback);
            if (status != OEM_OK) {
                CFE_EVS_SendEvent(GPS_DEV_HANDLER_INIT_ERR_EID,
                                CFE_EVS_EventType_ERROR,
                                "GPS: Handler callback add err: MID %d", entry->msgId);
                return status;
            }
        }
        OEM_Log_HandlerAcivate(entry->msgId);
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
