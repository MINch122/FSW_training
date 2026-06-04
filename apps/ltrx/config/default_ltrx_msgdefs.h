/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as "core Flight System: Bootes"
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Specification for the LTRX command and telemetry message constant definitions.
 *
 * Reference: Beacon OBC ICD Rev 4
 */

#ifndef LTRX_MSGDEFS_H
#define LTRX_MSGDEFS_H

#include "common_types.h"  
#include <stdint.h>

/* ICD Type IDs */
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

/* Pack attribute */
#if defined(__GNUC__) || defined(__clang__)
#define LTRX_PACKED __attribute__((packed))
#else
#define LTRX_PACKED
#endif

/* Node IDs */
#define LTRX_NODE_OBC            10
#define LTRX_NODE_BEACON         61

/* Message tracking (Type 13 CurrentNode examples) */
#define LTRX_MSGNODE_OBC         10
#define LTRX_MSGNODE_BEACON      20
#define LTRX_MSGNODE_RELAY       30
#define LTRX_MSGNODE_GROUND      40
#define LTRX_MSGNODE_CLOUD       50
#define LTRX_MSGNODE_MISSION_CTL 60

/* --------------------------------------------------------------------
 * Status / error code constants (used in Type10/Type15/Type13)
 * -------------------------------------------------------------------- */
/* Type 10: Confirm Message Part receipt status codes */
#define LTRX_PART_STATUS_SUCCESS    0
#define LTRX_PART_STATUS_LEN_ERR   10
#define LTRX_PART_STATUS_CRC_ERR   20
#define LTRX_PART_STATUS_OTHER_ERR 30

/* Type 15: Previous Command Ack status codes (ICD page 18) */
#define LTRX_ACK_STATUS_SUCCESS            0
#define LTRX_ACK_STATUS_LENGTH_MISMATCH   10
#define LTRX_ACK_STATUS_LENGTH_UNEXPECTED 12
#define LTRX_ACK_STATUS_HEADER_CORRUPT    14
#define LTRX_ACK_STATUS_CRC_MISMATCH      20
#define LTRX_ACK_STATUS_PARSE_FAILED      22
#define LTRX_ACK_STATUS_OTHER_ERR         30

/* Type 13: Message Status error codes (ICD page 15) */
#define LTRX_MSG_ERR_EXPIRED         10
#define LTRX_MSG_ERR_ACCESS_DENIED   20
#define LTRX_MSG_ERR_DATA_CORRUPTION 30

/* Beacon generic command header (ICD: general command structure) */
typedef struct LTRX_PACKED
{
    uint8  FromID;   /* 1 */
    uint8  ToID;     /* 1 */
    uint8  TypeID;   /* 1 */
    uint16 Length;   /* 2 (payload length) */
    uint32 CRC;      /* 4 (payload CRC32) */
} LTRX_BeaconCmdHeader_t;

/* Type 5: Send Message Header payload (Beacon -> OBC) */
typedef struct LTRX_PACKED
{
    uint32 MessageID;
    uint16 MessageLength;
    uint32 MessageCRC;
} LTRX_MessageHeader_Payload_t;

/* Type 7: Request Message Part payload (Beacon -> OBC) */
typedef struct LTRX_PACKED
{
    uint32 MessageID;
    uint16 PartStartByte;
    uint16 PartLength;
} LTRX_MessagePartRequest_Payload_t;

/* Type 9: Send Message Part header (Beacon -> OBC) */
typedef struct LTRX_PACKED
{
    uint32 MessageID;
    uint16 PartStartByte;
    uint16 PartLength;
    uint32 PartCRC;
    /* bytes follow */
} LTRX_MessagePartHeader_t;

#ifndef LTRX_MAX_MESSAGE_PART_SIZE
#define LTRX_MAX_MESSAGE_PART_SIZE 256
#endif

/* Type 10: Confirm Message Part receipt payload (OBC -> Beacon)
 * ICD says 25 bytes ASCII */
typedef struct LTRX_PACKED
{
    uint32 MessageID;
    uint16 PartStartByte;
    uint16 PartLength;
    uint8  StatusCode;
    uint8  ErrorDescription[25];
} LTRX_MessagePartReceipt_Payload_t;

/* Type 11: Confirm Message receipt payload (OBC -> Beacon) */
typedef struct LTRX_PACKED
{
    uint32 MessageID;
    uint8  CurrentNode;
} LTRX_MessageReceipt_Payload_t;

/* Type 13: Message Status update payload (Beacon -> OBC)
 * Timestamp is uint64 (ms or unix time) */
typedef struct LTRX_PACKED
{
    uint32 MessageID;
    uint8  CurrentNode;
    uint64 UpdateTimestamp;
    uint8  ErrorCode;
    uint8  ErrorDescription[25];
} LTRX_MessageStatus_Payload_t;

/* Type 15: Previous Command Acknowledgement payload (Beacon -> OBC) */
typedef struct LTRX_PACKED
{
    uint8  CommandFromID;
    uint8  CommandToID;
    uint8  CommandTypeID;
    uint16 CommandLength;
    uint32 CommandCRC;
    uint8  StatusCode;
    uint8  ErrorDescription[25];
} LTRX_PrevCmdAck_Payload_t;

/* Type 21: GNSS information payload (Beacon -> OBC) */
typedef struct LTRX_PACKED
{
    uint32 UTCTimeMs;      /* unix epoch in ms */
    int32  Latitude;       /* 1 unit = 1e-7 deg */
    int32  Longitude;      /* 1 unit = 1e-7 deg */
    uint32 Altitude;       /* 1 unit = 0.1 m */
    uint8  FixQuality;     /* 0/1/2 */
    uint8  NumSatellites;
} LTRX_GNSSInfo_Payload_t;

/* Type 23: Beacon status payload (Beacon -> OBC) */
typedef struct LTRX_PACKED
{
    uint16 Version;
    int16  Temperature;        /* 0.1 degC */
    int32  AngularVelocityX;   /* 0.1 dps */
    int32  AngularVelocityY;
    int32  AngularVelocityZ;
    int32  AccelerationX;      /* 0.1 g */
    int32  AccelerationY;
    int32  AccelerationZ;
    uint8  ConnectionQuality;  /* 0..10 */
    uint8  BatteryIsCharging;  /* 0 or 1 */
    uint16 BatteryCapacity;    /* 0.1% */
    uint8  Reserved[20];
} LTRX_BeaconStatus_Payload_t;

/* Type 24: Beacon Status full (GNSS + Status) */
typedef struct LTRX_PACKED
{
    /* GNSS fields (18 bytes) */
    uint32 UTCTimeMs;
    int32  Latitude;
    int32  Longitude;
    uint32 Altitude;
    uint8  FixQuality;
    uint8  NumSatellites;

    /* Status fields (52 bytes) */
    uint16 Version;
    int16  Temperature;
    int32  AngularVelocityX;
    int32  AngularVelocityY;
    int32  AngularVelocityZ;
    int32  AccelerationX;
    int32  AccelerationY;
    int32  AccelerationZ;
    uint8  ConnectionQuality;
    uint8  BatteryIsCharging;
    uint16 BatteryCapacity;
    uint8  Reserved[20];
} LTRX_BeaconStatusFull_Payload_t;

#endif /* LTRX_MSGDEFS_H */