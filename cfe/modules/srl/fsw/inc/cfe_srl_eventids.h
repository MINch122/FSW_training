/************************************************************************
 * Author : Hyeokjin Kweon
 * 
 * Last Modified : 2025 - 04 - 27
 * 
 * Brief : Serial Comm. Core Module's Event ID 
 ************************************************************************/

 /**
 * @file
 *
 *  Core Module Serial Comm. Services Event IDs
 */

 #ifndef CFE_SRL_EVENTS_H
 #define CFE_SRL_EVENTS_H

/**
 * Event ID of Each Comm. protocol Initialization
 * 
 * Type : Information
 */
#define CFE_SRL_TASK_INIT_EID           1

#define CFE_SRL_NOOP_INF_EID            2

#define CFE_SRL_RESET_INF_EID           3

#define CFE_SRL_RESET_HANDLE_INF_EID    4

#define CFE_SRL_GET_HANDLE_STATUS_INF_EID   5

#define CFE_SRL_INIT_HANDLE_INF_EID     6

#define CFE_SRL_CLOSE_HANDLE_INF_EID    7

#define CFE_SRL_MID_ERR_EID             8

#define CFE_SRL_CC_ERR_EID              9


#define CFE_SRL_LEN_ERR_EID             114

#define CFE_SRL_PRIOR_INIT_EID      0
#define CFE_SRL_I2C0_INIT_EID       1
#define CFE_SRL_I2C1_INIT_EID       2
#define CFE_SRL_I2C2_INIT_EID       3
#define CFE_SRL_CAN0_INIT_EID       4
#define CFE_SRL_UART0_INIT_EID      5
#define CFE_SRL_RS422_INIT_EID      6
#define CFE_SRL_GPIO_INIT_EID       7


/**
 * Event ID of Error in Initialization
 * Type : Error
 */
// typedef enum {
//     CFE_SRL_OK = 0,
//     CFE_SRL_ERROR = -1,
//     CFE_SRL_NULL_ERR,
//     CFE_SRL_TIMEOUT_ERR = -2,


//     CFE_SRL_PRIOR_INIT_ERR = -3,
//     CFE_SRL_UART_INIT_ERR,
//     CFE_SRL_NAME_PTR_NULL_ERR,
//     CFE_SRL_NAME_LEN_OVERFLOW_ERR,
//     CFE_SRL_FULL_ERR,

//     CFE_SRL_NOT_FOUND = -4,
//     CFE_SRL_DEV_NOT_FOUND = -5,
//     CFE_SRL_NOT_OPENED = -6,
//     CFE_SRL_REF_REMAIN,
//     CFE_SRL_ALREADY_EXIST,

//     CFE_SRL_MUTEX_ATTR_INIT_ERR,
//     CFE_SRL_MUTEX_SET_PROTOCOL_ERR,
//     CFE_SRL_MUTEX_INIT_ERR,
//     CFE_SRL_MUTEX_LOCK_ERR,
//     CFE_SRL_MUTEX_UNLOCK_ERR,
//     CFE_SRL_NOT_OPEN_ERR,
//     CFE_SRL_WRITE_ERR,
//     CFE_SRL_PARTIAL_WRITE_ERR,
//     CFE_SRL_READ_ERR,
//     CFE_SRL_PARTIAL_READ_ERR,
//     CFE_SRL_CLOSE_ERR,


//     /**
//      * CSP
//      */
//     CFE_SRL_TYPE_UNSUPPORTED,

// } CFE_SRL_Error_Type_t;

#endif /* CFE_SRL_EVENTS_H */