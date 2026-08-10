/**
 * Coverage tests for meow_shell.c — pure POSIX.
 */

#include "utassert.h"
#include "uttest.h"

#include "meow_shell.h"

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#define REDIR "/tmp/meow_ut_shell_out"

static void cleanup(void) { unlink(REDIR); }

/* -------------------------------------------------------------------------
 * Null argument rejection
 * ---------------------------------------------------------------------- */

void Test_ExecSync_NullCmd(void)
{
    meow_shell_result_t r;
    UtAssert_INT32_EQ(meow_shell_exec_sync(NULL, REDIR, 0, &r),
                      MEOW_SHELL_ERR_NULL_CMD);
}

void Test_ExecSync_NullRedir(void)
{
    meow_shell_result_t r;
    UtAssert_INT32_EQ(meow_shell_exec_sync("true", NULL, 0, &r),
                      MEOW_SHELL_ERR_NULL_CMD);
}

void Test_ExecSync_NullResult(void)
{
    UtAssert_INT32_EQ(meow_shell_exec_sync("true", REDIR, 0, NULL),
                      MEOW_SHELL_ERR_NULL_CMD);
}

void Test_ExecAsync_NullCmd(void)
{
    int pid;
    UtAssert_INT32_EQ(meow_shell_exec_async(NULL, REDIR, &pid),
                      MEOW_SHELL_ERR_NULL_CMD);
}

void Test_ExecAsync_NullRedir(void)
{
    int pid;
    UtAssert_INT32_EQ(meow_shell_exec_async("true", NULL, &pid),
                      MEOW_SHELL_ERR_NULL_CMD);
}

void Test_ExecAsync_NullPid(void)
{
    UtAssert_INT32_EQ(meow_shell_exec_async("true", REDIR, NULL),
                      MEOW_SHELL_ERR_NULL_CMD);
}

void Test_Poll_NullExitCode(void)
{
    UtAssert_INT32_EQ(meow_shell_poll(1, NULL), MEOW_SHELL_ERR_NULL_CMD);
}

/* -------------------------------------------------------------------------
 * Synchronous execution
 * ---------------------------------------------------------------------- */

void Test_ExecSync_TrueCommand(void)
{
    cleanup();
    meow_shell_result_t r;
    UtAssert_INT32_EQ(meow_shell_exec_sync("true", REDIR, 5000, &r), MEOW_SHELL_OK);
    UtAssert_INT32_EQ(r.exit_code, 0);
    cleanup();
}

void Test_ExecSync_AbsolutePath(void)
{
    cleanup();
    meow_shell_result_t r;
    UtAssert_INT32_EQ(meow_shell_exec_sync("/bin/true", REDIR, 5000, &r), MEOW_SHELL_OK);
    UtAssert_INT32_EQ(r.exit_code, 0);
    cleanup();
}

void Test_ExecSync_FalseCommand(void)
{
    cleanup();
    meow_shell_result_t r;
    UtAssert_INT32_EQ(meow_shell_exec_sync("false", REDIR, 5000, &r), MEOW_SHELL_OK);
    UtAssert_True(r.exit_code != 0, "false exits nonzero");
    cleanup();
}

void Test_ExecSync_Timeout(void)
{
    cleanup();
    meow_shell_result_t r;
    int ret = meow_shell_exec_sync("sleep 30", REDIR, 200, &r);
    UtAssert_INT32_EQ(ret, MEOW_SHELL_ERR_TIMEOUT);
    cleanup();
}

/* -------------------------------------------------------------------------
 * Blacklist
 * ---------------------------------------------------------------------- */

/* Test_Blacklist_RmRfSlash and Test_Blacklist_RmRfSlashReordered omitted —
 * require container isolation; run separately in a safe environment. */

void Test_Blacklist_Force_Bypasses(void)
{
    /* _force variant should skip the blacklist check; "rm -rf /" would be allowed
     * through but we redirect to a safe tempfile path so there's no actual damage.
     * We can't safely run "rm -rf /" even with force, so test with a safe command
     * that would normally pass the filter. */
    cleanup();
    meow_shell_result_t r;
    UtAssert_INT32_EQ(meow_shell_exec_sync_force("true", REDIR, 5000, &r), MEOW_SHELL_OK);
    cleanup();
}

/* -------------------------------------------------------------------------
 * Asynchronous execution
 * ---------------------------------------------------------------------- */

void Test_ExecAsync_PollReap(void)
{
    cleanup();
    int pid = -1;
    UtAssert_INT32_EQ(meow_shell_exec_async("true", REDIR, &pid), MEOW_SHELL_OK);
    UtAssert_True(pid > 0, "got valid pid");

    /* poll until reaped (max ~5 s) */
    int exit_code = -1;
    int ret = MEOW_SHELL_RUNNING;
    for (int i = 0; i < 500 && ret == MEOW_SHELL_RUNNING; i++) {
        usleep(10000);  /* 10 ms */
        ret = meow_shell_poll(pid, &exit_code);
    }
    UtAssert_INT32_EQ(ret, MEOW_SHELL_OK);
    UtAssert_INT32_EQ(exit_code, 0);
    cleanup();
}

