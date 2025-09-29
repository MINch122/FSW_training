#ifndef FTP_UTILS_H
#define FTP_UTILS_H

void FTP_LocalTimerCallback(osal_id_t object_id, void *arg);

void FTP_HandleReport(int32 Status, uint8_t ReturnType, uint8_t CC, void *Data, size_t Size);

#endif