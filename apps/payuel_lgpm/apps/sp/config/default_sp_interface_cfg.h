#ifndef SP_INTERFACE_CFG_H
#define SP_INTERFACE_CFG_H

/**
 * NanoPower DSP (Deployable Solar Panel) interface constants.
 * Each DSP board contains two AR6 release devices (Board A and Board B).
 * Both DSPs are connected directly to the OBC I2C1 bus (local GSSB).
 *
 * Reference: NanoPower DSP Datasheet DS-1018088-1-32
 *            GomSpace NanoSoft Product Interface Application Manual 4.0.0, Section 3.6
 */

/* I2C timeout in milliseconds */
#define SP_DSP_I2C_TIMEOUT_MS           1000

/**
 * I2C addresses of the four AR6 release boards (2 per DSP, 4 total on I2C1 bus).
 *
 * *** IMPORTANT: VALUES BELOW ARE PLACEHOLDERS — MUST BE CONFIRMED WITH HW TEAM ***
 *
 * AR6 I2C addresses are NOT factory-fixed. They must be programmed once during
 * hardware commissioning BEFORE cFS is used. The procedure is as follows:
 *
 * Background:
 *   - AR6 boards ship from GomSpace with the same factory default I2C address.
 *   - All four AR6 boards must be assigned unique addresses before use.
 *   - Since the OBC is not a GomSpace product (no GOSH shell available),
 *     address programming is done via the SP_SET_AR6_ADDR_CC cFS command,
 *     which calls gs_gssb_set_i2c_addr() + gs_gssb_commit_i2c_addr() at runtime.
 *   - Alternatively, a GomSpace Aardvark USB-to-I2C adapter + PC tool can be
 *     used to program AR6 addresses directly without the OBC.
 *
 * Commissioning procedure via cFS (SP_SET_AR6_ADDR_CC command):
 *   Step 1. Connect ONLY ONE AR6 board to the OBC I2C1 bus.
 *           Disconnect the other three.
 *   Step 2. Boot OBC with cFS. Send SP_SET_AR6_ADDR_CC with:
 *             CurrentAddr = <factory_default_addr>
 *             NewAddr     = 0x11  (for DSP1 Board A)
 *           The command calls gs_gssb_set_i2c_addr() then gs_gssb_commit_i2c_addr(),
 *           which saves the new address to the AR6's NVM permanently.
 *   Step 3. Disconnect this board and connect the next one.
 *           Repeat Step 2 with the next address (0x12, 0x13, 0x14).
 *   Step 4. After all four boards are programmed, connect all four together
 *           and send SP_SCAN_AR6_CC to verify all four respond at the
 *           expected addresses (0x11, 0x12, 0x13, 0x14).
 *
 * Naming convention:
 *   SP_DSPx_AR6y_I2C_ADDR  where x = DSP index (1 or 2), y = board (A or B)
 */
#define SP_DSP1_AR6A_I2C_ADDR   0x11  /**< TBD: DSP1 AR6 Board A address (confirm from HW ICD) */
#define SP_DSP1_AR6B_I2C_ADDR   0x12  /**< TBD: DSP1 AR6 Board B address (confirm from HW ICD) */
#define SP_DSP2_AR6A_I2C_ADDR   0x13  /**< TBD: DSP2 AR6 Board A address (confirm from HW ICD) */
#define SP_DSP2_AR6B_I2C_ADDR   0x14  /**< TBD: DSP2 AR6 Board B address (confirm from HW ICD) */

/** Default burn-wire activation duration [s] for manual deploy */
#define SP_DSP_BURN_DURATION_S          5

/** Auto deploy parameters for gs_autodeploy_release_two_dsp() */
#define SP_AUTO_DEPLOY_START_BURNTIME_S 3   /**< Initial burn duration [s] */
#define SP_AUTO_DEPLOY_INCREMENT_S      2   /**< Burn time increment per retry [s] */
#define SP_AUTO_DEPLOY_MAX_BURNTIME_S   10  /**< Maximum burn duration [s] */

#endif