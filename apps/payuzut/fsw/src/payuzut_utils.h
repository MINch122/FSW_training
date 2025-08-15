#ifndef PAYUZUT_UTILS_H
#define PAYUZUT_UTILS_H

#include "common_types.h"

/// @brief 
/// @param Idx Should be `0` or `1`
/// @return 
int PAYUZUT_OpenFile(uint8_t Idx);

/// @brief Write data to file
/// @param FD File descriptor
/// @param Data Data buffer
/// @param Size Size of data
/// @return only `CFE_SUCCESS` `0` is success
int32 PAYUZUT_WriteToFile(int FD, void *Data, size_t Size);

int32 PAYUZUT_CloseFile(int FD);

/// @brief Send Configuration data to ADC config register
/// @param Addr Address of ADC
/// @return Only `CFE_SUCCESS` is success.
int32 PAYUZUT_ConfigADC(uint8 Addr);

/// @brief Read converted ADC value via transaction
/// @param Addr [in] Address of ADC
/// @param Value [out] returned value
/// @return Only `CFE_SUCCESS` is success.
int32 PAYUZUT_ReadADC(uint8 Addr, uint16 *Value);


/// @brief Get temperature ADC value. This function cover the "config -> read". Not used in Child task.
/// @param Addr [in] Slave Address
/// @param Value [out] Pointer to put ADC value
/// @return only `CFE_SUCCESS` `0` is success
int32 PAYUZUT_GetADCValue(uint8 Addr, uint16 *Value);

#endif
