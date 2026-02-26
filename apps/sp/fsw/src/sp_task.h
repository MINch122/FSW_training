#ifndef SP_APP_H 
#define SP_APP_H 

#include "cfe.h"
#include "cfe_config.h"

#include "sp_mission_cfg.h"
#include "sp_platform_cfg.h"

#include "sp_perfids.h"
#include "sp_msgids.h"
#include "sp_msg.h"

typedef struct
{
    uint8 CmdCounter;
    uint8 ErrCounter;

    SP_HkTlm_t  HkTlm;   /**< Full DSP telemetry (both AR6 boards per DSP) */
    SP_BcnTlm_t BcnTlm;  /**< Lightweight beacon deploy status */

    uint32 RunStatus;

    CFE_SB_PipeId_t CommandPipe;

} SP_AppData_t;

extern SP_AppData_t SP_AppData;

void SP_AppMain(void);
CFE_Status_t SP_AppInit(void);

#endif

