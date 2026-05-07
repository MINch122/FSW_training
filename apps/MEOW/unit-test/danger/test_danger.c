/**
 * Danger test harness — each invocation runs exactly one test case.
 * Usage: test_danger <case_name>
 *
 * Exit codes used by this binary:
 *   0   — test PASSED (assertion held, function returned expected value)
 *   1   — meow_sys_force_kill(EXIT) fired — _exit(1) is the expected outcome
 *   2   — test FAILED (assertion failed, or function returned when it should not have)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "meow_sys.h"
#include "meow_shell.h"

#define REDIR "/tmp/danger_redir"

static void fail(const char* msg, int got)
{
    fprintf(stderr, "FAIL: %s (got %d)\n", msg, got);
    exit(2);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: test_danger <case_name>\n");
        return 2;
    }
    const char* tc = argv[1];

    /* -------------------------------------------------------------------------
     * Shutdown
     * ---------------------------------------------------------------------- */

    if (!strcmp(tc, "shutdown_invalid")) {
        int r = meow_sys_shutdown(99);
        if (r == MEOW_SYS_ERR_INVAL)
            exit(0);
        fail("expected MEOW_SYS_ERR_INVAL for invalid type", r);
    }

    if (!strcmp(tc, "shutdown_poweroff")) {
        meow_sys_shutdown(MEOW_SYS_SHUTDOWN_POWEROFF);
        fail("shutdown_poweroff: function returned instead of terminating", 0);
    }

    if (!strcmp(tc, "shutdown_halt")) {
        meow_sys_shutdown(MEOW_SYS_SHUTDOWN_HALT);
        fail("shutdown_halt: function returned instead of terminating", 0);
    }

    if (!strcmp(tc, "shutdown_reboot")) {
        meow_sys_shutdown(MEOW_SYS_SHUTDOWN_REBOOT);
        fail("shutdown_reboot: function returned instead of terminating", 0);
    }

    /* -------------------------------------------------------------------------
     * Force kill
     * ---------------------------------------------------------------------- */

    if (!strcmp(tc, "forcekill_inval")) {
        int r = meow_sys_force_kill(99);
        if (r == MEOW_SYS_ERR_INVAL)
            exit(0);
        fail("expected MEOW_SYS_ERR_INVAL for invalid mode", r);
    }

    if (!strcmp(tc, "forcekill_exit")) {
        /* _exit(1) — container exits with code 1 */
        meow_sys_force_kill(MEOW_SYS_KILLMODE_EXIT);
        fail("forcekill_exit: function returned instead of calling _exit", 0);
    }

    if (!strcmp(tc, "forcekill_unforgivable")) {
        /* null pointer write → SIGSEGV → container exits with code 139 */
        meow_sys_force_kill(MEOW_SYS_KILLMODE_UNFORGIVABLE);
        fail("forcekill_unforgivable: function returned instead of crashing", 0);
    }

    if (!strcmp(tc, "forcekill_abort")) {
        /* abort() → SIGABRT → container exits with code 134 */
        meow_sys_force_kill(MEOW_SYS_KILLMODE_ABORT);
        fail("forcekill_abort: function returned instead of aborting", 0);
    }

    if (!strcmp(tc, "forcekill_sigkill")) {
        /* kill(-1, SIGKILL) → test process dies → container exits with code 137.
         * Requires --init so the test binary is not PID 1 (kill(-1) exempts PID 1). */
        int r = meow_sys_force_kill(MEOW_SYS_KILLMODE_SIGKILL);
        fail("forcekill_sigkill: function returned instead of dying", r);
    }

    if (!strcmp(tc, "forcekill_sysrq")) {
        /* In an unprivileged container /proc/sysrq-trigger is not writable.
         * Verify the function returns MEOW_SYS_ERR_FAIL and does not panic the host. */
        int r = meow_sys_force_kill(MEOW_SYS_KILLMODE_SYSRQ);
        if (r == MEOW_SYS_ERR_FAIL)
            exit(0);
        fail("expected MEOW_SYS_ERR_FAIL for sysrq without access", r);
    }

    /* -------------------------------------------------------------------------
     * Shell blacklist
     * ---------------------------------------------------------------------- */

    if (!strcmp(tc, "blacklist_rm_rf")) {
        meow_shell_result_t r;
        int ret = meow_shell_exec_sync("rm -rf /", REDIR, 0, &r);
        if (ret == MEOW_SHELL_ERR_BLOCKED)
            exit(0);
        fail("expected MEOW_SHELL_ERR_BLOCKED for rm -rf /", ret);
    }

    if (!strcmp(tc, "blacklist_rm_fr")) {
        meow_shell_result_t r;
        int ret = meow_shell_exec_sync("rm -fr /", REDIR, 0, &r);
        if (ret == MEOW_SHELL_ERR_BLOCKED)
            exit(0);
        fail("expected MEOW_SHELL_ERR_BLOCKED for rm -fr /", ret);
    }

    if (!strcmp(tc, "blacklist_rm_fr_ws")) {
        meow_shell_result_t r;
        int ret = meow_shell_exec_sync("rm    -fr    /   ", REDIR, 0, &r);
        if (ret == MEOW_SHELL_ERR_BLOCKED)
            exit(0);
        fail("expected MEOW_SHELL_ERR_BLOCKED for rm    -fr    /   ", ret);
    }

    if (!strcmp(tc, "blacklist_rm_slash_fr")) {
        meow_shell_result_t r;
        int ret = meow_shell_exec_sync("rm    /  -f -r     ", REDIR, 0, &r);
        if (ret == MEOW_SHELL_ERR_BLOCKED)
            exit(0);
        fail("expected MEOW_SHELL_ERR_BLOCKED for rm / -f -r", ret);
    }

    if (!strcmp(tc, "blacklist_dd_dev")) {
        meow_shell_result_t r;
        int ret = meow_shell_exec_sync("dd if=/dev/zero of=/dev/sda", REDIR, 0, &r);
        if (ret == MEOW_SHELL_ERR_BLOCKED)
            exit(0);
        fail("expected MEOW_SHELL_ERR_BLOCKED for dd of=/dev/sda", ret);
    }

    if (!strcmp(tc, "blacklist_mkfs")) {
        meow_shell_result_t r;
        int ret = meow_shell_exec_sync("mkfs.ext4 /dev/sda", REDIR, 0, &r);
        if (ret == MEOW_SHELL_ERR_BLOCKED)
            exit(0);
        fail("expected MEOW_SHELL_ERR_BLOCKED for mkfs", ret);
    }

    fprintf(stderr, "unknown test case: %s\n", tc);
    return 2;
}
