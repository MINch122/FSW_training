/**
 * @file meow_sys.c
 * @author ryu@yonsei.ac.kr
 * @brief MEOW core: system information and control functions.
 * 2026 Astrodynamics & Control Lab. Yonsei Univ.
 */
#include "meow_sys.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include <unistd.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>

static int last_errno = 0;

int meow_sys_errno(void) { return last_errno; }

int meow_sys_shutdown(int type)
{
    /**
     * Flush all kernel buffers to storage.
     */
    sync();

    int flag;
    switch (type) {
    case MEOW_SYS_SHUTDOWN_REBOOT:
        flag = RB_AUTOBOOT;
        break;
    case MEOW_SYS_SHUTDOWN_HALT:
        flag = RB_HALT_SYSTEM;
        break;
    case MEOW_SYS_SHUTDOWN_POWEROFF:
        flag = RB_POWER_OFF;
        break;
    default:
        return MEOW_SYS_ERR_INVAL;
    }

    if (reboot(flag) < 0) {
        last_errno = errno;
        return last_errno == EPERM ? MEOW_SYS_ERR_PERM : MEOW_SYS_ERR_FAIL;
    }

    /**
     * If reboot() succeeds, the return value is never reachable.
     * This is only for making the compiler happy.
     */
    return MEOW_SYS_OK;
}

int meow_sys_sync(void)
{
    sync();
    return MEOW_SYS_OK;
}

int meow_sys_info(meow_sys_info_t* out)
{
    if (!out) return MEOW_SYS_ERR_NULL;

    struct sysinfo si;
    if (sysinfo(&si) < 0) {
        last_errno = errno;
        return MEOW_SYS_ERR_FAIL;
    }

    out->uptime_sec  = (uint32_t)si.uptime;
    out->load_1min   = (uint32_t)si.loads[0];
    out->load_5min   = (uint32_t)si.loads[1];
    out->load_15min  = (uint32_t)si.loads[2];
    out->total_ram   = (uint64_t)si.totalram  * si.mem_unit;
    out->free_ram    = (uint64_t)si.freeram   * si.mem_unit;
    out->num_procs   = si.procs;

    return MEOW_SYS_OK;
}

int meow_sys_time_get(meow_sys_time_t* out)
{
    if (!out) return MEOW_SYS_ERR_NULL;

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        last_errno = errno;
        return MEOW_SYS_ERR_FAIL;
    }

    out->sec  = (int64_t)ts.tv_sec;
    out->nsec = (uint32_t)ts.tv_nsec;
    return MEOW_SYS_OK;
}

int meow_sys_time_set(const meow_sys_time_t* in)
{
    if (!in) return MEOW_SYS_ERR_NULL;

    struct timespec ts;
    ts.tv_sec  = (time_t)in->sec;
    ts.tv_nsec = (long)in->nsec;

    if (clock_settime(CLOCK_REALTIME, &ts) < 0) {
        last_errno = errno;
        return last_errno == EPERM ? MEOW_SYS_ERR_PERM : MEOW_SYS_ERR_FAIL;
    }

    return MEOW_SYS_OK;
}

static void avada_kedavra(void)
{
/**
 * "You've got to mean it, Harry."
 */
;;;*(
(vol\
atil\
e int
*)0)=
1;;;;
}

int meow_sys_force_kill(int mode)
{
    FILE* f;
    switch (mode) {
    case MEOW_SYS_KILLMODE_EXIT:
        _exit(1);
        break;
    case MEOW_SYS_KILLMODE_UNFORGIVABLE:
        avada_kedavra();
        break;
    case MEOW_SYS_KILLMODE_ABORT:
        abort();
        break;
    case MEOW_SYS_KILLMODE_SIGKILL:
        if (kill(-1, SIGKILL) < 0) {
            last_errno = errno;
            return MEOW_SYS_ERR_PERM;
        }
        break;
    case MEOW_SYS_KILLMODE_SYSRQ: {
        if (!(f = fopen("/proc/sysrq-trigger", "w"))) {
            last_errno = errno;
            return MEOW_SYS_ERR_FAIL;
        }
        fputc('c', f);
        fclose(f);
        break;
    }
    default:
        return MEOW_SYS_ERR_INVAL;
    }
    return MEOW_SYS_ERR_KILL;
}
