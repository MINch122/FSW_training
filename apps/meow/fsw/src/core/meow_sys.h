/**
 * @file meow_sys.h
 * @author ryu@yonsei.ac.kr
 * @brief MEOW core: system information and control functions.
 * 2026 Astrodynamics & Control Lab. Yonsei Univ.
 */
#ifndef _MEOW_SYS_H_
#define _MEOW_SYS_H_

#include <stdint.h>


/* return codes */
typedef enum {
    MEOW_SYS_OK         =  0,
    MEOW_SYS_ERR_NULL   = -1,
    MEOW_SYS_ERR_PERM   = -2,
    MEOW_SYS_ERR_FAIL   = -3,
    MEOW_SYS_ERR_INVAL  = -4,
    MEOW_SYS_ERR_BOUNDS = -5,
    MEOW_SYS_ERR_KILL   = -6,
} meow_sys_ret_t;


/* mode argument for meow_sys_shutdown() */
#define MEOW_SYS_SHUTDOWN_REBOOT   0
#define MEOW_SYS_SHUTDOWN_HALT     1
#define MEOW_SYS_SHUTDOWN_POWEROFF 2

/* mode argument for meow_sys_force_kill() */
typedef enum {
    /* calls _exit(1) */
    MEOW_SYS_KILLMODE_EXIT          = 1,
    /* performs a null pointer write to invoke SIGSEGV */
    MEOW_SYS_KILLMODE_UNFORGIVABLE  = 2,
    /* calls abort() */
    MEOW_SYS_KILLMODE_ABORT         = 3,
    /* sends SIGKILL to all processes in the same process group */
    MEOW_SYS_KILLMODE_SIGKILL       = 4,
    /* writes to /proc/sysrq-trigger to invoke a kernel panic */
    MEOW_SYS_KILLMODE_SYSRQ         = 5,
} meow_sys_killmode_t;


typedef struct {
    uint32_t uptime_sec;
    uint32_t load_1min;
    uint32_t load_5min;
    uint32_t load_15min;
    uint64_t total_ram;   /* bytes */
    uint64_t free_ram;    /* bytes */
    uint16_t num_procs;
} meow_sys_info_t;

typedef struct {
    int64_t  sec;
    uint32_t nsec;
} meow_sys_time_t;


/**
 * @brief Flush all pending writes then reboot, halt, or power off.
 *
 * @details
 *      - Calls sync() before invoking reboot(2).
 *      - Does not return on success; the process is terminated by the kernel.
 *      - type must be one of MEOW_SYS_SHUTDOWN_REBOOT, MEOW_SYS_SHUTDOWN_HALT,
 *            or MEOW_SYS_SHUTDOWN_POWEROFF.
 *
 * @param type  Shutdown mode (MEOW_SYS_SHUTDOWN_*).
 * @return  MEOW_SYS_ERR_INVAL if type is not a recognized shutdown mode.
 *          MEOW_SYS_ERR_PERM if reboot() is not permitted (missing CAP_SYS_BOOT).
 *          MEOW_SYS_ERR_FAIL if reboot() fails for any other reason.
 */
int meow_sys_shutdown(int type);

/**
 * @brief Call sync() to flush all pending writes to storage.
 *
 * @return  Always MEOW_SYS_OK.
 */
int meow_sys_sync(void);

/**
 * @brief Populate a meow_sys_info_t from a single sysinfo(2) syscall.
 *
 * @details
 *      - No file parsing; all fields come directly from the kernel sysinfo struct.
 *      - load_1min / load_5min / load_15min are raw fixed-point values;
 *            divide by 65536 to get the float equivalent.
 *
 * @param[out] out  Destination struct; filled on MEOW_SYS_OK.
 * @return  MEOW_SYS_OK on success.
 *          MEOW_SYS_ERR_NULL if out is NULL.
 *          MEOW_SYS_ERR_FAIL if sysinfo() fails.
 */
int meow_sys_info(meow_sys_info_t* out);

/**
 * @brief Get the current system time from clock_gettime(CLOCK_REALTIME).
 *
 * @param[out] out  Destination buffer; filled on MEOW_SYS_OK.
 * @return  MEOW_SYS_OK on success.
 *          MEOW_SYS_ERR_NULL if out is NULL.
 *          MEOW_SYS_ERR_FAIL if clock_gettime() fails.
 */
int meow_sys_time_get(meow_sys_time_t* out);

/**
 * @brief Set the system time using clock_settime(CLOCK_REALTIME).
 *
 * @param[in] in  New time to set.
 * @return  MEOW_SYS_OK on success.
 *          MEOW_SYS_ERR_NULL if in is NULL.
 *          MEOW_SYS_ERR_PERM if the operation is not permitted (missing CAP_SYS_TIME).
 *          MEOW_SYS_ERR_FAIL if clock_settime() fails for any other reason.
 */
int meow_sys_time_set(const meow_sys_time_t* in);

/**
 * @brief Forcibly terminate or crash the process for testing/watchdog purposes.
 *
 * @details
 *      - mode=MEOW_SYS_KILLMODE_EXIT: _exit(1)
 *      - mode=MEOW_SYS_KILLMODE_UNFORGIVABLE: null pointer write (crash)
 *      - mode=MEOW_SYS_KILLMODE_ABORT: abort()
 *      - mode=MEOW_SYS_KILLMODE_SIGKILL: kill(-1, SIGKILL)
 *      - mode=MEOW_SYS_KILLMODE_SYSRQ: echo 'c' > /proc/sysrq-trigger
 *
 * @param mode  Termination mode. See meow_sys_killmode_t.
 * @return  MEOW_SYS_ERR_INVAL for invalid mode.
 *          MEOW_SYS_ERR_KILL for kill operation failure.
 *          MEOW_SYS_ERR_PERM if the kill() is not permitted for SIGKILL mode.
 *          MEOW_SYS_ERR_FAIL if the trigger file cannot be opened for SYSRQ mode.
 */
int meow_sys_force_kill(int mode);

/**
 * @brief Return the system errno from the most recent failed meow_sys_* call.
 */
int meow_sys_errno(void);

#endif /* MEOW_SYS_H */
