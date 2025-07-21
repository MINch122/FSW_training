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


int PAYUZUC_OpenTblFile(void) {
    // int32 Status;
    int ID;

    ID = open(PAYUZUC_TBL_PATH, O_CREAT | O_RDWR, 0666);
    OS_printf("ID: %u\n",ID);
    return ID;
}


int PAYUZUC_OpenFile(uint8_t MemorySlot, uint16_t StartLine, uint16_t LineNum) {
    int ID;
    char Path[64] = {0, };

    /**
     * One line Download
     */
    if (LineNum == 0) {
        sprintf(Path, "%s%u_%u_%03u", PAYUZUC_IMG_PATH, MemorySlot, 
            PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx, StartLine);    
    }
    else sprintf(Path, "%s%u_%u_%03u-%03u", PAYUZUC_IMG_PATH, MemorySlot, 
                PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LastImgIdx, StartLine, StartLine + LineNum - 1);
    
    OS_printf("Path: %s\n", Path);
    
    ID = open(Path, O_CREAT|O_TRUNC | O_WRONLY, 0666);
    return ID;
}


int32 PAYUZUC_WriteToFile(int ID, void *Data, size_t Size, bool IsTbl) {
    int32 Status;
    if (IsTbl) {
        lseek(ID, 0, SEEK_SET);
        Status = write(ID, Data, Size);
        if (Status != Size) return -1;
        Status = fsync(ID);
        if (Status != 0) return -2;
        return CFE_SUCCESS;
    }
    else {
        Status = write(ID, Data, Size);
        if (Status != Size) return -1;
        Status = fsync(ID);
        if (Status != 0) return -2;
        return CFE_SUCCESS;
    }
}


int32 PAYUZUC_ReadFile(int ID, void *Data, size_t Size) {
    return read(ID, Data, Size);
}


int32 PAYUZUC_CloseFile(int ID) {
    return close(ID);
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
    for (uint8_t i = 0; i < 60; i++) {
        PAYUZUC_Data.MemSlotStatus.Entry[MemorySlot].LineState[i] = 0;
    }
    
    return;
}