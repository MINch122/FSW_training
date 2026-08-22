/**
 * @file stx.h
 * @brief EnduroSat S-band Transmitter device driver, layered on the ESUP engine.
 *
 * Provides typed, range-validated operations for the S-band Transmitter:
 * configuration (symbol rate, power, frequency, ...), status readout, and mode
 * control. Each helper marshals the device-specific payload, then runs a generic
 * ESUP transaction via esup_start_session (blocking through esup_wait). The radio
 * semantics live here and in stx_proto.h; the engine below stays device-agnostic.
 *
 * The device does not own the transport or the engine: the caller sets up a
 * wire and an esup_engine_t, then binds this driver to that engine. That keeps
 * the layering explicit and lets one engine be reused across drivers.
 */
#ifndef ENDURO_STX_H
#define ENDURO_STX_H

#include <stdint.h>

#include "esup.h"
#include "stx_proto.h"
#include "stx_types.h"

#define STX_DEFAULT_MODULE_ID 0x1231u
#define STX_DIR_ENTRIES_MAX 32u
/**
 * @brief Device-layer (STX) return codes.
 *
 * A helper returns ESUP_OK on success. These local codes occupy the device
 * region of esup_ret_t (byte 1, bits 8-15), disjoint from the ESUP driver's
 * fields, so a device-local error never collides with a transaction outcome:
 * `ESUP_RET_DEVICE_LOCAL(ret)` isolates it, `ESUP_RET_EXEC_STATUS(ret)` extracts
 * the module's command-execution status, and `ESUP_RET_ENGINE(ret)` isolates the
 * transaction result. Test for failure with `ret != ESUP_OK`.
 */
enum {
    STX_ERR_ARG   = 0x00000100, /**< Null pointer or malformed argument. */
    STX_ERR_RANGE = 0x00000200, /**< A parameter was outside the device's valid range. */
    STX_ERR_REPLY = 0x00000300  /**< The result was shorter than the expected payload. */
};

/* Per-class transaction budgets (milliseconds). */
#define STX_TIMEOUT_CONFIG_MS    2000u  /**< GET/SET and other fast commands. */
#define STX_TIMEOUT_FILE_MS      3000u  /**< DIR / create / write / open / read / delete. */
#define STX_TIMEOUT_FORMAT_MS    30000u /**< Quick_Format (12-20 s). */
#define STX_TIMEOUT_MODE_MS      3000u  /**< Idle / safe-shutdown. */
#define STX_TIMEOUT_TRANSMIT_MS  60000u /**< Enter Transmit Mode (19-40 s). */
#define STX_TIMEOUT_UPDATE_FW_MS 5000u  /**< Update_FW (500 ms - 2 s + restart). */

/* stx_report_t and the other transaction payload types are in stx_types.h. */

/**
 * @brief S-band Transmitter driver handle.
 *
 * Holds a borrowed pointer to the engine; the caller owns the engine's lifetime.
 */
typedef struct {
    esup_engine_t* engine; /**< Bound ESUP engine (not owned). */
} stx_t;

/**
 * @brief Bind the driver to an initialised ESUP engine.
 *
 * @param dev    Driver handle to initialise.
 * @param engine An engine already created with esup_engine_create.
 * @return 0 on success; STX_ERR_ARG on a null argument.
 */
esup_ret_t stx_init(stx_t* dev, esup_engine_t* engine);

/**
 * @brief Issue a device transaction without blocking (concurrency-capable).
 *
 * A thin typed forwarder over esup_start_session on the device's engine: issue
 * several (up to ESUP_SESSION_MAX) and drive them together with esup_tick, or
 * block on a returned handle with esup_wait. Payload and result buffers are
 * borrowed and must outlive the session. Unlike the blocking helpers below it
 * performs no range validation, so pass values the device accepts.
 *
 * @param dev         Driver handle.
 * @param command     Command code (STX_CMD_* / ESUP_CMD_*).
 * @param type        Type code (STX_TYPE_* / STX_SEND_* / ESUP_TYPE_NONE).
 * @param data        Payload (borrowed), or NULL.
 * @param len         Payload length in bytes.
 * @param result      Result descriptor (borrowed), or NULL to discard the payload.
 * @param timeout_ms  Overall transaction budget in milliseconds.
 * @param out_session Out: the session handle on success, NULL on failure.
 * @return 0 on success; a negative esup_ret_t if a slot is unavailable or
 *         an argument is invalid.
 */
esup_ret_t stx_begin(stx_t* dev, uint16_t command, uint16_t type, const void* data,
              uint16_t len, esup_result_t* result, uint32_t timeout_ms,
              esup_session_t** out_session);

