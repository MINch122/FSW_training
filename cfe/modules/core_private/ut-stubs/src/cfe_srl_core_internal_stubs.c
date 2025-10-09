/***********************************************************************
 *  Copyright (c) 2025, Yonsei University as represented by the
 *  Department of Satellite Systems (DSS) & Astrodynamics & Control Lab (ACL)
 *  All rights reserved. This software was created at DSS
 ************************************************************************/
/**
 * @file
 *
 * Purpose:  cFE File Services (SRL) library API header file
 *
 * Author:   HyeokJin Kweon
 *
 */
#define CFE_SRL_CORE_INTERNAL_H

/**
 * @file
 *
 * Auto-Generated stub implementations for functions defined in cfe_srl_core_internal header
 */

#include "cfe_srl_core_internal.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_EarlyInit()
 * ----------------------------------------------------
 */
int32 CFE_SRL_EarlyInit(void)
{
    UT_GenStub_SetupReturnBuffer(CFE_SRL_EarlyInit, int32);

    UT_GenStub_Execute(CFE_SRL_EarlyInit, Basic, NULL);

    return UT_GenStub_GetReturnValue(CFE_SRL_EarlyInit, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for CFE_SRL_TaskMain()
 * ----------------------------------------------------
 */
void CFE_SRL_TaskMain(void)
{

    UT_GenStub_Execute(CFE_SRL_TaskMain, Basic, NULL);
}
