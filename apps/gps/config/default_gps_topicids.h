/**
 * @file  GPS application topic IDs
 */
#ifndef DEFAULT_GPS_TOPICIDS_H
#define DEFAULT_GPS_TOPICIDS_H

#define CFE_MISSION_GPS_CMD_TOPICID         0x94  /* MID 0x1894 */
#define CFE_MISSION_GPS_SEND_HK_TOPICID     0x95  /* MID 0x1895 */
#define CFE_MISSION_GPS_HK_TLM_TOPICID      0x94  /* MID 0x0894 */
#define CFE_MISSION_GPS_REPORT_TLM_TOPICID  0x95  /* MID 0x0895 */

/**
 * OEM7 stored-log telemetry. One packet per log, published by the receive
 * task's callbacks. Block 0x9A - 0xA1 was free in the mission's telemetry
 * topic allocation; see apps/ *​/config/default_*_topicids.h.
 */
#define CFE_MISSION_GPS_OEM_BESTXYZ_TLM_TOPICID     0x9A  /* MID 0x089A */
#define CFE_MISSION_GPS_OEM_BESTPOS_TLM_TOPICID     0x9B  /* MID 0x089B */
#define CFE_MISSION_GPS_OEM_RANGE_TLM_TOPICID       0x9C  /* MID 0x089C */
#define CFE_MISSION_GPS_OEM_TIME_TLM_TOPICID        0x9D  /* MID 0x089D */
#define CFE_MISSION_GPS_OEM_CLOCKMODEL_TLM_TOPICID  0x9E  /* MID 0x089E */
#define CFE_MISSION_GPS_OEM_HWMONITOR_TLM_TOPICID   0x9F  /* MID 0x089F */
#define CFE_MISSION_GPS_OEM_RXSTATUS_TLM_TOPICID    0xA0  /* MID 0x08A0 */
#define CFE_MISSION_GPS_OEM_SATVIS2_TLM_TOPICID     0xA1  /* MID 0x08A1 */

#endif
