#ifndef SP_FCNCODES_H
#define SP_FCNCODES_H

#define SP_NOOP_CC              0
#define SP_RESET_COUNTERS_CC    1
#define SP_GET_HK_CC            2  /**< Retrieve AR6 status from both DSPs */
#define SP_DEPLOY_CC            3  /**< Trigger burn-wire on specified DSP (both AR6 boards) */
#define SP_STOP_BURN_CC         4  /**< Stop burn on specified DSP */
#define SP_AUTO_DEPLOY_CC       5  /**< Auto deploy both DSPs with retry logic */
#define SP_SCAN_AR6_CC          6  /**< Scan I2C1 bus for AR6 devices; logs found addresses via events */
#define SP_SET_AR6_ADDR_CC      7  /**< Reprogram AR6 I2C address and commit to NVM */
#define SP_REPORT_BCN_CC        8  /**< Send cached BCN data as individual report */

#endif