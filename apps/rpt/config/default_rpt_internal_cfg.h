#ifndef RPT_INTERNAL_CFG_H
#define RPT_INTERNAL_CFG_H

#include "rpt_interface_cfg.h"


#define RPT_PIPE_DEPTH      16

#define RPT_CRITICAL_MSG_DEPTH  5
#define RPT_REPORT_MSG_DEPTH    3


/**
 * RPT Operation data file path
 */
#define RPT_OPS_DATA_PATH       "/cf/ops.bin" /* Internal FLASH */

#define RPT_CRITICAL_DATA_PATH  "/cf/critical.bin" /* Internal FLASH */

typedef struct {
    RPT_Report_t Entry[RPT_REPORT_QUEUE_LEN];
    uint8_t Head;
    uint8_t Count;
} RPT_ReportQueue_t;

typedef struct {
    RPT_Critical_t Entry[RPT_CRITICAL_QUEUE_LEN];
    uint8_t Head;
    uint8_t Count;

    uint32_t CRC;
} RPT_CriticalQueue_t;

typedef struct {
    /* Store Time epoch */
    uint32_t EpochSec;
    uint32_t EpochSubsec;

    uint8 ResetCause;
    uint16 BootCount;
    uint32 TimeSec;     /* <\brief S/C time seconds */
    uint32 TimeSubsec;

    /* Preserve the on-disk layout used by existing /cf/ops.bin files. */
    uint32 Reserved;

    uint32 CRC;
} RPT_OperationData_t;

#endif
