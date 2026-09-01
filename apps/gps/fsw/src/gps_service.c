/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file  GPS receiver service: device ownership, receive task, position cache
 */
#include <string.h>

#include "gps_app.h"
#include "gps_service.h"
#include "gps_eventids.h"

#include "oem.h"
#include "oem_cb.h"
#include "msg/oem_msg_logs.h"
#include "io_drivers/oem_io_serial_linux.h"


typedef struct {
    const char*         Name;
    oem_ushort          Mid;
    oem_ushort          Mlen;   /**< header inclusive; 0 = variable */
    oem_log_callback_t  Callback;
} GPS_BuiltinLogCallbackEntry_t;

static GPS_BuiltinLogCallbackEntry_t BuiltinLogCallbacks[] = {
    {"BESTPOS",    OEM_ID_LOG_BESTPOS,    sizeof(oem_binary_header_t) + sizeof(oem_log_bestpos),
     oem_callback_BESTPOS},
    {"BESTXYZ",    OEM_ID_LOG_BESTXYZ,    sizeof(oem_binary_header_t) + sizeof(oem_log_bestxyz),
     oem_callback_BESTXYZ},
    {"TIME",       OEM_ID_LOG_TIME,       sizeof(oem_binary_header_t) + sizeof(oem_log_time),
     oem_callback_TIME},
    {"CLOCKMODEL", OEM_ID_LOG_CLOCKMODEL, sizeof(oem_binary_header_t) + sizeof(oem_log_clockmodel),
     oem_callback_CLOCKMODEL},
    {"RANGE",      OEM_ID_LOG_RANGE,      OEM_LOG_HANDLER_MLEN_VARIABLE, oem_callback_RANGE},
    {"HWMONITOR",  OEM_ID_LOG_HWMONITOR,  OEM_LOG_HANDLER_MLEN_VARIABLE, oem_callback_HWMONITOR},
    {"RXSTATUS",   OEM_ID_LOG_RXSTATUS,   OEM_LOG_HANDLER_MLEN_VARIABLE, oem_callback_RXSTATUS},
    {"SATVIS2",    OEM_ID_LOG_SATVIS2,    OEM_LOG_HANDLER_MLEN_VARIABLE, oem_callback_SATVIS2},
};

/**
 * @brief Register the stored-log handlers the mission always wants.
 *
 * @details Registration only prepares the receive path. Nothing arrives until
 *          the ground asks the receiver for the log.
 */