/*
 * All helpers below block until the transaction completes (they wrap
 * esup_transact), so a driver must be running (esup_start). They return 0 on
 * success, an STX_ERR_* code on local validation failure, or a negative
 * esup_ret_t on transaction failure. Only the concurrency-capable stx_begin
 * above is non-blocking.
 */

/* --- Configuration: SET (Command 0x0101 by Type) ------------------------- */

/** @brief Set symbol rate, 1..5 Msym/s (Type 0x40). */
esup_ret_t stx_set_symbol_rate(stx_t* dev, uint8_t msps);
/** @brief Set transmit power, 27..33 dBm (Type 0x41). */
esup_ret_t stx_set_tx_power(stx_t* dev, uint8_t dbm);
/** @brief Set centre frequency in MHz; must fall in an S-band sub-range (Type 0x42). */
esup_ret_t stx_set_center_freq(stx_t* dev, float mhz);
/** @brief Set MODCOD index, 1..23 (Type 0x43). */
esup_ret_t stx_set_modcod(stx_t* dev, uint8_t modcod);
/** @brief Set roll-off code, 0/1/2 = 0.35/0.25/0.2 (Type 0x44). */
esup_ret_t stx_set_roll_off(stx_t* dev, uint8_t roll_off);
/** @brief Set pilot signal, 1 = on / 0 = off (Type 0x45). */
esup_ret_t stx_set_pilot(stx_t* dev, uint8_t on);
/** @brief Set FEC frame size, 1 = short / 0 = normal (Type 0x46). */
esup_ret_t stx_set_fec_frame(stx_t* dev, uint8_t short_frame);
/** @brief Set pre-transmission delay, 2000..10000 ms (Type 0x47). */
esup_ret_t stx_set_pretx_delay(stx_t* dev, uint16_t ms);
/** @brief Set all transmission parameters as one block (Type 0x48). */
esup_ret_t stx_set_params(stx_t* dev, const stx_params_t* params);
/** @brief Set the modulator data interface (Type 0x4D). */
esup_ret_t stx_set_mod_data_iface(stx_t* dev, const stx_mod_data_iface_t* iface);
/**
 * @brief Set the RS-485 baud-rate index, 0..4 (Type 0x4B, SET only).
 *
 * NOT IMPLEMENTED: refuses with ESUP_WIRE_ERR_UNSUPPORTED. A baud change is a
 * two-rate transaction (command at the old rate, result at the new) and needs a
 * dedicated policy that also switches the wire baud; a plain SET would desync the
 * link. Validation still runs, so an out-of-range setting returns STX_ERR_RANGE.
 */
esup_ret_t stx_set_rs485_baud(stx_t* dev, uint8_t setting);

/* --- Configuration: GET (Command 0x0100 by Type) ------------------------- */

/** @brief Read symbol rate in Msym/s (Type 0x40). */
esup_ret_t stx_get_symbol_rate(stx_t* dev, uint8_t* msps);
/** @brief Read transmit power in dBm (Type 0x41). */
esup_ret_t stx_get_tx_power(stx_t* dev, uint8_t* dbm);
/** @brief Read centre frequency in MHz (Type 0x42). */
esup_ret_t stx_get_center_freq(stx_t* dev, float* mhz);
/** @brief Read MODCOD index (Type 0x43). */
esup_ret_t stx_get_modcod(stx_t* dev, uint8_t* modcod);
/** @brief Read roll-off code (Type 0x44). */
esup_ret_t stx_get_roll_off(stx_t* dev, uint8_t* roll_off);
/** @brief Read pilot signal (Type 0x45). */
esup_ret_t stx_get_pilot(stx_t* dev, uint8_t* on);
/** @brief Read FEC frame size (Type 0x46). */
esup_ret_t stx_get_fec_frame(stx_t* dev, uint8_t* short_frame);
/** @brief Read pre-transmission delay in ms (Type 0x47). */
esup_ret_t stx_get_pretx_delay(stx_t* dev, uint16_t* ms);
/** @brief Read all transmission parameters as one block (Type 0x48). */
esup_ret_t stx_get_params(stx_t* dev, stx_params_t* params);
/** @brief Read the modulator data interface (Type 0x4D). */
esup_ret_t stx_get_mod_data_iface(stx_t* dev, stx_mod_data_iface_t* iface);
/** @brief Read and parse the status report (Type 0x49, GET only). */
esup_ret_t stx_get_report(stx_t* dev, stx_report_t* report);

/* --- File system --------------------------------------------------------- */

