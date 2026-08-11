/**
 * @file
 *
 * Define RPT Events IDs
 */

#ifndef RPT_EVENTIDS_H
#define RPT_EVENTIDS_H

#define RPT_RESERVED_EID        0
#define RPT_INIT_INF_EID        1
#define RPT_CC_ERR_EID          2
#define RPT_MID_ERR_EID         3
#define RPT_CMD_LEN_ERR_EID     4
#define RPT_PIPE_ERR_EID        5
#define RPT_CR_PIPE_ERR_EID     6
#define RPT_SUB_CMD_ERR_EID     7
#define RPT_SUB_BCN_ERR_EID     8
#define RPT_SUB_HK_ERR_EID      9
#define RPT_SUB_ONEHZ_ERR_EID   10
#define RPT_TBL_ERR_EID         11
#define RPT_REPORT_SUB_ERR_EID  12
#define RPT_PRIOR_INIT_ERR_EID  13
#define RPT_OPS_INIT_ERR_EID    14
#define RPT_CRIT_INIT_ERR_EID   15
#define RPT_MUTEX_INIT_ERR_EID  16

#define RPT_DATA_OPEN_ERR_EID   17
#define RPT_DATA_READ_ERR_EID   18
#define RPT_DATA_CRC_INVALID_ERR_EID    19
#define RPT_DATA_CRC_VALID_INF_EID  20
#define RPT_DATA_WRITE_ERR_EID  21
#define RPT_DATA_BACKUP_INF_EID 22

#define RPT_NOOP_CMD_INF_EID    23
#define RPT_DATA_BACKUP_ERR_EID 24

#endif
