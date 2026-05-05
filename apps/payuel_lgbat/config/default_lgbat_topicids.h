#ifndef LGBAT_TOPICIDS_H
#define LGBAT_TOPICIDS_H

// Command Topic IDs (Ground/SCH -> OBC)
#define CFE_MISSION_LGBAT_CMD_TOPICID               0xC6  // 0x18C6 Ground command
#define CFE_MISSION_LGBAT_WAKEUP_TOPICID            0xC7  // 0x18C7 SCH -> I2C poll trigger
#define CFE_MISSION_LGBAT_SEND_BCN_TOPICID          0xC8  // 0x18C8 SCH -> beacon send request

// Telemetry Topic IDs (OBC -> Ground)
#define CFE_MISSION_LGBAT_FULLDATA_TLM_TOPICID      0xC6  // 0x08C6 Full BMS data telemetry
#define CFE_MISSION_LGBAT_REPORT_TOPICID            0xC7  // 0x08C7 Command report
#define CFE_MISSION_LGBAT_BCN_TLM_TOPICID          0xC8  // 0x08C8 Beacon telemetry
#define CFE_MISSION_LGBAT_CRITICAL_TOPICID          0xC9  // 0x08C9 Critical BMS alert

#endif /* LGBAT_TOPICIDS_H */
