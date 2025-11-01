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
 * @brief Assembles an OEM Command packet and sends it. he binary header and
 *        the CRC are automatically appended. Uses OEM_IO_PortWrite() to send.
 * 
 * @param portIndex  I/O port descriptor specified by OEM_IO_PortInit().
 * @param msgId      Command Message ID.
 * @param body       Message body (header and CRC not inclusive).
 * @param bodyLength Size of @a body.
 * @return OEM_OK:          Success.
 *         OEM_ERR_NOMEM:   Message buffer allocation failed.
 *         Otherwise the return value of the write callback attached to the
 *         I/O port.
 */
int OEM_AssemblePublishCmd(int portIndex,
                           oem_ushort msgId,
                           oem_ushort bodyLength,
                           const void* body);

/**
 * @brief Sends the LOG command. See the Reference Manual for details.
 * 
 * @param portIndex I/O port descriptor specified by OEM_IO_PortInit().
 * @param msgId     Log Message ID to request.
 * @param port      OEM port to route the Log. See oem_port_t.
 * @param type      Log Message Type. See oem_msgtype_t. 0 recommended.
 * @param trigger   Log generation trigger. See oem_trigger_t.
 * @param period    Log issue interval. Applies only for the ONTIME trigger.
 * @param offset    Log issue offset.
 * @param hold      Boolean. Prevents unlogging if true.
 * @return See OEM_AssemblePublishCmd().
 */
int OEM_Cmd_LOG(int portIndex,
                oem_ushort msgId,
                oem_enum port,
                oem_char type,
                oem_enum trigger,
                oem_double period,
                oem_double offset,
                oem_enum hold);  

/**
 * @brief Sends the LOG command with the ONCE trigger. See OEM_Cmd_LOG().
 * 
 * @NOTE: If the requested log is currently unavailable, the next one will be
 *        output upon generation.
 * 
 * @return See OEM_AssemblePublishCmd().
 */
static inline int OEM_Cmd_LogOnce(int portIndex,
                                  oem_ushort msgId,
                                  oem_enum port)
{
    return OEM_Cmd_LOG(portIndex,
                       msgId,
                       port,
                       OEM_MSGTYPE_BINARY | OEM_MSGTYPE_ORIGINAL,
                       OEM_TRIGGER_ONCE,
                       0,
                       0,
                       0);
}

/**
 * @brief Sends the LOG command with the ONTIME trigger. See OEM_Cmd_LOG().
 * 
 * @return See OEM_AssemblePublishCmd().
 */
static inline int OEM_Cmd_LogOnTime(int portIndex,
                                    oem_ushort msgId,
                                    oem_enum port,
                                    oem_double period,
                                    oem_double offset)
{
    return OEM_Cmd_LOG(portIndex,
                       msgId,
                       port,
                       OEM_MSGTYPE_BINARY | OEM_MSGTYPE_ORIGINAL,
                       OEM_TRIGGER_ONTIME,
                       period,
                       offset,
                       0);
}

/**
 * @brief Sends the LOG command with the ONTIME trigger. See OEM_Cmd_LOG().
 * 
 * @return See OEM_AssemblePublishCmd().
 */
static inline int OEM_Cmd_LogOnChanged(int portIndex,
                                       oem_ushort msgId,
                                       oem_enum port)
{
    return OEM_Cmd_LOG(portIndex,
                       msgId,
                       port,
                       OEM_MSGTYPE_BINARY | OEM_MSGTYPE_ORIGINAL,
                       OEM_TRIGGER_ONCNANGED,
                       0,
                       0,
                       0);
}

/**
 * @brief Sends the LOG command with the ONNEW trigger. See OEM_Cmd_LOG().
 * 
 * @return See OEM_AssemblePublishCmd().
 */
static inline int OEM_Cmd_LogOnNew(int portIndex,
                                   oem_ushort msgId,
                                   oem_enum port)
{
    return OEM_Cmd_LOG(portIndex,
                       msgId,
                       port,
                       OEM_MSGTYPE_BINARY | OEM_MSGTYPE_ORIGINAL,
                       OEM_TRIGGER_ONNEW,
                       0,
                       0,
                       0);
}

/**
 * @brief Sends the UNLOG command. See the Reference Manual for details.
 * 
 * @param portIndex I/O port descriptor specified by OEM_IO_PortInit().
 * @param port      OEM port where the Log is being sent on.
 * @param msgId     Log Message ID to unlog.
 * @param msgType   
 * @return See OEM_AssemblePublishCmd(). 
 */
int OEM_Cmd_UNLOG(int portIndex,
                  oem_enum port,
                  oem_ushort msgId,
                  oem_char msgType);

/**
 * @brief Sends the UNLOGALL command. See the Reference Manual for details.
 * 
 * @return See OEM_AssemblePublishCmd().
 */

/**
 * @brief 
 * 
 * @param portIndex 
 * @param port 
 * @param held 
 * @return int 
 */
int OEM_Cmd_UNLOGALL(int portIndex,
                     oem_enum port,
                     oem_bool held);

/**
 * @brief Sends the ELEVATIONCUTOFF command. See the Reference Manual for 
 *        details.
 * 
 * @return See OEM_AssemblePublishCmd().
 */
int OEM_Cmd_ELEVATIONCUTOFF(int portIndex,
                            oem_enum constellation,
                            oem_float cutoff);

/**
 * @brief Sends the INTERFACEMODE command. See the Reference Manual for 
 *        details.
 * 
 * @return See OEM_AssemblePublishCmd().
 */
int OEM_Cmd_INTERFACEMODE(int portIndex,
                          oem_enum port,
                          oem_enum rxType,
                          oem_enum txType,
                          oem_enum responses);

/**
 * @brief Sends the SERIALCONFIG command. See the Reference Manual for details.
 *        
 * @return See OEM_AssemblePublishCmd().
 */

/**
 * @brief 
 * 
 * @param portIndex I/O port descriptor specified by OEM_IO_PortInit().
 * @param port      
 * @param baud      
 * @param parity    
 * @param databits  
 * @param stopbits  
 * @param handshake 
 * @param _break    
 * @return int 
 */
int OEM_Cmd_SERIALCONFIG(int portIndex,
                         oem_enum port,
                         oem_ulong baud,
                         oem_enum parity,
                         oem_ulong databits,
                         oem_ulong stopbits,
                         oem_enum handshake,
                         oem_enum _break);

#endif