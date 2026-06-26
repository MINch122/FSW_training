#ifndef paybee_kisscam_UTILS_H
#define paybee_kisscam_UTILS_H

#include "common_types.h"


/// @brief Set bit table true for downloaded line
/// @details Never call this function manually
/// @param MemSlot Memory Slot (0 ~ 5)
/// @param Line Line number want to make true
void paybee_kisscam_SetLineTrue(uint8_t MemSlot, uint16_t Line);


/// @brief Open paybee_kisscam Table file
/// @param None
/// @return File Descriptor
int paybee_kisscam_OpenTblFile(void);


/// @brief Open paybee_kisscam Image File
/// @param MemorySlot Memory Slot
/// @param StartLine Starting Line
/// @param LineNum Total line number. Up to `480`
/// @return File Descriptor
int paybee_kisscam_OpenFile(uint8_t MemorySlot, uint16_t StartLine, uint16_t LineNum);


/// @brief Read file data
/// @param ID File descriptor
/// @param Data Data buffer. Might be `paybee_kisscam_Memory_Status_t *`
/// @param Size Data size. Might be `sizeof(paybee_kisscam_Memory_Status_t)`
/// @return Read bytes. -1 for Error
int32 paybee_kisscam_ReadFile(int ID, void *Data, size_t Size);


/// @brief Write data to file
/// @param ID File descriptor
/// @param Data Data buffer. Might be memory status or image data
/// @param Size Size of data
/// @param IsTbl If table data, true. Else (i.e. image data), false
/// @return only `CFE_SUCCESS` `0` is success
int32 paybee_kisscam_WriteToFile(int ID, void *Data, size_t Size, bool IsTbl);


/// @brief Close file
/// @param ID File descriptor
/// @return `0` for success, -1 for Error
int32 paybee_kisscam_CloseFile(int ID);


/// @brief Check if Image data completely downloaded in specific memory slot
/// @param MemorySlot Memory Slot
void paybee_kisscam_Inspection(uint8_t MemorySlot);



/// @brief Handle H/W Error packet. (i.e. serial comm. success) If Data is insufficient, read residual bytes.
/// @param ErrPkt Received error packet pointer
/// @param Size Read size before this function
/// @param CC Command Code where error occur
void paybee_kisscam_HandleErrorPacket(void *ErrPkt, ssize_t Size, uint8_t CC);


/// @brief Handle Error situation. (i.e. serial comm. failed)
/// @param Status Error status code
/// @param CC Command code where error occur
/// @param ReadData Readed data from communication
/// @param ReadSize Readed data size from communication
void paybee_kisscam_HandleErrorSerial(int32 Status, uint8 CC, void *ReadData, ssize_t ReadSize);

/// @brief Handle Success situation (Just report the result)
/// @param CC Command code which is executed
/// @param ReadData Readed data from communication
/// @param ReadSize Readed data size from communication
void paybee_kisscam_HandleSuccess(uint8_t CC, void *ReadData, ssize_t ReadSize);

/// @brief Do comprehensive transaction with KissCAM. This function handle all case
/// @param Tx Tx data buffer pointer
/// @param Rx Rx data buffer pointer
/// @param CC Command Code which is invoked
void paybee_kisscam_Transaction(void *Tx, void *Rx, uint8_t CC);
// void paybee_kisscam_TransactionWithoutReport(void *Tx, void *Rx, uint8_t CC);

/// @brief Configure the Command packet for KissCAM
/// @param Payload [in] Data pointer of Parameter payload 
/// @param Packet [out] Configured packet. Type should be `paybee_kisscam_Cmd_t`
/// @param ParamNum [in] Number of parameter in specific Command
/// @param Command [in] KissCAM Command code of specific Command
void paybee_kisscam_ConfigurePacket(const void *Payload, void *Packet, uint8 ParamNum, uint8_t Command);

// void paybee_kisscam_DownloadTask(void);

#endif