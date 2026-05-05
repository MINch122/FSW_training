#ifndef LGBAT_EVENTIDS_H
#define LGBAT_EVENTIDS_H

// Event IDs used with CFE_EVS_SendEvent


// General application events
#define LGBAT_RESERVED_EID           0
#define LGBAT_INIT_INF_EID           1  // App initialized successfully
#define LGBAT_CC_ERR_EID             2  // Unknown command code received
#define LGBAT_MID_ERR_EID            3  // Unknown message ID received
#define LGBAT_CMD_LEN_ERR_EID        4  // Command message length mismatch
#define LGBAT_PIPE_ERR_EID           5  // Software bus pipe read error
#define LGBAT_CR_PIPE_ERR_EID        6  // Failed to create SB pipe
#define LGBAT_SUB_CMD_ERR_EID        7  // Failed to subscribe to CMD_MID
#define LGBAT_SUB_BCN_ERR_EID        8  // Failed to subscribe to SEND_BCN_MID
#define LGBAT_SUB_WAKEUP_ERR_EID     9  // Failed to subscribe to SEND_HK_MID

// I2C communication events
#define LGBAT_I2C_WRITE_ERR_EID      20 // I2C2 write operation failed
#define LGBAT_I2C_READ_ERR_EID       21 // I2C2 read operation failed
#define LGBAT_I2C_CHECKSUM_ERR_EID   22 // XOR checksum mismatch
#define LGBAT_I2C_TIMEOUT_ERR_EID    23 // I2C2 transaction timed out

// BMS state events
#define LGBAT_BMS_POWER_ON_INF_EID   30 // 3.3V power applied
#define LGBAT_BMS_POWER_OFF_INF_EID  31 // 3.3V power removed
#define LGBAT_BMS_FAILURE_ERR_EID    32 // BMS fault or warning detected
#define LGBAT_BMS_LOW_SOC_ERR_EID    33 // SOC dropped below sleep threshold

// Mission timer events
#define LGBAT_MISSION_START_INF_EID  40 // First successful full I2C cycle
#define LGBAT_MISSION_END_INF_EID    41 // 2-week mission duration exceeded

#endif // LGBAT_EVENTIDS_H