static void RegisterBuiltinLogs(void)
{
    unsigned int Done = 0;
    int          Ret;

    for (size_t i = 0; i < sizeof(BuiltinLogCallbacks) / sizeof(BuiltinLogCallbacks[0]); ++i) {
        Ret = oem_log_handler_register(BuiltinLogCallbacks[i].Name, BuiltinLogCallbacks[i].Mid, BuiltinLogCallbacks[i].Mlen);
        if (Ret != OEM_OK) {
            CFE_EVS_SendEvent(GPS_ADDCB_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: %s handler register failed, RC = %d", BuiltinLogCallbacks[i].Name, Ret);
            continue;
        }

        Ret = oem_log_add_callback(BuiltinLogCallbacks[i].Mid, BuiltinLogCallbacks[i].Callback);
        if (Ret != OEM_OK) {
            CFE_EVS_SendEvent(GPS_ADDCB_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: %s callback attach failed, RC = %d", BuiltinLogCallbacks[i].Name, Ret);
            oem_log_handler_unregister(BuiltinLogCallbacks[i].Mid);
            continue;
        }

        Ret = oem_log_handler_activate(BuiltinLogCallbacks[i].Mid);
        if (Ret != OEM_OK) {
            CFE_EVS_SendEvent(GPS_ADDCB_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: %s handler activate failed, RC = %d", BuiltinLogCallbacks[i].Name, Ret);
            continue;
        }

        Done++;
    }

    CFE_EVS_SendEvent(GPS_ADDCB_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "GPS: %u of %u stored-log handlers active", Done,
                      (unsigned int)(sizeof(BuiltinLogCallbacks) / sizeof(BuiltinLogCallbacks[0])));
}

static CFE_Status_t OpenInterface(void)
{
    int Ret = oem_io_init_interface(GPS_PLATFORM_IFACE_INDEX, GPS_PLATFORM_RX_BUF_SIZE,
                                    oem_io_driver_serial_write, oem_io_driver_serial_read);

    if (Ret != OEM_OK) {
        CFE_EVS_SendEvent(GPS_DEV_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: I/O interface init failed, RC = %d", Ret);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

static CFE_Status_t OpenDevice(void)
{
    int Ret;

    /* Must precede every other log call: it builds the handler mutex. */
    Ret = oem_log_init();
    if (Ret != OEM_OK) {
        CFE_EVS_SendEvent(GPS_DEV_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: log layer init failed, RC = %d", Ret);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    Ret = oem_io_driver_serial_init(GPS_PLATFORM_IFACE_INDEX, GPS_PLATFORM_SERIAL_DEV,
                                    GPS_PLATFORM_SERIAL_BAUD);
    if (Ret != OEM_OK) {
        CFE_EVS_SendEvent(GPS_DEV_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: %s open failed, RC = %d", GPS_PLATFORM_SERIAL_DEV, Ret);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (OpenInterface() != CFE_SUCCESS) {
        oem_io_driver_serial_close(GPS_PLATFORM_IFACE_INDEX);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    RegisterBuiltinLogs();

    CFE_EVS_SendEvent(GPS_DEV_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "GPS: %s open at %u baud", GPS_PLATFORM_SERIAL_DEV,
                      (unsigned int)GPS_PLATFORM_SERIAL_BAUD);

    return CFE_SUCCESS;
}

/**
 * Receive-path state. The receive task is the only writer; the main thread
 * reads it while building housekeeping and clears it on a reset command, so
 * every access goes through StatsLock.
 */
typedef struct {
    uint16 LogCount;
    uint16 ReadErrCount;
    uint16 StrayLogCount;
    uint16 LogErrCount;
    uint16 CallbackErrCount;
    uint16 HandlerCritCount;
    uint16 ResponseCount;
} GPS_RxStats_t;

static GPS_RxStats_t RxStats;

/* Zero-initialized, which is exactly OS_OBJECT_ID_UNDEFINED; that macro is a
 * compound literal and cannot initialize a static. */
static osal_id_t     StatsLock;

/** Not on the wire; used only to emit one event per distinct fault. */
static int LastError;

/* Cleared by the main thread to stop the task, and by the task once it has
 * left the driver for good. Neither is a handshake the compiler may fold
 * away. */
static volatile bool RxTaskRun;
static volatile bool RxTaskAlive;

static void StatsTake(void)
{
    if (OS_ObjectIdDefined(StatsLock))
        OS_MutSemTake(StatsLock);
}

static void StatsGive(void)
{
    if (OS_ObjectIdDefined(StatsLock))
        OS_MutSemGive(StatsLock);
}

void GPS_ServiceGetCounters(GPS_HkTlm_Payload_t* Hk)
{
    if (Hk == NULL)
        return;

    StatsTake();
    Hk->LogCount         = RxStats.LogCount;
    Hk->ReadErrCount     = RxStats.ReadErrCount;
    Hk->StrayLogCount    = RxStats.StrayLogCount;
    Hk->LogErrCount      = RxStats.LogErrCount;
    Hk->CallbackErrCount = RxStats.CallbackErrCount;
    Hk->HandlerCritCount = RxStats.HandlerCritCount;
    Hk->ResponseCount    = RxStats.ResponseCount;
    StatsGive();
}

void GPS_ServiceResetCounters(void)
{
    StatsTake();
    RxStats.LogCount         = 0;
    RxStats.ReadErrCount     = 0;
    RxStats.StrayLogCount    = 0;
    RxStats.LogErrCount      = 0;
    RxStats.CallbackErrCount = 0;
    RxStats.HandlerCritCount = 0;
    RxStats.ResponseCount    = 0;
    StatsGive();
}

/**
 * Bucket the outcome by the layer that produced it. oem_task_context_t
 * already carries that in task_level, so nothing has to be inferred from the
 * return code alone.
 */
static void NoteOutcome(int Ret, const oem_task_context_t* Ctx)
{
    StatsTake();

    switch (Ctx->task_level) {
    case TASK_MAIN:
        /* Never reached a handler: read, sync or framing failure. */
        RxStats.ReadErrCount++;
        break;

    case TASK_RESPONSE:
        RxStats.ResponseCount++;
        break;

    case TASK_LOG:
        if (Ret == OEM_ERR_LOG_STRAY)
            RxStats.StrayLogCount++;
        else if (Ret == OEM_ERR_NOBUF)
            RxStats.HandlerCritCount++;
        else if (Ret != OEM_OK)
            RxStats.LogErrCount++;
        else
            RxStats.LogCount++;
        break;

    case TASK_CALLBACK:
        if (Ret == OEM_ERR_NOT_FOUND || Ret == OEM_ERR_UTILS_LIST_NULL)
            RxStats.HandlerCritCount++;
        else if (Ret != OEM_OK)
            RxStats.CallbackErrCount++;
        else
            RxStats.LogCount++;
        break;

    default:
        RxStats.ReadErrCount++;
        break;
    }

    StatsGive();
}

static void RxTask(void)
{
    oem_task_context_t Ctx;
    int                Ret;

    RxTaskAlive = true;

    /* The read window is also the shutdown latency: the task cannot be
     * cancelled while it blocks inside the driver, so it must come back. */
    while (RxTaskRun)
    {
        Ret = oem_task_read_single_reply(GPS_PLATFORM_IFACE_INDEX,
                                         GPS_PLATFORM_RX_TIMEOUT_MS, &Ctx);

        /* No traffic in the window is the idle case, not a fault. */
        if (Ret == OEM_ERR_IO_TIMEOUT)
            continue;

        NoteOutcome(Ret, &Ctx);

        if (Ret == OEM_OK) {
            LastError = OEM_OK;
            continue;
        }

        /* One event per distinct code: EVS carries no filters for this app. */
        if (Ret != LastError) {
            LastError = Ret;
            CFE_EVS_SendEvent(GPS_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: receive failed at level %u, RC = %d, MID %u",
                              (unsigned int)Ctx.task_level, Ret,
                              (unsigned int)Ctx.message_id);
        }

        OS_TaskDelay(GPS_PLATFORM_RX_BACKOFF_MS);
    }

    /* Last thing the task touches: it is what releases the device. */
    RxTaskAlive = false;

    CFE_ES_ExitChildTask();
}

CFE_Status_t GPS_ServiceInit(void)
{
    CFE_Status_t Status;
    int32        OsStatus;

    LastError   = OEM_OK;
    RxTaskRun   = true;
    RxTaskAlive = false;

    if (!OS_ObjectIdDefined(StatsLock)) {
        OsStatus = OS_MutSemCreate(&StatsLock, "GPS_RXSTAT", 0);
        if (OsStatus != OS_SUCCESS) {
            CFE_EVS_SendEvent(GPS_DEV_ERR_EID, CFE_EVS_EventType_ERROR,
                              "GPS: counter mutex create failed, RC = %ld", (long)OsStatus);
            StatsLock = OS_OBJECT_ID_UNDEFINED;
            RxTaskRun = false;
            return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }
    }

    GPS_ServiceResetCounters();

    Status = OpenDevice();
    if (Status != CFE_SUCCESS) {
        RxTaskRun = false;
        return Status;
    }

    Status = CFE_ES_CreateChildTask(&GPS_AppData.RxTaskId, GPS_PLATFORM_RX_TASK_NAME,
                                    RxTask, CFE_ES_TASK_STACK_ALLOCATE,
                                    GPS_PLATFORM_RX_STACK_SIZE, GPS_PLATFORM_RX_PRIORITY, 0);
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(GPS_RX_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: receive task create failed, RC = 0x%08lX", (unsigned long)Status);
        GPS_ServiceShutdown();
        return Status;
    }

    return CFE_SUCCESS;
}

void GPS_ServiceShutdown(void)
{
    unsigned int Waited;

    /* The task may be parked inside the driver for a whole read window, and
     * it reads straight out of the interface buffer. Freeing that buffer
     * under it would be a use-after-free, so wait for it to come back. */
    RxTaskRun = false;
    for (Waited = 0; RxTaskAlive && Waited < GPS_PLATFORM_RX_JOIN_MS;
         Waited += GPS_PLATFORM_RX_JOIN_POLL_MS) {
        OS_TaskDelay(GPS_PLATFORM_RX_JOIN_POLL_MS);
    }

    if (RxTaskAlive) {
        /* Leaking the interface is the lesser fault: the task is still
         * reading from it. */
        CFE_EVS_SendEvent(GPS_DEV_ERR_EID, CFE_EVS_EventType_ERROR,
                          "GPS: receive task still running after %u ms, device left open",
                          (unsigned int)GPS_PLATFORM_RX_JOIN_MS);
        return;
    }

    oem_io_delete_interface(GPS_PLATFORM_IFACE_INDEX);
    oem_io_driver_serial_close(GPS_PLATFORM_IFACE_INDEX);
}
