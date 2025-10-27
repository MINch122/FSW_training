#include "eo_child.h"

/* Forward declaration for private function */
static void EO_PhaseDispatch(void);

static void EO_SantPhase(void);
static void EO_TCWaitPhase(void);
static void EO_SantConfirmPhase(void);
static void EO_PCDU2ndChannelOnPhase(void);
static void EO_SPDeployPhase(void);
static void EO_MMTDeployPhase(void);
static void EO_AttitudeControlPhase(void);
/* End of Forward declaration */


void EO_ChildTask(void) {
    for (;;) {
        OS_MutSemTake(EO_Data.EOMutex);
        if(EO_Data.CurrentStep.CurrentPhase == EO_DONE) {
            /* If Early Orbit Phase done, Exit several app */
            OS_MutSemGive(EO_Data.EOMutex);
            EO_ExitApps();
            return;
        }
        OS_MutSemGive(EO_Data.EOMutex);

        /* Send EPS to get vi */
        EO_Data.WaitingEPS = true;
        EO_RequestVbattEPS();

        if (OS_BinSemTake(EO_Data.EPS_SemId) == OS_SUCCESS) {
            /* Debug */
            EO_PRINTF("%s:EPS Vbatt: %u\n", __func__, EO_Data.Vbatt);

            EO_PhaseDispatch();
        }

        OS_TaskDelay(5000); /* Prevent CPU hogging */
    }
}


void EO_PhaseDispatch(void) {
    switch (EO_Data.CurrentStep.CurrentPhase)
    {
    case EO_SANT_DEPLOY_PHASE:
        EO_PRINTF("%s: SANT Deploy Phase.\n", __func__);
        EO_SantPhase();
        break;
    
    case EO_TC_WAIT_PHASE:
        EO_PRINTF("%s: TC wait Phase.\n", __func__);
        EO_TCWaitPhase();
        break;
    
    case EO_SANT_DEPLOY_CONFIRM_PHASE:
        EO_PRINTF("%s: SANT confirm Phase.\n", __func__);
        EO_SantConfirmPhase();
        break;
    
    case EO_PCDU_2ND_CHANNEL_ON_PHASE:
        EO_PRINTF("%s: PCDU ON Phase.\n", __func__);
        EO_PCDU2ndChannelOnPhase();
        break;

    case EO_SP_DEPLOY_PHASE:
        EO_PRINTF("%s: SP Deploy Phase.\n", __func__);
        EO_SPDeployPhase();
        break;

    case EO_MMT_DEPLOY_PHASE:
        EO_PRINTF("%s: MMT Deploy Phase.\n", __func__);
        EO_MMTDeployPhase();
        break;

    case EO_ATTITUDE_CONTROL_PHASE:
        EO_PRINTF("%s: Attitude Phase.\n", __func__);
        EO_AttitudeControlPhase();
        break;

    default:
        break;
    }

    EO_FinalizePhase();
}


void EO_SantPhase(void) {
    /* Check Vbatt */
    if (EO_Data.Vbatt <= EO_VBATT_THRESHOLD) {
        EO_PRINTF("%s: Vbatt Low, Exit.\n", __func__);
        return;
    }

    /* Send SANT Deploy Command */
    EO_SantDeploy();

    /* Sleep for SANT Burn time */
    OS_TaskDelay(1000 * (EO_SANT_DURATION + 1)); // +1 for margin
    /* Or, Polling. */

    /* Send SANT operation tlm */
    EO_Data.WaitingSANT = true;
    EO_SantConfirm();

    if (OS_BinSemTake(EO_Data.SANT_SemId) == OS_SUCCESS) {
        /* Check SANT Status */
        if (EO_Data.Status == EO_IS_DEPLOYED) { // If deployed,
            EO_PRINTF("%s:SANT Deployed.\n", __func__);
            OS_MutSemTake(EO_Data.EOMutex);
            /* Change Current Step */
            EO_Data.CurrentStep.CurrentPhase = EO_TC_WAIT_PHASE;
            EO_Data.CurrentStep.S_deploy = EO_IS_DEPLOYED;
            EO_Data.CurrentStep.S_tries = EO_Data.BurnTries;
            OS_MutSemGive(EO_Data.EOMutex);
            return;
        }
        else { // If not deployed,
            if (EO_Data.BurnTries > 6) { // if Too many tries,
                EO_PRINTF("%s:SANT Too many tries.\n", __func__);
                /* Forced to next phase */
                OS_MutSemTake(EO_Data.EOMutex);
                EO_Data.CurrentStep.CurrentPhase = EO_TC_WAIT_PHASE;
                EO_Data.CurrentStep.S_deploy = EO_NOT_DEPLOYED;
                EO_Data.CurrentStep.S_tries = EO_Data.BurnTries;
                OS_MutSemGive(EO_Data.EOMutex);
            }
            else {
                EO_PRINTF("%s:SANT retry.\n", __func__);
                OS_MutSemTake(EO_Data.EOMutex);
                EO_Data.CurrentStep.S_tries = EO_Data.BurnTries;
                OS_MutSemGive(EO_Data.EOMutex);
                return; // need more try, just return and come again.
            }
        }
    }
}


