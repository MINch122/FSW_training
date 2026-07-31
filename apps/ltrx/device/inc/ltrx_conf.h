// LTRX Device Configuration - Beacon L-band Transceiver

#ifndef LTRX_CONF_H
#define LTRX_CONF_H

// CSP addressing is owned by the cFS app layer.
// The node id comes from cfe/modules/srl/config/default_cfe_srl_mission_cfg.h.
#ifndef LTRX_CSP_TX_PORT
#define LTRX_CSP_TX_PORT        18
#endif

#ifndef LTRX_CSP_RX_PORT
#define LTRX_CSP_RX_PORT        10
#endif

#define LTRX_DEFAULT_OBC_ID     10
#define LTRX_DEFAULT_BEACON_ID  61


// Command header size: FromID 1byte + ToID 1byte + TypeID 1byte + Length 2bytes + CRC 4bytes = 9 bytes
#define LTRX_BEACON_HEADER_SIZE 9


// ICD command type IDs
#define LTRX_BEACON_CMD_REQUEST_MSG_TX      3
#define LTRX_BEACON_CMD_SEND_MSG_HEADER     5
#define LTRX_BEACON_CMD_OFFER_RECEIVE_MSG   6
#define LTRX_BEACON_CMD_REQUEST_MSG_PART    7
#define LTRX_BEACON_CMD_CONFIRM_READY       8
#define LTRX_BEACON_CMD_SEND_MSG_PART       9
#define LTRX_BEACON_CMD_CONFIRM_PART_RX     10
#define LTRX_BEACON_CMD_CONFIRM_MSG_RX      11
#define LTRX_BEACON_CMD_MSG_STATUS_UPDATE   13
#define LTRX_BEACON_CMD_PREV_CMD_ACK        15
#define LTRX_BEACON_CMD_GNSS_INFO           21
#define LTRX_BEACON_CMD_BEACON_STATUS       23
#define LTRX_BEACON_CMD_BEACON_STATUS_FULL  24


// Message size limits (this value is not written in ICD)
#define LTRX_MAX_TOTAL_MESSAGE_SIZE  65536
#define LTRX_MAX_MESSAGE_PART_SIZE   240
#define LTRX_RECOMMENDED_PART_SIZE   LTRX_MAX_MESSAGE_PART_SIZE


// Retry and concurrency limits (this value is not written in ICD) 
#define LTRX_MAX_PART_RETRIES        3
#define LTRX_MAX_CONCURRENT_MESSAGES 10


// CRC-32/IEEE 802.3: poly=0x04C11DB7, init=0xFFFFFFFF, xorout=0xFFFFFFFF, reflected
#define LTRX_CRC_POLYNOMIAL     0x04C11DB7
#define LTRX_CRC_INITIAL        0xFFFFFFFF
#define LTRX_CRC_XOR_OUT        0xFFFFFFFF


// All wire data is little-endian 
#define LTRX_BYTE_ORDER_LITTLE_ENDIAN 1

// Timeout values in milliseconds
#define LTRX_CMD_TIMEOUT_MS     100
#define LTRX_ACK_TIMEOUT_MS     500

#endif /* LTRX_CONF_H */
