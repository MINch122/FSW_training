#ifndef LGBAT_MSGDEFS_H
#define LGBAT_MSGDEFS_H

#include "common_types.h"
#include "lgbat_fcncodes.h"
#include "rpt_interface_cfg.h"

//BMS Data ID Structures


// 0x01: Pack Power & Time 
typedef struct {
    uint8_t  ID;
    uint16_t Pack_Voltage;
    int16_t  Pack_Current;
    uint16_t Average_Time_To_Empty;
    uint16_t Average_Time_To_Full;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x01_t;

// 0x02: Capacity & Status 
typedef struct {
    uint8_t  ID;
    uint8_t  Power_Supply_Status;
    uint8_t  SOH;
    uint16_t SOC;
    uint16_t RC_Remaining_Capacity;
    uint16_t AE_Available_Energy;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x02_t;

// 0x03: Battery Information 
typedef struct {
    uint8_t  ID;
    uint16_t System_Max_Voltage;
    uint8_t  Power_Supply_Health;
    uint16_t FETTestRequiredVoltage;
    uint8_t  Battery_Information_Reserved[3];
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x03_t;

// 0x04: Charge State 
typedef struct {
    uint8_t  ID;
    uint16_t Percentage;
    uint16_t Design_Capacity;
    uint16_t Capacity;
    uint16_t Charge;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x04_t;

// 0x05: Cell Voltage & Temperature Extremes 
typedef struct {
    uint8_t  ID;
    uint16_t Cell_Voltage_Max;
    uint16_t Cell_Voltage_Min;
    int16_t  Cell_Temperature_Max;
    int16_t  Cell_Temperature_Min;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x05_t;

// 0x06: Cell Voltages 
typedef struct {
    uint8_t  ID;
    uint16_t Cell_Voltage_01;
    uint16_t Cell_Voltage_02;
    uint16_t Reserved_01;
    uint16_t Reserved_02;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x06_t;

// 0x07: Cell Temperatures 
typedef struct {
    uint8_t  ID;
    int16_t  Cell_Temperature_02;      
    int16_t  Cell_Temperature_01;     
    int16_t  Balancing_R_Temperature;
    int16_t  PreCharge_R_Temperature;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x07_t;

// 0x08: FET Temperature & SW Version
typedef struct {
    uint8_t  ID;
    int16_t  FET_Down_Temperature;
    int16_t  FET_Up_Temperature;
    uint16_t CtrlCBStatus;
    uint8_t  Temperature_002_Reserved;
    uint8_t  SoftVersion;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x08_t;

// 0x09: BMS Status (bitfields)
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

// 0x09 bit-field extraction macros (Big-Endian bit numbering) 
#define LGBAT_0x09_GET_WAKEUP_HOLD_MCU(b)   (((b) >> 4) & 0x0F)  /* byte2 [7:4] */
#define LGBAT_0x09_GET_BMS_WAKEUP_SIGNAL(b) (((b) >> 1) & 0x01)  /* byte2 bit1  */
#define LGBAT_0x09_GET_VOLTAGE_DIAG(b)      (((b) >> 4) & 0x0F)  /* byte3 [7:4] */
#define LGBAT_0x09_GET_CURRENT_DIAG(b)      (((b) >> 0) & 0x0F)  /* byte3 [3:0] */
#define LGBAT_0x09_GET_TEMP_DIAG(b)         (((b) >> 4) & 0x0F)  /* byte4 [7:4] */
#define LGBAT_0x09_GET_FAILURE_LEVEL(b)     (((b) >> 0) & 0x0F)  /* byte4 [3:0] */
#define LGBAT_0x09_GET_DFET(b)              (((b) >> 5) & 0x01)  /* byte5 bit5  */
#define LGBAT_0x09_GET_CFET(b)              (((b) >> 4) & 0x01)  /* byte5 bit4  */

// 0x0A: Fail Status (bitfields)
typedef struct {
    uint8_t  ID;
    uint8_t  FailStatus2;
    uint8_t  FailStatus3;
    uint8_t  Reserved[6];
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x0A_t;

// 0x0A bit-field extraction macros (Big-Endian bit numbering) 
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
/* Any fault present checks */
#define LGBAT_0x0A_ANY_FAIL2(b)              ((b) != 0x00)
#define LGBAT_0x0A_ANY_FAIL3(b)              (((b) & 0xA0) != 0x00)  

// 0x0B: Boost & MCU Voltages 
typedef struct {
    uint8_t  ID;
    uint16_t BOOST_Out_Voltage;
    uint16_t MCU_B_Plus_Volt;
    uint16_t MCU_P_Plus_Volt;
    uint16_t MCU_BMIC_REG_Out_Volt;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x0B_t;

// 0x0C: BMS PCB & Boost Status 
typedef struct {
    uint8_t  ID;
    int16_t  BMS_Current;
    int16_t  PCB_Temperature;
    int16_t  BOOST_Temperature;
    uint16_t MCU_SBC_AMUX_Voltage;
    uint8_t  CheckSum;
} __attribute__((packed)) LGBAT_BmsData0x0C_t;


// Aggregate: all 12 Data IDs
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


//Critical Alert Telemetry Payload
typedef struct {
    uint8_t  FailureLevel;      
    uint8_t  VoltageDiag;       
    uint8_t  CurrentDiag;       
    uint8_t  TempDiag;          
    uint8_t  FailStatus2_Raw;   
    uint8_t  FailStatus3_Raw;   
} __attribute__((packed)) LGBAT_CriticalTlm_Payload_t;


// Beacon Telemetry Payload
typedef struct {
    // App counters 
    uint8_t  CmdCounter;
    uint8_t  CmdErrCounter;
    uint8_t  PowerApplied;           
    uint8_t  Spare;                

    // 0x01: Pack Power & Time 
    uint16_t Pack_Voltage_mV;        
    int16_t  Pack_Current_mA;       
    uint16_t Avg_Time_To_Empty_sec;
    uint16_t Avg_Time_To_Full_sec;

    // 0x02: Capacity & Status 
    uint16_t SOC_x100;              
    uint8_t  SOH_pct;              
    uint8_t  Power_Supply_Status;    
    uint16_t RC_Remaining_Cap;      
    uint16_t AE_Available_Energy;    

    // 0x04: Charge State 
    uint16_t Charge_Percentage;      
    uint16_t Design_Capacity;        
    uint16_t Capacity;               
    uint16_t Charge;                 

    // 0x05: Cell Voltage & Temp Extremes 
    uint16_t Cell_Voltage_Max_mV;
    uint16_t Cell_Voltage_Min_mV;
    int16_t  Cell_Temp_Max_x10;      
    int16_t  Cell_Temp_Min_x10;

    // 0x06: Cell Voltages 
    uint16_t Cell_Voltage_01_mV;
    uint16_t Cell_Voltage_02_mV;

    // 0x07: Cell Temperatures 
    int16_t  Cell_Temp_01_x10;                                        
    int16_t  Cell_Temp_02_x10;
    int16_t  Balancing_R_Temp_x10;
    int16_t  PreCharge_R_Temp_x10;

    // 0x08: FET Status & SW Version
    int16_t  FET_Down_Temp_x10;
    int16_t  FET_Up_Temp_x10;
    uint16_t CtrlCBStatus;
    uint8_t  SoftVersion;            
    uint8_t  Spare2;

    // 0x09: BMS Status 
    uint8_t  BMS_Wakeup;            
    uint8_t  BMS_Wakeup_Signal;      
    uint8_t  Failure_Level;         
    uint8_t  DFET_Status;           
    uint8_t  CFET_Status;           
    uint8_t  Voltage_Diag;           
    uint8_t  Current_Diag;         
    uint8_t  Temp_Diag;              

    // 0x0A: Fail Status 
    uint8_t  FailStatus2_Raw;
    uint8_t  FailStatus3_Raw;

    // 0x0B: Boost & MCU Voltages 
    uint16_t BOOST_Out_Voltage_mV;
    uint16_t MCU_B_Plus_Volt_mV;
    uint16_t MCU_P_Plus_Volt_mV;
    uint16_t MCU_BMIC_REG_Out_Volt_mV;

    // 0x0C: PCB Current & Boost Status 
    int16_t  BMS_Current_mA;         
    int16_t  PCB_Temperature_x10;   
    int16_t  BOOST_Temperature_x10;  
    uint16_t MCU_SBC_AMUX_Voltage_mV;

} __attribute__((packed)) LGBAT_BcnTlm_Payload_t;

#endif /* LGBAT_MSGDEFS_H */
