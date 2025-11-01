#ifndef _MLK_CMTYPES_H_
#define _MLK_CMTYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef enum {
    M_SUCCESS   =  0,   /* Operation successful. */
    M_ERROR     = -1,   /* Generic error. */
    M_NULL      = -2,   /* Null pointer passed. */
    M_NFOUND    = -3,   /* The entity was not found. */
    M_RANGE     = -4,   /* The value is not in a valid range. */
    M_NOMEM     = -5,   /* Memory allocation error. */
    M_INVALID   = -6,   /* Invalid value. */
    M_EMPTY     = -7,   /* The entity is empty. */
    M_NODATA    = -8,   /* No data available. */
    M_EXIST     = -9,   /* Already exists. */
    M_NOIMPL    = -10,  /* Feature not implemented. */
    M_FILE      = -11,  /* File handling error. */
    M_IO        = -12,  /* Input-output error. */
    M_SIZE      = -13,  /* Invalid size argument. */
    M_UNKNOWN   = -99,  /* Unknown error. */

    M_ALG_MIN   = -100, /* ALG-specific error placeholder. */
    M_ALG_MAX   = -149, /* ALG-specific error placeholder. */

    M_LIST_PHMIN  = -150, /* LIST-specific error placeholder. */
    M_LIST_PHMAX  = -199, /* LIST-specific error placeholder. */

    M_UT_PHMIN    = -200,
    M_UT_PHMAX    = -249,
} mlk_ret_t;


#endif
