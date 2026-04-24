#ifndef LGBAT_TASK_H
#define LGBAT_TASK_H

#include "cfe.h"
#include "lgbat_mission_cfg.h"
#include "lgbat_perfids.h"
#include "lgbat_msgids.h"
#include "lgbat_msg.h"

typedef struct
{
    // App state counters 
    uint8_t  CmdCounter;
    uint8_t  ErrCounter;
    uint32   RunStatus;

    // Software Bus 
    CFE_SB_PipeId_t CmdPipe;
    char            CmdPipeName[CFE_MISSION_MAX_API_LEN];
    uint16          PipeDepth;

    // Telemetry messages 
    LGBAT_BcnTlm_t       BcnTlm;       // Beacon: key BMS data for ground downlink  
    LGBAT_FullDataTlm_t  FullDataTlm;  // Full: all 12 Data IDs raw                 
    LGBAT_ReportTlm_t    ReportTlm;    // RPT: one per ground command               
    LGBAT_CriticalTlm_t  CriticalTlm;  // Critical BMS alert                        

    // BMS data cache — populated by I2C reads 
    LGBAT_BmsAllData_t BmsData;

  
    bool PowerApplied;       // True when 3.3V is applied (SET_POWER_CC PowerOn=1) 
    bool MissionActive;      // True while within 2-week mission window            
    bool FirstCommSuccess;   // True after first full I2C cycle succeeds           

    // Mission timer 
    CFE_TIME_SysTime_t MissionStartTime;

} LGBAT_Data_t;

extern LGBAT_Data_t LGBAT_Data;

void         LGBAT_Main(void);
CFE_Status_t LGBAT_Init(void);

#endif /* LGBAT_TASK_H */
