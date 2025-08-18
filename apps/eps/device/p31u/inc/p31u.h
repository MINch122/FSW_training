/**
 * @file p31u.h
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief GomSpace NanoPower P31u Power System driver implementation.
 *        Reference: NanoPower P31u Manual Rev. 10
 * Last modified: 2025-07-27
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _GOMSPACE_P31U_H_
#define _GOMSPACE_P31U_H_
  
#include "p31u_device_config.h"

#include <stdint.h>
#include <stddef.h>

/**
 * Driver function return codes.
 */
typedef enum {
    P31U_OK         =  0, /* Operation successful. */
    P31U_ERR_NULL   = -1, /* Null pointer passed. */
    P31U_ERR_WRITE  = -2, /* Write failed. */
    P31U_ERR_READ   = -3, /* Read failed. */
    P31U_ERR_PORT   = -4, /* Echoed port mismatch (slave I2C only). */
    P31U_ERR_XFER   = -5, /* Transaction failed. */
    P31U_ERR_SIZE   = -6, /* Too large tx/rx size specified (slave I2C only). */

    /**
     * When operating in I2C slave mode, the P31u returns a custom 8-bit Error
     * Code whose detailed behavior is apparently undocumented (thanks, GomSpace).
     * 
     * A nonzero Error Code indicates that the remaining reply is undefined and
     * should not be processed.
     * 
     * ┌───────────────────────────┬──────────────┬──────────────┐
     * │          [31:16]          │    [15:8]    │    [7:0]     │
     * │         Reserved          │  Error Code  │ P31U_ERR_XXX │
     * └───────────────────────────┴──────────────┴──────────────┘
     * 
     * This Error Code, if any, is embedded in the second LSB [15:8] of the  
     * driver return values. The following placeholder simply marks the minimum
     * return value for an error-bearing response in this sense.
     */
    P31U_ERR_I2CREP_MIN = -0x0000FF00,

} p31u_ret_t;


/**
 * P31u command ports.
 */
typedef enum {
/*  Mnemonic                      Port    Request (req)    Reply            */
    P31U_PORT_HK                =  8,  /* none             p31u_hkparam_t   */
                                       /* u8 type = 0      p31u_hk_t        */
                                       /* u8 type = 1      p31u_hk_vi_t     */
                                       /* u8 type = 2      p31u_hk_out_t    */
                                       /* u8 type = 3      p31u_hk_wdt_t    */
                                       /* u8 type = 4      p31u_hk_basic_t  */
    P31U_PORT_SET_OUTPUT        =  9,  /* u8 output_byte   none             */
    P31U_PORT_SET_SINGLE_OUTPUT = 10,  /* set_single_out   none             */
    P31U_PORT_SET_PV_VOLT       = 11,  /* set_pv_volt      none             */
    P31U_PORT_SET_PV_AUTO       = 12,  /* set_pv_auto      none             */
    P31U_PORT_SET_HEATER        = 13,  /* set_heater       reply_set_heater */
    P31U_PORT_RESET_COUNTERS    = 15,  /* u8 magic = 0x42  none             */
    P31U_PORT_RESET_WDT         = 16,  /* u8 magic = 0x78  none             */
    P31U_PORT_CONFIG_CMD        = 17,  /* u8 cmd           none             */
    P31U_PORT_CONFIG_GET        = 18,  /* none             p31u_config_t    */
    P31U_PORT_CONFIG_SET        = 19,  /* p31u_config_t    none             */
    P31U_PORT_HARD_RESET        = 20,  /* none             none             */
    P31U_PORT_CONFIG2_CMD       = 21,  /* u8 cmd           none             */
    P31U_PORT_CONFIG2_GET       = 22,  /* none             p31u_config2_t   */
    P31U_PORT_CONFIG2_SET       = 23,  /* p31u_config2_t   none             */
    P31U_PORT_CONFIG3           = 25,  /* p31u_config3_t   none             */
} p31u_port_t;


/**
 * Reply structure definition:
 */

/* 44 bytes. */
typedef struct {
    uint16_t pv[3]; // Photo-voltaic input voltage [mV]
    uint16_t pc;    // Total photo current [mA]
    uint16_t bv;    // Battery voltage [mV]
    uint16_t sc;    // Total system current [mA]
    int16_t  temp[4];       // Temp. of boost converters (1,2,3) 
                            //       and onboard battery [degC]
    int16_t  batt_temp[2];  // External board battery temperatures [degC];
    uint16_t latchup[6];    // Number of latch-ups on each 5V and 3V3 output.
                            // Order[5V1 5V2 5V3 3.3V1 3.3V2 3.3V3]
                            // Transmit as 5V1 first and 3.3V3 last
    uint8_t  reset;     // Cause of last EPS reset
    uint16_t bootcount; // Number of EPS reboots
    uint16_t sw_errors; // Number of errors in the eps software
    uint8_t  ppt_mode;  // 1 = MPPT, 2 = Fixed SW PPT.
    uint8_t  channel_status; // Mask of output channel status, 1=on, 0=off
                             // MSB [QH QS 3.3V3 3.3V2 3.3V1 5V3 5V2 5V1] LSB
                             // QH = Quadbat heater, QS = Quadbat switch
} p31u_hkparam_t;

