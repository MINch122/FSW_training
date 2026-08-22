/**
 * @file stx_proto.h
 * @brief S-band Transmitter device constants layered on generic ESUP.
 *
 * Everything here is specific to the EnduroSat S-band Transmitter: its radio
 * commands, parameter types and their ranges, MODCOD range, report layout,
 * system states and status flags. These are NOT part of ESUP (esup_proto.h) even
 * where the numeric codes coincide with another radio's, because they name radio
 * features. A per-device driver built on the esup engine uses these to construct
 * the Command and Type fields and to validate and parse payloads.
 *
 * Ranges are taken from the S-band Transmitter User Manual (rev 22/07/2024).
 */
#ifndef STX_PROTO_H
#define STX_PROTO_H

#include <stdint.h>

#include "esup_proto.h"

/**
 * @brief S-band radio-operation command codes.
 *
 * Only the commands whose meaning is a radio feature live here; the generic ESUP
 * transaction verbs (GET/SET, the file system, firmware update, safe shutdown,
 * Get_Results) are protocol-level and live in esup_proto.h as ESUP_CMD_*. These
 * codes are supplied to esup_start_session as the opaque Command field. The
 * numeric values match the X-band transmitter, but that reflects a shared
 * EnduroSat convention for the radio operations, not extra protocol.
 */
typedef enum {
    STX_CMD_SEND_FILES    = 0x010A, /**< Downlink file(s) over RF (Transmit Mode only). */
    STX_CMD_TRANSMIT_MODE = 0x0110, /**< Enter Transmit Mode (RF on; 19-40 s). */
    STX_CMD_IDLE_MODE     = 0x0111  /**< Return to Idle Mode (RF off immediately). */
} stx_command_code_t;

/**
 * @brief S-band GET/SET parameter Type codes.
 *
 * The numeric codes match other EnduroSat radios, but the valid values are
 * device-specific (see the ranges below). REPORT is GET-only; RS485_BAUD is
 * SET-only.
 */
typedef enum {
    STX_TYPE_SYMBOL_RATE    = 0x0040, /**< 1..5 Msym/s (1-byte). */
    STX_TYPE_TX_POWER       = 0x0041, /**< 27..33 dBm, step 1 dB (1-byte). */
    STX_TYPE_CENTER_FREQ    = 0x0042, /**< 2200.000..2290.000 or 2400.000..2450.000 MHz (4-byte float). */
    STX_TYPE_MODCOD         = 0x0043, /**< 1..23 (1-byte). */
    STX_TYPE_ROLL_OFF       = 0x0044, /**< 0/1/2 => 0.35/0.25/0.2 (1-byte). */
    STX_TYPE_PILOT          = 0x0045, /**< On/Off = 1/0 (1-byte). */
    STX_TYPE_FEC_FRAME      = 0x0046, /**< Short/Normal = 1/0 (1-byte). */
    STX_TYPE_PRETX_DELAY    = 0x0047, /**< 2000..10000 ms (2-byte). */
    STX_TYPE_ALL_PARAMS     = 0x0048, /**< All transmission parameters as a block. */
    STX_TYPE_REPORT         = 0x0049, /**< Status report (GET only). */
    STX_TYPE_RS485_BAUD     = 0x004B, /**< RS-485 baud setting 0..4 (SET only). */
    STX_TYPE_MOD_DATA_IFACE = 0x004D  /**< Modulator data interface selection. */
} stx_param_type_t;

/**
 * @brief Type values for the Send_File/s command.
 *
 * The S-band supports the normal and RF-tract-issue variants (the X-band adds
 * further variants; this divergence is exactly why send types are device-scoped).
 */
typedef enum {
    STX_SEND_NORMAL         = 0x0050, /**< Normal downlink. */
    STX_SEND_RF_TRACT_ISSUE = 0x0052  /**< Emergency downlink with incomplete SDR init. */
} stx_send_type_t;

