/**
 * @file oem_cmd.c
 * @brief OEM7 command sender.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#include <stdlib.h>
#include <string.h>

#include "oem_cmd.h"
#include "oem_io.h"
#include "oem_utils.h"


int oem_cmd_publish(int iface_idx,
                    oem_ushort msg_id,
                    oem_ushort body_size,
                    const void* body)
{
    oem_binary_header_t* cmd;
    size_t packet_size;
    oem_crc crc;
    int status;

    if (iface_idx < 0 || iface_idx >= OEM_IO_INTERFACES)
        return OEM_ERR_IO_IFACE_INDEX;

    packet_size = sizeof(oem_binary_header_t) + body_size + sizeof(crc);
    if ((cmd = calloc(1, packet_size)) == NULL) {
        oem_debug_error("cmd publish error: malloc failed for mid %d.\n",
                         msg_id);
        return OEM_ERR_NOMEM;
    }

    /**
     * Initialize the header.
     */
    cmd->sync[0]       = OEM_SYNC_BYTE1;
    cmd->sync[1]       = OEM_SYNC_BYTE2;
    cmd->sync[2]       = OEM_SYNC_BYTE3;
    cmd->headerLength  = sizeof(oem_binary_header_t);
    cmd->messageID     = msg_id;
    cmd->messageType   = OEM_MISSION_MSGTYPE;
    cmd->portAddress   = OEM_PORT_THIS;
    cmd->messageLength = body_size;

    /**
     * Append the body.
     */
    if (body && body_size > 0)
        memcpy(cmd + 1, body, body_size);

    /**
     * Append trailing CRC.
     */
    crc = oem_crc32(cmd, packet_size - sizeof(crc));
    memcpy(((uint8_t*) cmd) + packet_size - sizeof(crc),
           &crc,
           sizeof(crc));

    /**
     * Send out the packet.
     */
    status = oem_io_write(iface_idx, cmd, packet_size);

#if OEM_DEBUG
    if (status != OEM_OK)
        oem_debug_error("cmd publish error: msg_id %d, returned %d\n",
                         msg_id,
                         status);
#endif

    free(cmd);
    return status;
}

int oem_cmd_LOG(int iface_idx,
                oem_ushort msg_id,
                oem_enum port,
                oem_char type,
                oem_enum trigger,
                oem_double period,
                oem_double offset,
                oem_enum hold)
{
    oem_cmd_log body;

    memset(&body, 0, sizeof(body));

    body.port        = port;
    body.messageId   = msg_id;
    body.messageType = type;
    body.trigger     = trigger;
    if (trigger == OEM_TRIGGER_ONTIME) {
        body.period  = period;
        body.offset  = offset;
    }
    body.hold = hold;

    return oem_cmd_publish(iface_idx,
                           OEM_ID_CMD_LOG,
                           sizeof(body),
                           &body);
}

int oem_cmd_UNLOG(int iface_idx,
                  oem_enum port,
                  oem_ushort msg_id,
                  oem_char msg_type)
{
    oem_cmd_unlog body;

    body.port        = port;
    body.message     = msg_id;
    body.messageType = msg_type;
    body.reserved    = 0;

    return oem_cmd_publish(iface_idx,
                           OEM_ID_CMD_UNLOG,
                           sizeof(body),
                           &body);
}

int oem_cmd_UNLOGALL(int iface_idx,
                     oem_enum port,
                     oem_bool held)
{
    oem_cmd_unlog_all body;

    body.port = port;
    body.held = held;

    return oem_cmd_publish(iface_idx,
                           OEM_ID_CMD_UNLOGALL,
                           sizeof(body),
                           &body);
}

int oem_cmd_ELEVATIONCUTOFF(int iface_idx,
                            oem_enum constellation,
                            oem_float cutoff)
{
    oem_cmd_elevation_cutoff body;

    body.constellation = constellation;
    body.cutoff        = cutoff;
    body.reserved      = 0;

    return oem_cmd_publish(iface_idx,
                           OEM_ID_CMD_ELEVATIONCUTOFF,
                           sizeof(body),
                           &body);
}

int oem_cmd_INTERFACEMODE(int iface_idx,
                          oem_enum port,
                          oem_enum rx_type,
                          oem_enum tx_type,
                          oem_enum responses)
{
    oem_cmd_interface_mode body;

    body.port      = port;
    body.rxType    = rx_type;
    body.txType    = tx_type;
    body.responses = responses;

    return oem_cmd_publish(iface_idx,
                           OEM_ID_CMD_INTERFACEMODE,
                           sizeof(body),
                           &body);
}

int oem_cmd_SERIALCONFIG(int iface_idx,
                         oem_enum  port,
                         oem_ulong baud,
                         oem_enum  parity,
                         oem_ulong databits,
                         oem_ulong stopbits,
                         oem_enum  handshake,
                         oem_enum _break)
{
    oem_cmd_serial_config body;

    body.port = port;
    body.baud = baud;
    body.parity = parity;
    body.databits = databits;
    body.stopbits = stopbits;
    body.handshake = handshake;
    body._break = _break;

    return oem_cmd_publish(iface_idx,
                           OEM_ID_CMD_SERIALCONFIG,
                           sizeof(body),
                           &body);
}
