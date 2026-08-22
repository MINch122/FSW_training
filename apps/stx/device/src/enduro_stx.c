/**
 * @file stx.c
 * @brief Implementation of the EnduroSat S-band Transmitter driver.
 *
 * Every helper is a thin wrapper over the ESUP engine: it validates its
 * arguments against the device's ranges, marshals the little-endian payload, runs
 * one blocking transaction (esup_transact), and parses the reply. Multi-byte
 * fields are packed/unpacked explicitly, except raw float/short passthroughs that
 * assume a little-endian host (the ESUP wire and the CubeSat targets are both
 * little-endian).
 */
#include "enduro_stx.h"
#include "esup_utils.h"

#include <stdbool.h>
#include <string.h>

/*
 * Result Data-field offsets, counted from byte 0 (the execution-status byte).
 * VERIFY the report/params offsets against a logic-analyzer capture: the manual's
 * byte numbering is ambiguous about whether the structure includes the status
 * byte. See stx_types.h.
 */
#define STX_REPORT_OFF_STATE   1u  /**< System state byte. */
#define STX_REPORT_OFF_FLAGS   2u  /**< Status flags byte. */
#define STX_REPORT_OFF_TEMP    5u  /**< CPU temperature (4-byte float). */
#define STX_REPORT_OFF_VERSION 9u  /**< Firmware version (4-byte uint). */
#define STX_REPORT_MIN_LEN     13u /**< Shortest valid report Data field. */

#define STX_PARAMS_MIN_LEN     13u /**< status + 12-byte parameter block. */
#define STX_CREATE_MIN_LEN     5u  /**< status + 4-byte handle. */
#define STX_OPEN_MIN_LEN       9u  /**< status + 4-byte handle + 4-byte length. */
#define STX_READ_HDR_LEN       7u  /**< status + 2-byte length + 4-byte packet no. */
#define STX_DIR_HDR_LEN        4u  /**< status + more-files flag + 2-byte count. */

/* ------------------------------------------------------------------------- */
/* Little-endian marshalling                                                 */
/* ------------------------------------------------------------------------- */

static uint16_t unpack_u16_le(const uint8_t* p)
{
    return (uint16_t)((uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8));
}