/**
 * @brief Roll-off factor codes (Type 0x0044).
 */
typedef enum {
    STX_ROLL_OFF_0_35 = 0, /**< Roll-off 0.35. */
    STX_ROLL_OFF_0_25 = 1, /**< Roll-off 0.25. */
    STX_ROLL_OFF_0_20 = 2  /**< Roll-off 0.20. */
} stx_roll_off_t;

/*
 * Pilot Signal (Type 0x0045): 1 = on, 0 = off (1-byte).
 * FEC Frame Size (Type 0x0046): 1 = short, 0 = normal (1-byte).
 */

/**
 * @brief Modulator data interface type (Table 14, byte 0; Type 0x004D).
 */
typedef enum {
    STX_MOD_IFACE_LVDS     = 0, /**< External LVDS data interface. */
    STX_MOD_IFACE_INTERNAL = 1  /**< Internal interface: data from SD-card files. */
} stx_mod_iface_type_t;

/**
 * @brief LVDS I/O connector (Table 14, byte 1). Ignored for the internal interface.
 */
typedef enum {
    STX_LVDS_IO_PC104 = 0, /**< PC-104 connector. */
    STX_LVDS_IO_LVDS  = 1  /**< LVDS connector. */
} stx_lvds_io_t;

/* Parameter ranges for validation in device typed helpers. */
#define STX_SYMBOL_RATE_MIN   1u  /**< Minimum symbol rate (Msym/s). */
#define STX_SYMBOL_RATE_MAX   5u  /**< Maximum symbol rate (Msym/s). */
#define STX_TX_POWER_MIN      27u /**< Minimum transmit power (dBm). */
#define STX_TX_POWER_MAX      33u /**< Maximum transmit power (dBm). */
#define STX_MODCOD_MIN        1u  /**< Minimum MODCOD index. */
#define STX_MODCOD_MAX        23u /**< Maximum MODCOD index. */
#define STX_PRETX_DELAY_MIN   2000u  /**< Minimum pre-transmission delay (ms). */
#define STX_PRETX_DELAY_MAX   10000u /**< Maximum pre-transmission delay (ms). */
#define STX_ROLL_OFF_MAX      2u  /**< Maximum roll-off code (stx_roll_off_t). */
#define STX_PILOT_MAX         1u  /**< Pilot signal: 0 or 1. */
#define STX_FEC_FRAME_MAX     1u  /**< FEC frame size: 0 or 1. */
/* Table 13 defines RS-485 baud settings 0..4; Table 10's "0-11" is unverified. */
#define STX_RS485_BAUD_MAX    4u  /**< Maximum RS-485 baud setting index. */
#define STX_NAME_MIN_LEN      3u  /**< Shortest valid file name (chars, no NUL). */
#define STX_NAME_MAX_LEN      30u /**< Longest valid file name (chars, no NUL). */

/** Write_File fixed header (len + handle + packet number) before the file data. */
#define STX_WRITE_HEADER_LEN  10u
/** Largest file-data payload per Write_File packet (header must fit the frame). */
#define STX_WRITE_DATA_MAX    (1200)

/**
 * @brief System State values in the first byte of a status report's Data.
 */
typedef enum {
    STX_STATE_AFTER_RESET     = 1, /**< First read after a reset, then auto-goes to Idle. */
    STX_STATE_IDLE            = 2, /**< Idle Mode. */
    STX_STATE_TRANSMIT        = 3, /**< Transmit Mode. */
    STX_STATE_GOING_SHUTDOWN  = 4  /**< Shutting down. */
} stx_system_state_t;

/** Status flag bit: SDR not initialized (byte 1 of the status report Data). */
#define STX_FLAG_SDR_NOT_INIT   (1u << 2)
/** Status flag bit: system stopped on over-temperature. */
#define STX_FLAG_OVER_TEMP_STOP (1u << 4)

#endif /* STX_PROTO_H */
