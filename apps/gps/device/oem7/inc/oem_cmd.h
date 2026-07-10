/**
 * @file oem_cmd.h
 * @brief OEM7 command sender.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2024.
 */
#ifndef _OEM_CMD_H_
#define _OEM_CMD_H_

#include "oem_config.h"
#include "msg/oem_msg_commands.h"


/**
 * @brief Assemble an OEM Command packet and send it. The binary header and
 *        the CRC are automatically appended. Uses oem_io_write() to send.
 *
 * @param iface_idx  I/O interface to send the command over.
 * @param msg_id      Command Message ID.
 * @param body       Message body (header and CRC not inclusive).
 * @param body_size Size of @a body.
 * @return OEM_OK:          Success.
 *         OEM_ERR_NOMEM:   Message buffer allocation failed.
 *         Otherwise the return value of the write callback attached to the
 *         I/O port.
 */
int oem_cmd_publish(int iface_idx,
                    oem_ushort  msg_id,
                    oem_ushort  body_size,
                    const void* body);

/**
 * @brief Send the LOG command. See the Reference Manual for details.
 *
 * @param iface_idx I/O interface to send the command over.
 * @param msg_id    Log Message ID to request.
 * @param port      OEM port to route the Log. See oem_port_t.
 * @param type      Log Message Type. See oem_msgtype_t. 0 recommended.
 * @param trigger   Log generation trigger. See oem_trigger_t.
 * @param period    Log issue interval. Applies only for the ONTIME trigger.
 * @param offset    Log issue offset.
 * @param hold      Boolean. Prevents unlogging if true.
 * @return See oem_cmd_publish().
 */
int oem_cmd_LOG(int iface_idx,
                oem_ushort msg_id,
                oem_enum port,
                oem_char type,
                oem_enum trigger,
                oem_double period,
                oem_double offset,
                oem_enum hold);

/**
 * @brief Send the LOG command with the ONCE trigger. See oem_cmd_LOG().
 *
 * @NOTE: If the requested log is currently unavailable, the next one will be
 *        output upon generation.
 *
 * @return See oem_cmd_publish().
 */
static inline int oem_cmd_LOG_once(int iface_idx,
                                  oem_ushort msg_id,
                                  oem_enum port)
{
    return oem_cmd_LOG(iface_idx,
                       msg_id,
                       port,
                       OEM_MSGTYPE_BINARY | OEM_MSGTYPE_ORIGINAL,
                       OEM_TRIGGER_ONCE,
                       0,
                       0,
                       0);
}

/**
 * @brief Send the LOG command with the ONTIME trigger. See oem_cmd_LOG().
 *
 * @return See oem_cmd_publish().
 */
static inline int oem_cmd_LOG_ontime(int iface_idx,
                                    oem_ushort msg_id,
                                    oem_enum port,
                                    oem_double period,
                                    oem_double offset)
{
    return oem_cmd_LOG(iface_idx,
                       msg_id,
                       port,
                       OEM_MSGTYPE_BINARY | OEM_MSGTYPE_ORIGINAL,
                       OEM_TRIGGER_ONTIME,
                       period,
                       offset,
                       0);
}

/**
 * @brief Send the LOG command with the ONCHANGED trigger. See oem_cmd_LOG().
 *
 * @return See oem_cmd_publish().
 */
static inline int oem_cmd_LOG_onchanged(int iface_idx,
                                       oem_ushort msg_id,
                                       oem_enum port)
{
    return oem_cmd_LOG(iface_idx,
                       msg_id,
                       port,
                       OEM_MSGTYPE_BINARY | OEM_MSGTYPE_ORIGINAL,
                       OEM_TRIGGER_ONCHANGED,
                       0,
                       0,
                       0);
}

/**
 * @brief Send the LOG command with the ONNEW trigger. See oem_cmd_LOG().
 *
 * @return See oem_cmd_publish().
 */
static inline int oem_cmd_LOG_onnew(int iface_idx,
                                   oem_ushort msg_id,
                                   oem_enum port)
{
    return oem_cmd_LOG(iface_idx,
                       msg_id,
                       port,
                       OEM_MSGTYPE_BINARY | OEM_MSGTYPE_ORIGINAL,
                       OEM_TRIGGER_ONNEW,
                       0,
                       0,
                       0);
}

/**
 * @brief Send the UNLOG command. See the Reference Manual for details.
 *
 * @param iface_idx I/O interface to send the command over.
 * @param port      OEM port where the Log is being sent on.
 * @param msg_id    Log Message ID to unlog.
 * @param msg_type
 * @return See oem_cmd_publish().
 */
int oem_cmd_UNLOG(int iface_idx,
                  oem_enum port,
                  oem_ushort msg_id,
                  oem_char msg_type);

/**
 * @brief Send the UNLOGALL command. See the Reference Manual for details.
 *
 * @return See oem_cmd_publish().
 */

/**
 * @brief
 *
 * @param iface_idx  I/O interface to send the command over.
 * @param port       OEM port where the Log is being sent on.
 * @param held
 * @return See oem_cmd_publish().
 */
int oem_cmd_UNLOGALL(int iface_idx,
                     oem_enum port,
                     oem_bool held);

/**
 * @brief Send the ELEVATIONCUTOFF command. See the Reference Manual for
 *        details.
 *
 * @return See oem_cmd_publish().
 */
int oem_cmd_ELEVATIONCUTOFF(int iface_idx,
                            oem_enum constellation,
                            oem_float cutoff);

/**
 * @brief Send the INTERFACEMODE command. See the Reference Manual for
 *        details.
 *
 * @return See oem_cmd_publish().
 */
int oem_cmd_INTERFACEMODE(int iface_idx,
                          oem_enum port,
                          oem_enum rx_type,
                          oem_enum tx_type,
                          oem_enum responses);

/**
 * @brief Send the SERIALCONFIG command. See the Reference Manual for details.
 *
 * @param iface_idx  I/O interface to send the command over.
 * @param port
 * @param baud
 * @param parity
 * @param databits
 * @param stopbits
 * @param handshake
 * @param _break
 * @return See oem_cmd_publish().
 */
int oem_cmd_SERIALCONFIG(int iface_idx,
                         oem_enum port,
                         oem_ulong baud,
                         oem_enum parity,
                         oem_ulong databits,
                         oem_ulong stopbits,
                         oem_enum handshake,
                         oem_enum _break);

#endif