static uint32_t unpack_u32_le(const uint8_t* p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static int32_t unpack_i32_le(const uint8_t* p)
{
    return (int32_t)unpack_u32_le(p);
}

static float unpack_f32_le(const uint8_t* p)
{
    float v;

    memcpy(&v, p, 4); /* little-endian host */
    return v;
}

static void pack_u16_le(uint8_t* p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static void pack_u32_le(uint8_t* p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static void pack_i32_le(uint8_t* p, int32_t v)
{
    pack_u32_le(p, (uint32_t)v);
}

static void pack_f32_le(uint8_t* p, float v)
{
    memcpy(p, &v, 4); /* little-endian host */
}

/**
 * @brief Length of a NUL-terminated name, bounded so a missing NUL is rejected.
 *
 * @param s   Candidate name.
 * @param cap Bytes to scan before giving up.
 * @return The string length, or @p cap if no NUL was found within @p cap bytes.
 */
static size_t bounded_len(const char* s, size_t cap)
{
    size_t n = 0;

    while (n < cap && s[n] != '\0')
        n++;
    return n;
}

/**
 * @brief Validate and marshal a file name into a NUL-terminated payload.
 *
 * @param name    Candidate name (3..30 chars). Wildcards are accepted (length
 *                only is checked; Send_File_s permits '*').
 * @param out     Destination of at least 31 bytes.
 * @param out_len Out: bytes written (name + terminating NUL).
 * @return ESUP_OK, or STX_ERR_ARG if the length is out of range.
 */
static esup_ret_t marshal_name(const char* name, uint8_t* out, uint16_t* out_len)
{
    size_t n = bounded_len(name, STX_NAME_MAX_LEN + 1u);

    if (n < STX_NAME_MIN_LEN || n > STX_NAME_MAX_LEN) {
        ESUP_LOGW("file name length %zu out of range [%u,%u]", n,
                  STX_NAME_MIN_LEN, STX_NAME_MAX_LEN);
        return STX_ERR_ARG;
    }
    memcpy(out, name, n);
    out[n] = 0u;
    *out_len = (uint16_t)(n + 1u);
    return ESUP_OK;
}

/* ------------------------------------------------------------------------- */
/* Generic scalar SET/GET                                                    */
/* ------------------------------------------------------------------------- */

/**
 * @brief Issue a transaction and block until it completes.
 *
 * Requires a running engine driver (esup_start); the wait itself does not drive
 * the engine.
 */
static esup_ret_t stx_run(stx_t* dev, uint16_t command, uint16_t type,
                   const void* data, uint16_t len, esup_result_t* result,
                   uint32_t timeout_ms)
{
    return esup_transact(dev->engine, command, type, data, len, result,
                         timeout_ms);
}

/**
 * @brief Run a SET of a single-byte parameter.
 */
static esup_ret_t set_u8(stx_t* dev, uint16_t type, uint8_t value)
{
    return stx_run(dev, ESUP_CMD_SET, type, &value, 1, NULL,
                   STX_TIMEOUT_CONFIG_MS);
}

/**
 * @brief Run a GET and confirm the reply carries at least @p need bytes.
 *
 * @param dev  Driver handle.
 * @param type Parameter type code.
 * @param buf  Destination for the whole result Data field (incl. status byte).
 * @param cap  Capacity of @p buf.
 * @param need Minimum acceptable Data-field length.
 * @return ESUP_OK; STX_ERR_REPLY on a short reply; a negative esup_ret_t on
 *         transaction failure.
 */
static esup_ret_t get_payload(stx_t* dev, uint16_t type, uint8_t* buf, uint16_t cap,
                       uint16_t need)
{
    esup_result_t result = {.data = buf, .data_cap = cap};
    esup_ret_t rc = stx_run(dev, ESUP_CMD_GET, type, NULL, 0, &result,
                     STX_TIMEOUT_CONFIG_MS);

    if (rc != ESUP_OK)
        return rc;
    if (result.data_len < need) {
        ESUP_LOGW("short GET reply (type=0x%04X len=%u need %u)", type,
                  result.data_len, need);
        return STX_ERR_REPLY;
    }
    return ESUP_OK;
}

/**
 * @brief GET a single-byte parameter (value at result byte 1).
 */
static esup_ret_t get_u8(stx_t* dev, uint16_t type, uint8_t* out)
{
    uint8_t buf[2];
    esup_ret_t rc = get_payload(dev, type, buf, sizeof(buf), 2u);

    if (rc != ESUP_OK)
        return rc;
    *out = buf[1];
    return ESUP_OK;
}

/* ------------------------------------------------------------------------- */
/* Parameter validation                                                      */
/* ------------------------------------------------------------------------- */

/**
 * @brief Whether a centre frequency falls in a supported S-band sub-range.
 */
static bool freq_in_band(float mhz)
{
    bool in_commercial = (mhz >= 2200.0f && mhz <= 2290.0f);
    bool in_amateur = (mhz >= 2400.0f && mhz <= 2450.0f);

    return in_commercial || in_amateur;
}

/**
 * @brief Validate a full parameter block against every field range.
 *
 * @return ESUP_OK, or STX_ERR_RANGE on the first out-of-range field.
 */
static esup_ret_t validate_params(const stx_params_t* p)
{
    if (p->symbol_rate < STX_SYMBOL_RATE_MIN || p->symbol_rate > STX_SYMBOL_RATE_MAX
        || p->tx_power_dbm < STX_TX_POWER_MIN || p->tx_power_dbm > STX_TX_POWER_MAX
        || p->modcod < STX_MODCOD_MIN || p->modcod > STX_MODCOD_MAX
        || p->roll_off > STX_ROLL_OFF_MAX
        || p->pilot > STX_PILOT_MAX
        || p->fec_frame_short > STX_FEC_FRAME_MAX
        || p->pretx_delay_ms < STX_PRETX_DELAY_MIN
        || p->pretx_delay_ms > STX_PRETX_DELAY_MAX
        || !freq_in_band(p->center_freq_mhz)) {
        ESUP_LOGW("parameter block has an out-of-range field");
        return STX_ERR_RANGE;
    }
    return ESUP_OK;
}

/* ------------------------------------------------------------------------- */
/* Setup                                                                     */
/* ------------------------------------------------------------------------- */

esup_ret_t stx_init(stx_t* dev, esup_engine_t* engine)
{
    if (dev == NULL || engine == NULL) {
        ESUP_LOGE("bad argument (dev=%p engine=%p)", (void*)dev, (void*)engine);
        return STX_ERR_ARG;
    }
    dev->engine = engine;
    return ESUP_OK;
}

esup_ret_t stx_begin(stx_t* dev, uint16_t command, uint16_t type, const void* data,
              uint16_t len, esup_result_t* result, uint32_t timeout_ms,
              esup_session_t** out_session)
{
    if (out_session != NULL)
        *out_session = NULL;
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return ESUP_SESSION_ABORTED;
    }
    return esup_start_session(dev->engine, command, type, data, len, result,
                              timeout_ms, out_session);
}

/* ------------------------------------------------------------------------- */
/* Configuration: SET                                                        */
/* ------------------------------------------------------------------------- */

esup_ret_t stx_set_symbol_rate(stx_t* dev, uint8_t msps)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (msps < STX_SYMBOL_RATE_MIN || msps > STX_SYMBOL_RATE_MAX) {
        ESUP_LOGW("symbol rate %u out of range [%u,%u]", msps,
                  STX_SYMBOL_RATE_MIN, STX_SYMBOL_RATE_MAX);
        return STX_ERR_RANGE;
    }
    return set_u8(dev, STX_TYPE_SYMBOL_RATE, msps);
}

esup_ret_t stx_set_tx_power(stx_t* dev, uint8_t dbm)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (dbm < STX_TX_POWER_MIN || dbm > STX_TX_POWER_MAX) {
        ESUP_LOGW("tx power %u out of range [%u,%u]", dbm, STX_TX_POWER_MIN,
                  STX_TX_POWER_MAX);
        return STX_ERR_RANGE;
    }
    return set_u8(dev, STX_TYPE_TX_POWER, dbm);
}

