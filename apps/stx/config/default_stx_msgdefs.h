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
#include "../device/inc/esup.h"

typedef struct {
        uint8_t index;
        size_t size;
        char path[100];
} stx_custom_dir_entry_t;

// telemetry send gs -> obc
typedef struct STX_DisplayParam_Payload
{
    uint32 ValU32;                            /**< 32 bit unsigned integer value */
    int16  ValI16;                            /**< 16 bit signed integer value */
    char   ValStr[STX_STRING_VAL_LEN]; /**< An example string */
} STX_DisplayParam_Payload_t;

typedef struct STX_Set_Payload
{
    uint8 data;                            /**< 32 bit unsigned integer value */
} STX_Set_Payload_t;

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
    char filename_max[30];  // Null-terminated string
    int8_t filename_len;
    uint32_t  file_size;                       // 파일 크기 (bytes)
    uint32_t packet_number;
} STX_CREATEFILE_W_Payload_t;

typedef struct __attribute__((__packed__)){        // __attribute__((__packed__))
    uint16_t data_length;                           // 2-byte
    int32_t  file_handle;                           // 4-byte
    uint32_t packet_number;                         // 4-byte
    uint8_t  packet_data[ESUP_MAX_DATA_LENGTH];
} STX_ESUP_WRITEFILE_Payload_t;

typedef struct __attribute__((__packed__)){
    char filename[64];
    uint32_t size;
    uint32_t offset;
    uint8_t interpacket_delay;
} STX_TLM_WRITEFILE_Payload_t;

// typedef struct {
//     int32_t file_handle;  // File handle from Open_File
// } STX_READFILE_Payload_t;

typedef struct
{
    char filename_max[30];  // Null-terminated string
    int8_t filename_len;
} STX_SENDFILE_Payload_t;

/*************************************************************************/
/*
** Type definition (Sample App housekeeping)
** command send stx_app <-> obc 
*/
typedef struct __attribute__((__packed__)){

    uint8_t  commad_status;
    uint8_t  flag_MoreFiles;
    uint16_t file_cnt;
    uint8_t  listfile[ESUP_MAX_DATA_LENGTH];
} STX_DIR_t;

typedef struct __attribute__((__packed__)){
    char filename[31];   // 최대 30 + NULL
    uint32_t length;
} file_info_t;

typedef struct __attribute__((__packed__))
{
    uint8_t  commad_status;
    uint32_t  file_handle; 
} STX_FILE_CREATE_t;

typedef struct __attribute__((__packed__))
{
    uint8_t  commad_status;
    uint32_t  file_handle; 
    uint32_t  file_length; 
} STX_FILE_OPEN_t;

typedef struct __attribute__((__packed__))
{
    uint8_t  commad_status;
    uint16_t  Packet_length;
    uint32_t  Packet_number; 
    uint8_t  file_data[ESUP_MAX_DATA_LENGTH]; 
} STX_FILE_READ_t;

typedef struct STX_GET_U8
{
    uint8  commad_status;
    uint8  rxdata_val;
} STX_GET_U8_t;

typedef struct STX_GET_U16
{
    uint8  commad_status;
    uint16  rxdata_val;
} STX_GET_U16_t;

typedef struct STX_GET_FLOAT
{
    uint8  commad_status;
    float  rxdata_val;
} STX_GET_FLOAT_t;

/*  TLM COMMAND  */

typedef struct STX_SET_Tlm_Payload
{
    uint8 cstatus;
    uint8 Getresult;
}STX_SET_Tlm_Payload_t;

typedef struct STX_FILE_Tlm_Payload
{
    uint8 cstatus;
    uint8 Getresult;
}STX_FILE_Tlm_Payload_t;

typedef struct STX_GET_ALLPRAM
{
    uint8  symbol_rate;
    uint8  transmit_power;
    uint8  modcod;
    uint8  roll_off;
    uint8  pilot_signal;
    uint8  fec_frame_size;
    uint16 pretransmission_delay;
    float    center_frequency;
} STX_GET_ALLPRAM_t;

typedef struct STX_GET_Report
{
    uint8 SystemState;  
    uint8 StatusFlags;  
    uint16 Reserved;   
    float cputemperature;   
    uint32 Fmwversion;               
} STX_GET_Report_t;

typedef struct STX_GET_ModulationInterface
{
    uint8 modulator_interface_type;  
    uint8 lvds_io_type;              
} STX_GET_ModulationInterface_t;


typedef struct STX_GET_Tlm_Payload
{
    uint8 cstatus;
    uint8 symbolrate;
    uint8 txpower;
    float centerFreq;
    uint8 modcod;
    uint8 rolloff;
    uint8 pilotSignal;
    uint8 FECframesize;
    uint16 PretxDelay;
    STX_GET_ALLPRAM_t allpram;
    STX_GET_Report_t report;
    STX_GET_ModulationInterface_t Modulation;
    
}STX_GET_Tlm_Payload_t;

typedef struct STX_HkTlm_Payload
{
    STX_GET_ALLPRAM_t ALLPRAM;
    STX_GET_ModulationInterface_t Modulator;
    uint8 SystemState;  
    uint8 StatusFlags;  
    float cputemperature;   
} STX_HkTlm_Payload_t;

typedef struct STX_BCNTlm_Payload
{
    uint8 SystemState;  
    uint8 StatusFlags;  
} STX_BCNTlm_Payload_t;

#endif
