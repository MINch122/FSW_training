
#ifndef LGBAT_EVENTIDS_H
#define LGBAT_EVENTIDS_H

// Defining event ids - nums using in sending the log message through cFS Event Services
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
#define LGBAT_TBL_ERR_EID            10
#define LGBAT_SEM_INIT_ERR_EID       11
#define LGBAT_MUT_INIT_ERR_EID       12
#define LGBAT_CHILD_CREATE_ERR_EID   13


// for the I2C communication
#define LGBAT_I2C_WRITE_ERR_EID      20
#define LGBAT_I2C_READ_ERR_EID       21
#define LGBAT_I2C_CHECKSUM_ERR_EID   22
#define LGBAT_I2C_TIMEOUT_ERR_EID    23

// for the BMS state related
#define LGBAT_BMS_WAKEUP_INF_EID     30
#define LGBAT_BMS_SLEEP_INF_EID      31
#define LGBAT_BMS_FAILURE_ERR_EID    32
#define LGBAT_BMS_LOW_SOC_ERR_EID    33
#define LGBAT_BMS_OVERVOLT_ERR_EID   34
#define LGBAT_BMS_OVERTEMP_ERR_EID   35

// for the mission timer related
#define LGBAT_MISSION_START_INF_EID  40
#define LGBAT_MISSION_END_INF_EID    41

#endif /* LGBAT_EVENTIDS_H */
