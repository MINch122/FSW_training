/**
 * @file esup_framer.c
 * @brief Implementation of the ESUP frame codec and the wire-bound framer.
 */
#include "esup_framer.h"
#include "esup_utils.h"

#include <stdbool.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* Little-endian field access                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Read a little-endian 16-bit value.
 *
 * @param p Pointer to the first of two bytes.
 * @return The decoded value.
 */
static uint16_t read_u16_le(const uint8_t* p)
{
    return (uint16_t)((uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8));
}

/**
 * @brief Write a little-endian 16-bit value.
 *
 * @param p     Destination for two bytes.
 * @param value Value to write.
 */
static void write_u16_le(uint8_t* p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)((value >> 8) & 0xFFu);
}

/**
 * @brief Read a little-endian 32-bit value.
 *
 * @param p Pointer to the first of four bytes.
 * @return The decoded value.
 */
static uint32_t read_u32_le(const uint8_t* p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

/**
 * @brief Write a little-endian 32-bit value.
 *
 * @param p     Destination for four bytes.
 * @param value Value to write.
 */
static void write_u32_le(uint8_t* p, uint32_t value)
{
    p[0] = (uint8_t)(value & 0xFFu);
    p[1] = (uint8_t)((value >> 8) & 0xFFu);
    p[2] = (uint8_t)((value >> 16) & 0xFFu);
    p[3] = (uint8_t)((value >> 24) & 0xFFu);
}

/**
 * @brief Round a length up to the next multiple of the frame alignment.
 *
 * @param n Length to align.
 * @return The aligned length.
 */
static size_t align_up_frame(size_t n)
{
    size_t remainder = n % ESUP_ALIGN;

    if (remainder == 0)
        return n;
    return n + (ESUP_ALIGN - remainder);
}

/**
 * @brief Test whether a buffer starts with the ESUP header magic.
 *
 * @param p Buffer of at least ESUP_SIZE_HEADER bytes.
 * @return Non-zero if the first four bytes are the header magic.
 */
static bool has_magic(const uint8_t* p)
{
    return p[0] == ESUP_MAGIC_0
        && p[1] == ESUP_MAGIC_1
        && p[2] == ESUP_MAGIC_2
        && p[3] == ESUP_MAGIC_3;
}

/* ------------------------------------------------------------------------- */
/* CRC-32                                                                    */
/* ------------------------------------------------------------------------- */


static uint32_t crc32_tab[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t esup_crc32(const void *buf, size_t size)
{
	const uint8_t *p;
    uint32_t crc = 0;

	p = buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}


// uint32_t esup_crc32(const uint8_t* data, size_t len)
// {
//     uint32_t crc = ~ESUP_CRC32_INIT;

//     for (size_t i = 0; i < len; i++) {
//         crc ^= (uint32_t)data[i];
//         for (int bit = 0; bit < 8; bit++) {
//             if ((crc & 1u) != 0u)
//                 crc = (crc >> 1) ^ ESUP_CRC32_POLY;
//             else
//                 crc >>= 1;
//         }
//     }
//     return crc ^ ~ESUP_CRC32_XOROUT;
// }

/* ------------------------------------------------------------------------- */
/* Frame geometry and codec                                                  */
/* ------------------------------------------------------------------------- */

size_t esup_frame_wire_len(uint16_t data_len)
{
    if (data_len > ESUP_DATA_MAX)
        return 0;
    return align_up_frame((size_t)ESUP_SIZE_OVERHEAD + (size_t)data_len);
}

esup_ret_t esup_frame_encode(const esup_frame_t* frame, uint8_t* out, size_t out_cap,
                      size_t* out_len)
{
    if (frame == NULL || out == NULL || out_len == NULL)
        return ESUP_FRAME_ERR_ARG;
    if (frame->data_len > ESUP_DATA_MAX)
        return ESUP_FRAME_ERR_ARG;
    if (frame->data_len > 0 && frame->data == NULL)
        return ESUP_FRAME_ERR_ARG;

    size_t wire_len = esup_frame_wire_len(frame->data_len);

    if (wire_len == 0)
        return ESUP_FRAME_ERR_ARG;
    if (out_cap < wire_len)
        return ESUP_FRAME_ERR_CAPACITY;

    out[ESUP_OFST_HEADER + 0] = ESUP_MAGIC_0;
    out[ESUP_OFST_HEADER + 1] = ESUP_MAGIC_1;
    out[ESUP_OFST_HEADER + 2] = ESUP_MAGIC_2;
    out[ESUP_OFST_HEADER + 3] = ESUP_MAGIC_3;
    write_u16_le(&out[ESUP_OFST_MODULE_ID], frame->module_id);
    write_u16_le(&out[ESUP_OFST_DATA_LEN], frame->data_len);
    write_u16_le(&out[ESUP_OFST_CMD_STATUS], frame->cmd_status);
    write_u16_le(&out[ESUP_OFST_COMMAND], frame->command);
    write_u16_le(&out[ESUP_OFST_TYPE], frame->type);

    if (frame->data_len > 0)
        memcpy(&out[ESUP_OFST_DATA], frame->data, frame->data_len);

    size_t crc_off = (size_t)ESUP_OFST_DATA + frame->data_len;
    uint32_t crc = esup_crc32(out, crc_off);

    write_u32_le(&out[crc_off], crc);

    size_t padding_off = crc_off + ESUP_SIZE_CRC;

    if (padding_off < wire_len)
        memset(&out[padding_off], 0, wire_len - padding_off);

    *out_len = wire_len;
    return ESUP_OK;
}

esup_ret_t esup_frame_decode(const uint8_t* buf, size_t len, esup_frame_t* out)
{
    if (buf == NULL || out == NULL)
        return ESUP_FRAME_ERR_ARG;
    if (len < ESUP_SIZE_OVERHEAD)
        return ESUP_FRAME_ERR_LENGTH;
    if (!has_magic(buf))
        return ESUP_FRAME_ERR_MAGIC;

    uint16_t data_len = read_u16_le(&buf[ESUP_OFST_DATA_LEN]);

    if (data_len > ESUP_DATA_MAX)
        return ESUP_FRAME_ERR_LENGTH;

    size_t wire_len = esup_frame_wire_len(data_len);

    if (len != wire_len)
        return ESUP_FRAME_ERR_LENGTH;

    size_t crc_off = (size_t)ESUP_OFST_DATA + data_len;
    uint32_t computed = esup_crc32(buf, crc_off);
    uint32_t stored = read_u32_le(&buf[crc_off]);

    if (computed != stored)
        return ESUP_FRAME_ERR_CRC;

    out->module_id = read_u16_le(&buf[ESUP_OFST_MODULE_ID]);
    out->cmd_status = read_u16_le(&buf[ESUP_OFST_CMD_STATUS]);
    out->command = read_u16_le(&buf[ESUP_OFST_COMMAND]);
    out->type = read_u16_le(&buf[ESUP_OFST_TYPE]);
    out->data_len = data_len;
    out->data = (data_len > 0) ? &buf[ESUP_OFST_DATA] : NULL;
    return ESUP_OK;
}

/* ------------------------------------------------------------------------- */
/* Incremental reassembly (wire-free)                                        */
/* ------------------------------------------------------------------------- */

void esup_framer_reset(esup_framer_t* framer)
{
    if (framer == NULL)
        return;
    framer->len = 0;
    framer->pending = 0;
}

/**
 * @brief Discard the first @p count bytes from the reassembly buffer.
 *
 * @param framer Framer to modify.
 * @param count  Number of leading bytes to drop (clamped to the buffer length).
 */
static void framer_drop_front(esup_framer_t* framer, size_t count)
{
    if (count >= framer->len) {
        framer->len = 0;
        return;
    }
    memmove(framer->buf, &framer->buf[count], framer->len - count);
    framer->len -= count;
}

/**
 * @brief Reclaim the bytes of a previously returned frame.
 *
 * The frame returned by esup_framer_pop is left in place so its data pointer
 * stays valid; the next framer call reclaims those bytes first, preserving the
 * zero-copy contract.
 *
 * @param framer Framer to modify.
 */
static void framer_reclaim_pending(esup_framer_t* framer)
{
    if (framer->pending == 0)
        return;
    framer_drop_front(framer, framer->pending);
    framer->pending = 0;
}

/**
 * @brief Offset of the first header-magic candidate in the reassembly buffer.
 *
 * A candidate is a full magic match, or a partial match in the final 1..3 bytes
 * that could complete on the next push. Anything before it is line noise.
 *
 * @param framer Framer to scan.
 * @return Offset of the first candidate, or framer->len if none exists.
 */
static size_t framer_find_magic(const esup_framer_t* framer)
{
    if (framer->len == 0)
        return 0;

    for (size_t i = 0; i < framer->len; i++) {
        size_t avail = framer->len - i;

        if (avail >= ESUP_SIZE_HEADER) {
            if (has_magic(&framer->buf[i]))
                return i;
            continue;
        }

        /* Fewer than a full magic remains: keep it only if it is a prefix of
         * the magic, so the header can complete on the next push. */
        bool prefix = true;
        const uint8_t magic[ESUP_SIZE_HEADER] = {
            ESUP_MAGIC_0, ESUP_MAGIC_1, ESUP_MAGIC_2, ESUP_MAGIC_3
        };

        for (size_t k = 0; k < avail; k++) {
            if (framer->buf[i + k] != magic[k]) {
                prefix = false;
                break;
            }
        }
        if (prefix)
            return i;
    }
    return framer->len;
}

esup_ret_t esup_framer_push(esup_framer_t* framer, const uint8_t* bytes, size_t n)
{
    if (framer == NULL || (bytes == NULL && n > 0))
        return ESUP_FRAME_ERR_ARG;

    framer_reclaim_pending(framer);

    for (size_t i = 0; i < n; i++) {
        if (framer->len == ESUP_FRAME_MAX) {
            /* The buffer is full without yielding a frame, so the leading
             * candidate is false. Drop one byte to force resynchronisation and
             * make room for forward progress. */
            framer_drop_front(framer, 1);
        }
        framer->buf[framer->len] = bytes[i];
        framer->len++;
    }
    return ESUP_OK;
}

int esup_framer_pop(esup_framer_t* framer, esup_frame_t* out)
{
    if (framer == NULL || out == NULL)
        return 0; /* no frame (a null argument is a caller bug) */

    framer_reclaim_pending(framer);

    for (;;) {
        size_t magic_off = framer_find_magic(framer);

        if (magic_off > 0)
            framer_drop_front(framer, magic_off);
        if (framer->len < ESUP_OFST_DATA_LEN + ESUP_SIZE_DATA_LEN)
            return 0;

        uint16_t data_len = read_u16_le(&framer->buf[ESUP_OFST_DATA_LEN]);

        if (data_len > ESUP_DATA_MAX) {
            /* Header-shaped noise: the length is impossible. Skip past this
             * magic and keep hunting. */
            framer_drop_front(framer, 1);
            continue;
        }

        size_t wire_len = esup_frame_wire_len(data_len);

        if (framer->len < wire_len)
            return 0;

        esup_ret_t rc = esup_frame_decode(framer->buf, wire_len, out);

        if (rc == ESUP_OK) {
            /* The decoded fields point into the buffer, so the bytes are kept in
             * place and reclaimed on the next framer call (see
             * framer_reclaim_pending). This preserves the zero-copy contract. */
            ESUP_LOG_HEX("RX", framer->buf, wire_len);
            framer->pending = wire_len;
            return 1;
        }

        /* A complete-looking but invalid frame (CRC or length): the magic was
         * coincidental or the frame was corrupted. Drop one byte and rescan. */
        framer_drop_front(framer, 1);
    }
}

/* ------------------------------------------------------------------------- */
/* Wire-bound link operations                                                */
/* ------------------------------------------------------------------------- */

/*
 * The pure codec (esup_frame_encode/decode) and the reassembly loop
 * (esup_framer_pop) return errors on every speculative resync candidate, so they
 * are deliberately left un-logged: logging there would flood on line noise. The
 * wire-bound operations below are the real error boundary and are instrumented.
 */
esup_ret_t esup_framer_init(esup_framer_t* framer, const esup_wire_t* wire)
{
    if (framer == NULL || wire == NULL || wire->ops == NULL) {
        ESUP_LOGE("bad argument (framer=%p wire=%p)", (void*)framer,
                  (const void*)wire);
        return ESUP_FRAME_ERR_ARG;
    }
    framer->wire = *wire;
    esup_framer_reset(framer);
    return ESUP_OK;
}

esup_ret_t esup_framer_send(esup_framer_t* framer, const esup_frame_t* frame,
                     uint32_t deadline)
{
    size_t len = 0;
    esup_ret_t enc = esup_frame_encode(frame, framer->tx, sizeof(framer->tx),
                                       &len);

    if (enc != ESUP_OK) {
        ESUP_LOGE("encode failed (data_len=%u)",
                  frame != NULL ? frame->data_len : 0u);
        return enc; /* honest framer code (ARG/CAPACITY), not a fake IO */
    }
    ESUP_LOG_HEX("TX", framer->tx, len);

    esup_ret_t wret = esup_wire_write(&framer->wire, framer->tx, len, deadline);

    if (wret != ESUP_OK) {
        ESUP_LOGE("wire write failed (%zu bytes)", len);
        return ESUP_FRAME_ERR_IO | wret; /* attach the wire cause */
    }
    return ESUP_OK;
}

esup_ret_t esup_framer_recv(esup_framer_t* framer, uint32_t deadline)
{
    uint8_t scratch[128];
    size_t got = 0;
    esup_ret_t rc = esup_wire_read(&framer->wire, scratch, sizeof(scratch), &got,
                                   deadline);

    if (rc == ESUP_OK && got > 0)
        esup_framer_push(framer, scratch, got);
    else if (rc != ESUP_OK && rc != ESUP_WIRE_ERR_TIMEOUT) {
        ESUP_LOGE("wire read failed (ret=0x%08X)", (unsigned)rc);
        return ESUP_FRAME_ERR_IO | rc; /* attach the wire cause */
    }

    /* Drain whatever else is already buffered, without blocking. */
    for (;;) {
        rc = esup_wire_read(&framer->wire, scratch, sizeof(scratch), &got,
                            esup_framer_now(framer));
        if (rc == ESUP_OK && got > 0)
            esup_framer_push(framer, scratch, got);
        else
            break;
    }
    return ESUP_OK;
}

uint32_t esup_framer_now(const esup_framer_t* framer)
{
    return esup_wire_now_ms(&framer->wire);
}

esup_ret_t esup_framer_set_baud(esup_framer_t* framer, uint32_t baud)
{
    esup_ret_t wret = esup_wire_set_baud(&framer->wire, baud);

    if (wret != ESUP_OK) {
        ESUP_LOGE("wire set_baud failed (baud=%u)", baud);
        return wret; /* pass the wire cause through (IO or UNSUPPORTED) */
    }
    return ESUP_OK;
}
