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
 *   Specification for the STX command and telemetry
 *   message constant definitions.
 *
 *  For STX this is only the function/command code definitions
 */           
#ifndef STX_MSGDEFS_H
#define STX_MSGDEFS_H

#include "common_types.h"
#include "stx_fcncodes.h"
#include "enduro_stx.h"

// telemetry send gs -> obc
typedef struct STX_DisplayParam_Payload
{
    uint32 ValU32;                            /**< 32 bit unsigned integer value */
    int16  ValI16;                            /**< 16 bit signed integer value */
    char   ValStr[STX_STRING_VAL_LEN]; /**< An example string */
} STX_DisplayParam_Payload_t;

typedef struct STX_SetModuleId_Payload
{
    uint16 ModuleId;
} STX_SetModuleId_Payload_t;

typedef struct STX_Set1_Payload
{
    uint8 data;                            /**< 32 bit unsigned integer value */
} STX_Set1_Payload_t;

typedef struct STX_Set2_Payload
{
    float data;                          /**< 32 bit unsigned integer value */
} STX_Set2_Payload_t;

typedef struct STX_Set3_Payload
{
    uint16 data;                           /**< 32 bit unsigned integer value */
} STX_Set3_Payload_t;

#pragma pack(push, 1)
typedef struct STX_Set4_Payload
{
    uint8  symbol_rate;
    uint8  transmit_power;
    uint8  modcod;
    uint8  roll_off;
    uint8  pilot_signal;
    uint8  fec_frame_size;
    uint16 pretransmission_delay;
    float    center_frequency;
} STX_Set4_Payload_t;
#pragma pack(pop)

typedef struct STX_Set5_Payload
{
    uint8 modulator_interface_type;  
    uint8 lvds_io_type;              
} STX_Set5_Payload_t;

typedef struct {
    char filename_max[30];  // Null-terminated string
    int8_t filename_len;
} STX_DELFILE_Payload_t;

typedef struct __attribute__((__packed__)){
    char filename_max[30];  // Null-terminated string
    int8_t filename_len;
    uint32_t  file_size;                       // 파일 크기 (bytes)
} STX_CREATEFILE_Payload_t;

typedef struct __attribute__((__packed__)){
    char filename_max[30];  // Null-terminated string
    int8_t filename_len;
} STX_OPENFILE_Payload_t;

typedef struct __attribute__((__packed__)){
    char filename[64];
    uint32_t size;
    uint32_t offset;
    uint8_t interpacket_delay;
} STX_TLM_WRITEFILE_Payload_t;

typedef struct
{
    char filename_max[30];  // Null-terminated string
    int8_t filename_len;
} STX_SENDFILE_Payload_t;

typedef struct __attribute__((__packed__)){
    char filename[30];
    int8_t filename_len;
} STX_FWUPDATE_Payload_t;

/*  TLM COMMAND  */

typedef struct STX_GET_ALLPRAM
{
    uint8_t command_status;
    uint8  symbol_rate;
    uint8  transmit_power;
    uint8  modcod;
    uint8  roll_off;
    uint8  pilot_signal;
    uint8  fec_frame_size;
    uint16 pretransmission_delay;
    float    center_frequency;
} STX_GET_ALLPRAM_t;

typedef struct STX_GET_ModulationInterface
{
    uint8 modulator_interface_type;  
    uint8 lvds_io_type;              
} STX_GET_ModulationInterface_t;

typedef struct STX_HkTlm_Payload
{
    STX_GET_ALLPRAM_t ALLPRAM;
    STX_GET_ModulationInterface_t Modulator;
    uint8 SystemState;  
    uint8 StatusFlags;  
    float cputemperature;   
} STX_HkTlm_Payload_t;

/* REPLY */
typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    stx_dir_entry_t entries[STX_DIR_ENTRIES_MAX];
} DIR_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint32_t file_handle;
} CREATEFILE_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint32_t file_handle;
    uint32_t remaining;
    uint32_t packet_number;
    uint8_t err_count;
} WRITEFILE_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    stx_rep_file_open_t orp;
} OPENFILE_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint16_t packet_len;
    uint32_t packet_number;
    uint8_t data[ESUP_DATA_MAX];
} READFILE_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint8_t msps;
} GET_SYMBOLRATE_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint8_t dbm;
} GET_TX_POWER_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    float mhz;
} GET_CENTERFREQ_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint8_t modcod;
} GET_MODCOD_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint8_t roll_off;
} GET_ROLLOFF_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint8_t pilot;
} GET_PILOT_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint8_t frame;
} GET_FECFRAME_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    uint16_t ms;
} GET_PRETXDELAY_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    stx_params_t params;
} GET_ALLPARAM_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    stx_report_t report;
} GET_REPORT_REPLY_t;

typedef struct __attribute__((packed))
{
    esup_ret_t ret;
    stx_mod_data_iface_t iface;
} GET_MODDATAIFACE_REPLY_t;

#endif
