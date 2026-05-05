#ifndef LGBAT_INTERNAL_CFG_H
#define LGBAT_INTERNAL_CFG_H

// Software Bus pipe depth
#define LGBAT_PIPE_DEPTH    8

// Mission duration limit: max 2 weeks per ICD
#define LGBAT_MAX_MISSION_DURATION_SEC   (14u * 24u * 3600u)   // 1,209,600 seconds

// BMS Power_Supply_Status field values (Data ID 0x02)
#define LGBAT_BMS_STATUS_REST        0x00
#define LGBAT_BMS_STATUS_CHARGE      0x01
#define LGBAT_BMS_STATUS_DISCHARGE   0x02

// Failure level values extracted from Data ID 0x09 TempFailLevel[3:0]
#define LGBAT_BMS_FAILURE_NONE       0x00
#define LGBAT_BMS_FAILURE_WARNING    0x01
#define LGBAT_BMS_FAILURE_FAULT      0x02
#define LGBAT_BMS_FAILURE_CRITICAL   0x03

// SOC low threshold: BMS will enter sleep when SOC < 10%
#define LGBAT_SOC_SLEEP_THRESHOLD_X100   1000

// BMS Data ID enum matching ICD Data IDs 0x01 through 0x0C
typedef enum {
    LGBAT_DATA_ID_POWER_TIME       = 0x01,
    LGBAT_DATA_ID_CAPACITY_STATUS  = 0x02,
    LGBAT_DATA_ID_BATTERY_INFO     = 0x03,
    LGBAT_DATA_ID_CHARGE_STATE     = 0x04,
    LGBAT_DATA_ID_CELL_EXTREMES    = 0x05,
    LGBAT_DATA_ID_CELL_VOLTAGE     = 0x06,
    LGBAT_DATA_ID_CELL_TEMPERATURE = 0x07,
    LGBAT_DATA_ID_FET_STATUS       = 0x08,
    LGBAT_DATA_ID_BMS_STATUS       = 0x09,
    LGBAT_DATA_ID_FAIL_STATUS      = 0x0A,
    LGBAT_DATA_ID_MCU_VOLTAGE      = 0x0B,
    LGBAT_DATA_ID_DCDC_STATUS      = 0x0C,
} LGBAT_DataID_t;

#endif // LGBAT_INTERNAL_CFG_H