void EO_TCWaitPhase(void) {
    /* Check Vbatt */
    if (EO_Data.Vbatt <= EO_VBATT_THRESHOLD) {
        EO_PRINTF("%s: Vbatt Low, Disable Beacon.\n", __func__);
        EO_DisableBeacon();
    }
    /* If Vbatt OK, */
    /* Forced via to STRX of bcn transmission and, */
    else {
        EO_PRINTF("%s: Vbatt enough, Enable Beacon.\n", __func__);
        CFE_SRL_ApiChangeVia(CSP_NODE_STRX);

        /* Enable beacon */
        EO_EnableBeacon();
    }

    /* Check Elapsed time */
    CFE_TIME_SysTime_t EpochTime = EO_Data.Epoch;
    CFE_TIME_SysTime_t CurTime = CFE_TIME_GetTime();

    CFE_TIME_SysTime_t Result = CFE_TIME_Subtract(CurTime, EpochTime);
    EO_PRINTF("%s: Elapsed Time sec: %u\n", __func__, Result.Seconds);
    /* If Elapsed too much, */
    if (Result.Seconds > EO_MAX_ELAPSED_TIME) {
        EO_PRINTF("%s: Too much time elapsed. Goto SANT confirm phase.\n", __func__);
        
        /* Forced to Next Phase */
        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.CurrentPhase = EO_SANT_DEPLOY_CONFIRM_PHASE;
        EO_Data.CurrentStep.IsTC = false;
        OS_MutSemGive(EO_Data.EOMutex);
        return;
    }

    /* Check the elapsed time & TC receive flag */
    if (EO_Data.CurrentStep.IsTC == true) { // If TC received,
        EO_PRINTF("%s: TC Received. Goto SANT Confirm Phase.\n", __func__);

        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.CurrentPhase = EO_SANT_DEPLOY_CONFIRM_PHASE;
        OS_MutSemGive(EO_Data.EOMutex);
    }
}


void EO_SantConfirmPhase(void) {
    /* Send SANT operation tlm */
    EO_Data.WaitingSANT = true;
    EO_SantConfirm();

    if (OS_BinSemTake(EO_Data.SANT_SemId) == OS_SUCCESS) {
        EO_PRINTF("%s: SANT Sem Take.\n", __func__);
        /* Check SANT Status */
        if (EO_Data.Status == EO_IS_DEPLOYED) { // If deployed,
            EO_PRINTF("%s: SANT Deployed.\n", __func__);
            /* Change Current Step */
            OS_MutSemTake(EO_Data.EOMutex);
            EO_Data.CurrentStep.CurrentPhase = EO_PCDU_2ND_CHANNEL_ON_PHASE;
            EO_Data.CurrentStep.S_deploy = EO_IS_DEPLOYED;
            EO_Data.CurrentStep.S_tries = EO_Data.BurnTries;
            OS_MutSemGive(EO_Data.EOMutex);
            return;
        }
        else { // If not deployed,
            EO_PRINTF("%s: SANT NOT Deployed.\n", __func__);
            /* Check Vbatt */
            if (EO_Data.Vbatt <= EO_VBATT_THRESHOLD) {
                EO_PRINTF("%s: VBatt Low. Exit.\n", __func__);
                return;
            }

            /* If too many tries, */
            if (EO_Data.BurnTries > 9) { 
                EO_PRINTF("%s: SANT Too many tries. Goto next phase.\n", __func__);
                /* Forced to next phase */
                OS_MutSemTake(EO_Data.EOMutex);
                EO_Data.CurrentStep.CurrentPhase = EO_PCDU_2ND_CHANNEL_ON_PHASE;
                EO_Data.CurrentStep.S_deploy = EO_NOT_DEPLOYED;
                EO_Data.CurrentStep.S_tries = EO_Data.BurnTries;
                OS_MutSemGive(EO_Data.EOMutex);
                return;
            }

            /* If Vbatt OK && proper tries */
            EO_PRINTF("%s: Try SANT deploy again.\n", __func__);
            /* Send SANT Deploy Command */
            EO_SantDeploy();
            OS_TaskDelay(EO_SANT_DURATION + 1); /* sleep during burn. +1 for margin */
        }
    }
    return;
}


