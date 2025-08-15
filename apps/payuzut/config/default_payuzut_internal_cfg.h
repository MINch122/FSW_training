#ifndef PAYUZUT_INTERNAL_CFG_H
#define PAYUZUT_INTERNAL_CFG_H

#include "common_types.h"


#define PAYUZUT_PIPE_DEPTH  50 /* Depth of the Command Pipe for Application */

/**
 * Temperature file name
 * Corresponded to each ADC
 */
#define PAYUZUT_TEMPERATURE_FILE_1    "./cf/sdcard/PAYUZUT_WOLFERL"
#define PAYUZUT_TEMPERATURE_FILE_2    "./cf/sdcard/PAYUZUT_AMADE"

/**
 * ADC converter slave Address
 */
#define PAYUZUT_ADC_SLAVE_ADDR_1      0x49  //1001001b
#define PAYUZUT_ADC_SLAVE_ADDR_2      0x48  //1001000b

/**
 * ADC Register Address
 */
#define PAYUZUT_ADC_CONV_REG_ADDR       0x00 // 0b00000000 : Conversion Register -> ADC return the converted value
#define PAYUZUT_ADC_CONF_REG_ADDR       0x01 // 0b00000001 : Config Register, need two more data bytes


/*****************************************************************************************
 * ADC Register Configuration
 ******************************************************************************************/
#define ADC_CFG_MSB(OS, MUX, PGA, MODE) \
        (  (((OS)&1u) << 7) | (((MUX)&7u) << 4) | (((PGA)&7u) << 1) | ((MODE)&1u)  )

#define ADC_CFG_LSB(DR, COMP_MODE, COMP_POL, COMP_LAT, COMP_QUE) \
        (  (((DR)&7u) << 5) | (((COMP_MODE)&1u) << 4) \
        | (((COMP_POL)&1u) << 3) | (((COMP_LAT)&1u) << 2) | ((COMP_QUE)&3u)  )
        
#define PAYUZUT_CONF_MSB            ADC_CFG_MSB(1, 4, 1, 1)     //0b11000011 : 1 + 100 + 001 + 1
#define PAYUZUT_CONF_LSB            ADC_CFG_LSB(0, 0, 0, 0, 3)  //0b00000011 : 000 + 0 + 0 + 0 + 11

/* Assertion for build time */
#if PAYUZUT_CONF_MSB != 0xC3u
#error "ADC config MSB mismatch"
#endif
#if PAYUZUT_CONF_LSB != 0x03u
#error "ADC config LSB mistmatch"
#endif

#define PAYUZUT_ADC_POLL_MSEC       9   // milliseconds polling time

/**
 * Temperature Task Configuration
 */
#define PAYUZUT_TEMP_TASK_STACK_SIZE        4096
#define PAYUZUT_TEMP_TASK_STACK_PRIORITY    150
#define PAYUZUT_TEMP_GATHER_TIME            30      // sec
#define PAYUZUT_TEMP_GATHER_TERM            1000   // milli sec

#endif
