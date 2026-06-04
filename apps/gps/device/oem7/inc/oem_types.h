/**
 * @file oem_types.h
 * @brief Standard C type extensions and additional types defined per the
 *        OEM7 Commands and Logs Reference Manual
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2024.
 */
#ifndef _OEM_TYPES_H_
#define _OEM_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OEM_PACK        __attribute__((packed))

typedef int8_t          oem_char;
typedef int16_t         oem_short;
typedef int32_t         oem_long;
typedef uint8_t         oem_uchar;
typedef uint16_t        oem_ushort;
typedef uint32_t        oem_ulong;
typedef double          oem_double;
typedef float           oem_float;
typedef uint32_t        oem_enum;
typedef bool            oem_bool;
typedef uint32_t        oem_crc;
typedef int32_t         oem_gpsec;

typedef enum { 
    OEM_OK                  =   0,  /* success*/

    /* Generic errors. */
    OEM_ERR_NULL            =  -1,  /* null pointer passed */
    OEM_ERR_NOMEM           =  -2,  /* malloc failed */
    OEM_ERR_RANGE           =  -3,  /* value out of range */
    OEM_ERR_NOT_FOUND       =  -4,  /* item not found */
    OEM_ERR_INVALID         =  -5,  /* invalid argument */
    OEM_ERR_EXISTS          =  -6,  /* item already exists */
    OEM_ERR_FULL            =  -7,  /* container full */
    OEM_ERR_EMPTY           =  -8,  /* container empty */
    OEM_ERR_NOBUF           = -10,  /* no buffer set/available */

    /* Log/Task layer errors. */
    OEM_ERR_LOG_HEADER_SIZE = -20,  /* log-encoded header size error */
    OEM_ERR_LOG_BODY_SIZE   = -21,  /* log-encoded body size error */
    OEM_ERR_LOG_RESP_SIZE   = -22,  /* too short response message */
    OEM_ERR_LOG_TOO_LARGE   = -23,  /* log message size exceeds buffer capacity */
    OEM_ERR_LOG_CRC         = -24,  /* CRC mismatch */
    OEM_ERR_LOG_MISSING_CRC = -25,  /* CRC did not arrive */
    OEM_ERR_LOG_STRAY       = -26,  /* no handler for this log */
    OEM_ERR_LOG_LIST        = -27,  /* linked list operation error */
    OEM_ERR_LOG_MUTEX_INIT  = -28,  /* log mutex initialization error */
    OEM_ERR_LOG_MUTEX_LOCK  = -29,  /* log mutex lock error */
    OEM_ERR_LOG_MUTEX_UNLOCK= -30,  /* log mutex unlock error */
    OEM_ERR_LOG_SM_STATE    = -31,  /* invalid state machine state */
    OEM_ERR_LOG_SM_TARGET   = -32,  /* invalid state machine target */
    OEM_ERR_LOG_SM_PREFILL  = -33,  /* state machine filled more than expected at prep state */
    OEM_ERR_LOG_SM_SIZE_MISMATCH = -34, /* state machine fed with incorrect size */

    /* I/O layer errors. */
    OEM_ERR_IO_IFACE_INDEX  = -40,    /* invalid I/O interface index */
    OEM_ERR_IO_IFACE_UNSET  = -41,    /* oem_io_init_interface not called */
    OEM_ERR_IO_WRITE        = -42,    /* I/O write error */
    OEM_ERR_IO_READ         = -43,    /* I/O read error */
    OEM_ERR_IO_TIMEOUT      = -44,    /* I/O read timeout */
    OEM_ERR_IO_WRITE_PARTIAL= -45,    /* I/O partial write */
    OEM_ERR_IO_CONFIG       = -46,    /* I/O port configuration error */

    /* Utility layer errors. */
    OEM_ERR_UTILS_LIST_NULL = -50,    /* list object is null */

    /* Miscellaneous/callback-invoked errors. */
    OEM_ERR_FILE_OPEN       = -100,
    OEM_ERR_FILE_READ       = -101,
    OEM_ERR_FILE_WRITE      = -102,
    OEM_ERR_NO_SOLUTION     = -103,
    OEM_ERR_TIME            = -104,

    OEM_ERR_UNKNOWN         = -9999,
} oem_ret_t;

#endif