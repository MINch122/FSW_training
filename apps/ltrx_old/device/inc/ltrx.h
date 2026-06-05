// LTRX Device Driver Interface - Beacon L-band Transceiver

#ifndef LTRX_H
#define LTRX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ltrx_conf.h"
#include "ltrx_msgdefs.h"


// Return codes - range -1..-6
typedef enum {
    LTRX_SUCCESS          = 0,
    LTRX_ERROR            = -1,
    LTRX_ERROR_NULL_PTR   = -2,
    LTRX_ERROR_TIMEOUT    = -3,
    LTRX_ERROR_CRC        = -4,
    LTRX_ERROR_LENGTH     = -5,
    LTRX_ERROR_PROTOCOL   = -6,
} LTRX_ReturnCode_t;


// Logging abstraction - replace with OS_printf or CFE_EVS_SendEvent for cFS integration
  #ifndef LTRX_LOG
  #include "osapi.h"
  #define LTRX_LOG(...) OS_printf(__VA_ARGS__)
  #endif


// Serial interface wrapper - returns bytes received >=0 when success, <0 when error
// rx_size=-1 tells transport to accept any response length
typedef int32_t (*LTRX_TransportFn_t)(const void *tx_buf,
                                      uint16_t    tx_len,
                                      void       *rx_buf,
                                      int32_t     rx_size,
                                      uint16_t    timeout_ms);


// Register platform transport before calling any LTRX API
void LTRX_RegisterTransport(LTRX_TransportFn_t fn);


// CRC-32/IEEE 802.3 with input and output reflection
uint32_t LTRX_CalculateCRC32(const void *data, uint16_t length);
int32_t  LTRX_VerifyCRC32(const void *data, uint16_t length, uint32_t expected_crc);





// Build or parse the 9-byte little-endian command header
int32_t LTRX_BuildCommandHeader(LTRX_BeaconCmdHeader_t *header,
                                uint8_t  from_id,
                                uint8_t  to_id,
                                uint8_t  type_id,
                                uint16_t payload_length,
                                uint32_t payload_crc);


int32_t LTRX_ParseCommandHeader(LTRX_BeaconCmdHeader_t *header,
                                const void *raw_data);




// Low-level send: serialise header + payload and transmit via registered transport
int32_t LTRX_SendCommand(const LTRX_BeaconCmdHeader_t *header,
                         const void *payload,
                         uint16_t    payload_length);

// Low-level receive: read one command, verify CRC, copy payload into caller buffer
int32_t LTRX_ReceiveCommand(LTRX_BeaconCmdHeader_t *header,
                             void     *payload,
                             uint16_t  max_payload_length,
                             uint16_t *actual_length,
                             uint32_t  timeout_ms);





// Downstream : OBC -> Beacon

// Type 5: Send Message Header - respond to Type 3 with message metadata
int32_t LTRX_SendMessageHeader(uint32_t message_id,
                               uint16_t message_length,
                               uint32_t message_crc);

// Type 9: Send Message Part - send requested part bytes to Beacon
int32_t LTRX_SendMessagePart(uint32_t    message_id,
                              uint16_t    part_start_byte,
                              uint16_t    part_length,
                              const void *part_data);

// Type 10: Confirm Message Part receipt - ICD ErrorDescription is fixed 25 bytes, null-terminated
int32_t LTRX_ConfirmMessagePartReceipt(uint32_t    message_id,
                                       uint16_t    part_start_byte,
                                       uint16_t    part_length,
                                       uint8_t     status_code,
                                       const char *error_desc);

// Type 11: Confirm Message receipt - sent after full message integrity verified
int32_t LTRX_ConfirmMessageReceipt(uint32_t message_id, uint8_t current_node);





// Upstream : Beacon -> OBC

// Type 6: Receive and parse Offer to receive a Message pushed by Beacon
int32_t LTRX_ReceiveOfferMessage(LTRX_MessageHeader_Payload_t *offer);

// Type 8: Confirm ready for Message - OBC tells Beacon it is ready to receive
int32_t LTRX_ConfirmReadyForMessage(void);

// Type 7: Request Message Part - ask Beacon to send a specific part
int32_t LTRX_RequestMessagePart(uint32_t message_id,
                                uint16_t part_start_byte,
                                uint16_t part_length);

// Type 9: Receive and parse a Message Part sent by Beacon - upstream direction
int32_t LTRX_ReceiveMessagePart(LTRX_MessagePartHeader_t *part_hdr,
                                void     *part_buf,
                                uint16_t  part_buf_size,
                                uint16_t *actual_part_bytes);





// Telemetry :  Beacon pushes, OBC receives

// Type 21: Receive GNSS information pushed by Beacon
int32_t LTRX_GetGNSSInfo(LTRX_GNSSInfo_Payload_t *gnss_info);

// Type 23: Receive Beacon status pushed by Beacon
int32_t LTRX_GetBeaconStatus(LTRX_BeaconStatus_Payload_t *beacon_status);

// Type 24: Receive Full Beacon status - GNSS + Beacon Status combined
int32_t LTRX_GetBeaconStatusFull(LTRX_BeaconStatusFull_Payload_t *beacon_status_full);


// Type 13: Receive Message Status update pushed by Beacon
int32_t LTRX_GetMessageStatusUpdate(LTRX_MessageStatus_Payload_t *status_update);






// Acknowledgement 

// Type 15: Previous Command Acknowledgement - ICD ErrorDescription is fixed 25 bytes, null-terminated
int32_t LTRX_SendCommandAck(const LTRX_BeaconCmdHeader_t *cmd_header,
                            uint8_t     status_code,
                            const char *error_desc);

#endif /* LTRX_H */
