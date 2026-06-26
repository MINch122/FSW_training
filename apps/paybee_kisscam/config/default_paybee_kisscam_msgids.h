/**
 * @file
 *   PAY UZURO CAM Application Message IDs
 */
#ifndef paybee_kisscam_MSGIDS_H
#define paybee_kisscam_MSGIDS_H

#include "cfe_core_api_base_msgids.h"
#include "paybee_kisscam_topicids.h"

#define paybee_kisscam_CMD_MID             CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_paybee_kisscam_CMD_TOPICID)
// #define paybee_kisscam_SEND_HK_MID         CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_paybee_kisscam_SEND_HK_TOPICID)
// #define paybee_kisscam_SEND_BCN_MID        CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_paybee_kisscam_SEND_BCN_TOPICID)

// #define paybee_kisscam_HK_TLM_MID          CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_paybee_kisscam_HK_TLM_TOPICID)
// #define paybee_kisscam_BCN_TLM_MID         CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_paybee_kisscam_BCN_TLM_TOPICID)
#define paybee_kisscam_REPORT_TLM_MID      CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_paybee_kisscam_REPORT_TOPICID)
/**
 * KissCAM Image Data MID
 */

// #define paybee_kisscam_THUMBNAIL_IMG_MID       CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_paybee_kisscam_TUMBNAIL_IMG_TOPICID)

#endif