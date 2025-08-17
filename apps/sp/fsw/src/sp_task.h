#ifndef SP_APP_H 
#define SP_APP_H 

#include "cfe.h"
#include "cfe_config.h"

#include "sp_mission_cfg.h"
#include "sp_platform_cfg.h"

#include "sp_perfids.h"
#include "sp_msgids.h"
#include "sp_msg.h"

typedef struct{
    uint8 CmdCounter;
    uint8 ErrCounter;

    SP_APP_BcnTlm_t BcnTlm;
    
    //SP_APP_DeployCmd_t DeployCmd;

    uint32 RunStatus;

    CFE_SB_PipeId_t CommandPipe;

    CFE_TBL_Handle_t TblHandles[SP_APP_NUMBER_OF_TABLES];
} SP_APP_Data_t;

extern SP_APP_Data_t SP_APP_Data;

void SP_APP_Main(void);
CFE_Status_t SP_APP_Init(void);

#endif

