#ifndef TO_LAB_UTILS_H
#define TO_LAB_UTILS_H

#include "common_types.h"

void TO_LAB_ForwardTelemetryRF(void);
void TO_LAB_ForwardTelemetryUDP(void);

void TO_HandleReport(int32 Status, uint8 CC, const void *Data, size_t DataSize);

#endif