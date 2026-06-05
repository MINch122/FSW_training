// LTRX Message Definitions - ICD command/payload struct definitions

#ifndef LTRX_MSGDEFS_H
#define LTRX_MSGDEFS_H

#include <stdint.h>
#include <stddef.h>

// Packed struct attribute for GCC/Clang
#if defined(__GNUC__) || defined(__clang__)
#define LTRX_PACKED __attribute__((packed)) // used for general command header
#else
#define LTRX_PACKED
#endif

// Node IDs per ICD Default IDs table
#define LTRX_NODE_OBC            10
#define LTRX_NODE_BEACON         61

// CurrentNode field values used in Type 11 & Type 13
#define LTRX_MSGNODE_OBC         10
#define LTRX_MSGNODE_BEACON      20
#define LTRX_MSGNODE_RELAY       30
#define LTRX_MSGNODE_GROUND      40
#define LTRX_MSGNODE_CLOUD       50
#define LTRX_MSGNODE_MISSION_CTL 60

// Type 10: Confirm Message Part receipt - StatusCode values
#define LTRX_PART_STATUS_SUCCESS    0
#define LTRX_PART_STATUS_LEN_ERR   10
#define LTRX_PART_STATUS_CRC_ERR   20
#define LTRX_PART_STATUS_OTHER_ERR 30

// Type 15: Previous Command Ack - StatusCode values
#define LTRX_ACK_STATUS_SUCCESS            0
#define LTRX_ACK_STATUS_LENGTH_MISMATCH   10
#define LTRX_ACK_STATUS_LENGTH_UNEXPECTED 12
#define LTRX_ACK_STATUS_HEADER_CORRUPT    14
#define LTRX_ACK_STATUS_CRC_MISMATCH      20
#define LTRX_ACK_STATUS_PARSE_FAILED      22
#define LTRX_ACK_STATUS_OTHER_ERR         30

// Type 13: Message Status update - ErrorCode values
#define LTRX_MSG_ERR_EXPIRED         10
#define LTRX_MSG_ERR_ACCESS_DENIED   20
#define LTRX_MSG_ERR_DATA_CORRUPTION 30

// Max part data bytes (this value is not written in ICD)
#ifndef LTRX_MAX_MESSAGE_PART_SIZE
#define LTRX_MAX_MESSAGE_PART_SIZE 256
#endif

// ErrorDescription field size shared across Type 10, 13, 15
#define LTRX_ERROR_DESC_SIZE 25





// General command header - 9 bytes total, little-endian
typedef struct LTRX_PACKED
{
    uint8_t  FromID;
    uint8_t  ToID;
    uint8_t  TypeID;
    uint16_t Length;   
    uint32_t CRC;      
} LTRX_BeaconCmdHeader_t;


// Type 5 / Type 6 payload - 10 bytes : Send Message Header / Offer to Receive
typedef struct LTRX_PACKED
{
    uint32_t MessageID;
    uint16_t MessageLength;
    uint32_t MessageCRC;
} LTRX_MessageHeader_Payload_t;


// Type 7 payload - 8 bytes : Request Message Part
typedef struct LTRX_PACKED
{
    uint32_t MessageID;
    uint16_t PartStartByte;
    uint16_t PartLength;
} LTRX_MessagePartRequest_Payload_t;


// Type 9 payload fixed header - 12 bytes, followed by part bytes : Send Message Part
typedef struct LTRX_PACKED
{
    uint32_t MessageID;
    uint16_t PartStartByte;
    uint16_t PartLength;
    uint32_t PartCRC;
} LTRX_MessagePartHeader_t;


// Type 10 payload - 34 bytes : Confirm Message Part receipt
typedef struct LTRX_PACKED
{
    uint32_t MessageID;
    uint16_t PartStartByte;
    uint16_t PartLength;
    uint8_t  StatusCode;
    uint8_t  ErrorDescription[LTRX_ERROR_DESC_SIZE]; // null-terminated ASCII
} LTRX_MessagePartReceipt_Payload_t;


// Type 11 payload - 5 bytes : Confirm Message receipt
typedef struct LTRX_PACKED
{
    uint32_t MessageID;
    uint8_t  CurrentNode;
} LTRX_MessageReceipt_Payload_t;


// Type 13 payload - 39 bytes : Message Status update
typedef struct LTRX_PACKED
{
    uint32_t MessageID;
    uint8_t  CurrentNode;
    uint64_t UpdateTimestamp; // unix epoch ms
    uint8_t  ErrorCode;
    uint8_t  ErrorDescription[LTRX_ERROR_DESC_SIZE]; // null-terminated ASCII
} LTRX_MessageStatus_Payload_t;


// Type 15 payload - 35 bytes : Previous Command Acknowledgement
typedef struct LTRX_PACKED
{
    uint8_t  CommandFromID;
    uint8_t  CommandToID;
    uint8_t  CommandTypeID;
    uint16_t CommandLength;
    uint32_t CommandCRC;
    uint8_t  StatusCode;
    uint8_t  ErrorDescription[LTRX_ERROR_DESC_SIZE]; // null-terminated ASCII
} LTRX_PrevCmdAck_Payload_t;



// Type 21 payload - 18 bytes : GNSS information
typedef struct LTRX_PACKED
{
    uint32_t UTCTimeMs;      // unix epoch ms
    int32_t  Latitude;       // 1 unit = 1e-7 deg
    int32_t  Longitude;      // 1 unit = 1e-7 deg
    uint32_t Altitude;       // 1 unit = 0.1 m, unsigned
    uint8_t  FixQuality;     // 0, 1, or 2
    uint8_t  NumSatellites;
} LTRX_GNSSInfo_Payload_t;


// Type 23 payload - 52 bytes : Beacon status
typedef struct LTRX_PACKED
{
    uint16_t Version;
    int16_t  Temperature;        // 0.1 degC
    int32_t  AngularVelocityX;   // 0.1 dps
    int32_t  AngularVelocityY;
    int32_t  AngularVelocityZ;
    int32_t  AccelerationX;      // 0.1 g
    int32_t  AccelerationY;
    int32_t  AccelerationZ;
    uint8_t  ConnectionQuality;  // 0..10
    uint8_t  BatteryIsCharging;  // 0 or 1
    uint16_t BatteryCapacity;    // 0.1%
    uint8_t  Reserved[20];
} LTRX_BeaconStatus_Payload_t;


// Type 24 payload - 70 bytes : Beacon Status full
typedef struct LTRX_PACKED
{
    // GNSS fields - 18 bytes
    uint32_t UTCTimeMs;          // unix epoch ms
    int32_t  Latitude;           // 1 unit = 1e-7 deg
    int32_t  Longitude;          // 1 unit = 1e-7 deg
    uint32_t Altitude;           // 1 unit = 0.1 m, unsigned
    uint8_t  FixQuality;         // 0, 1, or 2
    uint8_t  NumSatellites;

    // Beacon Status fields - 52 bytes
    uint16_t Version;
    int16_t  Temperature;        // 0.1 degC
    int32_t  AngularVelocityX;   // 0.1 dps
    int32_t  AngularVelocityY;
    int32_t  AngularVelocityZ;
    int32_t  AccelerationX;      // 0.1 g
    int32_t  AccelerationY;
    int32_t  AccelerationZ;
    uint8_t  ConnectionQuality;  // 0..10
    uint8_t  BatteryIsCharging;  // 0 or 1
    uint16_t BatteryCapacity;    // 0.1%
    uint8_t  Reserved[20];
} LTRX_BeaconStatusFull_Payload_t;

#endif /* LTRX_MSGDEFS_H */
