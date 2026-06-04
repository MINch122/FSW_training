#ifndef MEOW_EVENTS_H
#define MEOW_EVENTS_H

/* Infrastructure */
#define MEOW_RESERVED_EID           0
#define MEOW_INIT_INF_EID           1
#define MEOW_CC_ERR_EID             2
#define MEOW_MID_ERR_EID            3
#define MEOW_CMD_LEN_ERR_EID        4
#define MEOW_PIPE_ERR_EID           5
#define MEOW_CR_PIPE_ERR_EID        6
#define MEOW_SUB_HK_ERR_EID         7
#define MEOW_SUB_CMD_ERR_EID        8
#define MEOW_NOOP_INF_EID           9
#define MEOW_RESET_INF_EID          10

/* Shell */
#define MEOW_SHELL_EXEC_ERR_EID     11
#define MEOW_SHELL_EXEC_CODE_ERR_EID 12
#define MEOW_SHELL_POLL_ERR_EID     13
#define MEOW_SHELL_KILL_ERR_EID     14
#define MEOW_SHELL_FORCE_MAGIC_EID  15

/* File */
#define MEOW_FILE_READ_ERR_EID      16
#define MEOW_FILE_WRITE_ERR_EID     17
#define MEOW_FILE_REMOVE_ERR_EID    18
#define MEOW_FILE_COPY_ERR_EID      19
#define MEOW_FILE_MOVE_ERR_EID      20
#define MEOW_FILE_STAT_ERR_EID      21
#define MEOW_FILE_TRUNC_ERR_EID     22
#define MEOW_FILE_TAIL_ERR_EID      23
#define MEOW_FILE_CKSUM_ERR_EID     24
#define MEOW_DISK_STAT_ERR_EID      25

/* Sys */
#define MEOW_SYS_SHUTDOWN_MAGIC_EID   26
#define MEOW_SYS_SHUTDOWN_ERR_EID     27
#define MEOW_SYS_SYNC_ERR_EID         28
#define MEOW_SYS_INFO_ERR_EID         29
#define MEOW_SYS_TIME_GET_ERR_EID     30
#define MEOW_SYS_TIME_SET_ERR_EID     31
#define MEOW_SYS_FORCE_KILL_MAGIC_EID 32
#define MEOW_SYS_FORCE_KILL_ERR_EID   33

/* CSP */
#define MEOW_CSP_SERVER_ERR_EID       34
#define MEOW_CSP_HANDLER_ERR_EID      35
#define MEOW_CSP_SEND_ERR_EID         36
#define MEOW_CSP_FTP_ERR_EID          37
#define MEOW_CSP_ROUTE_ERR_EID        38
#define MEOW_CSP_REROUTE_ERR_EID      39

#endif /* MEOW_EVENTS_H */
