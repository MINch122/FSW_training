#ifndef LGBAT_EVENTIDS_H
#define LGBAT_EVENTIDS_H

// Event IDs used with CFE_EVS_SendEvent


// General application events
#define LGBAT_RESERVED_EID           0
#define LGBAT_INIT_INF_EID           1  
#define LGBAT_CC_ERR_EID             2  
#define LGBAT_MID_ERR_EID            3 
#define LGBAT_CMD_LEN_ERR_EID        4  
#define LGBAT_PIPE_ERR_EID           5  
#define LGBAT_CR_PIPE_ERR_EID        6  
#define LGBAT_SUB_CMD_ERR_EID        7  
#define LGBAT_SUB_BCN_ERR_EID        8  
#define LGBAT_SUB_WAKEUP_ERR_EID     9  

// I2C communication events
#define LGBAT_I2C_WRITE_ERR_EID      20 
#define LGBAT_I2C_READ_ERR_EID       21 
#define LGBAT_I2C_CHECKSUM_ERR_EID   22 
#define LGBAT_I2C_TIMEOUT_ERR_EID    23 

// BMS state events
#define LGBAT_BMS_WAKEUP_INF_EID     30
#define LGBAT_BMS_SLEEP_INF_EID      31 
#define LGBAT_BMS_FAILURE_ERR_EID    32 
#define LGBAT_BMS_LOW_SOC_ERR_EID    33 

// Mission timer events
#define LGBAT_MISSION_START_INF_EID  40 
#define LGBAT_MISSION_END_INF_EID    41 

#endif // LGBAT_EVENTIDS_H
