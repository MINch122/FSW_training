#ifndef LGBAT_INTERFACE_CFG_H
#define LGBAT_INTERFACE_CFG_H

#include "common_types.h"
#include "cfe.h"


#define LGBAT_I2C_HANDLE_INDEXER     CFE_SRL_I2C2_HANDLE_INDEXER  
#define LGBAT_BMS_I2C_SLAVE_ADDR     0x1F    // BMS Slave Address (31 decimal)            
#define LGBAT_BMS_I2C_WRITE_BYTE     0x3E    // SlaveAddr(0x1F) + Write(0)               
#define LGBAT_BMS_I2C_READ_BYTE      0x3F    // SlaveAddr(0x1F) + Read(1)                
#define LGBAT_BMS_I2C_FREQ_HZ        400000  // 400 kHz per ICD                          
#define LGBAT_I2C_TIMEOUT_MS         500     // Per-transaction timeout (ms)             

//Power Interface
#define LGBAT_POWER_CHANNEL          21
#define LGBAT_POWER_CONVERTER        0
#define LGBAT_SUPPLY_VOLTAGE_V       3.3f
#define LGBAT_SUPPLY_CURRENT_A       1.0f   

// BMS Data ID range (ICD: 0x01 ~ 0x0C)
#define LGBAT_BMS_DATA_ID_MIN        0x01
#define LGBAT_BMS_DATA_ID_MAX        0x0C
#define LGBAT_BMS_DATA_ID_COUNT      12
#define LGBAT_BMS_BYTES_PER_ID       10

// Cell Specifications — INR21700M52V 
#define LGBAT_CELL_NOMINAL_VOLTAGE_MV    3690                        
#define LGBAT_CELL_MAX_CHARGE_MV         4200                        
#define LGBAT_CELL_MIN_DISCHARGE_MV      2500                          
#define LGBAT_CELL_CAPACITY_MAH          5000            
#define LGBAT_CELL_MAX_CHARGE_MA         3500   
#define LGBAT_CELL_MAX_DISCHARGE_MA      15000           

// Pack Properties — 2S1P configuration 
#define LGBAT_PACK_NOMINAL_VOLTAGE_MV    (2 * LGBAT_CELL_NOMINAL_VOLTAGE_MV)  
#define LGBAT_PACK_MAX_VOLTAGE_MV        8200   
#define LGBAT_PACK_DESIGN_CAPACITY_MAH   5000

// BMS Power Consumption
#define LGBAT_BMS_SLEEP_CURRENT_UA       205    
#define LGBAT_BMS_ACTIVE_CURRENT_MA      90     
#define LGBAT_BMS_IDLE_POWER_MW          670    
#define LGBAT_BMS_TRANSMIT_POWER_W_MAX   3     

#endif /* LGBAT_INTERFACE_CFG_H */