/**
 * @brief List files in the root directory (Command 0x0102).
 *
 * Fills @p rep from the reply. If @p rep->more_files is true, more files remain;
 * fetch them with stx_file_dir_next. The caller provides the @p rep->entries
 * buffer and @p rep->entries_cap.
 *
 * @return 0 on success; STX_ERR_ARG on a null argument; a negative esup_ret_t
 *         on transaction failure.
 */
esup_ret_t stx_file_dir(stx_t* dev, stx_rep_file_dir_t* rep);

/** @brief Continue a directory listing (Command 0x0103); same reply as stx_file_dir. */
esup_ret_t stx_file_dir_next(stx_t* dev, stx_rep_file_dir_t* rep);

/** @brief Delete one named file (Command 0x0104). */
esup_ret_t stx_file_delete(stx_t* dev, const stx_cmd_file_delete_t* cmd);

/** @brief Delete all non-service files (Command 0x0105). Spares service logs. */
esup_ret_t stx_file_delete_all(stx_t* dev);

/**
 * @brief Quick-format the SD card with FAT32 (Command 0x010C). Takes 12-20 s.
 *
 * No payload. The module's execution status rides in the least-significant
 * return-code byte: `ESUP_RET_EXEC_STATUS(ret)` is 0x00 OK, 0x01 File Not Found, or
 * 0x03 Card Error. Test overall success with `ret == ESUP_OK`.
 */
esup_ret_t stx_file_format(stx_t* dev);

/**
 * @brief Create a file and get its handle (Command 0x0106).
 *
 * On success @p rep->handle is the file handle for stx_file_write (valid != -1).
 * Only stx_file_write may follow; any other command closes the file.
 */
esup_ret_t stx_file_create(stx_t* dev, const stx_cmd_file_create_t* cmd,
                    stx_rep_file_create_t* rep);

/**
 * @brief Write one data packet to a created file (Command 0x0107).
 *
 * @p cmd->data_len must be 1..STX_WRITE_DATA_MAX and, except for the last packet,
 * a multiple of 4. Packet numbers start at 0.
 */
esup_ret_t stx_file_write(stx_t* dev, const stx_cmd_file_write_t* cmd);

/**
 * @brief Open a file for reading and get its handle and length (Command 0x0108).
 *
 * On success @p rep->handle is the file handle for stx_file_read (valid != -1)
 * and @p rep->file_length is the file size. Only stx_file_read may follow.
 */
esup_ret_t stx_file_open(stx_t* dev, const stx_cmd_file_open_t* cmd,
                 stx_rep_file_open_t* rep);

/**
 * @brief Read one data packet from an opened file (Command 0x0109).
 *
 * Copies up to @p rep->data_cap bytes into @p rep->data and reports the packet
 * length and number. A @p rep->packet_len larger than @p rep->data_cap means the
 * packet was truncated into the buffer.
 */
esup_ret_t stx_file_read(stx_t* dev, const stx_cmd_file_read_t* cmd,
                 stx_rep_file_read_t* rep);

/**
 * @brief Downlink one or more SD-card files over RF (Command 0x010A).
 *
 * Requires Transmit Mode. @p cmd->name may contain the '*' wildcard.
 * @p timeout_ms should be generous; the duration depends on file size and rate.
 */
esup_ret_t stx_file_send(stx_t* dev, const stx_cmd_file_send_t* cmd, uint32_t timeout_ms);

/* --- System configuration ------------------------------------------------ */

/** @brief Enter Transmit Mode (RF on; Command 0x0110). Takes 19-40 s. */
esup_ret_t stx_enter_transmit(stx_t* dev);

/** @brief Enter Idle Mode (RF off immediately; Command 0x0111). */
esup_ret_t stx_enter_idle(stx_t* dev);

/**
 * @brief Apply an on-card firmware update (Command 0x0112).
 *
 * The module auto-restarts on success or error, so the result may never arrive;
 * a returned ESUP_SESSION_UNREACHABLE / ESUP_SESSION_EXPIRED is expected. Confirm the
 * update afterwards by reading the report (Sys After Reset state + a higher
 * firmware version). This wrapper only issues the command.
 */
esup_ret_t stx_update_fw(stx_t* dev, const stx_cmd_update_fw_t* cmd);

/**
 * @brief Persist state and halt the processor (Command 0x0113).
 *
 * After ~0.6 s the CPU stops answering, so a returned ESUP_SESSION_UNREACHABLE is the
 * normal success indication; cut power afterwards. This wrapper only issues the
 * command and does not interpret the outcome.
 */
esup_ret_t stx_safe_shutdown(stx_t* dev);

#endif /* STX_H */
