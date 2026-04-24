/**
 * @file
 *
 * Main header file for the EO Init function
 */

#ifndef EO_INIT_H
#define EO_INIT_H

#include "common_types.h"
#include "cfe.h"

CFE_Status_t EO_StepInit(void);
CFE_Status_t EO_WriteStep(void);

#endif