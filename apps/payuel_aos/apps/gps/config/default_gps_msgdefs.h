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

#define GRX_CMD_OEM_HANDLER_NAME_LEN 16

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
    int portIndex;

    uint16 msgId;
    uint8  type;
    uint8  padding;
    uint32 port;
    uint32 trigger;
    uint32 hold;
    double period;
    double offset;
} GPS_OEMCmd_LogCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int portIndex;

    uint16 msgId;
    uint8  padding[2];
    uint32 port;
} GPS_OEMCmd_LogOnceCmd_Payload_t;

typedef struct PACKED {
    int portIndex;

    uint16 msgId;
    uint32 port;
    double period;
    double offset;
} GPS_OEMCmd_LogOnTimeCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int portIndex;

    uint16 msgId;
    uint8  padding[2];
    uint32 port;
} GPS_OEMCmd_LogOnChangedCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int portIndex;

    uint16 msgId;
    uint8  padding[2];
    uint32 port;
} GPS_OEMCmd_LogOnNewCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int portIndex;

    uint16 msgId;
    uint8  type;
    uint8  padding;
    uint32 port;
} GPS_OEMCmd_UnlogCmd_Payload_t;

typedef struct PACKED {
    int portIndex;

    uint32 port;
    bool held;
} GPS_OEMCmd_UnlogAllCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int portIndex;

    uint32 constellation;
    float cutoff;
} GPS_OEMCmd_ElevationCutoffCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int portIndex;

    uint32 port;
    uint32 rxType;
    uint32 txType;
    uint32 responses;
} GPS_OEMCmd_InterfaceModeCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int portIndex;

    uint32 port;
    uint32 baud;
    uint32 parity;
    uint32 databits;
    uint32 stopbits;
    uint32 handshake;
    uint32 _break;
} GPS_OEMCmd_SerialConfigCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    int portIndex;

    uint16 msgId;
    uint16 bodylength;
    uint8 body[256];
} GPS_OEMCmd_PublishCmd_Payload_t;


/*
** OEM log handler command types.
*/
typedef struct NATURALLY_ALIGNED {
    char name[16];
    uint16 msgId;
    uint16 msgLength;
} GPS_OEMLog_HandlerRegisterCmd_Payload_t;

typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_HandlerUnregisterCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    uint16 msgId;
    int32  options;
    char   libpath[64];
    char   funcName[32];
} GPS_OEMLog_AddCallbackCmd_Payload_t;

typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_ClearCallbackCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_GetHandlerHkCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_GetMsgStatCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    uint16 msgId;
    uint8_t status;
    bool override;
} GPS_OEMLog_SetHandlerStatusCmd_Payload_t;

typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_HandlerActivateCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_HandlerDeactivateCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_HandlerGoDormantCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_GetHandlerStatusCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_HandlerWakeupCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_HandlerActivateAllCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_HandlerDeactivateAllCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_HandlerSetBrokenCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_GetHandlerMsgLengthCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_GetHandlerNameCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_ResetHandlerCountersCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_DumpRecentMsgCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_IgnoreChecksumCmd_Payload_t;
typedef GPS_OEM_HandlerNoArgCmd_Payload_t GPS_OEMLog_DoNotIgnoreChecksumCmd_Payload_t;

/**
 * These two commands temper with the global handler lock from OUTSIDE the
 * driver task - which may invoke undefined behavior if the sync is off.
 * To prevent an unintentional execution, a magic key is first validated.
 */
typedef struct NATURALLY_ALIGNED {
    uint32  Magic; //  C01DCAFE
} GPS_OEMLog_LockHandlersCmd_Payload_t;

typedef struct NATURALLY_ALIGNED {
    uint32  Magic; //  A7EDECAF
} GPS_OEMLog_UnlockHandlersCmd_Payload_t;


/*************************************************************************/
/*
** Type definition (GPS housekeeping)
*/



typedef struct {
    uint32 dummy;
} GPS_HkTlm_Payload_t;










#endif
