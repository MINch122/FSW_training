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
 *   Specification for the PAYUEL_LGPM command and telemetry
 *   message constant definitions.
 *
 *  For PAYUEL_LGPM this is only the function/command code definitions
 */
#ifndef PAYUEL_LGPM_MSGDEFS_H
#define PAYUEL_LGPM_MSGDEFS_H

#include "common_types.h"
#include "payuel_lgpm_fcncodes.h"



/*************************************************************************/
/*
** OBC to Payload (CMD code : 0x23)
*/

# define PWR_cmd_code 0x23
# define RWA_cmd_code 0x20


#pragma pack(push, 1) 


typedef struct {
    uint8 cmd_code;  
    uint8 sync;      
    uint8 length;    
} PWR_CommonHeader_cmd_t;

typedef struct {
    uint8 cmd_code;
    uint8 sync;      
    uint8 length;    
} RWA_CommonHeader_cmd_t;

typedef struct PAYUEL_LGPM_OBC2Payload_MCU_ALIVE_CHECK_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 MCU_ALIVE_CHECK;
    uint32 SYNC_Timestamp;                            
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_MCU_ALIVE_CHECK_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_3V3_PWR_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_ON_3V3;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_3V3_PWR_ON_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_3V3_PWR_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_OFF_3V3;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_3V3_PWR_OFF_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_MAIN_BOOST_SW_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 MAIN_BOOST_SW_ON;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_MAIN_BOOST_SW_ON_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_MAIN_BOOST_SW_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 MAIN_BOOST_SW_OFF;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_MAIN_BOOST_SW_OFF_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_SUB_BOOST_SW_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 SUB_BOOST_SW_ON;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_SUB_BOOST_SW_ON_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_SUB_BOOST_SW_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 SUB_BOOST_SW_OFF;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_SUB_BOOST_SW_OFF_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_V28_MAIN_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V28_MAIN_ON;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_V28_MAIN_ON_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_V28_MAIN_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V28_MAIN_OFF;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_V28_MAIN_OFF_Payload_t;


typedef struct PAYUEL_LGPM_OBC2Payload_V28_SUB_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V28_SUB_ON;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_V28_SUB_ON_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_V28_SUB_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V28_SUB_OFF;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_V28_SUB_OFF_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_V12_MAIN_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V12_MAIN_ON;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_V12_MAIN_ON_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_V12_MAIN_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V12_MAIN_OFF;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_V12_MAIN_OFF_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_PWR_SENSE_INFO_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_SENSE_INFO;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_PWR_SENSE_INFO_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_PWR_SEQ_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_SEQ_ON;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_PWR_SEQ_ON_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_PWR_SEQ_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_SEQ_OFF;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_PWR_SEQ_OFF_Payload_t;




/*************************************************************************/
/*
** OBC to Payload (CMD code : 0x20)
*/