void EO_PCDU2ndChannelOnPhase(void) {
    /* 1. PCDU Channel on */
    uint8_t i = 0;
    do {
        /* PCDU 2nd channel ON */
        EO_RequestSetOutSingle(EO_PCDU_SP_CHANNEL_IDX, 1);

        OS_TaskDelay(500); // Wait until channel on

        /* Request EPS output channel */
        EO_Data.WaitingEPS = true;
        EO_RequestOutEPS();

        if (OS_BinSemTake(EO_Data.EPS_SemId) == OS_SUCCESS) {
            /* If 2nd channel ON, (Success) */
            if (EO_Data.Output[EO_PCDU_SP_CHANNEL_IDX]) {
                EO_PRINTF("%s: PCDU 2nd Channel ON.\n", __func__);
                break; // break the loop
            }

            /* If 2nd channel OFF, (Fail) */
            else {
                EO_PRINTF("%s: PCDU 2nd Channel `NOT` ON.\n", __func__);
                OS_TaskDelay(500);  // Wait for a moment,
            }

            i++;
        }
        /* If Sem Take fail, */
        else break; // break the loop
    } while (i < EO_PCDU_MAX_TRIES);


    if (i >= EO_PCDU_MAX_TRIES) { // PCDU channel on fail,
        EO_PRINTF("%s: PCDU 2nd Channel FAIL.\n", __func__);
        /* Forced to next phase - skip the SP deploy */
        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.CurrentPhase = EO_MMT_DEPLOY_PHASE;
        EO_Data.CurrentStep.SP_deploy = EO_NOT_DEPLOYED;
        EO_Data.CurrentStep.SP_tries1 = 0;
        EO_Data.CurrentStep.SP_tries2 = 0;
        OS_MutSemGive(EO_Data.EOMutex);
    }
    else { // PCDU channel on success,
        EO_PRINTF("%s: PCDU 2nd Channel SUCCESS.\n", __func__);

        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.CurrentPhase = EO_SP_DEPLOY_PHASE;
        OS_MutSemGive(EO_Data.EOMutex);
    }
}




void EO_SPDeployPhase(void) {

    CFE_SRL_GPIO_Handle_t *Out1 = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_OUT1_GPIO_INDEXER);
    CFE_SRL_GPIO_Handle_t *Out2 = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_OUT2_GPIO_INDEXER);
    CFE_SRL_GPIO_Handle_t *In = CFE_SRL_ApiGetGpioHandle(CFE_SRL_SP_IN_GPIO_INDEXER);

    uint8_t i = 0;
    bool IsDeploy = true; /* In this phase, `false` indicate deployed */
    do {
        /* Calculate the duration */
        uint8_t Duration = (EO_DEFAULT_SP_DEPLOY_TIME + (i*10));
        EO_PRINTF("%s: SP duration time: %u.\n", __func__, Duration);
        /*------------------------------------*/
        /*              SP1 Deploy            */
        /*------------------------------------*/
        /* Check the Vbatt */
        if (EO_Data.Vbatt <= EO_VBATT_THRESHOLD_FOR_SP) return;

        /* SP1 deploy trial */
        EO_SPDeploy(Out1, Duration);
        
        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.SP_tries1 += Duration;
        OS_MutSemGive(EO_Data.EOMutex);

        /* Write the SP deploy time */
        EO_WriteStep();

        /* Check deploy status */
        CFE_SRL_ApiGpioGet(In, &IsDeploy);
        if (!IsDeploy) break;

        /*------------------------------------*/
        /*              SP2 Deploy            */
        /*------------------------------------*/
        /* Check the Vbatt */
        if (EO_Data.Vbatt <= EO_VBATT_THRESHOLD_FOR_SP) return;

        /* SP2 deploy trial */
        EO_SPDeploy(Out2, Duration);

        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.SP_tries2 += Duration;
        OS_MutSemGive(EO_Data.EOMutex);

        /* Write the SP deploy time */
        EO_WriteStep();

        /* Check deploy status */
        CFE_SRL_ApiGpioGet(In, &IsDeploy);
        if (!IsDeploy) break;

        i++;

    } while (i < EO_SP_MAX_TRIES);

    /* Confirm the phase results */
    if (!IsDeploy) { // If Deployed,
        EO_PRINTF("%s: SP deployed SUCCESS.\n", __func__);
        /* Change to Next Phase */
        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.CurrentPhase = EO_MMT_DEPLOY_PHASE;
        EO_Data.CurrentStep.SP_deploy = EO_IS_DEPLOYED;
        OS_MutSemGive(EO_Data.EOMutex);
    }
    else {
        EO_PRINTF("%s: SP deployed FAIL. Goto next Phase\n", __func__);
        /* Forced to Next Phase */
        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.CurrentPhase = EO_MMT_DEPLOY_PHASE;
        EO_Data.CurrentStep.SP_deploy = EO_NOT_DEPLOYED;
        OS_MutSemGive(EO_Data.EOMutex);
    }
    
}