esup_ret_t stx_set_center_freq(stx_t* dev, float mhz)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (!freq_in_band(mhz)) {
        ESUP_LOGW("center freq %.3f MHz outside allowed bands", (double)mhz);
        return STX_ERR_RANGE;
    }

    uint8_t buf[4];

    pack_f32_le(buf, mhz);
    return stx_run(dev, ESUP_CMD_SET, STX_TYPE_CENTER_FREQ, buf, 4, NULL,
                   STX_TIMEOUT_CONFIG_MS);
}

esup_ret_t stx_set_modcod(stx_t* dev, uint8_t modcod)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (modcod < STX_MODCOD_MIN || modcod > STX_MODCOD_MAX) {
        ESUP_LOGW("modcod %u out of range [%u,%u]", modcod, STX_MODCOD_MIN,
                  STX_MODCOD_MAX);
        return STX_ERR_RANGE;
    }
    return set_u8(dev, STX_TYPE_MODCOD, modcod);
}

esup_ret_t stx_set_roll_off(stx_t* dev, uint8_t roll_off)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (roll_off > STX_ROLL_OFF_MAX) {
        ESUP_LOGW("roll-off %u out of range [0,%u]", roll_off, STX_ROLL_OFF_MAX);
        return STX_ERR_RANGE;
    }
    return set_u8(dev, STX_TYPE_ROLL_OFF, roll_off);
}

esup_ret_t stx_set_pilot(stx_t* dev, uint8_t on)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (on > STX_PILOT_MAX) {
        ESUP_LOGW("pilot %u out of range [0,%u]", on, STX_PILOT_MAX);
        return STX_ERR_RANGE;
    }
    return set_u8(dev, STX_TYPE_PILOT, on);
}

esup_ret_t stx_set_fec_frame(stx_t* dev, uint8_t short_frame)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (short_frame > STX_FEC_FRAME_MAX) {
        ESUP_LOGW("fec frame %u out of range [0,%u]", short_frame,
                  STX_FEC_FRAME_MAX);
        return STX_ERR_RANGE;
    }
    return set_u8(dev, STX_TYPE_FEC_FRAME, short_frame);
}

esup_ret_t stx_set_pretx_delay(stx_t* dev, uint16_t ms)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (ms < STX_PRETX_DELAY_MIN || ms > STX_PRETX_DELAY_MAX) {
        ESUP_LOGW("pretx delay %u out of range [%u,%u]", ms,
                  STX_PRETX_DELAY_MIN, STX_PRETX_DELAY_MAX);
        return STX_ERR_RANGE;
    }

    uint8_t buf[2];

    pack_u16_le(buf, ms);
    return stx_run(dev, ESUP_CMD_SET, STX_TYPE_PRETX_DELAY, buf, 2, NULL,
                   STX_TIMEOUT_CONFIG_MS);
}

