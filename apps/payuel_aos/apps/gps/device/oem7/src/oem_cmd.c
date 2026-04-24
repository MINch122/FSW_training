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

/**
 * @brief Length of the buffer containing the entire command, including the CRC.
 */
#define OEM_CMD_BUFSIZ(msgPtr)      (sizeof(*msgPtr) + sizeof(oem_crc))


int OEM_AssemblePublishCmd(int portIndex,
                           oem_ushort mid,
                           oem_ushort bodyLength,
                           const void* body) {
    oem_binary_header_t* cmd;
    size_t packetSize;
    oem_crc crc;
    int status;

    packetSize = sizeof(oem_binary_header_t) + bodyLength + sizeof(crc);
    if ((cmd = calloc(1, packetSize)) == NULL) {
        DebugError("cmd publish error: malloc failed for mid %d.\n",
                   mid);
        return OEM_ERR_NOMEM;
    }

    /**
     * Initialize the header.
     */
    cmd->sync[0]       = OEM_SYNC_BYTE1;
    cmd->sync[1]       = OEM_SYNC_BYTE2;
    cmd->sync[2]       = OEM_SYNC_BYTE3;
    cmd->headerLength  = sizeof(oem_binary_header_t);
    cmd->messageID     = mid;
    cmd->messageType   = OEM_MISSION_MSGTYPE;
    cmd->portAddress   = OEM_PORT_THIS;
    cmd->messageLength = bodyLength;

    /**
     * Append the body.
     */
    if (body && bodyLength > 0) {
        memcpy(cmd + 1, body, bodyLength);
    }

    /**
     * Append trailing CRC.
     */
    crc = OEM_CalculateBlockCRC32(cmd,
                                  packetSize - sizeof(crc));
    memcpy(((uint8_t*) cmd) + packetSize - sizeof(crc),
           &crc,
           sizeof(crc));

// #if OEM_DEBUG
//     for (int i = 0; i < OEM_CMD_BUFSIZ(msg); i++) {
//         if (i != 0)
//             if (i % 32 == 0) printf("\n");
//             else if (i % 4 == 0) printf(" ");
//         printf("%02X ", ((uint8*)msg)[i]);
//     }
//     printf("\n");
// #endif

    /**
     * Send out the packet.
     */
    status = OEM_IO_PortWrite(portIndex, cmd, packetSize);

#if OEM_DEBUG
    if (status != OEM_OK) {
        DebugError("cmd publish error: mid %d, returned %d\n",
                   mid,
                   status);
    }
#endif

    free(cmd);
    return status;
}

int OEM_Cmd_LOG(int portIndex,
                oem_ushort msgId,
                oem_enum port,
                oem_char type,
                oem_enum trigger,
                oem_double period,
                oem_double offset,
                oem_enum hold) {
    oem_cmd_log body;

    memset(&body, 0, sizeof(body));

    body.port = port;
    body.messageId = msgId;
    body.messageType = type;
    body.trigger = trigger;
    if (trigger == OEM_TRIGGER_ONTIME) {
        body.period = period;
        body.offset = offset;
    }
    body.hold = hold;

    return OEM_AssemblePublishCmd(portIndex,
                                  OEM_ID_CMD_LOG,
                                  sizeof(body),
                                  &body);
}

int OEM_Cmd_UNLOG(int portIndex,
                  oem_enum port,
                  oem_ushort msgId,
                  oem_char msgType) {
    oem_cmd_unlog body;

    body.port = port;
    body.message = msgId;
    body.messageType = msgType;
    body.reserved = 0;

    return OEM_AssemblePublishCmd(portIndex,
                                  OEM_ID_CMD_UNLOG,
                                  sizeof(body),
                                  &body);
}

int OEM_Cmd_UNLOGALL(int portIndex,
                     oem_enum port,
                     oem_bool held) {
    oem_cmd_unlog_all body;

    body.port = port;
    body.held = held;

    return OEM_AssemblePublishCmd(portIndex,
                                  OEM_ID_CMD_UNLOGALL,
                                  sizeof(body),
                                  &body);
}

int OEM_Cmd_ELEVATIONCUTOFF(int portIndex,
                            oem_enum constellation,
                            oem_float cutoff) {
    oem_cmd_elevation_cutoff body;

    body.constellation = constellation;
    body.cutoff = cutoff;
    body.reserved = 0;

    return OEM_AssemblePublishCmd(portIndex,
                                  OEM_ID_CMD_ELEVATIONCUTOFF,
                                  sizeof(body),
                                  &body);
}

int OEM_Cmd_INTERFACEMODE(int portIndex,
                          oem_enum port,
                          oem_enum rxType,
                          oem_enum txType,
                          oem_enum responses) {
    oem_cmd_interface_mode body;

    body.port = port;
    body.rxType = rxType;
    body.txType = txType;
    body.responses = responses;

    return OEM_AssemblePublishCmd(portIndex,
                                  OEM_ID_CMD_INTERFACEMODE,
                                  sizeof(body),
                                  &body);
}

int OEM_Cmd_SERIALCONFIG(int portIndex,
                         oem_enum port,
                         oem_ulong baud,
                         oem_enum parity,
                         oem_ulong databits,
                         oem_ulong stopbits,
                         oem_enum handshake,
                         oem_enum _break) {
    oem_cmd_serial_config body;

    body.port = port;
    body.baud = baud;
    body.parity = parity;
    body.databits = databits;
    body.stopbits = stopbits;
    body.handshake = handshake;
    body._break = _break;

    return OEM_AssemblePublishCmd(portIndex,
                                  OEM_ID_CMD_SERIALCONFIG,
                                  sizeof(body),
                                  &body);
}
