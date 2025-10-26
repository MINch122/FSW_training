/**
 * @file oem_basetype.h
 * @brief Standard C type extensions and additional types defined per the
 *        OEM7 Commands and Logs Reference Manual
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2024.
 */
#ifndef _OEM_BASETYPE_H_
#define _OEM_BASETYPE_H_

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
    OEM_OK              =   0,
    OEM_ERR_NULL        =  -1,
    OEM_ERR_NOMEM       =  -2,
    OEM_ERR_RANGE       =  -3,
    OEM_ERR_NOTFOUND    =  -4,
    OEM_ERR_INVALID     =  -5,
    OEM_ERR_EXISTS      =  -6,
    OEM_ERR_FULL        =  -7,
    OEM_ERR_EMPTY       =  -8,
    OEM_ERR_LIST        =  -9,
    OEM_ERR_READ        = -10,
    OEM_ERR_WRITE       = -11,
    OEM_ERR_TIMEOUT     = -12,
    OEM_ERR_MUTEX_INIT  = -13,
    OEM_ERR_MUTEX_LOCK  = -14,
    OEM_ERR_MUTEX_UNLOCK = -15,

    OEM_ERR_READ_HEADER,
    OEM_ERR_READ_BODY,
    OEM_ERR_READ_CRC,
    OEM_ERR_IO_CLOCK,

    OEM_ERR_LEN_HDR     = -20,
    OEM_ERR_LEN_MSG     = -21,
    OEM_ERR_CRC         = -22,
    OEM_ERR_NOBUF       = -23,
    OEM_ERR_TOOLONG     = -24,
    OEM_ERR_STRAY       = -25,
    OEM_ERR_MID         = -26,

    OEM_ERR_IO_PORT_INDEX   = 0,    /* Invalid IO port index.   */
    OEM_ERR_IO_PORT_UNSET   = 0,    /* IO port not initialized. */

    OEM_ERR_LIB         = -30,
    OEM_ERR_UNKNOWN     = -99,
} oem_ret;

#endif