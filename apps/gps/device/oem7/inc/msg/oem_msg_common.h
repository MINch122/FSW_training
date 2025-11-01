#ifndef _OEM_MSG_COMMON_H_
#define _OEM_MSG_COMMON_H_


#include "oem_basetype.h"
#include "oem_ports.h"


/**
 * @brief Log message types.
 */
typedef enum {
    OEM_MSGTYPE_MASK_SOURCE = 0x1F,

    OEM_MSGTYPE_MASK_FORMAT = 0x60,
    OEM_MSGTYPE_BINARY      = 0x00,
    OEM_MSGTYPE_ASCII       = 0x20,
    OEM_MSGTYPE_ABBR_ASCII  = 0x40,

    OEM_MSGTYPE_MASK_RESPONSE = 0x80,
    OEM_MSGTYPE_ORIGINAL    = 0x00,
    OEM_MSGTYPE_RESPONSE    = 0x80,
} oem_msgtype_t;


/**
 * @brief Log trigger types.
 */
typedef enum {
    OEM_TRIGGER_ONNEW       = 0,
    OEM_TRIGGER_ONCNANGED   = 1,
    OEM_TRIGGER_ONTIME      = 2,
    OEM_TRIGGER_ONNEXT      = 3,
    OEM_TRIGGER_ONCE        = 4,
    OEM_TRIGGER_ONMARK      = 5,
} oem_trigger_t;


typedef enum {
    /**
     * @brief No satellite has been tracked and the current time is unknown.
     *        The reference week and second are set to 0.
     */
    TIME_UNKNOWN = 20,

    /**
     * @brief An approximate time input has been applied by the SETAPPROXIMATE
     *        command.
     */
    TIME_APPROXIMATE = 60,

    /**
     * @brief The receiver time is approaching coarse precision (10 ms resolution).
     */
    TIME_COARSEADJUSTING = 80,

    /**
     * @brief The receiver time is set to coarse precision but not being steered
     *        (The CLOCKADJUST switch is disabled).
     */
    TIME_COARSE = 100,

    /**
     * @brief The receiver time is set to coarse precision and internally being
     *        steered to minimize the range bias.
     */
    TIME_COARSESTEERING = 120,

    /**
     * @brief The position has been lost and the clock is not being steered.
     */
    TIME_FREEWHEELING = 130,

    /**
     * @brief Time is adjusting to fine precision (1 us resolution).
     *
     */
    TIME_FINEADJUSTING = 140,

    /**
     * @brief The receiver time is set to fine precision but not being steered
     *        (The CLOCKADJUST switch is disabled).
     */
    TIME_FINE = 160,

    /**
     * @brief The solution from the primary satellite system is not obtained
     *        and the backup system is being used for steering.
     */
    TIME_FINEBACKUPSTEERING = 170,

    /**
     * @brief The receiver time is set to fine precision and internally being
     *        steered to minimize the range bias.
     */
    TIME_FINESTEERING = 180,

    /**
     * @brief Time is from a satellite.
     */
    TIME_SATTIME = 200,

} oem_timestat;


/**
 * @struct oem_binary_header_t
 * @brief Common binary message header.
 */
typedef struct {
    /**
     * @brief   Sync bytes. Always filled with 0xAA, 0x44 and 0x12.
     */
    oem_uchar   sync[3];

    /**
     * @brief   Length of the header in bytes (including sync[]).
     */
    oem_uchar   headerLength;

    /**
     * @brief   Message ID number of the log.
     */
    oem_ushort  messageID;

    /**
     * @brief   Message type.
     *          bits 0-4: Measurement source (0 for single-ANT receivers).
     *          bits 5-6: Format (0 = Binary, 1 = ASCII, 2 = Abbr. ASCII, 3 = Reserved).
     *          bit  7: Response bit (0 = original, 1 = response).
     */
    oem_char    messageType;

    /**
     * @brief   Log port identifier.
     */
    oem_uchar   portAddress;

    /**
     * @brief   Length of the message body in bytes. Header and CRC not included.
     */
    oem_ushort  messageLength;

    /**
     * @brief   A number which counts down from N-1 to 0 where N is the number of
     *          related logs. Most logs come out one at a time, setting this value to 0.
     */
    oem_ushort  sequence;

    /**
     * @brief   Percentage of time of the processor being idle. 0% indicates fully
     *          occupied. Ignored on input.
     */
    oem_uchar   idleTime;

    /**
     * @brief   Quality of the GPS reference time. 1-byte enum.
     */
    oem_uchar   timeStatus;  // This is a 1-byte enum.

    /**
     * @brief   GPS reference week number (0 - ).
     */
    oem_ushort  week;

    /**
     * @brief   Milliseconds from the beginning of the GPS reference week (0 - 604800000).
     */
    oem_gpsec   ms;

    /**
     * @brief   See Table 182: Receiver Status on page 760 of the manual.
     *          Ignored on input.
     */
    oem_ulong   receiverStatus;

    /** @brief   Unused. */
    oem_ushort  reserved;

    /**
     * @brief   Software build number. Ignored on input.
     */
    oem_ushort  receiverSwVersion;

} OEM_PACK oem_binary_header_t;


/**
 * @struct oem_binary_header_t
 * @brief Common binary response.
 */
typedef struct OEM_PACK {
    /**
     * @brief Common binary header.
     */
    oem_binary_header_t header;

    /**
     * @brief Response enumeration (OK = 1). See Table 210.
     */
    oem_enum            responseId;

    /**
     * @brief Human-readable response (ASCII).
     */
    char                response[];
} oem_binary_response;

#endif
