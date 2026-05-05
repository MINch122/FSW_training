#ifndef LGBAT_MSGDEFS_H
#define LGBAT_MSGDEFS_H

#include "common_types.h"
#include "lgbat_fcncodes.h"
#include "rpt_interface_cfg.h"

typedef struct {
    uint8_t  ID;
    uint16_t Pack_Voltage;
    int16_t  Pack_Current;
    uint16_t Average_Time_To_Empty;
    uint16_t Average_Time_To_Full;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x01_t;

typedef struct {
    uint8_t  ID;
    uint8_t  Power_Supply_Status;
    uint8_t  SOH;
    uint16_t SOC;
    uint16_t RC_Remaining_Capacity;
    uint16_t AE_Available_Energy;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x02_t;

typedef struct {
    uint8_t  ID;
    uint16_t System_Max_Voltage;
    uint8_t  Power_Supply_Health;
    uint16_t FETTestRequiredVoltage;
    uint8_t  Battery_Information_Reserved[3];
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x03_t;

typedef struct {
    uint8_t  ID;
    uint16_t Percentage;
    uint16_t Design_Capacity;
    uint16_t Capacity;
    uint16_t Charge;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x04_t;

typedef struct {
    uint8_t  ID;
    uint16_t Cell_Voltage_Max;
    uint16_t Cell_Voltage_Min;
    int16_t  Cell_Temperature_Max;
    int16_t  Cell_Temperature_Min;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x05_t;

typedef struct {
    uint8_t  ID;
    uint16_t Cell_Voltage_01;
    uint16_t Cell_Voltage_02;
    uint16_t Reserved_01;
    uint16_t Reserved_02;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x06_t;

/* Data ID 0x07 ICD byte order: byte[1-2]=T02, byte[3-4]=T01 */
typedef struct {
    uint8_t  ID;
    int16_t  Cell_Temperature_02;
    int16_t  Cell_Temperature_01;
    int16_t  Balancing_R_Temperature;
    int16_t  PreCharge_R_Temperature;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x07_t;

typedef struct {
    uint8_t  ID;
    int16_t  FET_Down_Temperature;
    int16_t  FET_Up_Temperature;
    uint16_t CtrlCBStatus;
    uint8_t  Temperature_002_Reserved;
    uint8_t  SoftVersion;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x08_t;

typedef struct {
    uint8_t  ID;
    uint8_t  BMS_Wakeup;
    uint8_t  WakeupHoldStatus;
    uint8_t  VoltCurrDiag;
    uint8_t  TempFailLevel;
    uint8_t  FETStatus;
    uint8_t  Reserved[3];
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x09_t;

#define LGBAT_0x09_GET_WAKEUP_HOLD_MCU(b)   (((b) >> 4) & 0x0F)
#define LGBAT_0x09_GET_VOLTAGE_DIAG(b)      (((b) >> 4) & 0x0F)
#define LGBAT_0x09_GET_CURRENT_DIAG(b)      (((b) >> 0) & 0x0F)
#define LGBAT_0x09_GET_TEMP_DIAG(b)         (((b) >> 4) & 0x0F)
#define LGBAT_0x09_GET_FAILURE_LEVEL(b)     (((b) >> 0) & 0x0F)
#define LGBAT_0x09_GET_DFET(b)              (((b) >> 5) & 0x01)
#define LGBAT_0x09_GET_CFET(b)              (((b) >> 4) & 0x01)

typedef struct {
    uint8_t  ID;
    uint8_t  FailStatus2;
    uint8_t  FailStatus3;
    uint8_t  Reserved[6];
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x0A_t;

#define LGBAT_0x0A_GET_TIMEOUT_PRECHARGE(b)  (((b) >> 7) & 0x01)
#define LGBAT_0x0A_GET_OVERTEMP_PRECHARGE(b) (((b) >> 6) & 0x01)
#define LGBAT_0x0A_GET_OVERTEMP_CB(b)        (((b) >> 5) & 0x01)
#define LGBAT_0x0A_GET_AFE_COMMLOSS(b)       (((b) >> 4) & 0x01)
#define LGBAT_0x0A_GET_AFE_SHUTDOWN(b)       (((b) >> 3) & 0x01)
#define LGBAT_0x0A_GET_TEMPOPEN_CELL(b)      (((b) >> 2) & 0x01)
#define LGBAT_0x0A_GET_TEMPOPEN_CB(b)        (((b) >> 1) & 0x01)
#define LGBAT_0x0A_GET_TEMPOPEN_FET(b)       (((b) >> 0) & 0x01)
#define LGBAT_0x0A_GET_DFET_FAULT(b)         (((b) >> 7) & 0x01)
#define LGBAT_0x0A_GET_OVERTEMP_CDFET(b)     (((b) >> 5) & 0x01)
#define LGBAT_0x0A_ANY_FAIL2(b)              ((b) != 0x00)
#define LGBAT_0x0A_ANY_FAIL3(b)              (((b) & 0xA0) != 0x00)

typedef struct {
    uint8_t  ID;
    uint16_t BOOST_Out_Voltage;
    uint16_t MCU_B_Plus_Volt;
    uint16_t MCU_P_Plus_Volt;
    uint16_t MCU_BMIC_REG_Out_Volt;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x0B_t;

typedef struct {
    uint8_t  ID;
    int16_t  BMS_Current;
    int16_t  PCB_Temperature;
    int16_t  BOOST_Temperature;
    uint16_t MCU_SBC_AMUX_Voltage;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x0C_t;

typedef struct {
    LGBAT_BmsData0x01_t Data01;
    LGBAT_BmsData0x02_t Data02;
    LGBAT_BmsData0x03_t Data03;
    LGBAT_BmsData0x04_t Data04;
    LGBAT_BmsData0x05_t Data05;
    LGBAT_BmsData0x06_t Data06;
    LGBAT_BmsData0x07_t Data07;
    LGBAT_BmsData0x08_t Data08;
    LGBAT_BmsData0x09_t Data09;
    LGBAT_BmsData0x0A_t Data0A;
    LGBAT_BmsData0x0B_t Data0B;
    LGBAT_BmsData0x0C_t Data0C;
} LGBAT_BmsAllData_t;


/* HK Telemetry Payload (MID 0x08C6) */
typedef struct {
    uint8_t  CmdCounter;
    uint8_t  CmdErrCounter;
    uint8_t  PowerApplied;
    uint8_t  MissionActive;
    uint8_t  FirstCommDone;
    uint8_t  Spare[3];
    uint16_t Pack_Voltage_mV;
    int16_t  Pack_Current_mA;
    uint16_t SOC_x100;
    uint8_t  SOH_pct;
    uint8_t  Power_Supply_Status;
    uint8_t  Failure_Level;
    uint8_t  FailStatus2_Raw;
    uint8_t  FailStatus3_Raw;
} __attribute__((packed)) LGBAT_HkTlm_Payload_t;


/* Beacon Telemetry Payload (MID 0x08C8) */
typedef struct {
    uint8_t  CmdCounter;
    uint8_t  CmdErrCounter;
    uint8_t  PowerApplied;
    uint8_t  Spare;

    uint16_t Pack_Voltage_mV;
    int16_t  Pack_Current_mA;
    uint16_t Avg_Time_To_Empty_sec;
    uint16_t Avg_Time_To_Full_sec;

    uint16_t SOC_x100;
    uint8_t  SOH_pct;
    uint8_t  Power_Supply_Status;
    uint16_t RC_Remaining_Cap;
    uint16_t AE_Available_Energy;

    uint16_t System_Max_Voltage;
    uint8_t  Power_Supply_Health;
    uint8_t  Spare2;
    uint16_t FETTestRequiredVoltage;

    uint16_t Charge_Percentage;
    uint16_t Design_Capacity;
    uint16_t Capacity;
    uint16_t Charge;

    uint16_t Cell_Voltage_Max_mV;
    uint16_t Cell_Voltage_Min_mV;
    int16_t  Cell_Temp_Max_x10;
    int16_t  Cell_Temp_Min_x10;

    uint16_t Cell_Voltage_01_mV;
    uint16_t Cell_Voltage_02_mV;

    /* ICD 0x07: byte[1-2]=T02 stored first, byte[3-4]=T01 stored second */
    int16_t  Cell_Temp_02_x10;
    int16_t  Cell_Temp_01_x10;
    int16_t  Balancing_R_Temp_x10;
    int16_t  PreCharge_R_Temp_x10;

    int16_t  FET_Down_Temp_x10;
    int16_t  FET_Up_Temp_x10;
    uint16_t CtrlCBStatus;
    uint8_t  SoftVersion;
    uint8_t  Spare3;

    uint8_t  Failure_Level;
    uint8_t  DFET_Status;
    uint8_t  CFET_Status;
    uint8_t  Voltage_Diag;
    uint8_t  Current_Diag;
    uint8_t  Temp_Diag;

    uint8_t  FailStatus2_Raw;
    uint8_t  FailStatus3_Raw;

    uint16_t BOOST_Out_Voltage_mV;
    uint16_t MCU_B_Plus_Volt_mV;
    uint16_t MCU_P_Plus_Volt_mV;
    uint16_t MCU_BMIC_REG_Out_Volt_mV;

    int16_t  BMS_Current_mA;
    int16_t  PCB_Temperature_x10;
    int16_t  BOOST_Temperature_x10;
    uint16_t MCU_SBC_AMUX_Voltage_mV;

} __attribute__((packed)) LGBAT_BcnTlm_Payload_t;


/* Critical Alert Telemetry Payload */
typedef struct {
    uint8_t  FailureLevel;
    uint8_t  VoltageDiag;
    uint8_t  CurrentDiag;
    uint8_t  TempDiag;
    uint8_t  FailStatus2_Raw;
    uint8_t  FailStatus3_Raw;
} __attribute__((packed)) LGBAT_CriticalTlm_Payload_t;

#define LGBAT_CRITICAL_ALERT_CC  0xFE

#endif /* LGBAT_MSGDEFS_H */
