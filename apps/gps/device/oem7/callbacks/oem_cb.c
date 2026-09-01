/**
 * @file oem_cb.c
 * @brief OEM callback utilities.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#include "oem_cb.h"

void OEM_CB_FillHead(OEM_Log_Head_t* Head, const oem_binary_header_t* Hdr)
{
    Head->messageID      = Hdr->messageID;
    Head->timeStatus     = Hdr->timeStatus;
    Head->week           = Hdr->week;
    Head->ms             = (uint32_t) Hdr->ms;
    Head->receiverStatus = Hdr->receiverStatus;
}

uint8_t OEM_CB_U8Sat(uint32_t u8)
{
    return (u8 > UINT8_MAX) ? UINT8_MAX : u8;
}

uint16_t OEM_CB_U16Sat(uint32_t u16)
{
    return (u16 > UINT16_MAX) ? UINT16_MAX : u16;
}