/* 131 bytes. */
typedef struct __attribute__((packed)) {
    uint16_t vboost[3]; //! Voltage of boost converters [mV] [PV1, PV2, PV3]
    uint16_t vbatt;     //! Voltage of battery [mV]
    uint16_t curin[3];  //! Current in [mA]
    uint16_t cursun;    //! Current from boost converters [mA]
    uint16_t cursys;    //! Current out of battery [mA]
    uint16_t reserved1; //! Reserved for future use
    uint16_t curout[6]; //! Current out (switchable outputs) [mA]
    uint8_t  output[8]; //! Status of outputs**
    uint16_t output_on_delta[8];    //! Time till power on** [s]
    uint16_t output_off_delta[8];   //! Time till power off** [s]
    uint16_t latchup[6];            //! Number of latch-ups
    uint32_t wdt_i2c_time_left;     //! Time left on I2C wdt [s]
    uint32_t wdt_gnd_time_left;     //! Time left on I2C wdt [s]
    uint8_t  wdt_csp_pings_left[2]; //! Pings left on CSP wdt
    uint32_t counter_wdt_i2c;       //! Number of WDT I2C reboots
    uint32_t counter_wdt_gnd;       //! Number of WDT GND reboots
    uint32_t counter_wdt_csp[2];    //! Number of WDT CSP reboots
    uint32_t counter_boot;          //! Number of EPS reboots
    int16_t  temp[6];   //! Temperatures [degC] 
                        //  [0 = TEMP1, TEMP2, TEMP3, TEMP4, BP4a, BP4b]
    uint8_t  bootcause; //! Cause of last EPS reset
    uint8_t  battmode;  //! Mode for battery [0 = initial, 1 = undervoltage,
                        //                2 = safemode, 3 = nominal, 4=full]
    uint8_t  pptmode;   //! Mode of PPT tracker [1=MPPT, 2=FIXED]
    uint16_t reserved2;
} p31u_hk_t;

/* 20 bytes. */
typedef struct __attribute__((packed)) {
    uint16_t vboost[3]; //! Voltage of boost converters [mV] [PV1, PV2, PV3]
    uint16_t vbatt; //! Voltage of battery [mV]
    uint16_t curin[3]; //! Current in [mA]
    uint16_t cursun; //! Current from boost converters [mA]
    uint16_t cursys; //! Current out of battery [mA]
    uint16_t reserved1; //! Reserved for future use
} p31u_hk_vi_t;   

/* 64 bytes. */
typedef struct __attribute__((packed)) {
    uint16_t curout[6]; //! Current out (switchable outputs) [mA]
    uint8_t  output[8]; //! Status of outputs**
    uint16_t output_on_delta[8]; //! Time till power on** [s]
    uint16_t output_off_delta[8]; //! Time till power off** [s]
    uint16_t latchup[6]; //! Number of latch-ups
} p31u_hk_out_t; //**[6] is BP4 heater, [7] is BP4 switch

/* 26 bytes. */
typedef struct __attribute__((packed)) {
    uint32_t wdt_i2c_time_left; //! Time left on I2C wdt [s]
    uint32_t wdt_gnd_time_left; //! Time left on I2C wdt [s]
    uint8_t  wdt_csp_pings_left[2]; //! Pings left on CSP wdt
    uint32_t counter_wdt_i2c; //! Number of WDT I2C reboots
    uint32_t counter_wdt_gnd; //! Number of WDT GND reboots
    uint32_t counter_wdt_csp[2]; //! Number of WDT CSP reboots
} p31u_hk_wdt_t;

/* 21 bytes. */
typedef struct __attribute__((packed)) {
    uint32_t counter_boot; //! Number of EPS reboots
    int16_t  temp[6]; //! Temperatures [degC] [0 = TEMP1, TEMP2, TEMP3,
    //TEMP4, BATT0, BATT1]
    uint8_t  bootcause; //! Cause of last EPS reset
    uint8_t  battmode; //! Mode for battery [0 = initial, 1 =
    //undervoltage, 2 = safemode, 3 = nominal, 4=full]
    uint8_t  pptmode; //! Mode of PPT tracker [1=MPPT, 2=FIXED]
    uint16_t reserved2;
} p31u_hk_basic_t;