typedef struct PAYUEL_LGPM_OBC2Payload_RWA_CONTROL_Payload
{
    RWA_CommonHeader_cmd_t header;
    uint8 RWA_CONTROL;       
    int16 TargetSpeed_RPM;    
    int16 TargetAcc_RPM;
    int16 Operating_Time;
    uint8 sub_index;
    uint8 Checksum;                
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_RWA_CONTROL_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_RWA_PWR_ON_Payload
{
    RWA_CommonHeader_cmd_t header;
    uint8 RWA_PWR_ON;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_RWA_PWR_ON_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_RWA_PWR_OFF_Payload
{
    RWA_CommonHeader_cmd_t header;
    uint8 RWA_PWR_OFF;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_RWA_PWR_OFF_Payload_t;

typedef struct PAYUEL_LGPM_OBC2Payload_RWA_SENSE_INFO_Payload
{
    RWA_CommonHeader_cmd_t header;
    uint8 RWA_SENSE_INFO;                     
    uint16 CRC16; 
} PAYUEL_LGPM_OBC2Payload_RWA_SENSE_INFO_Payload_t;



/*************************************************************************/
/*
** Payload to OBC(CMD code : 0x23)
*/

typedef struct PAYUEL_LGPM_Payload2OBC_MCU_ALIVE_CHECK_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 MCU_ALIVE_CHECK;
    uint32 Execution_Timestamp;    
    char Reply_Message[12];                        
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_MCU_ALIVE_CHECK_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_3V3_PWR_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_ON_3V3;
    uint32 Execution_Timestamp;    
    char Reply_Message[30];                        
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_3V3_PWR_ON_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_3V3_PWR_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_OFF_3V3;
    uint32 Execution_Timestamp;    
    char Reply_Message[30];                        
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_3V3_PWR_OFF_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_MAIN_BOOST_SW_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 MAIN_BOOST_SW_ON;
    uint32 Execution_Timestamp;    
    char Reply_Message[23];                       
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_MAIN_BOOST_SW_ON_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_MAIN_BOOST_SW_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 MAIN_BOOST_SW_OFF;
    uint32 Execution_Timestamp;    
    char Reply_Message[24];                       
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_MAIN_BOOST_SW_OFF_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_SUB_BOOST_SW_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 SUB_BOOST_SW_ON;
    uint32 Execution_Timestamp;    
    char Reply_Message[22];                   
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_SUB_BOOST_SW_ON_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_SUB_BOOST_SW_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 SUB_BOOST_SW_OFF;
    uint32 Execution_Timestamp;    
    char Reply_Message[23];                       
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_SUB_BOOST_SW_OFF_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_V28_MAIN_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V28_MAIN_ON;
    uint32 Execution_Timestamp;    
    char Reply_Message[14];                      
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_V28_MAIN_ON_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_V28_MAIN_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V28_MAIN_OFF;
    uint32 Execution_Timestamp;    
    char Reply_Message[15];                         
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_V28_MAIN_OFF_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_V28_SUB_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V28_SUB_ON;
    uint32 Execution_Timestamp;    
    char Reply_Message[13];                        
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_V28_SUB_ON_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_V28_SUB_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V28_SUB_OFF;
    uint32 Execution_Timestamp;    
    char Reply_Message[14];                        
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_V28_SUB_OFF_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_V12_MAIN_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V12_MAIN_ON;
    uint32 Execution_Timestamp;    
    char Reply_Message[14];                          
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_V12_MAIN_ON_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_V12_MAIN_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 V12_MAIN_OFF;
    uint32 Execution_Timestamp;    
    char Reply_Message[15];                      
    uint16 CRC16; 
} PAYUEL_LGPM_Payload2OBC_V12_MAIN_OFF_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_PWR_SENSE_INFO_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_SENSE_INFO;
    uint32 Execution_Timestamp;    
    char Reply_Message[185];  
    uint16 CRC16;                           
} PAYUEL_LGPM_Payload2OBC_PWR_SENSE_INFO_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_PWR_SEQ_ON_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_SEQ_ON;
    uint32 Execution_Timestamp;    
    char Reply_Message[79];  
    uint16 CRC16;                           
} PAYUEL_LGPM_Payload2OBC_PWR_SEQ_ON_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_PWR_SEQ_OFF_Payload
{
    PWR_CommonHeader_cmd_t header;
    uint8 PWR_SEQ_OFF;
    uint32 Execution_Timestamp;    
    char Reply_Message[83]; 
    uint16 CRC16;                           
} PAYUEL_LGPM_Payload2OBC_PWR_SEQ_OFF_Payload_t;


/*************************************************************************/
/*
** Payload to OBC (CMD code : 0x20)
*/


typedef struct PAYUEL_LGPM_Payload2OBC_RWA_CONTROL_Payload
{
    RWA_CommonHeader_cmd_t header;
    uint8 RWA_CONTROL;
    uint32 Execution_Timestamp;    
    char Reply_Message[14]; 
    uint16 CRC16;                           
} PAYUEL_LGPM_Payload2OBC_RWA_CONTROL_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_RWA_PWR_ON_Payload
{
    RWA_CommonHeader_cmd_t header;
    uint8 RWA_PWR_ON_val;
    uint32 Execution_Timestamp;    
    char Reply_Message[42]; 
    uint16 CRC16;                           
} PAYUEL_LGPM_Payload2OBC_RWA_PWR_ON_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_RWA_PWR_OFF_Payload
{
    RWA_CommonHeader_cmd_t header;
    uint8 RWA_PWR_OFF_val;
    uint32 Execution_Timestamp;    
    char Reply_Message[46]; 
    uint16 CRC16;                           
} PAYUEL_LGPM_Payload2OBC_RWA_PWR_OFF_Payload_t;

typedef struct PAYUEL_LGPM_Payload2OBC_RWA_SENSE_INFO_Payload
{
    RWA_CommonHeader_cmd_t header;
    uint8 RWA_SENSE_INFO;
    uint32 Execution_Timestamp;    
    char Reply_Message[258];   
    uint16 CRC16;                           
} PAYUEL_LGPM_Payload2OBC_RWA_SENSE_INFO_Payload_t;


/*************************************************************************/
/*
** Type definition (Sample App housekeeping)
*/

typedef struct PAYUEL_LGPM_HkTlm_Payload
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    uint8 spare[2];
} PAYUEL_LGPM_HkTlm_Payload_t;


#pragma pack(pop)


#endif
