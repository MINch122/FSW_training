#ifndef paybee_kisscam_UTILS_H
#define paybee_kisscam_UTILS_H

#include "common_types.h"
#include "rpt_interface_cfg.h"

typedef struct
{
    int32  ReturnCode;
    uint8  ReturnType;
    size_t ReadSize;
} paybee_kisscam_TransactionResult_t;

/// @brief Publish one command result for the RPT application.
CFE_Status_t paybee_kisscam_SendReport(uint8_t CC, uint8 ReturnType, int32 ReturnCode,
                                       const void *Data, size_t DataSize);


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



/// @brief Execute one KissCAM transaction without publishing a report.
/// @param Tx Command packet buffer.
/// @param Rx Receive buffer.
/// @param RxCapacity Physical size of Rx.
/// @param ExpectedRxSize Expected success response size including header and terminator.
/// @param CC Command code, used for event logging and download timing.
/// @return Transaction result to be reported once by the command handler.
paybee_kisscam_TransactionResult_t paybee_kisscam_Transaction(const void *Tx, void *Rx,
                                                              size_t RxCapacity, size_t ExpectedRxSize,
                                                              uint8_t CC);
// void paybee_kisscam_TransactionWithoutReport(void *Tx, void *Rx, uint8_t CC);

/// @brief Configure the Command packet for KissCAM
/// @param Payload [in] Data pointer of Parameter payload 
/// @param Packet [out] Configured packet. Type should be `paybee_kisscam_Cmd_t`
/// @param ParamNum [in] Number of parameter in specific Command
/// @param Command [in] KissCAM Command code of specific Command
void paybee_kisscam_ConfigurePacket(const void *Payload, void *Packet, uint8 ParamNum, uint8_t Command);

// void paybee_kisscam_DownloadTask(void);

#endif
