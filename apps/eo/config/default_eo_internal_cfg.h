#ifndef EO_INTERNAL_CFG_H
#define EO_INTERNAL_CFG_H

#include "rpt_interface_cfg.h"

/* Pre definition to use debug */
#define EO_DEBUG
// #undef EO_DEBUG
/*----End of Pre definition----*/


/* Pipe Depth */
#define EO_PIPE_DEPTH      8

/* Child Task Configuration */
#define EO_CHILD_TASK_NAME          "EO_Child"
#define EO_CHILD_TASK_STACK_SIZE    16384
#define EO_CHILD_TASK_PRIORITY      40

/**
 * EO Current Operation data file path
 */
#define EO_CURRENT_DATA_PATH       "/cf/WAM.bin" /* Internal FLASH */

/**
 * EO Deployment definition for SANT, SP, UANT(?)
 */
#define EO_NOT_DEPLOYED     0
#define EO_IS_DEPLOYED      1

/**
 * EO Mut Sem name definition
 */
#define EO_EPS_VI_SEM       "EO_EPS_VI_SEM"
#define EO_EPS_OUT_SEM      "EO_EPS_OUT_SEM"
#define EO_SANT_SEM         "EO_SANT_SEM"
#define EO_ADCS_SEM         "EO_ADCS_SEM"

#define EO_PHASE_MUT        "EO_PHASE_MUT"


/****************************************/
/*                                      */
/*  Subsystem Threshold configuration   */
/*                                      */
/****************************************/
/* EPS Configuration */
#define EO_VBATT_THRESHOLD_DEFAULT      14000u /* [mV] */

/* SANT Configuration */
#define EO_SANT_DURATION    6u /* [sec] */
#define EO_VBATT_THRESHOLD_FOR_SANT_CONFIRM       (EO_VBATT_THRESHOLD_DEFAULT - 900u) /* [mV] */

/* TC Wait configuration */
#define EO_VBATT_THRESHOLD_FOR_TC       (EO_VBATT_THRESHOLD_DEFAULT - 500u) /* [mV] */
#ifdef EO_DEBUG
#define EO_MAX_ELAPSED_TIME     ((uint32)300)    /* <\brief 3 minute */
#else
#define EO_MAX_ELAPSED_TIME     ((uint32)(60 * 60 * 24 * 4)) /* <\brief 4 days */
#endif

/* PCDU Configuration */
#define EO_PCDU_MAX_TRIES               5
#define EO_PCDU_SP_CHANNEL_IDX          2

/* SP Configuration */
#define EO_DEFAULT_SP_DEPLOY_TIME       20 /* <\brief [sec]*/
#define EO_VBATT_THRESHOLD_FOR_SP       (EO_VBATT_THRESHOLD_DEFAULT - 900u) /* <\brief [mV]*/
#define EO_SP_MAX_TRIES                 3

/* MMT Configuration */
#define EO_MMT_MAX_TRIES                3                                   /* <\brief Deprecated */
#define EO_VBATT_THRESHOLD_FOR_MMT      (EO_VBATT_THRESHOLD_DEFAULT - 900u) /* <\brief [mV] Deprecated */

/* ADCS Detumbling Configuration */
#define EO_VBATT_THRESHOLD_FOR_DETUMBLE (EO_VBATT_THRESHOLD_DEFAULT - 900u) /* <\brief [mV] */




/**
 * EO PHASE Enumeration
 */
typedef enum {
    
    EO_SANT_DEPLOY_PHASE,
    EO_TC_WAIT_PHASE,
    EO_SANT_DEPLOY_CONFIRM_PHASE,
    EO_PCDU_2ND_CHANNEL_ON_PHASE,
    EO_SP_DEPLOY_PHASE,
    // EO_MMT_DEPLOY_PHASE, /* <\brief Deprecated. */
    EO_DETUMBLE_PHASE,      /* <\brief Changed from `EO_ATTITUDE_CONTROL_PHASE` */
    
    EO_DONE = 0xFF
} EO_StepEnum_t;


/**
 * EO Phase Information struct
 */
typedef struct {

    /* EO Phase */
    uint8_t CurrentPhase; /* <\brief EO_StepEnum_t */

    /* Specific EO state info  */
    /* SANT */
    uint8_t S_deploy; /* <\brief `EO_NOT_DEPLOYED` or `EO_IS_DEPLOYED` */
    uint8_t S_tries;
    
    /* Solar Panel */
    uint8_t SP_deploy;  /* <\brief `EO_NOT_DEPLOYED` or `EO_IS_DEPLOYED` */
    uint8_t SP_tries;   /* <\brief Trial for SP deploy. Max 3 */
    uint8_t SP_Sec1;    /* <\brief PC3 High time */
    uint8_t SP_Sec2;    /* <\brief PA28 High time */

    /* ADCS */
    uint8_t MMT_Deploy; /* <\brief `EO_NOT_DEPLOYED` or `EO_IS_DEPLOYED` */
    uint8_t MMT_tries;
    uint8_t IsExecuteDetumble;

    // /* UANT */
    // uint8_t U_deploy; /* <\brief `EO_NOT_DEPLOYED` or `EO_IS_DEPLOYED` */
    // uint8_t U_tries;

    /*
     * " TC receive flag "
     * The "IsTC" member is marked volatile to help
     * ensure that an optimizing compiler does not rearrange
     * or eliminate reads/writes of this value.  It is read
     * outside of any locking to determine whether or not
     * the performance log function is enabled.
     */
    volatile bool IsTC;

} EO_CurrentStep_t;

#endif