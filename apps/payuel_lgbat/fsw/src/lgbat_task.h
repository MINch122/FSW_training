#ifndef LGBAT_TASK_H
#define LGBAT_TASK_H

#include "cfe.h"
#include "lgbat_mission_cfg.h"
#include "lgbat_perfids.h"
#include "lgbat_msgids.h"
#include "lgbat_msg.h"

// Application global data structure
typedef struct
{
    // Command counters
    uint8_t  CmdCounter;
    uint8_t  ErrCounter;
    uint32   RunStatus;

    // Software bus
    CFE_SB_PipeId_t CmdPipe;
    char            CmdPipeName[CFE_MISSION_MAX_API_LEN];
    uint16          PipeDepth;

    // Telemetry messages
    LGBAT_HkTlm_t      HkTlm;
    LGBAT_ReportTlm_t  ReportTlm;
    LGBAT_BcnTlm_t     BcnTlm;
    LGBAT_CriticalTlm_t CriticalTlm;

    // BMS data cache filled by I2C reads
    LGBAT_BmsAllData_t BmsData;

    // Application state flags
    bool PowerApplied;      // True when 3.3V has been turned on by SET_POWER_CC
    bool MissionActive;     // True while within the 2-week mission window
    bool FirstCommSuccess;  // True after the first full I2C cycle succeeds

    // Mission timer start point
    CFE_TIME_SysTime_t MissionStartTime;

} LGBAT_Data_t;

extern LGBAT_Data_t LGBAT_Data;

// Function prototypes
void         LGBAT_Main(void);
CFE_Status_t LGBAT_Init(void);

#endif // LGBAT_TASK_H
