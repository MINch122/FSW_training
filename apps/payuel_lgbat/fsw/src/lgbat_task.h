#ifndef LGBAT_TASK_H
#define LGBAT_TASK_H

#include "cfe.h"
#include "lgbat_mission_cfg.h"
#include "lgbat_perfids.h"
#include "lgbat_msgids.h"
#include "lgbat_msg.h"

typedef struct
{
    uint8_t  CmdCounter;
    uint8_t  ErrCounter;
    uint32   RunStatus;

    CFE_SB_PipeId_t CmdPipe;
    char            CmdPipeName[CFE_MISSION_MAX_API_LEN];
    uint16          PipeDepth;

    LGBAT_HkTlm_t      HkTlm;
    LGBAT_ReportTlm_t  ReportTlm;
    LGBAT_BcnTlm_t     BcnTlm;
    LGBAT_CriticalTlm_t CriticalTlm;

    LGBAT_BmsAllData_t BmsData;

    bool PowerApplied;
    bool MissionActive;
    bool FirstCommSuccess;

    CFE_TIME_SysTime_t MissionStartTime;

} LGBAT_Data_t;

extern LGBAT_Data_t LGBAT_Data;

void         LGBAT_Main(void);
CFE_Status_t LGBAT_Init(void);

#endif /* LGBAT_TASK_H */
