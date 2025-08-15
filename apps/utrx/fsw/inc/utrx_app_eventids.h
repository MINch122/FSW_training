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
 *
 * Define Sample App Events IDs
 */

#ifndef UTRX_APP_EVENTS_H
#define UTRX_APP_EVENTS_H


typedef enum{
    
    UTRX_APP_RESERVED_EID      ,
    UTRX_APP_INIT_INF_EID      ,
    UTRX_APP_CC_ERR_EID        ,
    UTRX_APP_GETFCN_ERR_EID    ,
    UTRX_APP_NOOP_INF_EID      ,
    UTRX_APP_RESET_INF_EID     ,
    UTRX_APP_MID_ERR_EID       ,
    UTRX_APP_CMD_LEN_ERR_EID   ,
    UTRX_APP_PIPE_ERR_EID      ,
    UTRX_APP_VALUE_INF_EID     ,
    UTRX_APP_CR_PIPE_ERR_EID   ,
    UTRX_APP_SUB_HK_ERR_EID    ,
    UTRX_APP_SUB_CMD_ERR_EID   ,
    UTRX_APP_TABLE_REG_ERR_EID ,
    UTRX_APP_SET_BAUD_SUCCESS_EID,
    UTRX_APP_SET_FREQ_SUCCESS_EID,
    UTRX_APP_GET_BAUD_SUCCESS_EID,
    UTRX_APP_GET_FREQ_SUCCESS_EID,
    UTRX_GNDWDT_ERR_EID             ,
    UTRX_REBOOT_ERR_EID             ,
    UTRX_RXCONF_GET_BAUD_ERR_EID    ,
    UTRX_RXCONF_GET_GUARD_ERR_EID   ,
    UTRX_TXCONF_GET_BAUD_ERR_EID    ,
    UTRX_RXCONF_GET_FREQ_ERR_EID    ,
    UTRX_TXCONF_GET_FREQ_ERR_EID    ,
    UTRX_TLM_GET_TEMP_BRD_EID       ,
    UTRX_TLM_GET_LAST_RSSI_EID      ,
    UTRX_TLM_GET_LAST_RFERR_EID     ,
    UTRX_TLM_GET_ACTIVE_CONF_EID    ,
    UTRX_TLM_GET_BOOT_COUNT_EID     ,
    UTRX_TLM_GET_BOOT_CAUSE_EID     ,
    UTRX_TLM_GET_LAST_CONTACT_EID   ,
    UTRX_TLM_GET_TOT_TX_BYTES_EID   ,
    UTRX_TLM_GET_TOT_RX_BYTES_EID   ,
    UTRX_RXCONF_SET_BAUD_ERR_EID    ,
    UTRX_TXCONF_SET_BAUD_ERR_EID    ,
    UTRX_RXCONF_SET_FREQ_ERR_EID    ,
    UTRX_TXCONF_SET_FREQ_ERR_EID    ,
    UTRX_RPARAM_SAVE_1_ERR_EID      ,
    UTRX_RPARAM_SAVE_5_ERR_EID      , 
    UTRX_RPARAM_SAVE_ALL_ERR_EID   ,
    UTRX_APP_RESET_APP_COUNTER_EID,
    UTRX_APP_RESET_DEVICE_COUNTER_EID,
    UTRX_APP_GETMSGID_ERR_EID

} utrx_events_n_t;

#endif 
/* UTRX_APP_EVENTS_H */