esup_ret_t stx_set_params(stx_t* dev, const stx_params_t* params)
{
    if (dev == NULL || params == NULL) {
        ESUP_LOGE("bad argument (dev=%p params=%p)", (void*)dev,
                  (const void*)params);
        return STX_ERR_ARG;
    }

    esup_ret_t rc = validate_params(params);

    if (rc != ESUP_OK)
        return rc;

    uint8_t buf[12];

    buf[0] = params->symbol_rate;
    buf[1] = params->tx_power_dbm;
    buf[2] = params->modcod;
    buf[3] = params->roll_off;
    buf[4] = params->pilot;
    buf[5] = params->fec_frame_short;
    pack_u16_le(&buf[6], params->pretx_delay_ms);
    pack_f32_le(&buf[8], params->center_freq_mhz);
    return stx_run(dev, ESUP_CMD_SET, STX_TYPE_ALL_PARAMS, buf, sizeof(buf), NULL,
                   STX_TIMEOUT_CONFIG_MS);
}

esup_ret_t stx_set_mod_data_iface(stx_t* dev, const stx_mod_data_iface_t* iface)
{
    if (dev == NULL || iface == NULL) {
        ESUP_LOGE("bad argument (dev=%p iface=%p)", (void*)dev,
                  (const void*)iface);
        return STX_ERR_ARG;
    }
    if (iface->interface_type > STX_MOD_IFACE_INTERNAL
        || iface->lvds_io_type > STX_LVDS_IO_LVDS) {
        ESUP_LOGW("modulator interface fields out of range");
        return STX_ERR_RANGE;
    }

    uint8_t buf[2] = {iface->interface_type, iface->lvds_io_type};

    return stx_run(dev, ESUP_CMD_SET, STX_TYPE_MOD_DATA_IFACE, buf, sizeof(buf),
                   NULL, STX_TIMEOUT_CONFIG_MS);
}

esup_ret_t stx_set_rs485_baud(stx_t* dev, uint8_t setting)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    if (setting > STX_RS485_BAUD_MAX) {
        ESUP_LOGW("rs485 baud setting %u out of range [0,%u]", setting,
                  STX_RS485_BAUD_MAX);
        return STX_ERR_RANGE;
    }
    /*
     * REFUSED on purpose. A baud change is a special two-rate transaction:
     * command+ACK go out at the OLD rate, then the module switches and the
     * Get_Results/result exchange must happen at the NEW rate. A plain SET here
     * would ACK, the module would move to the new baud, and every subsequent
     * frame (including our own polls) would be at the wrong rate -- desyncing the
     * whole link, not just failing this command. Until a dedicated baud-change
     * policy (switch the wire baud inside the confirm window, then poll at the new
     * rate) is implemented and verified on a capture, do not attempt it.
     */
    ESUP_LOGE("rs485 baud change not implemented (would desync the link)");
    return ESUP_WIRE_ERR_UNSUPPORTED;
}

/* ------------------------------------------------------------------------- */
/* Configuration: GET                                                        */
/* ------------------------------------------------------------------------- */

esup_ret_t stx_get_symbol_rate(stx_t* dev, uint8_t* msps)
{
    if (dev == NULL || msps == NULL) {
        ESUP_LOGE("bad argument (dev=%p out=%p)", (void*)dev, (void*)msps);
        return STX_ERR_ARG;
    }
    return get_u8(dev, STX_TYPE_SYMBOL_RATE, msps);
}

esup_ret_t stx_get_tx_power(stx_t* dev, uint8_t* dbm)
{
    if (dev == NULL || dbm == NULL) {
        ESUP_LOGE("bad argument (dev=%p out=%p)", (void*)dev, (void*)dbm);
        return STX_ERR_ARG;
    }
    return get_u8(dev, STX_TYPE_TX_POWER, dbm);
}

