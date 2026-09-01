/**
 * @file oem_cb.h
 * @brief OEM default callback declarations and utilities.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2026.
 */
#ifndef _OEM_CB_H_
#define _OEM_CB_H_

#include <stdint.h>
#include "oem_types.h"
#include "msg/oem_msg_common.h"

#define OEM_CALLBACK_LOG_SAVE_PATH_ROOT  "/cf/sdcard/"

/**
 * @brief Header fields every stored-log telemetry packet carries.
 *
 * @details week and ms are the GPS reference time of the log, which the
 *          telemetry timestamp (OBC time) cannot replace. The sync bytes are
 *          dropped: they are the constant 0xAA 0x44 0x12. timeStatus is an
 *          enum of 20..180, so a byte holds it.
 */
 /* original 16 -> lossless compact 13 B */
typedef struct __attribute__((packed)) {
    uint16_t messageID;
    uint8_t  timeStatus;
    uint16_t week;
    uint32_t ms;
    uint32_t receiverStatus;
} OEM_Log_Head_t;

/**
 * @brief Copy the log header fields kept in telemetry.
 */
void OEM_CB_FillHead(OEM_Log_Head_t* Head, const oem_binary_header_t* Hdr);

/**
 * @brief Saturate an u8 int. If @a u8 > UINT8_MAX, return UINT8_MAX.
 */
uint8_t OEM_CB_U8Sat(uint32_t u8);

/**
 * @brief Saturate a u16 int. If @a u16 > UINT16_MAX, return UINT16_MAX.
 */
uint16_t OEM_CB_U16Sat(uint32_t u16);

/** Test purpose printf callbacks */
int oem_callback_VERSION_print(void* msg);
int oem_callback_HWMONITOR_print(void* msg);

/**  TLM sender callbacks. Publish on the software bus so that DS can record. */
int oem_callback_BESTPOS(void* msg);
int oem_callback_BESTXYZ(void* msg);
int oem_callback_RANGE(void* msg);
int oem_callback_TIME(void* msg);
int oem_callback_CLOCKMODEL(void* msg);
int oem_callback_HWMONITOR(void* msg);
int oem_callback_RXSTATUS(void* msg);
int oem_callback_SATVIS2(void* msg);

#endif
