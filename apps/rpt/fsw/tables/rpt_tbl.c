#include "cfe_tbl_filedef.h"
#include "rpt_tbl.h"
#include "cfe_sb_api_typedefs.h"

/**************************
 * Add More `*_msgids.h`
 **************************/
#include "to_lab_msgids.h"
#include "ci_lab_msgids.h"

#include "adcs_msgids.h"
#include "adcs2_msgids.h"
#include "eps_msgids.h"

#include "eo_msgids.h"
#include "gps_msgids.h"
#include "sp_msgids.h"
#include "stx_msgids.h"

#include "lgbat_msgids.h"
#include "payuel_cam_msgids.h"
#include "payuel_obc_msgids.h"
#include "payuel_roma_msgids.h"
#include "uant_app_msgids.h"
#include "utrx_msgids.h"


RPT_Table_t RPT_Subs[RPT_MAX_TBL_ENTRY] = {
    /* Entry 0 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 1 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(ADCS_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 2 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(EPS_REPORT_MID),
        .Entry.IsCritical = RPT_CRITICAL},
        
    /* Entry 3 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(STX_APP_RPT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 4 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(SP_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},
    
    /* Entry 5 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(GPS_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 6 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(ADCS2_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 7 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(UANT_APP_RPT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 8 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(UTRX_RPT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},
        
    /* Entry 9 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(PAYUEL_CAM_RPT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 10 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(LGBAT_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},
    
    /* Entry 11 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(LGBAT_HK_TLM_MID),
        .Entry.IsCritical = RPT_CRITICAL},

    /* Entry 12 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(PAYUEL_OBC_RPT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 13 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(TO_LAB_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 14 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(CI_LAB_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},
        
    /* Entry 15 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(PAYUEL_ROMA_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 16 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(EO_REPORT_TLM_MID),
        .Entry.IsCritical = RPT_NOT_CRITICAL},
    
    /* Entry 17 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},
    
    /* Entry 18 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 19 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 20 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},
        
    /* Entry 21 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 22 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},
    
    /* Entry 23 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},
    
    /* Entry 24 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 25 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 26 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},
        
    /* Entry 27 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* Entry 28 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},
    
    /* Entry 29 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},
    
};


CFE_TBL_FILEDEF(RPT_Subs, RPT.RPT_Subs, RPT Sub Tbl, rpt_tbl.tbl)