/**
 * Config structure definition:
 */

/* 58 bytes. */
typedef struct __attribute__((packed)) {
    uint8_t  ppt_mode; //! Mode for PPT [1 = AUTO, 2 = FIXED]
    uint8_t  battheater_mode; //! Mode for battheater [0 = Manual, 1 = Auto]
    int8_t   battheater_low; //! Turn heater on at [degC]
    int8_t   battheater_high; //! Turn heater off at [degC]
    uint8_t  output_normal_value[8]; //! Nominal mode output value
    uint8_t  output_safe_value[8]; //! Safe mode output value
    uint16_t output_initial_on_delay[8]; //! Output switches: init with these on delays [s]
    uint16_t output_initial_off_delay[8];//! Output switches: init with these off delays [s]
    uint16_t vboost[3]; //! Fixed PPT point for boost converters [mV]
} p31u_config_t;

/* 20 bytes. */
typedef struct __attribute__((packed)) {
    uint16_t batt_maxvoltage;
    uint16_t batt_safevoltage;
    uint16_t batt_criticalvoltage;
    uint16_t batt_normalvoltage;
    uint32_t reserved1[2];
    uint8_t  reserved2[4];
} p31u_config2_t;

/* 25 bytes. */
typedef struct __attribute__((packed)) {
    uint8_t  version;
    uint8_t  cmd;
    uint8_t  length;
    uint8_t  flags;
    uint16_t cur_lim[8];
    uint8_t  cur_ema_gain;
    uint8_t  cspwdt_channel[2];
    uint8_t  cspwdt_address[2];
} p31u_config3_t;


/**
 * Drops the const qualifier if we are converting the input data byte-order.
 */
#if P31U_USE_HTON
  #define p31u_req_const
#else
  #define p31u_req_const const
#endif


/**
 * @brief Performs a generic write-read transaction.
 * 
 * @details p31u_transaction() shall be separately defined per the mission
 *          driver layers. See the p31u/src/ifc/ sources.
 * 
 *          In I2C slave mode, each command is replied by at least two bytes:
 *          the echoed port number and an Error Code. Even with a null @a tx, 
 *          the transaction may still return a corresponding read Error Code.
 *          See p31u_ret_t or the manual.
 *  
 * @param port      Command port (cc). See p31u_port_t.
 * @param tx        Additional data following the request. Null if none.
 * @param txSize    Size of @a tx.
 * @param[out] rx   Reply data buffer. Null if none. Does not include the
 *                  echoed port or the Error Code.
 * @param rxSize    Size of @a rx.
 * 
 * @return P31U_OK:         Success.
 *         P31U_ERR_WRITE:  Failed to send a request.
 *         P31U_ERR_READ:   Failed to receive a reply.
 *         P31U_ERR_XFER:   Transaction failed.
 *         P31U_ERR_SIZE:   Invalid @a txSize or @a rxSize (Returned only in
 *                          I2C slave mode).
 *         P31U_ERR_PORT:   Reply port number mismatch (Returned only in I2C
 *                          slave mode).
 *         Others:          P31u custom Error Code (Returned only in I2C
 *                          slave mode. See p31u_ret_t).
 */
int p31u_transaction(uint8_t port,
                     const void* tx,
                     uint16_t txSize,
                        void* rx,
                     uint16_t rxSize);

/**
 * @brief Wraps p31u_transaction() with tx data only (no rx data expected).
 * 
 * @return See p31u_transaction().
 */
int p31u_set(uint8_t port,
             const void* data,
             int size);

/**
 * @brief Wraps p31u_transaction() with rx data only (no tx data sent).
 * 
 * @return See p31u_transaction().
 */
int p31u_get(uint8_t port,
             void* data,
             int size);

/**
 * @brief Requests P31u housekeeping data (P31U_PORT_HK).
 * 
 * @param[out] hk   Housekeeping buffer.
 * @param type      Housekeeping type.
 *                  0: all. 1: VI. 2: output. 3: WDT. 4: basic.
 * @param size      Size of @a hk.
 * 
 * @return See p31u_transaction().
 */
int p31u_gethk(void* hk,
               uint8_t type,
               int size);

/**
 * @brief Requests all HK data. See p31u_gethk().
 */
int p31u_gethk_all(p31u_hk_t* hk);

/**
 * @brief Requests VI HK data. See p31u_gethk().
 */
int p31u_gethk_vi(p31u_hk_vi_t* hk);

/**
 * @brief Requests output channel HK data. See p31u_gethk().
 */
int p31u_gethk_out(p31u_hk_out_t* hk);

