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
        int8   i8[256];
        int16  i16[128];
        char   str[256];
        float  flt[64];
    } param;
} PAY_SLT_Params_t;


/**
 * @brief Fetch one or more remote parameters into a parameter request structure.
 *
 * Uses the PAY_SLT-compatible RParam GET transaction. On success, @p Payload
 * contains the received values and its len field is updated to the element count
 * actually returned.
 *
 * @param Payload  Request and response parameter structure.
 * @return CFE_SUCCESS on success; otherwise an RParam or argument error code.
 */
int32 PAY_SLT_FetchParam(PAY_SLT_Params_t *Payload);

/**
 * @brief Fetch remote parameters into a caller-provided buffer.
 *
 * This is a convenience wrapper around PAY_SLT_FetchParam(). For scalar and
 * array types, @p len is the requested element count. For strings, it is the
 * maximum destination buffer size in bytes.
 *
 * @param type     GS parameter type.
 * @param node     CSP destination node.
 * @param table    Remote parameter table ID.
 * @param addr     Starting parameter address in the table.
 * @param len      Requested element count or string buffer size.
 * @param out_ptr  Destination buffer for the received value or values.
 * @return CFE_SUCCESS on success; otherwise an RParam or argument error code.
 */
int32 PAY_SLT_FetchParam_Simple(uint8 type, uint8 node, uint8 table, uint16 addr, uint8 len, void *out_ptr);

/**
 * @brief Set a remote parameter with an RParam SET transaction.
 *
 * Supported types are GS_PARAM_UINT8, GS_PARAM_UINT16, GS_PARAM_UINT32, and
 * GS_PARAM_STRING. Numeric values are encoded in big-endian byte order.
 *
 * @param node   CSP destination node.
 * @param table  Remote parameter table ID.
 * @param addr   Parameter address in the table.
 * @param type   GS parameter type to write.
 * @param value  Pointer to the value to send.
 * @return CFE_SUCCESS on success; otherwise an RParam or argument error code.
 */
int32 PAY_SLT_SetParam(uint8 node, uint8 table, uint16 addr, uint8 type, void *value);

/**
 * @brief Read a payload data chunk through the configured I2C interface.
 *
 * Fetches the payload I2C address from the board parameter table, sends
 * @p start_addr as a 4-byte big-endian address, then reads @p size bytes.
 *
 * @param handle      I2C handle to validate; the configured I2C1 handle performs the transfer.
 * @param start_addr  Starting payload memory address.
 * @param data        Destination buffer for received bytes.
 * @param size        Number of bytes to read.
 * @return CFE_SUCCESS on success; SLT_IFB_DEVICE_BAD_ARG for invalid input;
 *         otherwise a parameter-fetch or serial I/O error code.
 */
int32 PAY_SLT_ReadExpI2CChunk(CFE_SRL_IO_Handle_t *handle, uint32 start_addr, void *data, size_t size);

/**
 * @brief Request and read a payload data frame through the RS422 interface.
 *
 * Sends the RS422 DOWNLOAD command and stores the complete received frame,
 * including its 7-byte header and trailing END byte, in @p data. The response
 * payload length controls how many additional bytes are read.
 *
 * @param handle  RS422 serial I/O handle.
 * @param data    Destination buffer for the frame header and payload.
 * @param size    Capacity of @p data; must include the header and END byte.
 * @return CFE_SUCCESS on success; SLT_IFB_DEVICE_BAD_ARG for invalid input;
 *         CFE_SRL_PARTIAL_READ_ERR for an oversized or malformed length;
 *         otherwise a serial I/O error code.
 */
int32 PAY_SLT_ReadExpRS422Chunk(CFE_SRL_IO_Handle_t *handle, void *data, size_t size);

/**
 * @brief Continue a CRC-32/ISO-HDLC calculation over a data block.
 *
 * The caller supplies the current, non-finalized CRC state. Use an initial
 * value of 0xFFFFFFFF and apply the final XOR of 0xFFFFFFFF after the last
 * block when calculating a complete CRC.
 *
 * @param current_crc  CRC state before processing @p data.
 * @param data         Data block to include in the CRC.
 * @param length       Number of bytes in @p data.
 * @return Updated, non-finalized CRC state.
 */
uint32 PAY_SLT_UpdateCRC32(uint32 current_crc, const uint8 *data, uint32 length);

/**
 * @brief Calculate the finalized CRC-32/ISO-HDLC value for a data block.
 *
 * Uses the reversed polynomial 0xEDB88320, initial value 0xFFFFFFFF, and
 * final XOR value 0xFFFFFFFF. The result is compatible with zlib CRC-32.
 *
 * @param data    Data block to calculate.
 * @param length  Number of bytes in @p data.
 * @return Finalized CRC-32 value.
 */
uint32 PAY_SLT_CalculateCRC32(const uint8 *data, uint32 length);

/**
 * @brief Download a remote table specification and all of its parameter values.
 *
 * Allocates the rows and value memory owned by @p tinst. Call
 * gs_param_table_free() when the returned table instance is no longer needed.
 *
 * @param node        CSP destination node.
 * @param table       Remote parameter table ID.
 * @param tinst       Destination table instance.
 * @param timeout_ms  Timeout for each RParam operation, in milliseconds.
 * @return GS_OK on success; otherwise a GomSpace parameter-library error code.
 */
gs_error_t PAY_SLT_GetFullTable(uint8_t node, uint8_t table, gs_param_table_instance_t *tinst, uint32_t timeout_ms);

/**
 * @brief Print a decoded remote parameter table to the application console.
 *
 * Fetches the board device name for display and prints each table row whose
 * address and size are valid for @p tinst.
 *
 * @param node   CSP destination node used to fetch the display name.
 * @param table  Remote parameter table ID displayed in the heading.
 * @param tinst  Downloaded table instance to print; may be NULL.
 */
void PAY_SLT_PrintParamTable(uint8 node, uint8 table, const gs_param_table_instance_t *tinst);

/**
 * @brief Build and publish a PAY_SLT command result report.
 *
 * Updates error counters, copies up to the RPT return-value capacity from
 * @p read_data, and transmits the report telemetry packet.
 *
 * @param status        Command or device operation status.
 * @param command_code  PAY_SLT command code being reported.
 * @param device_error  True when a failure originated from the external device.
 * @param read_data     Optional response data to include in the report.
 * @param read_size     Number of bytes available at @p read_data.
 * @return CFE_SUCCESS for a successful command report; otherwise the command
 *         status or a message-buffer/transmit error.
 */
CFE_Status_t PAY_SLT_HandleReport(int32 status, uint8 command_code, bool device_error, const void *read_data, uint16 read_size);

CFE_Status_t PAY_SLT_HandleReportForMid(uint16 message_id, int32 status, uint8 command_code, bool device_error,
                                        const void *read_data, uint16 read_size);

#endif
