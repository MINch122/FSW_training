/************************************************************************
 * NASA Docket No. GSC-18,919-1, and identified as “Core Flight
 * System (cFS) Housekeeping (HK) Application version 2.5.1”
 *
 * Copyright (c) 2021 United States Government as represented by the
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
 *  The CFS Housekeeping (HK) Application Copy Table Definition
 */

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "cfe_msgids.h"
#include "hk_extern_typedefs.h"
#include "hk_msgids.h"
#include "hk_tbldefs.h"
#include "cfe_tbl_filedef.h"

/**
 * Include the hdr `*_msgids.h` of each app
 */
#include "rpt_msgids.h"
#include "rpt_msg.h"

#include "payuzuc_msgids.h"
#include "payuzuc_msg.h"

#include "payuzut_msgids.h"
#include "payuzut_msg.h"


/************************************************************************
** Define
*************************************************************************/
#define CFE_MSG_TLM_HDR_SIZE    sizeof(CFE_MSG_TelemetryHeader_t)

/* Beacon Packet */
#define BCN_OFFSET_0            sizeof(CFE_MSG_TelemetryHeader_t)
#define BCN_OFFSET_1            BCN_OFFSET_0 + sizeof(RPT_BcnTlm_Payload_t)
#define BCN_OFFSET_2            BCN_OFFSET_1 + sizeof(SANT_BcnTlm_Payload_t)
#define BCN_OFFSET_3            BCN_OFFSET_2 + sizeof(STRX_BcnTlm_Payload_t)
#define BCN_OFFSET_4            BCN_OFFSET_3 + sizeof(UANT_BcnTlm_Payload_t)
#define BCN_OFFSET_5            BCN_OFFSET_4 + sizeof(UTRX_BcnTlm_Payload_t)
#define BCN_OFFSET_6            BCN_OFFSET_5 + sizeof(EPS_BcnTlm_Payload_t)
#define BCN_OFFSET_7            BCN_OFFSET_6 + sizeof(ADCS_BcnTlm_Payload_t)
#define BCN_OFFSET_8            BCN_OFFSET_7 + sizeof(PAYUZUC_BcnTlm_Payload_t)
#define BCN_OFFSET_9            BCN_OFFSET_8 + sizeof(PAYUZUT_BcnTlm_Payload_t)
#define BCN_OFFSET_10           BCN_OFFSET_9 + sizeof(PAYUELM_BcnTlm_Payload_t)
#define BCN_OFFSET_11           BCN_OFFSET_10 + sizeof(PAYUELC_BcnTlm_Payload_t)


/* HouseKeeping Packet */
#define HK_OFFSET_0            sizeof(CFE_MSG_TelemetryHeader_t)
#define HK_OFFSET_1            HK_OFFSET_0 + sizeof(RPT_HkTlm_Payload_t)
#define HK_OFFSET_2            HK_OFFSET_1 + sizeof(SANT_HkTlm_Payload_t)
#define HK_OFFSET_3            HK_OFFSET_2 + sizeof(STRX_HkTlm_Payload_t)
#define HK_OFFSET_4            HK_OFFSET_3 + sizeof(UANT_HkTlm_Payload_t)
#define HK_OFFSET_5            HK_OFFSET_4 + sizeof(UTRX_HkTlm_Payload_t)
#define HK_OFFSET_6            HK_OFFSET_5 + sizeof(EPS_HkTlm_Payload_t)
#define HK_OFFSET_7            HK_OFFSET_6 + sizeof(ADCS_HkTlm_Payload_t)
#define HK_OFFSET_8            HK_OFFSET_7 + sizeof(PAYUZUC_HkTlm_Payload_t)
#define HK_OFFSET_9            HK_OFFSET_8 + sizeof(PAYUZUT_HkTlm_Payload_t)
#define HK_OFFSET_10           HK_OFFSET_9 + sizeof(PAYUELM_HkTlm_Payload_t)
#define HK_OFFSET_11           HK_OFFSET_10 + sizeof(PAYUELC_HkTlm_Payload_t)


hk_copy_table_entry_t HK_CopyTable[HK_COPY_TABLE_ENTRIES] = {
    /*         inputMid        inputOffset     outputMid    outputOffset  numBytes*/

    /*   0 */ {
        CFE_SB_MSGID_WRAP_VALUE(RPT_BCN_TLM_MID),
        CFE_MSG_TLM_HDR_SIZE,
        CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
        BCN_OFFSET_0,
        sizeof(RPT_BcnTlm_Payload_t),
    },
    /*   1 */
    {
        CFE_SB_MSGID_WRAP_VALUE(PAYUZUC_HK_TLM_MID),
        CFE_MSG_TLM_HDR_SIZE,
        CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
        BCN_OFFSET_1,
        4,
    },
    /*   2 */
    {
        CFE_SB_MSGID_WRAP_VALUE(CFE_SB_HK_TLM_MID),
        12,
        CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
        20,
        4,
    },
    /*   3 */
    {
        CFE_SB_MSGID_WRAP_VALUE(CFE_ES_HK_TLM_MID),
        12,
        CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
        24,
        4,
    },
    /*   4 */
    {
        CFE_SB_MSGID_WRAP_VALUE(CFE_TBL_HK_TLM_MID),
        12,
        CFE_SB_MSGID_WRAP_VALUE(HK_COMBINED_PKT1_MID),
        28,
        4,
    },

    /*   5 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*   6 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*   7 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*   8 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },

    /*   9 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  10 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  11 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  12 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  13 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  14 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  15 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  16 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  17 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  18 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  19 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  20 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  21 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  22 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  23 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  24 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  25 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  26 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  27 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  28 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  29 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  30 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  31 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  32 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  33 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  34 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  35 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  36 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  37 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  38 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  39 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  40 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  41 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  42 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  43 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  44 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  45 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  46 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  47 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  48 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  49 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  50 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  51 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  52 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  53 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  54 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  55 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  56 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  57 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  58 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  59 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  60 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  61 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  62 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  63 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  64 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  65 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  66 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  67 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  68 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  69 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  70 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  71 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  72 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  73 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  74 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  75 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  76 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  77 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  78 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  79 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  80 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  81 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  82 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  83 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  84 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  85 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  86 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  87 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  88 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  89 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  90 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  91 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  92 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  93 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  94 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  95 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  96 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  97 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  98 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /*  99 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 100 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 101 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 102 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 103 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 104 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 105 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 106 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 107 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 108 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 109 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 110 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 111 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 112 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 113 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 114 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 115 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 116 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 117 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 118 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 119 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 120 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 121 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 122 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 123 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 124 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 125 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 126 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
    /* 127 */
    {
        CFE_SB_MSGID_RESERVED,
        0,
        CFE_SB_MSGID_RESERVED,
        0,
        0,
    },
};

/*
** Table file header
*/
CFE_TBL_FILEDEF(HK_CopyTable, HK.CopyTable, HK Copy Tbl, hk_cpy_tbl.tbl)

/************************/
/*  End of File Comment */
/************************/