void EO_MMTDeployPhase(void) {
    /* Check the Deploy tries */
    /* If exceed `3`, force to next phase */
    OS_MutSemTake(EO_Data.EOMutex);
    if (EO_Data.CurrentStep.MMT_tries >= EO_MMT_MAX_TRIES) {
        EO_PRINTF("%s: MMT deployed FAIL. Goto next Phase\n", __func__);
        EO_Data.CurrentStep.CurrentPhase = EO_ATTITUDE_CONTROL_PHASE;
        EO_Data.CurrentStep.MMT_Deploy = EO_NOT_DEPLOYED;
        OS_MutSemGive(EO_Data.EOMutex);
        return;
    }
    OS_MutSemGive(EO_Data.EOMutex);

    /* Check the Vbatt */
    if (EO_Data.Vbatt <= EO_VBATT_THRESHOLD) return;

    /* Send ADCS MMT deploy cmd - Target ID 52 */
    /* Only try when previous Burn pin state is `false` */
    if (!EO_Data.MagBurnPinState) {
        EO_PRINTF("%s: MMT still Burn.\n", __func__);
        EO_RequestMMTDeploy();

        OS_MutSemTake(EO_Data.EOMutex);
        EO_Data.CurrentStep.MMT_tries ++;
        OS_MutSemGive(EO_Data.EOMutex);
    }

    OS_TaskDelay(3000); // Wait until deploy procedure complete

    /* Request to ADCS, MMT deploy status */
    EO_Data.WaitingADCS = true;
    EO_RequestMMTTlm();

    /* Check the deployment status - Target ID 167 deploy pin state */
    if (OS_BinSemTimedWait(EO_Data.ADCS_SemId, 2000) == OS_SUCCESS) {
        if (EO_Data.MagDeployPinState == true) { // If deployed,
            EO_PRINTF("%s: MMT deployed.\n", __func__);
            /* Change to next Phase */
            OS_MutSemTake(EO_Data.EOMutex);
            EO_Data.CurrentStep.CurrentPhase = EO_ATTITUDE_CONTROL_PHASE;
            EO_Data.CurrentStep.MMT_Deploy = EO_IS_DEPLOYED;
            OS_MutSemGive(EO_Data.EOMutex);
            return;
        }
        else if (EO_Data.MagDeployPinState == false && 
                EO_Data.MagBurnPinState == true) { // If not deployed and tring to deploy,
            /* Wait */
            EO_PRINTF("%s: MMT NOT deployed and still burn.\n", __func__);
            /* i.e.) just return. the Burn request will be skipped, and look up the deploy state */
            return;
        }
        else { // If not deployed and not tring,
            /* Try again */
            EO_PRINTF("%s: MMT NOT deployed and not burn.\n", __func__);
            return;
        }
    }
}

void EO_AttitudeControlPhase(void) {

    /* Check the Vbatt */
    if (EO_Data.Vbatt <= EO_VBATT_THRESHOLD) return;

    /* Send */
}