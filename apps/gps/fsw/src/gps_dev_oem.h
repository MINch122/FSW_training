#ifndef _GPS_DEV_OEM_H_
#define _GPS_DEV_OEM_H_

#include "gps_app.h"

#define GPS_PORT_INDEX_COM1     0

#define GPS_DEV_SUCCESS                0
#define GPS_DEV_ERR_NULL              -1
#define GPS_DEV_ERR_MODULE_LOAD       -2
#define GPS_DEV_ERR_SYMBOL_LOAD       -3

/**
 * @brief Log receiving task statistics.
 */
typedef struct {
    uint16 readErrorCount;
    uint16 handlerCritErrCount;
    uint16 callbackErrCount;
    uint16 strayLogCount;
    uint16 responseCount;
    uint16 responseErrorCount;
    uint16 lastResponseEnum;
    uint16 lastResponseMessageId;
} GPS_DeviceData_Counters_t;

/**
 * @brief Initializes OEM7 driver. Call this once and once only at the App
 *        initialization stage.
 *        1) Opens and attaches serial writer/readr.
 *        2) Initialzes the log handler (mutex create).
 *        3) Registers default handlers and callbacks.
 *        4) Creates the log receiving task. 
 * 
 * @return CFE_SUCCESS if OK.
 *         Otherwise oem_ret_t or CFE retcode if CFE_ES_CreateChildTask fails.
 */
int GPS_Device_Init(void);

/**
 * @brief Clears the device log counters.
 */
void GPS_Device_ClearCounters(void);

/**
 * @brief Copies the device log counters.
 */
void GPS_Device_GetCounters(GPS_DeviceData_Counters_t* hk);

/**
 * @brief Loads the dynamically linkable file named @a filename and stores
 *        the function @a functionName to @a pfunc.
 *        Issues an Event Message upon failure.
 *
 * @param filename      The dynamic file to load. If NULL, the symbol will be
 *                      searched within the main program, in which case the 
 *                      search also covers other libraries or modules that have
 *                      already been loaded, either implicitly or explicitly.
 *                      This includes other cFS Applications or Libraries
 *                      loaded by the Executable Service.
 * @param functionName  Function name.
 * @param[out] pfunc    Pointer to a function pointer for the symbol to be stored.
 * @param options       Options for dlopen().
 * @return  GPS_UTILS_SUCCESS: Success.
 *          GPS_UTILS_ERR_NULL: Null @a functionName.
 *          GPS_UTILS_ERR_MODULE_LOAD: dlopen() failed.
 *          GPS_UTILS_ERR_SYMBOL_LOAD: dlsym() failed.
 */
int GPS_Device_LoadFunctionSymbol(const char* filename,
                                  const char* functionName,
                                  int (**pfunc)(void*),
                                  int options,
                                  char* err);

/**
 * @brief OEM7 log receiving task.
 *        Launched as a child task by GPS_Device_Init().
 */
void GPS_Device_Task(void);

#endif
