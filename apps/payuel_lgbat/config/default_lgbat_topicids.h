#ifndef LGBAT_TOPICIDS_H
#define LGBAT_TOPICIDS_H



// Command Topic IDs (Ground → OBC) 
#define CFE_MISSION_LGBAT_CMD_TOPICID          0xA0  // Ground command 
#define CFE_MISSION_LGBAT_SEND_BCN_TOPICID     0xA1  // SCH → beacon send request          
#define CFE_MISSION_LGBAT_WAKEUP_TOPICID       0xA2  // SCH → periodic I2C polling trigger  

// Telemetry Topic IDs (OBC → Ground) 
#define CFE_MISSION_LGBAT_BCN_TLM_TOPICID      0xB0  // Beacon telemetry 
#define CFE_MISSION_LGBAT_FULLDATA_TLM_TOPICID 0xB1  // Full BMS data
#define CFE_MISSION_LGBAT_REPORT_TOPICID        0xB2  
#define CFE_MISSION_LGBAT_CRITICAL_TOPICID      0xB3  

#endif /* LGBAT_TOPICIDS_H */
