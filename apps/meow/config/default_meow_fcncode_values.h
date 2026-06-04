#ifndef DEFAULT_MEOW_FCNCODE_VALUES_H
#define DEFAULT_MEOW_FCNCODE_VALUES_H

#define MEOW_CCVAL(x) MEOW_FunctionCode_##x

enum MEOW_FunctionCode_ {
    MEOW_FunctionCode_NOOP                   =  0,
    MEOW_FunctionCode_RESET_COUNTERS         =  1,

    MEOW_FunctionCode_SHELL_EXEC_SYNC        = 10,
    MEOW_FunctionCode_SHELL_EXEC_ASYNC       = 11,
    MEOW_FunctionCode_SHELL_POLL             = 12,
    MEOW_FunctionCode_SHELL_KILL             = 13,
    MEOW_FunctionCode_SHELL_EXEC_SYNC_FORCE  = 14,
    MEOW_FunctionCode_SHELL_EXEC_ASYNC_FORCE = 15,

    MEOW_FunctionCode_FILE_READ              = 20,
    MEOW_FunctionCode_FILE_WRITE             = 21,
    MEOW_FunctionCode_FILE_REMOVE            = 22,
    MEOW_FunctionCode_FILE_COPY              = 23,
    MEOW_FunctionCode_FILE_MOVE              = 24,
    MEOW_FunctionCode_FILE_STAT              = 25,
    MEOW_FunctionCode_FILE_TRUNCATE          = 26,
    MEOW_FunctionCode_FILE_TAIL              = 27,
    MEOW_FunctionCode_FILE_CHECKSUM          = 28,
    MEOW_FunctionCode_DISK_STAT              = 29,

    MEOW_FunctionCode_SYS_SHUTDOWN           = 40,
    MEOW_FunctionCode_SYS_SYNC               = 41,
    MEOW_FunctionCode_SYS_INFO               = 42,
    MEOW_FunctionCode_SYS_TIME_GET           = 43,
    MEOW_FunctionCode_SYS_TIME_SET           = 44,
    MEOW_FunctionCode_SYS_FORCE_KILL         = 45,

    MEOW_FunctionCode_CSP_SERVER_START       = 50,
    MEOW_FunctionCode_CSP_SERVER_STOP        = 51,
    MEOW_FunctionCode_CSP_SET_READ_TIMEOUT   = 52,
    MEOW_FunctionCode_CSP_HANDLER_LOAD       = 53,
    MEOW_FunctionCode_CSP_HANDLER_CLEAR      = 54,
    MEOW_FunctionCode_CSP_SEND               = 55,
    MEOW_FunctionCode_CSP_FTP_UPLOAD         = 56,
    MEOW_FunctionCode_CSP_FTP_DOWNLOAD       = 57,
    MEOW_FunctionCode_CSP_IFSTATS            = 58,
    MEOW_FunctionCode_CSP_ROUTE_SET          = 59,
    MEOW_FunctionCode_CSP_REROUTE_SET        = 60,
    MEOW_FunctionCode_CSP_REROUTE_CLEAR      = 61,
};

#endif /* DEFAULT_MEOW_FCNCODE_VALUES_H */
