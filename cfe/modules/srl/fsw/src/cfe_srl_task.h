/**
 * @file
 *
 *  Purpose:
 *  cFE Serial Services (SRL) task header file
 *
 *  References:
 *     Flight Software Branch C Coding Standard Version 1.0a
 *     cFE Flight Software Application Developers Guide
 *
 *  Notes:
 *
 */

 #ifndef CFE_SRL_TASK_H
 #define CFE_SRL_TASK_H

/**
 * Include
 */
#include "cfe_platform_cfg.h"
#include "common_types.h"

#include "cfe_sb_api_typedefs.h"
#include "cfe_es_api_typedefs.h"
#include "cfe_sbr_api_typedefs.h"
#include "cfe_msg_api_typedefs.h"
#include "cfe_fs_api_typedefs.h"
#include "cfe_resourceid_api_typedefs.h"
#include "cfe_sb_destination_typedef.h"
#include "cfe_srl_api_typedefs.h"
#include "cfe_srl_msgstruct.h"


#define CFE_SRL_PIPE_NAME   "SRL_CMD_PIPE"
#define CFE_SRL_PIPE_DEPTH  12


typedef struct {
    CFE_ES_AppId_t AppId;

    CFE_SB_PipeId_t CmdPipe;
    
    CFE_SRL_HousekeepingTlm_t HKTlmMsg;
} CFE_SRL_TaskData_t;


int32 CFE_SRL_TaskInit(void);

int32 CFE_SRL_NoopCmd(const CFE_SRL_NoopCmd_t *Cmd);

int32 CFE_SRL_ResetCounterCmd(const CFE_SRL_ResetCounterCmd_t *Cmd);

int32 CFE_SRL_ResetHandleCounterCmd(const CFE_SRL_ResetHandleCounterCmd_t *Cmd);

int32 CFE_SRL_GetHandleStatusCmd(const CFE_SRL_GetHandleStatusCmd_t *Cmd);

int32 CFE_SRL_InitHandleCmd(const CFE_SRL_InitHandleCmd_t *Cmd);

int32 CFE_SRL_CloseHandleCmd(const CFE_SRL_CloseHandleCmd_t *Cmd);

int32 CFE_SRL_SendHkCmd(const CFE_SRL_SendHkCmd_t *data);

#endif /* CFE_SRL_TASK_H */