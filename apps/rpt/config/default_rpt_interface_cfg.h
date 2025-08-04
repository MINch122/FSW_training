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
     * If cmd acquires **any kind of data**, use this member.
     * Ex 1) Get CMD -> earned values 
     *      (In this case, `ReturnType` must be `RPT_RETTYPE_SUCCESS`)
     * 
     * Ex 2) HW's Error packet -> that error values
     *      (In this case, `ReturnType` must be `RPT_RETTYPE_SUCCESS`)
     * 
     * Ex 3) Partially readed packet -> segmented packet
     *      (In this case, `ReturnType` must be `RPT_RETTYPE_CFE`
     *       and `ReturnCode` must be `CFE_SRL_PARTIAL_READ_ERR`)
     */
    uint16 ReturnDataSize;
    uint8 ReturnValue[RPT_RET_VALUE_BUF_SIZE];

} RPT_Report_t;



typedef struct RPT_Critical {

    RPT_Report_t Report;
    CFE_TIME_SysTime_t Time;

} RPT_Critical_t;


/*****************************************************
 * Return Type definition
 * Should matched with return function
 * If RETTYPE is CFE core error, `RPT_RETTYPE_CFE`
 * If RETTYPE is App error, `RPT_RETTYPE_APP`
 ****************************************************/
typedef enum {

    RPT_RETTYPE_SUCCESS,

    /**
     * `osapi-error.h`
     */
    RPT_RETTYPE_OSAL,

    /**
     * `cfe_error.h`
     */
    RPT_RETTYPE_CFE,

    /**
     * App layer library
     */
    RPT_RETTYPE_LIB,

    /**
     * App layer Application
     * Also `cfe_error.h`
     */
    RPT_RETTYPE_APP,

    /**
     * H/W error packet case
     * This means the **serial communication is successfully (or partially) done**
     */
    RPT_RETTYPE_HW,

} RPT_ReturnType_t;


#endif
