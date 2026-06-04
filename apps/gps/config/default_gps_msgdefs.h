/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
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
 *   Specification for the GPS command and telemetry
 *   message constant definitions.
 */
#ifndef DEFAULT_GPS_MSGDEFS_H
#define DEFAULT_GPS_MSGDEFS_H

#include "common_types.h"
#include "default_gps_fcncodes.h"

#define NATURALLY_ALIGNED

#define PACKED __attribute__((packed))

/**
 * Handler-MsgId-only payload template.
 */
typedef struct NATURALLY_ALIGNED {
    uint16 msgId;
} GPS_OEM_HandlerNoArgCmd_Payload_t;


/*
** OEM receiver command types.
*/
typedef struct PACKED {
    int interfaceIndex;

    uint16 msgId;
    uint8  type;
    uint8  padding;
    uint32 port;
    uint32 trigger;
    uint32 hold;
    double period;
    double offset;
} GPS_OEM_Cmd_Log_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int interfaceIndex;

    uint16 msgId;
    uint8  padding[2];
    uint32 port;
} GPS_OEM_Cmd_LogOnce_Payload_t;

typedef struct PACKED {
    int interfaceIndex;

    uint16 msgId;
    uint32 port;
    double period;
    double offset;
} GPS_OEM_Cmd_LogOnTime_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int interfaceIndex;

    uint16 msgId;
    uint8  padding[2];
    uint32 port;
} GPS_OEM_Cmd_LogOnChanged_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int interfaceIndex;

    uint16 msgId;
    uint8  padding[2];
    uint32 port;
} GPS_OEM_Cmd_LogOnNew_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int interfaceIndex;

    uint16 msgId;
    uint8  type;
    uint8  padding;
    uint32 port;
} GPS_OEM_Cmd_Unlog_Payload_t;

typedef struct PACKED {
    int interfaceIndex;

    uint32 port;
    bool held;
} GPS_OEM_Cmd_UnlogAll_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int interfaceIndex;

    uint32 constellation;
    float cutoff;
} GPS_OEM_Cmd_ElevationCutoff_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int interfaceIndex;

    uint32 port;
    uint32 rxType;
    uint32 txType;
    uint32 responses;
} GPS_OEM_Cmd_InterfaceMode_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int interfaceIndex;

    uint32 port;
    uint32 baud;
    uint32 parity;
    uint32 databits;
    uint32 stopbits;
    uint32 handshake;
    uint32 _break;
} GPS_OEM_Cmd_SerialConfig_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int interfaceIndex;

    uint16 msgId;
    uint16 bodylength;
    uint8 body[256];
} GPS_OEM_Cmd_Publish_Payload_t;


/*
** OEM log handler command types.
*/
typedef struct NATURALLY_ALIGNED {
    char name[16];
    uint16 msgId;
    uint16 msgLength;
} GPS_OEM_Log_HandlerRegister_Payload_t;

typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerUnregister_Payload_t;

typedef struct NATURALLY_ALIGNED {
    uint16 msgId;
    int32  options;
    char   libpath[64];
    char   funcName[32];
} GPS_OEM_Log_AddCallback_Payload_t;

typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_ClearCallbacks_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_GetHandlerHk_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_GetStat_Payload_t;

typedef struct NATURALLY_ALIGNED {
    uint16 msgId;
    uint8_t status;
    bool override;
} GPS_OEM_Log_HandlerSetStatus_Payload_t;

typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerActivate_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerDeactivate_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerGoDormant_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerGetStatus_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerWakeup_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerActivateAll_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerDeactivateAll_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_HandlerMarkBroken_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_GetMessageLength_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_GetHandlerName_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_ResetStat_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_RejectMissingCrc_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEM_Log_IgnoreMissingCrc_Payload_t;

/**
 * oem_log_get_recent_message() takes an offset, allowing a large message to be
 * retrieved in windows across multiple commands.
 */
typedef struct NATURALLY_ALIGNED {
    uint16 msgId;
    uint16 padding;
    uint32 offset;
} GPS_OEM_Log_GetRecentMessage_Payload_t;

/**
 * These two commands temper with the global handler lock from OUTSIDE the
 * driver task - which may invoke undefined behavior if the sync is off.
 * To prevent an unintentional execution, a magic key is first validated.
 */
typedef struct NATURALLY_ALIGNED {
    uint32  Magic; //  C01DCAFE
} GPS_OEM_Log_LockHandlers_Payload_t;

typedef struct NATURALLY_ALIGNED {
    uint32  Magic; //  A7EDECAF
} GPS_OEM_Log_UnlockHandlers_Payload_t;


/*************************************************************************/
/*
** Type definition (GPS housekeeping)
*/



typedef struct {
    uint32 dummy;
} GPS_HkTlm_Payload_t;










#endif
