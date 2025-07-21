#ifndef PAYUZUC_UTILS_H
#define PAYUZUC_UTILS_H

#include "common_types.h"


/// @brief Set bit table true for downloaded line
/// @details Never call this function manually
/// @param MemSlot Memory Slot (0 ~ 5)
/// @param Line Line number want to make true
void PAYUZUC_SetLineTrue(uint8_t MemSlot, uint16_t Line);


/// @brief Open PAYUZUC Table file
/// @param None
/// @return File Descriptor
int PAYUZUC_OpenTblFile(void);


/// @brief Open PAYUZUC Image File
/// @param MemorySlot Memory Slot
/// @param StartLine Starting Line
/// @param LineNum Total line number. Up to `480`
/// @return File Descriptor
int PAYUZUC_OpenFile(uint8_t MemorySlot, uint16_t StartLine, uint16_t LineNum);


/// @brief Read file data
/// @param ID File descriptor
/// @param Data Data buffer. Might be `PAYUZUC_Memory_Status_t *`
/// @param Size Data size. Might be `sizeof(PAYUZUC_Memory_Status_t)`
/// @return Read bytes. -1 for Error
int32 PAYUZUC_ReadFile(int ID, void *Data, size_t Size);


/// @brief Write data to file
/// @param ID File descriptor
/// @param Data Data buffer. Might be memory status or image data
/// @param Size Size of data
/// @param IsTbl If table data, true. Else (i.e. image data), false
/// @return Write Bytes. -1 for Error
int32 PAYUZUC_WriteToFile(int ID, void *Data, size_t Size, bool IsTbl);


/// @brief Close file
/// @param ID File descriptor
/// @return `0` for success, -1 for Error
int32 PAYUZUC_CloseFile(int ID);


/// @brief Check if Image data completely downloaded in specific memory slot
/// @param MemorySlot Memory Slot
void PAYUZUC_Inspection(uint8_t MemorySlot);

#endif