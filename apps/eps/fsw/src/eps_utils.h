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
 *   This file contains the prototypes for the Sample App utility functions
 */

#ifndef EPS_UTILS_H
#define EPS_UTILS_H

/*
** Required header files.
*/
#include "eps_app.h"
#include "eps_p80_drv.h"
#include "eps_bp8_drv.h"

#include <stddef.h>
#include <stdint.h>
#include <gs/param/table.h>
#include <gs/param/types.h>
#include <gs/util/error.h>

CFE_Status_t EPS_TblValidationFunc(void *TblData);
void         EPS_GetCrc(const char *TableName);
CFE_Status_t EPS_UpdateBcnTlmFromHw(void);
void         EPS_SendReport(const void *cmd, const void *data, uint16 dataSize, int32 retCode, uint8 retType);
const char  *EPS_GetCspNodeDeviceName(uint8 cspNode);
const char  *EPS_GetRParamTableName(uint8 cspNode, uint8 tableId);
void         EPS_CopyCmdString(char *dst, size_t dst_size, const char *src, size_t src_size);
void         EPS_PrintP80PowerIfStatus(const char *title, const power_if_ch_status_t *status);
void         EPS_PrintP80PowerIfList(const power_if_cmd_list_response_t *list);
void         EPS_PrintP80PmuHk(const EPS_P80_PMU_HkTlm_Payload_t *hk);
void         EPS_PrintP80PduHk(const EPS_P80_PDU_HkTlm_Payload_t *hk);
void         EPS_PrintP80AcuHk(uint8_t cspNode, const EPS_P80_ACU_HkTlm_Payload_t *hk);
void         EPS_PrintBcnReport(const EPS_BcnTlm_Full_Payload_t *bcn);
void         EPS_PrintBP8Hk(const EPS_BP8_HkTlm_Payload_t *hk);
void         EPS_PrintHk(uint8_t cspNode);
gs_error_t   EPS_ReadHk(uint8_t cspNode, uint32_t timeoutMs);
void         EPS_PrintParamTable(const char *title, uint8 cspNode, uint8 tableId,
                                  const char *tableName, const gs_param_table_instance_t *tinst);
void         EPS_PrintRParamFullTable(uint8_t cspNode, uint8_t tableId,
                                       const gs_param_table_instance_t *tinst);
gs_error_t   EPS_RParamSet(uint8_t cspNode, uint8_t tableId, uint16_t addr, uint8_t type,
                            const uint8_t *data, uint16_t size, uint32_t timeoutMs);
gs_error_t   EPS_RParamGet(uint8_t cspNode, uint8_t tableId, uint16_t addr, uint8_t type,
                            uint8_t *data, uint16_t size, uint32_t timeoutMs);
gs_error_t   EPS_RParamFetchFullTable(uint8_t cspNode, uint8_t tableId,
                                       gs_param_table_instance_t *tinst, uint32_t timeoutMs);
gs_error_t   EPS_RParamTableSave(uint8_t cspNode, uint8_t tableId, uint32_t timeoutMs);
gs_error_t   EPS_RParamTableLoad(uint8_t cspNode, uint8_t tableId, uint32_t timeoutMs);
gs_error_t   EPS_RParamSaveAll(uint8_t cspNode, uint32_t timeoutMs);
gs_error_t   EPS_RParamSaveToStore(uint8_t cspNode, uint8_t tableId, const char *store,
                                    const char *slot, uint32_t timeoutMs);
gs_error_t   EPS_RParamLoadFromStore(uint8_t cspNode, uint8_t tableId, const char *store,
                                      const char *slot, uint32_t timeoutMs);

#endif /* EPS_UTILS_H */
