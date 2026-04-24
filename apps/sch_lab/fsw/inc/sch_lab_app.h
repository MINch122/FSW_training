#ifndef SCH_LAB_APP_H
#define SCH_LAB_APP_H

#include <stdbool.h>

#include "cfe.h"

#include "sch_lab_fcncodes.h"
#include "sch_lab_mission_cfg.h"
#include "sch_lab_msgids.h"
#include "sch_lab_msgstruct.h"
#include "sch_lab_perfids.h"
#include "sch_lab_tbl.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader;
    uint16                  MessageBuffer[SCH_LAB_MAX_ARGS_PER_ENTRY];
    uint16                  PayloadLength;
    uint32                  PacketRate;
    uint32                  Counter;
    bool                    Enabled;
} SCH_LAB_StateEntry_t;

typedef struct
{
    SCH_LAB_StateEntry_t State[SCH_LAB_MAX_SCHEDULE_ENTRIES];
    osal_id_t            TimerId;
    osal_id_t            TimingSem;
    CFE_TBL_Handle_t     TblHandle;
    CFE_SB_PipeId_t      CmdPipe;
    uint16               CmdCounter;
    uint16               ErrCounter;
} SCH_LAB_GlobalData_t;

extern SCH_LAB_GlobalData_t SCH_LAB_Global;

CFE_Status_t SCH_LAB_AppInit(void);
void         SCH_LAB_AppMain(void);

#endif
