#ifndef _OEM_MSG_LOG_REQ_H_
#define _OEM_MSG_LOG_REQ_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_CMD_LOG 1

/**
 * @struct oem_cmd_log
 * @brief Log request command. MID = 1.
 *
 * @details A log falls in one of the following three types:
 *          1) Regularly generated "Synch" type. ONTIME is recommended. ONNEW
 *             and ONCHANGED are illegal.
 *          2) Irregularly generated "Asynch" type. ONCHANGED or ONCE triggers
 *             are recommended.
 *          3) "Polled" types are generated on demand. ONCE or ONTIME triggers
 *              are recommended, ONNEW and ONCHANGED are illegal.
 *          e.g., VERSION is generated only by request, which makes it a 
 *                polled type. BESTXYZ is automatically generated (synch).
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif
    /**
     * @brief Output port. See Table 4: Detailed Port Identifier on page 34.
     */
    oem_enum    port;

    oem_ushort  messageId;

    /**
     * @brief   Message types.
     *          bits 0-4: Measurement source (0 for a single ANT).
     *          bits 5-6: Format (0 = binary, 1 = ASCII,
     *                    2 = Abbr. ASCII, 3 = Reserved).
     *          bit 7: Response bit (0 = original, 1 = response).
     */
    oem_char    messageType;

    oem_char    reserved;

    /**
     * @brief   Trigger enumeration
     *          0 (ONNEW)       : Does not output current message but outputs
     *                            when the message is updated (not necessarily
     *                            changed).
     *          1 (ONCHANGED)   : Output the current message and continue to
     *                            output when the message is changed.
     *          2 (ONTIME)      : Output on a time interval.
     *          3 (ONNEXT)      : Output only the next message.
     *          4 (ONCE)        : Output only the current message. If not,
     *                            the next one is output when available.
     *          5 (ONMARK)      : Output when a pulse is detected on the MK1.
     */
    oem_enum    trigger;

    /**
     * @brief   Log period for the ONTIME trigger (in seconds).
     */
    oem_double  period;

    /**
     * @brief   Period offset for the ONTIME trigger (in seconds). Must be an
     *          integer smaller than the period. For example, to log data at 5
     *          second after every minute, set the period to 60 and the offset to 5.
     */
    oem_double  offset;

    /**
     * @brief   Boolean. If set, prevents the log from being removed by the
     *          UNLOGALL command.
     */
    oem_enum   hold;

} OEM_PACK oem_cmd_log;

#endif