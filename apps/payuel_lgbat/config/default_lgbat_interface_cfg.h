#ifndef LGBAT_INTERFACE_CFG_H
#define LGBAT_INTERFACE_CFG_H

#include "common_types.h"
#include "cfe.h"

// I2C2 interface settings

#define LGBAT_I2C_HANDLE_INDEXER     CFE_SRL_I2C2_HANDLE_INDEXER 
#define LGBAT_BMS_I2C_SLAVE_ADDR     0x1F  
#define LGBAT_BMS_I2C_WRITE_BYTE     0x3E   
#define LGBAT_BMS_I2C_READ_BYTE      0x3F    
#define LGBAT_BMS_I2C_FREQ_HZ        400000  
#define LGBAT_I2C_TIMEOUT_MS         500     

// Power interface

#define LGBAT_POWER_CHANNEL          21
#define LGBAT_POWER_CONVERTER        0
#define LGBAT_SUPPLY_VOLTAGE_V       3.3f
#define LGBAT_SUPPLY_CURRENT_A       1.0f

// BMS Data ID range defined by ICD
#define LGBAT_BMS_DATA_ID_MIN        0x01
#define LGBAT_BMS_DATA_ID_MAX        0x0C
#define LGBAT_BMS_DATA_ID_COUNT      12
#define LGBAT_BMS_BYTES_PER_ID       10  

// Cell specifications: INR21700M52V (2S1P configuration)
#define LGBAT_CELL_NOMINAL_VOLTAGE_MV    3690
#define LGBAT_CELL_MAX_CHARGE_MV         4200
#define LGBAT_CELL_MIN_DISCHARGE_MV      2500
#define LGBAT_CELL_CAPACITY_MAH          5000
#define LGBAT_CELL_MAX_CHARGE_MA         3500
#define LGBAT_CELL_MAX_DISCHARGE_MA      15000

// Pack properties: 2S1P
#define LGBAT_PACK_NOMINAL_VOLTAGE_MV    (2 * LGBAT_CELL_NOMINAL_VOLTAGE_MV)
#define LGBAT_PACK_MAX_VOLTAGE_MV        8200
#define LGBAT_PACK_DESIGN_CAPACITY_MAH   5000

// BMS power consumption from ICD electrical characteristics table
#define LGBAT_BMS_SLEEP_CURRENT_UA       205    // Leakage current in sleep (uA)
#define LGBAT_BMS_ACTIVE_CURRENT_MA      90     // Active mode current (mA)
#define LGBAT_BMS_IDLE_POWER_MW          670    // Idle mode power (mW)
#define LGBAT_BMS_TRANSMIT_POWER_W_MAX   3      // Max transmit power (W)

#endif // LGBAT_INTERFACE_CFG_H
