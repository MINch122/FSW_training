/**
 * @file
 *
 * Purpose:  cFE File Services (SRL) library API header file
 *
 * Author:   HyeokJin Kweon
 *
 */
#ifndef CFE_SRL_CORE_INTERNAL_H
#define CFE_SRL_CORE_INTERNAL_H

#include "common_types.h"

/*****************************************************************************/
/**
** \brief Entry Point for cFE Core Application
**
** \par Description
**        This is the entry point to the cFE SRL Core Application.
**
** \par Assumptions, External Events, and Notes:
**          None
**
******************************************************************************/
void CFE_SRL_TaskMain(void);

/*****************************************************************************/
/**
** \brief Initializes the cFE core module SRL API Library
**
** \par Description
**        Initializes the cFE core module API Library
**
** \par Assumptions, External Events, and Notes:
**        -# This function MUST be called before any module API's are called.
**
******************************************************************************/
int32 CFE_SRL_EarlyInit(void);

#endif /* CFE_SRL_CORE_INTERNAL_H */