esup_ret_t stx_get_center_freq(stx_t* dev, float* mhz)
{
    if (dev == NULL || mhz == NULL) {
        ESUP_LOGE("bad argument (dev=%p out=%p)", (void*)dev, (void*)mhz);
        return STX_ERR_ARG;
    }

    uint8_t buf[5];
    esup_ret_t rc = get_payload(dev, STX_TYPE_CENTER_FREQ, buf, sizeof(buf), 5u);

    if (rc != ESUP_OK)
        return rc;
    *mhz = unpack_f32_le(&buf[1]);
    return ESUP_OK;
}

esup_ret_t stx_get_modcod(stx_t* dev, uint8_t* modcod)
{
    if (dev == NULL || modcod == NULL) {
        ESUP_LOGE("bad argument (dev=%p out=%p)", (void*)dev, (void*)modcod);
        return STX_ERR_ARG;
    }
    return get_u8(dev, STX_TYPE_MODCOD, modcod);
}

esup_ret_t stx_get_roll_off(stx_t* dev, uint8_t* roll_off)
{
    if (dev == NULL || roll_off == NULL) {
        ESUP_LOGE("bad argument (dev=%p out=%p)", (void*)dev, (void*)roll_off);
        return STX_ERR_ARG;
    }
    return get_u8(dev, STX_TYPE_ROLL_OFF, roll_off);
}

esup_ret_t stx_get_pilot(stx_t* dev, uint8_t* on)
{
    if (dev == NULL || on == NULL) {
        ESUP_LOGE("bad argument (dev=%p out=%p)", (void*)dev, (void*)on);
        return STX_ERR_ARG;
    }
    return get_u8(dev, STX_TYPE_PILOT, on);
}

esup_ret_t stx_get_fec_frame(stx_t* dev, uint8_t* short_frame)
{
    if (dev == NULL || short_frame == NULL) {
        ESUP_LOGE("bad argument (dev=%p out=%p)", (void*)dev, (void*)short_frame);
        return STX_ERR_ARG;
    }
    return get_u8(dev, STX_TYPE_FEC_FRAME, short_frame);
}

esup_ret_t stx_get_pretx_delay(stx_t* dev, uint16_t* ms)
{
    if (dev == NULL || ms == NULL) {
        ESUP_LOGE("bad argument (dev=%p out=%p)", (void*)dev, (void*)ms);
        return STX_ERR_ARG;
    }

    uint8_t buf[3];
    esup_ret_t rc = get_payload(dev, STX_TYPE_PRETX_DELAY, buf, sizeof(buf), 3u);

    if (rc != ESUP_OK)
        return rc;
    *ms = unpack_u16_le(&buf[1]);
    return ESUP_OK;
}

esup_ret_t stx_get_params(stx_t* dev, stx_params_t* params)
{
    if (dev == NULL || params == NULL) {
        ESUP_LOGE("bad argument (dev=%p params=%p)", (void*)dev, (void*)params);
        return STX_ERR_ARG;
    }

    uint8_t buf[STX_PARAMS_MIN_LEN];
    esup_ret_t rc = get_payload(dev, STX_TYPE_ALL_PARAMS, buf, sizeof(buf),
                         STX_PARAMS_MIN_LEN);

    if (rc != ESUP_OK)
        return rc;
    params->symbol_rate = buf[1];
    params->tx_power_dbm = buf[2];
    params->modcod = buf[3];
    params->roll_off = buf[4];
    params->pilot = buf[5];
    params->fec_frame_short = buf[6];
    params->pretx_delay_ms = unpack_u16_le(&buf[7]);
    params->center_freq_mhz = unpack_f32_le(&buf[9]);
    return ESUP_OK;
}

esup_ret_t stx_get_mod_data_iface(stx_t* dev, stx_mod_data_iface_t* iface)
{
    if (dev == NULL || iface == NULL) {
        ESUP_LOGE("bad argument (dev=%p iface=%p)", (void*)dev, (void*)iface);
        return STX_ERR_ARG;
    }

    uint8_t buf[3];
    esup_ret_t rc = get_payload(dev, STX_TYPE_MOD_DATA_IFACE, buf, sizeof(buf), 3u);

    if (rc != ESUP_OK)
        return rc;
    iface->interface_type = buf[1];
    iface->lvds_io_type = buf[2];
    return ESUP_OK;
}

