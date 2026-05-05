#ifndef LGBAT_TOPICIDS_H
#define LGBAT_TOPICIDS_H

// PAYUEL_LGBAT Topic ID definitions


// Command Topic IDs (Ground/SCH -> OBC)
#define CFE_MISSION_LGBAT_CMD_TOPICID          0xC6  // 0x18C6 Ground command
#define CFE_MISSION_LGBAT_SEND_HK_TOPICID      0xC7  // 0x18C7 SCH -> HK/I2C poll trigger
#define CFE_MISSION_LGBAT_SEND_BCN_TOPICID     0xC8  // 0x18C8 SCH -> beacon send request

// Telemetry Topic IDs (OBC -> Ground)
#define CFE_MISSION_LGBAT_HK_TLM_TOPICID       0xC6  // 0x08C6 Housekeeping telemetry
#define CFE_MISSION_LGBAT_REPORT_TOPICID        0xC7  // 0x08C7 Command report
#define CFE_MISSION_LGBAT_BCN_TLM_TOPICID      0xC8  // 0x08C8 Beacon telemetry

#endif // LGBAT_TOPICIDS_H
