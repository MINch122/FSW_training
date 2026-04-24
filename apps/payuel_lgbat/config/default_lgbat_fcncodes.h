#ifndef LGBAT_FCNCODES_H
#define LGBAT_FCNCODES_H


#define LGBAT_NOOP_CC                0   // No-op 
#define LGBAT_RESET_COUNTER_CC       1   // Reset CmdCounter and ErrCounter            
#define LGBAT_SEND_BCN_CC            2   // Send beacon TLM       
#define LGBAT_REQUEST_DATA_CC        3   // Read one DataID (0x01–0x0C) 
#define LGBAT_REQUEST_ALL_DATA_CC    4   // Read all 12 DataIDs + transmit FullData TLM
#define LGBAT_SET_POWER_CC           5   // Control 3.3V power (PowerOn=1/0)           
#define LGBAT_RESET_BMS_CC           6   // Reset cached BMS data and power state      

#endif /* LGBAT_FCNCODES_H */