/**
 * @brief Requests watchdog timer HK data. See p31u_gethk().
 */
int p31u_gethk_wdt(p31u_hk_wdt_t* hk);

/**
 * @brief Requests basic HK data. See p31u_gethk().
 */
int p31u_gethk_basic(p31u_hk_basic_t* hk);

// /**
//  * @brief Requests backward-compatible old P31u-6 HK data. See p31u_gethk().
//  */
// int p31u_gethk_old(p31u_hkparam_t* hk);

/**
 * @brief Sets output channel on/off status.
 * 
 * @param mask  Binary mask. 1 ON, 0 OFF. LSB is channel 1.
 * @return See p31u_set().
 */
int p31u_set_outputs(uint8_t mask);


/**
 * @brief Sets a single output channel on/off status.
 * 
 * @param channel   Channel number.
 *                  0-5: Output channels.
 *                  6:   BP4 heater.
 *                  7:   BP4 switch.
 * @param value     1 = ON, 0 = OFF.
 * @param delay     Delay in seconds.
 * 
 * @return See p31u_set().
 */
int p31u_set_output_single(uint8_t channel,
                           uint8_t value,
                           int16_t delay);

/**
 * @brief Sets the photovoltaic input voltages. Takes effect when PV MODE=2
 *        (See p31u_set_pv_auto).
 * 
 * @param voltage1  V1 voltage (mV).
 * @param voltage2  V2 voltage (mV).
 * @param voltage3  V3 voltage (mV).
 * 
 * @return See p31u_set().
 */                   
int p31u_set_pv_volt(int16_t voltage1,
                     int16_t voltage2,
                     int16_t voltage3);

/**
 * @brief Sets the solar cell power tracking mode.
 * 
 * @param mode  0: Hardware default.
 *              1: Maximum power point tracking.
 *              2: Fixed voltages set by SET_PV_VOLT (See p31u_set_pv_volt).
 *                 Default 4 V.
 * 
 * @return See p31u_set().
 */ 
int p31u_set_pv_auto(uint8_t mode);

typedef struct {
    uint8_t bp4_heater;
    uint8_t onboard_heater;
} p31u_reply_set_heater;

/**
 * @brief Sets the heater on/off.
 * 
 * @param cmd       Always zero.
 * @param heater    0: BP4. 1: Onboard. 2: Both.
 * @param mode      0: OFF, 1: ON.
 * @param[out] reply   Command reply. See p31u_reply_set_heater. Null allowed.
 *
 * @return See p31u_transaction().
 */  
int p31u_set_heater(uint8_t cmd,
                    uint8_t heater,
                    uint8_t mode,
                    p31u_reply_set_heater* reply);

/**
 * @brief Clears the device boot/watchdog counters.
 *
 * @return See p31u_set().
 */             
int p31u_reset_counters(void);

/**
 * @brief Resets the watchdog timer.
 *
 * @return See p31u_set().
 */   
int p31u_reset_wdt(void);

/**
 * @brief Config restoration command.
 * 
 * @param cmd   1: Restore default config.
 * @return See p31u_set().
 */ 
int p31u_config_cmd(uint8_t cmd);

/**
 * @brief Requests current device config.
 * 
 * @param[out] config  Data buffer. See p31u_config_t.
 * 
 * @return p31u_get().
 */ 
int p31u_get_config(p31u_config_t* config);

/**
 * @brief Sets device config.
 * 
 * @param config  Config data. See p31u_config_t.
 * 
 * @return See p31u_set().
 */               
int p31u_set_config(p31u_req_const p31u_config_t* config);

/**
 * @brief Performs a device hard-reset.
 * 
 * @return See p31u_set().
 */ 
#define p31u_hard_reset() \
        p31u_set(P31U_PORT_HARD_RESET, NULL, 0)

/**
 * @brief Config2 restoration command.
 * 
 * @param cmd   1: Restore default config.
 *              2: Confirm current config.
 * 
 * @return See p31u_set().
 */ 
int p31u_config2_cmd(uint8_t cmd);

/**
 * @brief Requests current device config2.
 * 
 * @param[out] config2 Data buffer. See p31u_config2_t.
 * 
 * @return See p31u_get().
 */       
int p31u_get_config2(p31u_config2_t* config2);

/**
 * @brief Sets device config2.
 * 
 * @param config2 Config2 data. See p31u_config2_t.
 * 
 * @return See p31u_set().
 */  
int p31u_set_config2(p31u_req_const p31u_config2_t* config2);

/**
 * @brief Sets device config3.
 * 
 * @param config3 Config3 data. See p31u_config3_t.
 * 
 * @return See p31u_set().
 */  
int p31u_set_config3(p31u_req_const p31u_config3_t* config3);

#endif
