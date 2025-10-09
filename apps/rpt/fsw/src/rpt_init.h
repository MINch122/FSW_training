/**
 * @file
 *
 * Main header file for the RPT Init function
 */

#ifndef RPT_INIT_H
#define RPT_INIT_H

#include "common_types.h"
#include "cfe.h"

int32 RPT_PriorInit(void);
CFE_Status_t RPT_TableInit(void);
CFE_Status_t RPT_OpsDataInit(void);
CFE_Status_t RPT_CriticalQInit(void);

#endif