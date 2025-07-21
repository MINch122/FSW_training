#include "payuzuc_internal_cfg.h"
#include "payuzuc_interface_cfg.h"
#include "payuzuc_tblstruct.h"

#include "payuzuc_task.h"

#include <fcntl.h>
#include <unistd.h>

void PAYUZUC_SetLineTrue(uint8_t MemSlot, uint16_t Line) {
    
    if(PAYUZUC_Data.MemSlotStatus.Entry[MemSlot].MemoryState == PAYUZUC_DOWNLOAD_DONE) return;
    
    PAYUZUC_Data.MemSlotStatus.Entry[MemSlot].LineState[Line / 8] |= (1 << (Line % 8));

    return;
}


osal_id_t PAYUZUC_OpenFile(uint8_t MemorySlot, uint16_t StartLine, uint16_t LineNum, int32 Flags) {
    osal_id_t ID;
    char Path[64] = {0, };

    /**
     * One line Download
     */
    if (LineNum == 0) {
        sprintf(Path, "./cf/sdcard/Kiss%u_%u_%03u", MemorySlot, 
            PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx, StartLine);    
    }
    else sprintf(Path, "./cf/sdcard/Kiss%u_%u_%03u-%03u", MemorySlot, 
                PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx, StartLine, StartLine + LineNum - 1);
    // OS_OpenCreate(&ID, Path, Flags, OS_READ_WRITE);
    
    ID = open(Path, O_CREAT|O_TRUNC | O_WRONLY, 0666);
    return ID;
}

int32 PAYUZUC_WriteLineToFile(osal_id_t ID, void *Data, size_t Size) {
    return write(ID, Data, Size);
    return OS_write(ID, Data, Size);
}

int32 PAYUZUC_CloseFile(osal_id_t ID) {
    return close(ID);
    return OS_close(ID);
}

void PAYUZUC_Inspection(uint8_t MemorySlot) {
    for (uint8_t i = 0; i < 60; i++) {
        if (PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LineState[i] != 0xFF) {
            return;
        }
    }
    OS_printf("Memory Slot %u Download Done.\n", MemorySlot);
    PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].MemoryState = PAYUZUC_DOWNLOAD_DONE;
    PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx ++;
    
    return;
}