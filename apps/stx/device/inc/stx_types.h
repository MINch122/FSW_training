/**
 * @file stx_types.h
 * @brief Typed request/result payloads for the S-band Transmitter transactions.
 *
 * One C type per non-trivial transaction Data field, from the S-band Transmitter
 * User Manual (rev 22/07/2024). Naming: request payloads are stx_cmd_*_t, reply
 * payloads are stx_rep_*_t, and shared parameter blocks keep a plain name
 * (stx_params_t, stx_mod_data_iface_t, stx_report_t). Trivial single-value
 * parameters (symbol rate, power, ...) use plain scalars, not structs.
 *
 * These are LOGICAL structs with natural host alignment, NOT wire images: the
 * driver marshals each field to/from the little-endian wire byte-by-byte, so
 * never memcpy a whole struct onto the wire. Each type's comment gives the wire
 * byte order it maps to.
 *
 * Universal ESUP rule: the first byte of a command *result* Data field is the
 * command-execution status. It is retained as result Data byte 0 and mirrored in
 * the least-significant byte of esup_ret_t. The "result byte N"
 * offsets below are counted from the start of that Data field, so the payload
 * proper starts at byte 1. Offsets flagged VERIFY still need a logic-analyzer
 * capture to confirm.
 */
#ifndef STX_TYPES_H
#define STX_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/* ------------------------------------------------------------------------- */
/* Configuration parameters (GET / SET)                                      */
/* ------------------------------------------------------------------------- */

/**
 * @brief All transmission parameters as one block (Type 0x0048; Table 12).
 *
 * Wire layout (12 bytes, little-endian):
 *   [0]     symbol_rate       1-byte
 *   [1]     tx_power_dbm      1-byte
 *   [2]     modcod            1-byte
 *   [3]     roll_off          1-byte
 *   [4]     pilot             1-byte
 *   [5]     fec_frame_short   1-byte
 *   [6..7]  pretx_delay_ms    2-byte unsigned short
 *   [8..11] center_freq_mhz   4-byte float
 * As a GET result the same block follows the status byte (result bytes 1..12).
 */
typedef struct {
    uint8_t  symbol_rate;      /**< 1..5 Msym/s. */
    uint8_t  tx_power_dbm;     /**< 27..33 dBm. */
    uint8_t  modcod;           /**< 1..23. */
    uint8_t  roll_off;         /**< 0/1/2 = 0.35/0.25/0.2 (stx_roll_off_t). */
    uint8_t  pilot;            /**< 1 = on, 0 = off. */
    uint8_t  fec_frame_short;  /**< 1 = short, 0 = normal. */
    uint16_t pretx_delay_ms;   /**< 2000..10000 ms. */
    float    center_freq_mhz;  /**< Centre frequency in MHz. */
} stx_params_t;

/**
 * @brief Modulator data interface selection (Type 0x004D; Table 14).
 *
 * Wire layout (2 bytes): [0] interface_type, [1] lvds_io_type.
 * When @ref interface_type is internal, @ref lvds_io_type is insignificant.
 */
typedef struct {
    uint8_t interface_type; /**< 1 = internal (SD card), 0 = LVDS (stx_mod_iface_type_t). */
    uint8_t lvds_io_type;   /**< 0 = PC104 connector, 1 = LVDS connector (stx_lvds_io_t). */
} stx_mod_data_iface_t;

/**
 * @brief Parsed status report (GET Type 0x0049; Table 15).
 *
 * Result byte offsets, counted from the result Data field (byte 0 = exec status):
 *   [1]     state       System State
 *   [2]     flags       Status Flags bit field
 *   [3..4]  (reserved)
 *   [5..8]  cpu_temp_c  4-byte float, -40..125 C
 *   [9..12] fw_version  4-byte unsigned int (default 10200 = 0x27D8)
 * The report structure is assumed to begin after the status byte; VERIFY.
 */
typedef struct {
    uint8_t  state;      /**< System state (stx_system_state_t). */
    uint8_t  flags;      /**< Status flags (STX_FLAG_*). */
    float    cpu_temp_c; /**< CPU temperature in degrees Celsius. */
    uint32_t fw_version; /**< Firmware version. */
} stx_report_t;

/* ------------------------------------------------------------------------- */
/* File-system command requests                                              */
/* ------------------------------------------------------------------------- */

/**
 * @brief Delete_File request (Command 0x0104).
 * Wire: NUL-terminated name, 3..30 chars.
 */
typedef struct {
    char name[31]; /**< NUL-terminated file name. */
} stx_cmd_file_delete_t;

/**
 * @brief Create_File request (Command 0x0106; Table 62).
 * Wire: NUL-terminated name, then a 4-byte unsigned int file length.
 */
typedef struct {
    char     name[31];    /**< NUL-terminated file name, 3..30 chars. */
    uint32_t file_length; /**< Total file size in bytes. */
} stx_cmd_file_create_t;

