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
 *   Specification for the SAMPLE_APP command and telemetry
 *   message constant definitions.
 *
 *  For SAMPLE_APP this is only the function/command code definitions
 */
#ifndef SLT_IFB_MSGDEFS_H
#define SLT_IFB_MSGDEFS_H

#include "common_types.h"
#include "pay_slt_fcncodes.h"

#define PAY_SLT_BRD_UID_SIZE      16
#define PAY_SLT_RTABLE_STR_SIZE   96

typedef struct __attribute__((__packed__))
{
    // 4 byte 데이터의 alignment를 위해서 순서를 재배치 하였음
    uint32 sys_uptime;
    uint32 sys_now;
    uint32 wdt_left;
    float att_q[4];
    float rot_r[3];
    float lin_acc[3];
    float fld_vec[3];

    int16 sys_status;
    uint16 boot_cnt;
    int16 brd_temp;
    int16 ntc_data[4];
    uint16 pw_cur[2];
    uint16 pw_vol[2];
    uint16 sen_rst;

    uint8 boot_his[8];
    uint8 sen_online;
    uint8 sen_qlvl;
    uint8 att_ql; 
    
    uint8 BcnSbEnabled;
    
}SLT_IFB_HkTlm_Payload_t;

typedef struct __attribute__((__packed__)) {
    // RPT
    uint8 CmdCounter;
    uint8 ErrCounter;

    // PAY-EXP-A7
    uint16        boot_cnt_p;

    uint32        sys_uptime_a7;
    uint32        sys_now_a7;
    uint32        wdt_left_a7;
    uint16        slf_data[4];

    uint16        imu_data[8];
    int16         ntc_data_a7[8];
    uint16        pwr_volt[8];
    uint16        pwr_current[8];

    int16         brd_temp_a7;
    int16         sys_status_a7;
    uint16        boot_cnt_c;
    uint16        brm_data[3];

    uint8         boot_his_c[8];
    uint8         boot_his_p[8];    
    

    // PAY-IFB
    uint32        sys_uptime_ifb;
    uint32        sys_now_ifb;
    uint16        boot_cnt;
    int16         sys_status_ifb;

    uint8         boot_his[8];
    uint32        wdt_left_ifb;
    int16         brd_temp_ifb;
    uint16        sen_rst;

    int16         ntc_data_ifb[4];
    uint16        pw_cur[2];
    uint16        pw_vol[2];
    float         att_q[4];
    float         rot_r[3];
    float         lin_acc[3];
    float         fld_vec[3];
    
    uint8         sen_online;
    uint8         sen_qlvl;
    uint8         att_ql;
    

} SLT_IFB_BcnTlm_Payload_t;



/* 만들어 보아요 */

typedef struct __attribute__((packed)) {
    uint8  node;      // 11(PAY-EXP-A7), 12(PAY-EXP-M7), 13(PAY-IFB) 중 하나
    uint8  table;     // 0, 3, 4 중 하나
    uint16 addr;      // table의 address, 0xNNNN 형식
    
    uint8  type;      // GS_PARAM 참조
    uint8  len;       // 유효 데이터 길이 (배열 요소 개수)
} PAY_SLT_ParGet_Payload_t;

typedef struct __attribute__((packed)) {
    uint8  node;         // 11(PAY-EXP-A7), 12(PAY-EXP-M7), 13(PAY-IFB) 중 하나
    uint8  table;        // 0, 3 중 하나
    uint16 addr;         // table의 address, 0xNNNN 형식
    
    uint8  type;         // GS_PARAM 참조
    uint8  padding[3];

    uint32  value;       // 바꾸려고 하는 값
} PAY_SLT_ParSet_Payload_t;

typedef struct __attribute__((packed)) {
    uint8 node;          // file scan을 진행할 payload node: 11(PAY-EXP-A7) or 12(PAY-EXP-M7)
} PAY_SLT_ScanFiles_Payload_t;

typedef struct __attribute__((packed)) {
    // uint8  node;          // file scan을 진행할 payload node: 11(PAY-EXP-A7) or 12(PAY-EXP-M7) // 일단 11 고정
    uint32 file_index;
    uint32 start_chunk;
} PAY_SLT_DownloadFile_Payload_t;

typedef struct __attribute__((packed)) {
    uint32 snap_id;
    uint32 file_count;
} PAY_SLT_ScanFileRpl_t;

typedef struct __attribute((packed)) {
    uint8 node;
    uint8 table;
} PAY_SLT_GetFullTable_Payload_t;

#endif