esup_ret_t stx_get_report(stx_t* dev, stx_report_t* report)
{
    if (dev == NULL || report == NULL) {
        ESUP_LOGE("bad argument (dev=%p report=%p)", (void*)dev, (void*)report);
        return STX_ERR_ARG;
    }

    uint8_t buf[STX_REPORT_MIN_LEN];
    esup_ret_t rc = get_payload(dev, STX_TYPE_REPORT, buf, sizeof(buf),
                         STX_REPORT_MIN_LEN);

    if (rc != ESUP_OK)
        return rc;
    report->state = buf[STX_REPORT_OFF_STATE];
    report->flags = buf[STX_REPORT_OFF_FLAGS];
    report->cpu_temp_c = unpack_f32_le(&buf[STX_REPORT_OFF_TEMP]);
    report->fw_version = unpack_u32_le(&buf[STX_REPORT_OFF_VERSION]);
    return ESUP_OK;
}

/* ------------------------------------------------------------------------- */
/* File system                                                               */
/* ------------------------------------------------------------------------- */

/**
 * @brief Run DIR or DIR_Next and parse the file-name list into @p rep.
 */
static int run_dir(stx_t* dev, uint16_t command, stx_rep_file_dir_t* rep)
{
    if (dev == NULL || rep == NULL) {
        ESUP_LOGE("bad argument (dev=%p rep=%p)", (void*)dev, (void*)rep);
        return STX_ERR_ARG;
    }

    uint8_t buf[ESUP_DATA_MAX];
    esup_result_t result = {.data = buf, .data_cap = sizeof(buf)};
    esup_ret_t rc = stx_run(dev, command, ESUP_TYPE_NONE, NULL, 0, &result,
                     STX_TIMEOUT_FILE_MS);

    if (rc != ESUP_OK)
        return rc;
    if (result.data_len < STX_DIR_HDR_LEN) {
        ESUP_LOGW("short DIR reply (len=%u)", result.data_len);
        return STX_ERR_REPLY;
    }

    rep->more_files = buf[1] != 0u;
    rep->count = unpack_u16_le(&buf[2]);

    uint16_t cap = (rep->entries != NULL) ? rep->entries_cap : 0u;
    uint16_t parsed = 0;
    size_t p = STX_DIR_HDR_LEN;

    while (parsed < rep->count && parsed < cap && p < result.data_len) {
        size_t start = p;

        while (p < result.data_len && buf[p] != 0u)
            p++;
        if (p >= result.data_len)
            break; /* name not NUL-terminated within the reply */

        size_t name_len = p - start;

        p++; /* skip the NUL */
        if (p + 4u > result.data_len)
            break; /* missing the 4-byte length */

        stx_dir_entry_t* e = &rep->entries[parsed];
        size_t copy = (name_len <= STX_NAME_MAX_LEN) ? name_len : STX_NAME_MAX_LEN;

        memcpy(e->name, &buf[start], copy);
        e->name[copy] = '\0';
        e->length = unpack_u32_le(&buf[p]);
        p += 4u;
        parsed++;
    }
    rep->entries_len = parsed;
    return ESUP_OK;
}

esup_ret_t stx_file_dir(stx_t* dev, stx_rep_file_dir_t* rep)
{
    return run_dir(dev, ESUP_CMD_DIR, rep);
}

esup_ret_t stx_file_dir_next(stx_t* dev, stx_rep_file_dir_t* rep)
{
    return run_dir(dev, ESUP_CMD_DIR_NEXT, rep);
}

esup_ret_t stx_file_delete(stx_t* dev, const stx_cmd_file_delete_t* cmd)
{
    if (dev == NULL || cmd == NULL) {
        ESUP_LOGE("bad argument (dev=%p cmd=%p)", (void*)dev, (const void*)cmd);
        return STX_ERR_ARG;
    }

    uint8_t buf[31];
    uint16_t len = 0;
    esup_ret_t rc = marshal_name(cmd->name, buf, &len);

    if (rc != ESUP_OK)
        return rc;
    return stx_run(dev, ESUP_CMD_DELETE_FILE, ESUP_TYPE_NONE, buf, len, NULL,
                   STX_TIMEOUT_FILE_MS);
}

esup_ret_t stx_file_delete_all(stx_t* dev)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    return stx_run(dev, ESUP_CMD_DELETE_ALL_FILES, ESUP_TYPE_NONE, NULL, 0, NULL,
                   STX_TIMEOUT_FILE_MS);
}

