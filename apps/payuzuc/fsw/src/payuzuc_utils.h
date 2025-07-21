#ifndef PAYUZUC_UTILS_H
#define PAYUZUC_UTILS_H

#include "common_types.h"

void PAYUZUC_SetLineTrue(uint8_t MemSlot, uint16_t Line);

osal_id_t PAYUZUC_OpenFile(uint8_t MemorySlot, uint16_t StartLine, uint16_t LineNum, int32 Flags);
int32 PAYUZUC_WriteLineToFile(osal_id_t ID, void *Data, size_t Size);
int32 PAYUZUC_CloseFile(osal_id_t ID);
void PAYUZUC_Inspection(uint8_t MemorySlot);

#endif