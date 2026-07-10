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
 * Main header file for the Sample application
 */

#ifndef PAY_SLT_UTILS_H
#define PAY_SLT_UTILS_H

/*
** Required header files.
*/
#include "cfe_error.h"
#include "pay_slt_msg.h"
#include "pay_slt.h"
#include <gs/param/rparam.h>

#define PAY_SLT_EXP_A7_NODE  11
#define PAY_SLT_EXP_M7_NODE  12
#define PAY_SLT_IFB_NODE     13

#define TABLE_BOARD_PARAM          0
#define TABLE_DATA_CONTROL_PARAM   3
#define TABLE_TELEMETRY            4


typedef struct __attribute__((packed)) {
    uint8  node;        // 11(PAY-EXP-A7), 12(PAY-EXP-M7), 13(PAY-IFB) 중 하나
    uint8  table;       // 0, 3, 4 중 하나
    uint16 addr;        // table의 address, 0xNNNN 형식
    
    uint8  type;        // GS_PARAM 참조
    uint8  len;         // 유효 데이터 길이 (배열 요소 개수)

    uint8 padding[2];   // 밑의 4 byte 이상 데이터(uint32, double)가 들어오는 것을 고려한 padding, 4byte alignment을 위해서 추가
    
    // real data
    union {
        uint8  u8[256];
        uint16 u16[128];
        uint32 u32[64];
        int16  i16[128];
        char   str[256];
    } param;
} PAY_SLT_Params_t;


/**
 * @brief Perform a CSP transaction with the Payload.
 *        Perform the same function as gs_rparam_get.
 * 
 * @param payload   struct: PAY_SLT_Params_t
 * @return Only CFE_SUCCESS(which is 0) is success
 */
int32 PAY_SLT_FetchParam(PAY_SLT_Params_t *Payload);


/**
 * @brief Perform a CSP transaction with the Payload.
 *        Perform the same function as gs_rparam_get.
 *        PAY_SLT_FetchParam 구조체를 풀어놓은 버전.
 * 
 * @param type     type of parameter. GS_PARAM
 * @param node     CSP destination node
 * @param table    GS table index
 * @param addr     Parameter address in table
 * @param len      Length of parameter. Default 1
 * @param out_ptr  Pointer for the address to which reply data will be placed
 * @return Only CFE_SUCCESS(which is 0) is success
 */
int32 PAY_SLT_FetchParam_Simple(uint8 type, uint8 node, uint8 table, uint16 addr, uint8 len, void *out_ptr);


/**
 * @brief Perform a CSP transaction with the Payload.
 *        Perform the same function as gs_rparam_set.
 *        This function only Set parameter for Table 0 (SLT, Board Parameters)
 *        and uint8, uint16, uint32
 * 
 * @param node     CSP destination node
 * @param table    GS table index
 * @param addr     Parameter address in table
 * @param type     type of parameter. GS_PARAM
 * @param value    Pointer of the desiring parameter value
 * @return Only CFE_SUCCESS(which is 0) is success
 */
int32 PAY_SLT_SetParam(uint8 node, uint8 table, uint16 addr, uint8 type, void *value);

/**
 * @brief Read a chunk of payload experiment data via I2C interface.
 *        This function dynamically fetches the Payload I2C address, 
 *        writes a 4-byte initial memory address (Big-Endian), and 
 *        then reads the specified size of data into the buffer.
 *
 * @param handle      Pointer to the SRL I2C IO handle
 * @param start_addr  The 32-bit starting memory address to read from (usually 0x00000000)
 * @param data        Pointer to the buffer where the read data will be stored
 * @param size        The number of bytes to read (e.g., chunk size, typically 512)
 * @return CFE_SUCCESS on success, SLT_IFB_DEVICE_BAD_ARG on invalid inputs, or error code
 */
int32 PAY_SLT_ReadExpI2CChunk(CFE_SRL_IO_Handle_t *handle, uint32 start_addr, void *data, size_t size);


uint32 PAY_SLT_UpdateCRC32(uint32 current_crc, const uint8 *data, uint32 length);

/**
 * @brief Calculate the CRC-32 value for a given data block.
 *        Implements the CRC-32 ISO HDLC algorithm (zlib compatible).
 *        Uses Polynomial: 0xEDB88320 (Reversed 0x04C11DB7) and 
 *        Initialization/XorOut: 0xFFFFFFFF.
 * 
 * @param data      Pointer to the base of the memory block (e.g., chunk buffer)
 * @param length    The number of bytes in the memory block
 * @return The 32-bit calculated CRC value
 */
uint32 PAY_SLT_CalculateCRC32(const uint8 *data, uint32 length);


/**
 * @brief Perform a CSP transaction with the Payload.
 *        Perform the same function as gs_rparam_get_full_table.
 * 
 * @param node     CSP destination node
 * @param table    GS table index
 * @param tinst    Table instance
 * @param timeout_ms timeout
 * @return gs_error_t, Only GS_OK=0 success
 */
gs_error_t PAY_SLT_GetFullTable(uint8_t node, uint8_t table, gs_param_table_instance_t *tinst, uint32_t timeout_ms);


/**
 * @brief Display Full Table in the terminal by PAY_SLT_APP_printf
 * 
 * @param node     CSP destination node
 * @param table    GS table index
 * @param tinst    Table instance
 * @return gs_error_t, Only GS_OK=0 success
 */
void PAY_SLT_PrintParamTable(uint8 node, uint8 table, const gs_param_table_instance_t *tinst);


CFE_Status_t PAY_SLT_HandleReport(int32 status, uint8 command_code, bool device_error, const void *read_data, uint16 read_size);

#endif