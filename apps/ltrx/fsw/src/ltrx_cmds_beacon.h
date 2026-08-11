/*
 * LTRX Beacon protocol (RX-driven)
 *
 * Definitions:
 *  - Downlink: OBC -> Beacon -> GS
 *    Beacon driven, Type 3/7 requests used
 *  - Uplink:   GS -> Beacon -> OBC  
 *    Beacon offers Type 6, sends header/parts -> OBC requests parts and confirms
 *
 * Description:
 *  - Holds one "downlink message" buffer to be sent when Beacon requests it.
 *  - Holds one "uplink message" buffer being received from Beacon.
 *  - LTRX_BcnProcessOneRx(): handle one incoming ICD frame & send any required response
 * 
 */

#ifndef LTRX_BEACON_CMDS_H
#define LTRX_BEACON_CMDS_H

#include <stdint.h>
#include <stdbool.h>
#include "cfe.h"

/* ---------------- DOWNLINK (OBC -> GS) ---------------- */
/* Set message (OBC -> Beacon) (Type 3/7). */
int32_t LTRX_Downlink_SetMessage(uint32_t message_id,
                                const uint8_t *data,
                                uint16_t length);

/* Clear prepared downlink message. */
void    LTRX_Downlink_ClearMessage(void);

bool     LTRX_Downlink_IsReady(void);
bool     LTRX_Downlink_HasPending(void);
uint32_t LTRX_Downlink_GetMessageId(void);
uint16_t LTRX_Downlink_GetLength(void);

/* Downstream staging gate (Bus Beacon -> downlink staging on/off) */
void     LTRX_Downstream_SetEnabled(bool enabled);
bool     LTRX_Downstream_IsEnabled(void);
void     LTRX_Downstream_SetBeaconPeriod(uint16_t period);
uint16_t LTRX_Downstream_GetBeaconPeriod(void);
uint16_t LTRX_Downstream_GetBeaconCount(void);

/* ---------------- UPLINK (GS -> OBC) ---------------- */
bool     LTRX_Uplink_HasCompleteMessage(void);

/* Copy out the completed uplink message.
 * - dst_len_inout: input = dst buffer size, output = copied length
 * - succeed -> complete flag */
int32_t  LTRX_Uplink_CopyOut(uint32_t *message_id_out,
                            uint8_t  *dst,
                            uint16_t *dst_len_inout);

void     LTRX_Uplink_Clear(void);

/* ---------------- BUS BEACON (HK Combined -> Downlink staging) ---- */
void LTRX_OnBusBeaconReceived(const CFE_SB_Buffer_t *SBBufPtr);
 
/* ---------------- UPLINK FORWARD (completed uplink -> SB CMD) ----- */
/* Called periodically from child task after session tick. */
void LTRX_Uplink_ForwardToSB(void);

/* Process one received ICD frame (timeout: timeout_ms)
 * Ticked by child loop */
int32_t LTRX_BcnProcessOneRx(uint32_t timeout_ms);

#endif /* LTRX_BEACON_CMDS_H */
