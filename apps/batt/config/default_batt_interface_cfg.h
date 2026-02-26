/**
 * @file
 *   BATT Application Public Definitions
 *
 * Configurable items for the NanoPower BP8 battery pack interface.
 */
#ifndef BATT_INTERFACE_CFG_H
#define BATT_INTERFACE_CFG_H

/**
 * \brief Default CSP node address for the NanoPower BP8
 *
 * The BP8 default address is 7 (from board table parameter 'addr').
 * This can be overridden per mission.
 */
#define BATT_BP8_CSP_NODE  7

/**
 * \brief BP8 Parameter Table IDs
 *
 * As defined in the NanoPower BP8 user manual (Section 3).
 */
#define BATT_BP8_TABLE_BOARD          0
#define BATT_BP8_TABLE_CONFIGURATION  1
#define BATT_BP8_TABLE_CALIBRATION    2
#define BATT_BP8_TABLE_CONTROL        3
#define BATT_BP8_TABLE_TELEMETRY      4

/*
** BP8 Telemetry Table Parameter Addresses (Table 3.7 in manual)
*/
#define BATT_BP8_TLM_UPTIME       0x00  /* uint32, seconds */
#define BATT_BP8_TLM_BOOTCOUNT    0x04  /* uint16 */
#define BATT_BP8_TLM_BOOTCAUSE    0x06  /* uint16 */
#define BATT_BP8_TLM_RESETCAUSE   0x08  /* uint16 */
#define BATT_BP8_TLM_SOC          0x0C  /* float, 0.0=empty 1.0=full */
#define BATT_BP8_TLM_INT_TEMP     0x10  /* int16, ddegC */
#define BATT_BP8_TLM_BAT_AVR_TEMP 0x14  /* float, degC */
#define BATT_BP8_TLM_BAT_1_TEMP   0x18  /* int16, ddegC */
#define BATT_BP8_TLM_BAT_2_TEMP   0x1A  /* int16, ddegC */
#define BATT_BP8_TLM_BAT_3_TEMP   0x1C  /* int16, ddegC */
#define BATT_BP8_TLM_BAT_4_TEMP   0x1E  /* int16, ddegC */
#define BATT_BP8_TLM_VBAT         0x20  /* uint16, mV */
#define BATT_BP8_TLM_I            0x24  /* float, A */
#define BATT_BP8_TLM_IN_I         0x28  /* uint16, mA */
#define BATT_BP8_TLM_HEATER_I     0x2A  /* uint16, mA */
#define BATT_BP8_TLM_OUT_I        0x2C  /* uint16, mA */
#define BATT_BP8_TLM_O_VOLT_COUNT 0x2E  /* uint16 */
#define BATT_BP8_TLM_BAT_FAULT    0x30  /* bool */

/*
** BP8 Control Table Parameter Addresses (Table 3.6 in manual)
*/
#define BATT_BP8_CTRL_SOC_RESET    0x00  /* bool */
#define BATT_BP8_CTRL_FAULT_RESET  0x01  /* bool */
#define BATT_BP8_CTRL_FUSE_BURN    0x02  /* string, max 16 */
#define BATT_BP8_CTRL_HEAT_MANUAL  0x12  /* uint16, seconds (1-600) */

/*
** BP8 Configuration Table Parameter Addresses (Table 3.5 in manual)
*/
#define BATT_BP8_CFG_AUTO_HEAT_EN  0x00  /* bool */
#define BATT_BP8_CFG_LOW_TEMP_LIM  0x02  /* int16, ddegC */
#define BATT_BP8_CFG_HIGH_TEMP_LIM 0x04  /* int16, ddegC */
#define BATT_BP8_CFG_VBAT_CRITICAL 0x08  /* uint32, mV */
#define BATT_BP8_CFG_FUSE_BURN_EN  0x0C  /* bool */

#endif