esup_ret_t stx_file_format(stx_t* dev)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }

    /* The module's execution status (0x00 OK, 0x01 File Not Found, 0x03 Card
     * Error) already rides in the execution-status byte of the return code. */
    return stx_run(dev, ESUP_CMD_QUICK_FORMAT, ESUP_TYPE_NONE, NULL, 0, NULL,
                   STX_TIMEOUT_FORMAT_MS);
}

esup_ret_t stx_file_create(stx_t* dev, const stx_cmd_file_create_t* cmd,
                    stx_rep_file_create_t* rep)
{
    if (dev == NULL || cmd == NULL || rep == NULL) {
        ESUP_LOGE("bad argument (dev=%p cmd=%p rep=%p)", (void*)dev,
                  (const void*)cmd, (void*)rep);
        return STX_ERR_ARG;
    }

    uint8_t buf[31 + 4];
    uint16_t name_len = 0;
    esup_ret_t rc = marshal_name(cmd->name, buf, &name_len);

    if (rc != ESUP_OK)
        return rc;
    pack_u32_le(&buf[name_len], cmd->file_length);

    uint8_t rbuf[STX_CREATE_MIN_LEN];
    esup_result_t result = {.data = rbuf, .data_cap = sizeof(rbuf)};

    rc = stx_run(dev, ESUP_CMD_CREATE_FILE, ESUP_TYPE_NONE, buf,
                 (uint16_t)(name_len + 4u), &result, STX_TIMEOUT_FILE_MS);
    if (rc != ESUP_OK)
        return rc;
    if (result.data_len < STX_CREATE_MIN_LEN) {
        ESUP_LOGW("short Create_File reply (len=%u)", result.data_len);
        return STX_ERR_REPLY;
    }
    rep->handle = unpack_i32_le(&rbuf[1]);
    return ESUP_OK;
}

esup_ret_t stx_file_write(stx_t* dev, const stx_cmd_file_write_t* cmd)
{
    if (dev == NULL || cmd == NULL || cmd->data == NULL) {
        ESUP_LOGE("bad argument (dev=%p cmd=%p)", (void*)dev, (const void*)cmd);
        return STX_ERR_ARG;
    }
    if (cmd->data_len == 0u || cmd->data_len > STX_WRITE_DATA_MAX) {
        ESUP_LOGW("write data_len %u out of range [1,%u]", cmd->data_len,
                  (unsigned)STX_WRITE_DATA_MAX);
        return STX_ERR_ARG;
    }

    uint8_t buf[ESUP_DATA_MAX];

    pack_u16_le(&buf[0], cmd->data_len);
    pack_i32_le(&buf[2], cmd->handle);
    pack_u32_le(&buf[6], cmd->packet_number);
    memcpy(&buf[STX_WRITE_HEADER_LEN], cmd->data, cmd->data_len);
    return stx_run(dev, ESUP_CMD_WRITE_FILE, ESUP_TYPE_NONE, buf,
                   (uint16_t)(STX_WRITE_HEADER_LEN + cmd->data_len), NULL,
                   STX_TIMEOUT_FILE_MS);
}

esup_ret_t stx_file_open(stx_t* dev, const stx_cmd_file_open_t* cmd,
                 stx_rep_file_open_t* rep)
{
    if (dev == NULL || cmd == NULL || rep == NULL) {
        ESUP_LOGE("bad argument (dev=%p cmd=%p rep=%p)", (void*)dev,
                  (const void*)cmd, (void*)rep);
        return STX_ERR_ARG;
    }

    uint8_t buf[31];
    uint16_t len = 0;
    esup_ret_t rc = marshal_name(cmd->name, buf, &len);

    if (rc != ESUP_OK)
        return rc;

    uint8_t rbuf[STX_OPEN_MIN_LEN];
    esup_result_t result = {.data = rbuf, .data_cap = sizeof(rbuf)};

    rc = stx_run(dev, ESUP_CMD_OPEN_FILE, ESUP_TYPE_NONE, buf, len, &result,
                 STX_TIMEOUT_FILE_MS);
    if (rc != ESUP_OK)
        return rc;
    if (result.data_len < STX_OPEN_MIN_LEN) {
        ESUP_LOGW("short Open_File reply (len=%u)", result.data_len);
        return STX_ERR_REPLY;
    }
    rep->handle = unpack_i32_le(&rbuf[1]);
    rep->file_length = unpack_u32_le(&rbuf[5]);
    return ESUP_OK;
}