void Test_ExecAsync_Kill(void)
{
    cleanup();
    int pid = -1;
    UtAssert_INT32_EQ(meow_shell_exec_async("sleep 30", REDIR, &pid), MEOW_SHELL_OK);
    UtAssert_True(pid > 0, "got valid pid");

    UtAssert_INT32_EQ(meow_shell_kill(pid, SIGTERM), MEOW_SHELL_OK);

    /* reap after kill */
    int exit_code = -1;
    int ret = MEOW_SHELL_RUNNING;
    for (int i = 0; i < 200 && ret == MEOW_SHELL_RUNNING; i++) {
        usleep(10000);
        ret = meow_shell_poll(pid, &exit_code);
    }
    UtAssert_True(ret == MEOW_SHELL_OK || ret == MEOW_SHELL_ERR_NO_PROC,
                  "process reaped after kill");
    cleanup();
}

void Test_Poll_NoSuchProc(void)
{
    int exit_code;
    /* PID 1 is init/systemd; waitpid on a non-child returns ECHILD → ERR_NO_PROC */
    int ret = meow_shell_poll(1, &exit_code);
    UtAssert_True(ret == MEOW_SHELL_ERR_NO_PROC || ret == MEOW_SHELL_RUNNING,
                  "poll on non-child");
}

void Test_Kill_NoSuchProc(void)
{
    /* kill on non-child returns MEOW_SHELL_ERR_NO_PROC.
     * Use a large invalid PID that is unlikely to exist. */
    UtAssert_INT32_EQ(meow_shell_kill(999999999, SIGTERM), MEOW_SHELL_ERR_NO_PROC);
}

/* -------------------------------------------------------------------------
 * Async force variant (smoke test)
 * ---------------------------------------------------------------------- */

void Test_ExecAsyncForce_Smoke(void)
{
    cleanup();
    int pid = -1;
    UtAssert_INT32_EQ(meow_shell_exec_async_force("true", REDIR, &pid), MEOW_SHELL_OK);
    UtAssert_True(pid > 0, "got valid pid");
    int exit_code;
    for (int i = 0; i < 500; i++) {
        usleep(10000);
        if (meow_shell_poll(pid, &exit_code) == MEOW_SHELL_OK)
            break;
    }
    cleanup();
}

/* -------------------------------------------------------------------------
 * Test registration
 * ---------------------------------------------------------------------- */

void UtTest_Setup(void)
{
    UtTest_Add(Test_ExecSync_NullCmd,           NULL, NULL, "ExecSync_NullCmd");
    UtTest_Add(Test_ExecSync_NullRedir,         NULL, NULL, "ExecSync_NullRedir");
    UtTest_Add(Test_ExecSync_NullResult,        NULL, NULL, "ExecSync_NullResult");
    UtTest_Add(Test_ExecAsync_NullCmd,          NULL, NULL, "ExecAsync_NullCmd");
    UtTest_Add(Test_ExecAsync_NullRedir,        NULL, NULL, "ExecAsync_NullRedir");
    UtTest_Add(Test_ExecAsync_NullPid,          NULL, NULL, "ExecAsync_NullPid");
    UtTest_Add(Test_Poll_NullExitCode,          NULL, NULL, "Poll_NullExitCode");

    UtTest_Add(Test_ExecSync_TrueCommand,       NULL, NULL, "ExecSync_TrueCommand");
    UtTest_Add(Test_ExecSync_AbsolutePath,      NULL, NULL, "ExecSync_AbsolutePath");
    UtTest_Add(Test_ExecSync_FalseCommand,      NULL, NULL, "ExecSync_FalseCommand");
    UtTest_Add(Test_ExecSync_Timeout,           NULL, NULL, "ExecSync_Timeout");

    UtTest_Add(Test_Blacklist_Force_Bypasses,   NULL, NULL, "Blacklist_Force_Bypasses");

    UtTest_Add(Test_ExecAsync_PollReap,         NULL, NULL, "ExecAsync_PollReap");
    UtTest_Add(Test_ExecAsync_Kill,             NULL, NULL, "ExecAsync_Kill");
    UtTest_Add(Test_Poll_NoSuchProc,            NULL, NULL, "Poll_NoSuchProc");
    UtTest_Add(Test_Kill_NoSuchProc,            NULL, NULL, "Kill_NoSuchProc");
    UtTest_Add(Test_ExecAsyncForce_Smoke,       NULL, NULL, "ExecAsyncForce_Smoke");
}
