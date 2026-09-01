/**
 * @file  GPS application public definitions
 *
 * Values that affect this module's command and telemetry interface.
 */
#ifndef GPS_INTERFACE_CFG_H
#define GPS_INTERFACE_CFG_H

/**
 * Cap on the data GPS copies into RPT_Report_t.ReturnValue. The packet always
 * carries RPT_RET_VALUE_BUF_SIZE; this is how much of it GPS ever fills.
 */
#define GPS_MISSION_REPORT_DATA_SIZE        256

/* Indices into RPT_ReturnType_t. Kept as plain values so the command layer
 * does not have to pull in the RPT header. */
#define GPS_MISSION_REPORT_RETTYPE_APP      4

#define GPS_MISSION_REPORT_RETTYPE_HW       5

#endif