/**
 * @brief Write_File request (Command 0x0107; Table 73).
 *
 * Wire layout:
 *   [0..1]   data_len        2-byte unsigned short (bytes in this packet)
 *   [2..5]   handle          4-byte signed int (from Create_File)
 *   [6..9]   packet_number   4-byte unsigned int (from 0)
 *   [10..N]  data            data_len bytes (multiple of 4 except the last packet)
 */
typedef struct {
    int32_t        handle;        /**< File handle from Create_File. */
    uint32_t       packet_number; /**< Packet index, starting at 0. */
    const uint8_t* data;          /**< Packet bytes (borrowed). */
    uint16_t       data_len;      /**< Number of bytes in @ref data. */
} stx_cmd_file_write_t;

/**
 * @brief Open_File request (Command 0x0108; Table 82).
 * Wire: NUL-terminated name, 3..30 chars.
 */
typedef struct {
    char name[31]; /**< NUL-terminated file name. */
} stx_cmd_file_open_t;

/**
 * @brief Read_File request (Command 0x0109; Table 93).
 * Wire: a single 4-byte signed int file handle (from Open_File).
 */
typedef struct {
    int32_t handle; /**< File handle from Open_File. */
} stx_cmd_file_read_t;

/**
 * @brief Send_File_s request (Command 0x010A; Table 105).
 * Wire: NUL-terminated name/pattern (the '*' wildcard is allowed, Table 104).
 * @ref send_type selects the wire Type field (stx_send_type_t).
 */
typedef struct {
    char     name[31];  /**< NUL-terminated name or wildcard pattern. */
    uint16_t send_type; /**< STX_SEND_NORMAL or STX_SEND_RF_TRACT_ISSUE. */
} stx_cmd_file_send_t;

/* ------------------------------------------------------------------------- */
/* File-system command replies                                               */
/* ------------------------------------------------------------------------- */

/**
 * @brief Create_File reply (Table 64). Result bytes [1..4]: 4-byte signed
 *        handle. A successful handle is != -1.
 */
typedef struct {
    int32_t handle; /**< File handle for later Write_File, or -1 on failure. */
} stx_rep_file_create_t;

/**
 * @brief Open_File reply (Table 84). Result bytes [1..4]: signed handle
 *        (!= -1 on success); [5..8]: 4-byte unsigned file length.
 */
typedef struct {
    int32_t  handle;      /**< File handle for later Read_File, or -1 on failure. */
    uint32_t file_length; /**< Opened file size in bytes. */
} stx_rep_file_open_t;

/**
 * @brief Read_File reply (Table 95).
 *
 * Result bytes:
 *   [1..2]  packet_len      2-byte unsigned short (bytes in this packet)
 *   [3..6]  packet_number   4-byte unsigned int
 *   [7..N]  file data       packet_len bytes, copied into @ref data
 * @ref data / @ref data_cap are a caller-owned buffer; a @ref packet_len larger
 * than @ref data_cap means the packet was truncated into the buffer.
 */
typedef struct {
    uint16_t packet_len;    /**< Out: bytes in this packet. */
    uint32_t packet_number; /**< Out: packet index. */
    uint8_t* data;          /**< In: caller buffer for the file bytes. */
    uint16_t data_cap;      /**< In: capacity of @ref data. */
} stx_rep_file_read_t;

/**
 * @brief One entry in a directory listing (Table 32, per-file part).
 * Wire per entry: NUL-terminated name (3..30 chars) then a 4-byte unsigned length.
 */
typedef struct {
    char     name[31]; /**< NUL-terminated file name. */
    uint32_t length;   /**< File length in bytes. */
} stx_dir_entry_t;

/**
 * @brief DIR / DIR_Next reply (Commands 0x0102 / 0x0103; Table 32).
 *
 * Result bytes:
 *   [1]     more_files      1-byte flag: 0x01 = more files (use DIR_Next)
 *   [2..3]  count           2-byte unsigned short: names in this reply
 *   [4..N]  entries         count x {NUL-terminated name, 4-byte length}
 * @ref entries / @ref entries_cap are a caller-owned array; @ref entries_len is
 * how many were parsed (<= count and <= entries_cap).
 */
typedef struct {
    bool             more_files;  /**< True if more files remain; fetch with DIR_Next. */
    uint16_t         count;       /**< File names the device reported in this reply. */
    stx_dir_entry_t* entries;     /**< In: caller buffer for parsed entries. */
    uint16_t         entries_cap; /**< In: capacity of @ref entries. */
    uint16_t         entries_len; /**< Out: number of entries parsed. */
} stx_rep_file_dir_t;

/* ------------------------------------------------------------------------- */
/* System-configuration command requests                                     */
/* ------------------------------------------------------------------------- */

/**
 * @brief Update_FW request (Command 0x0112; Table 127).
 * Wire: NUL-terminated firmware file name, 3..30 chars.
 */
typedef struct {
    char name[31]; /**< NUL-terminated firmware file name. */
} stx_cmd_update_fw_t;

#endif /* STX_TYPES_H */
