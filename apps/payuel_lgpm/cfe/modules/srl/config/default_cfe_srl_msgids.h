/**
 * @file
 *   CFE Executive Services (CFE_SRL) Application Message IDs
 */
#ifndef CFE_SRL_MSGIDS_H
#define CFE_SRL_MSGIDS_H

#include "cfe_core_api_base_msgids.h"
#include "cfe_srl_topicids.h"
#include "cfe_srl_fcncodes.h"

/**
 * cFE SRL Command Message Id's
 */
#define CFE_SRL_CMD_MID     CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_SRL_CMD_TOPICID) /* 0x180D */
#define CFE_SRL_SEND_HK_MID CFE_PLATFORM_CMD_TOPICID_TO_MIDV(CFE_MISSION_SRL_SEND_HK_TOPICID) /* 0x180E */

/**
 * cFE SRL Telemetry Message Id's
 */
#define CFE_SRL_HK_TLM_MID      CFE_PLATFORM_TLM_TOPICID_TO_MIDV(CFE_MISSION_SRL_HK_TLM_TOPICID) /* 0x080E */

#endif /* CFE_SRL_MSGIDS_H */