esup_ret_t stx_file_read(stx_t* dev, const stx_cmd_file_read_t* cmd,
                 stx_rep_file_read_t* rep)
{
    if (dev == NULL || cmd == NULL || rep == NULL) {
        ESUP_LOGE("bad argument (dev=%p cmd=%p rep=%p)", (void*)dev,
                  (const void*)cmd, (void*)rep);
        return STX_ERR_ARG;
    }

    uint8_t handle[4];

    pack_i32_le(handle, cmd->handle);

    uint8_t buf[ESUP_DATA_MAX];
    esup_result_t result = {.data = buf, .data_cap = sizeof(buf)};
    esup_ret_t rc = stx_run(dev, ESUP_CMD_READ_FILE, ESUP_TYPE_NONE, handle,
                     sizeof(handle), &result, STX_TIMEOUT_FILE_MS);

    if (rc != ESUP_OK)
        return rc;
    if (result.data_len < STX_READ_HDR_LEN) {
        ESUP_LOGW("short Read_File reply (len=%u)", result.data_len);
        return STX_ERR_REPLY;
    }

    rep->packet_len = unpack_u16_le(&buf[1]);
    rep->packet_number = unpack_u32_le(&buf[3]);

    uint16_t avail = (uint16_t)(result.data_len - STX_READ_HDR_LEN);
    uint16_t n = (rep->packet_len < avail) ? rep->packet_len : avail;

    if (n > rep->data_cap)
        n = rep->data_cap;
    if (rep->data != NULL && n > 0u)
        memcpy(rep->data, &buf[STX_READ_HDR_LEN], n);
    return ESUP_OK;
}

esup_ret_t stx_file_send(stx_t* dev, const stx_cmd_file_send_t* cmd, uint32_t timeout_ms)
{
    if (dev == NULL || cmd == NULL) {
        ESUP_LOGE("bad argument (dev=%p cmd=%p)", (void*)dev, (const void*)cmd);
        return STX_ERR_ARG;
    }

    uint8_t buf[31];
    uint16_t len = 0;
    esup_ret_t rc = marshal_name(cmd->name, buf, &len);

    if (rc != ESUP_OK)
        return rc;
    return stx_run(dev, STX_CMD_SEND_FILES, cmd->send_type, buf, len, NULL,
                   timeout_ms);
}

/* ------------------------------------------------------------------------- */
/* System configuration                                                      */
/* ------------------------------------------------------------------------- */

esup_ret_t stx_enter_transmit(stx_t* dev)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    return stx_run(dev, STX_CMD_TRANSMIT_MODE, ESUP_TYPE_NONE, NULL, 0, NULL,
                   STX_TIMEOUT_TRANSMIT_MS);
}

esup_ret_t stx_enter_idle(stx_t* dev)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    return stx_run(dev, STX_CMD_IDLE_MODE, ESUP_TYPE_NONE, NULL, 0, NULL,
                   STX_TIMEOUT_MODE_MS);
}

esup_ret_t stx_update_fw(stx_t* dev, const stx_cmd_update_fw_t* cmd)
{
    if (dev == NULL || cmd == NULL) {
        ESUP_LOGE("bad argument (dev=%p cmd=%p)", (void*)dev, (const void*)cmd);
        return STX_ERR_ARG;
    }

    uint8_t buf[31];
    uint16_t len = 0;
    esup_ret_t rc = marshal_name(cmd->name, buf, &len);

    if (rc != ESUP_OK)
        return rc;
    return stx_run(dev, ESUP_CMD_UPDATE_FW, ESUP_TYPE_NONE, buf, len, NULL,
                   STX_TIMEOUT_UPDATE_FW_MS);
}

esup_ret_t stx_safe_shutdown(stx_t* dev)
{
    if (dev == NULL) {
        ESUP_LOGE("dev is NULL");
        return STX_ERR_ARG;
    }
    return stx_run(dev, ESUP_CMD_SAFE_SHUTDOWN, ESUP_TYPE_NONE, NULL, 0, NULL,
                   STX_TIMEOUT_MODE_MS);
}
