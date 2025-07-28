#ifndef RPT_INTERFACE_CFG_H
#define RPT_INTERFACE_CFG_H

#include "common_types.h"
#include "cfe.h"

/**
 * Queue Length
 */
#define RPT_REPORT_QUEUE_LEN    10
#define RPT_CRITICAL_QUEUE_LEN  40

/**
 * Report Buffer Size
 */
#define RPT_RET_VALUE_BUF_SIZE  60


/**
 * RPT Subscribe Table Entry size
 */
#define RPT_MAX_TBL_ENTRY       30

typedef struct RPT_Report {
    uint16 MsgID;
    uint8 CommandCode;

    /**
     * `enum RPT_ReturnType_t`
     */
    uint8 ReturnType;
    int32 ReturnCode;

    /**
     * This member can be used to 2 types
     * 1. Get CMD -> earned value
     * 2. HW's Error packet -> HW error value
     */
    uint16 ReturnDataSize;
    uint8 ReturnValue[RPT_RET_VALUE_BUF_SIZE];
} RPT_Report_t;

typedef struct RPT_Critical {
    RPT_Report_t Report;
    CFE_TIME_SysTime_t Time;
} RPT_Critical_t;


/********************************
 * Return Type definition
 ********************************/
typedef enum {
    RPT_RETCODE_SUCCESS,
    /**
     * `osapi-error.h`
     */
    RPT_RETCODE_OSAL,

    /**
     * `cfe_error.h`
     */
    RPT_RETCODE_CFE,
    RPT_RETCODE_LIB,
    RPT_RETCODE_APP,
    RPT_RETCODE_APPUTIL,
    RPT_RETCODE_KNL, /* Kernel layer */

    /**
     * Use this if HW send error packet
     */
    RPT_RETCODE_HW,

} RPT_ReturnType_t;
#endif
