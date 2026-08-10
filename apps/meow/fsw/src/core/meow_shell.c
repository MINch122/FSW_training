/**
 * @file meow_shell.c
 * @author ryu@yonsei.ac.kr
 * @brief MEOW core: shell command execution functions.
 * 2026 Astrodynamics & Control Lab. Yonsei Univ.
 */

#include "meow_shell.h"

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>

#define POLL_INTERVAL_US  10000u /* 10 ms per waitpid poll tick */
#define BL_MAX_TOKENS     16
#define BL_MAX_TOKEN_LEN  64

static int last_errno = 0;

int meow_shell_errno(void) { return last_errno; }

typedef struct {
    const char* cmd;        /* basename to match; NULL = any              */
    const char* req_flags;  /* all chars must be in parsed flag set        */
    const char* target_pfx; /* at least one target must start with this    */
} meow_shell_rule_t;

/* a rule fires when ALL non-NULL fields match */
static const meow_shell_rule_t g_blacklist[] = {
    {"rm",    "r",  "/"},
    {"rm",    "R",  "/"},
    {"rm",    "r",  "/root"},
    {"rm",    "R",  "/root"},
    {"rm",    "r",  "/etc"},
    {"rm",    "R",  "/etc"},
    {"mkfs",  NULL, NULL},
    {"dd",    NULL, "/dev/"},
    {"shred", "r",  "/"},
    {"shred", "R",  "/"},
    {NULL,    NULL, NULL}
};

static const char* cmd_basename(const char* s)
{
    const char* p = s;
    while (*s)
        if (*s++ == '/')
            p = s;
    return p;
}

/* Matches target against a protected path: exact match or child path.
 * "/etc" matches "/etc" and "/etc/passwd" but not "/etcstuff".
 * "/dev/" (trailing slash) matches "/dev/sda" and "/dev/null". */
static int target_matches(const char* target, const char* pfx)
{
    size_t len = strlen(pfx);
    size_t cmp_len = (len > 1 && pfx[len - 1] == '/') ? len - 1 : len;
    if (strncmp(target, pfx, cmp_len) != 0)
        return 0;
    return target[cmp_len] == '\0' || target[cmp_len] == '/';
}

static int has_all_flags(const char* found, const char* required)
{
    while (*required)
        if (!strchr(found, *required++))
            return 0;
    return 1;
}

static int cmd_is_blocked(const char* input)
{
    char cmd[BL_MAX_TOKEN_LEN] = {0};
    char flags[BL_MAX_TOKEN_LEN] = {0};
    char targets[BL_MAX_TOKENS][BL_MAX_TOKEN_LEN];
    int ntargets = 0;
    int nflags   = 0;
    const char* p = input;

    /* first token → command basename */
    while (*p == ' ' || *p == '\t')
        p++;

    int i = 0;
    while (*p && *p != ' ' && *p != '\t' && i < BL_MAX_TOKEN_LEN - 1)
        cmd[i++] = *p++;
    cmd[i] = '\0';
    const char* base = cmd_basename(cmd);
    memmove(cmd, base, strlen(base) + 1);

    /* remaining tokens → flags or targets */
    while (*p && ntargets < BL_MAX_TOKENS) {
        while (*p == ' ' || *p == '\t')
            p++;

        if (!*p)
            break;

        char tok[BL_MAX_TOKEN_LEN] = {0};
        i = 0;
        while (*p && *p != ' ' && *p != '\t' && i < BL_MAX_TOKEN_LEN - 1)
            tok[i++] = *p++;
        tok[i] = '\0';

        if (tok[0] == '-' && tok[1] != '-' && tok[1] != '\0') {
            /* short flags: merge individual chars into set, skip duplicates */
            const char* f = tok + 1;
            while (*f) {
                if (!strchr(flags, *f) && nflags < BL_MAX_TOKEN_LEN - 1)
                    flags[nflags++] = *f;
                f++;
            }
        } else if (tok[0] != '-') {
            /* strip key= prefix (e.g. of=/dev/sda → /dev/sda) */
            const char* val = strchr(tok, '=');
            strncpy(targets[ntargets++], val ? val + 1 : tok, BL_MAX_TOKEN_LEN - 1);
        }
        /* -- and long options (--recursive) are ignored for now */
    }

    for (int r = 0; g_blacklist[r].cmd != NULL; r++) {
        const meow_shell_rule_t* rule = &g_blacklist[r];

        /* match exact name or name.variant (e.g. "mkfs" matches "mkfs.ext4") */
        size_t rlen = strlen(rule->cmd);
        if (strncmp(cmd, rule->cmd, rlen) != 0)
            continue;
        if (cmd[rlen] != '\0' && cmd[rlen] != '.')
            continue;
        if (rule->req_flags && !has_all_flags(flags, rule->req_flags))
            continue;
        if (rule->target_pfx) {
            int matched = 0;
            for (int t = 0; t < ntargets; t++) {
                if (target_matches(targets[t], rule->target_pfx)) {
                    matched = 1;
                    break;
                }
            }
            if (!matched)
                continue;
        }
        return 1;
    }

    return 0;
}

/* -------------------------------------------------------------------------
 * Core fork/exec helpers
 * ---------------------------------------------------------------------- */

static int exec_sync_impl(const char* cmd,
                          const char* redir_path,
                          uint32_t timeout,
                          meow_shell_result_t* result,
                          int skip_check)
{
    if (!cmd || !redir_path || !result)
        return MEOW_SHELL_ERR_NULL_CMD;

    if (!skip_check && cmd_is_blocked(cmd))
        return MEOW_SHELL_ERR_BLOCKED;

    int rfd = open(redir_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (rfd < 0) {
        last_errno = errno;
        return MEOW_SHELL_ERR_REDIR;
    }

    pid_t pid = fork();
    if (pid < 0) {
        last_errno = errno;
        close(rfd);
        return MEOW_SHELL_ERR_FORK;
    }

    if (pid == 0) {
        setpgid(0, 0);
        dup2(rfd, STDOUT_FILENO);
        dup2(rfd, STDERR_FILENO);
        close(rfd);
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127);
    }

    setpgid(pid, pid);  /* parent side: race-free group creation; EACCES is OK */
    close(rfd);

    uint32_t elapsed_us = 0;
    uint32_t timeout_us = timeout * 1000u;

    while (1) {
        int   status;
        pid_t ret = waitpid(pid, &status, WNOHANG);

        if (ret > 0) {
            result->pid = pid;
            result->exit_code = WIFEXITED(status) ? WEXITSTATUS(status)
                                                  : MEOW_SHELL_ERR_EXEC;
            return MEOW_SHELL_OK;
        }

        if (ret < 0) {
            last_errno = errno;
            return MEOW_SHELL_ERR_EXEC;
        }

        if (timeout > 0 && elapsed_us >= timeout_us) {
            if (killpg(pid, SIGKILL) == 0)
                waitpid(pid, NULL, 0);
            else
                waitpid(pid, NULL, WNOHANG);
            result->pid = pid;
            return MEOW_SHELL_ERR_TIMEOUT;
        }

        usleep(POLL_INTERVAL_US);
        elapsed_us += POLL_INTERVAL_US;
    }
}

static int exec_async_impl(const char* cmd,
                           const char* redir_path,
                           int* out_pid,
                           int skip_check)
{
    if (!cmd || !redir_path || !out_pid)
        return MEOW_SHELL_ERR_NULL_CMD;

    if (!skip_check && cmd_is_blocked(cmd))
        return MEOW_SHELL_ERR_BLOCKED;

    int rfd = open(redir_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (rfd < 0) {
        last_errno = errno;
        return MEOW_SHELL_ERR_REDIR;
    }

    pid_t pid = fork();
    if (pid < 0) {
        last_errno = errno;
        close(rfd);
        return MEOW_SHELL_ERR_FORK;
    }

    if (pid == 0) {
        setpgid(0, 0);
        dup2(rfd, STDOUT_FILENO);
        dup2(rfd, STDERR_FILENO);
        close(rfd);
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127);
    }

    setpgid(pid, pid);  /* parent side: race-free group creation; EACCES is OK */
    close(rfd);
    *out_pid = pid;
    return MEOW_SHELL_OK;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

int meow_shell_exec_sync(const char* cmd,
                         const char* redir_path,
                         uint32_t timeout_ms,
                         meow_shell_result_t* result)
{
    return exec_sync_impl(cmd, redir_path, timeout_ms, result, 0);
}

int meow_shell_exec_sync_force(const char* cmd,
                               const char* redir_path,
                               uint32_t timeout_ms,
                               meow_shell_result_t* result)
{
    return exec_sync_impl(cmd, redir_path, timeout_ms, result, 1);
}

int meow_shell_exec_async(const char* cmd,
                          const char* redir_path,
                          int* out_pid)
{
    return exec_async_impl(cmd, redir_path, out_pid, 0);
}

int meow_shell_exec_async_force(const char* cmd,
                                const char* redir_path,
                                int* out_pid)
{
    return exec_async_impl(cmd, redir_path, out_pid, 1);
}

int meow_shell_poll(int pid, int* exit_code)
{
    if (!exit_code)
        return MEOW_SHELL_ERR_NULL_CMD;

    int   stat;
    pid_t ret = waitpid(pid, &stat, WNOHANG);

    if (ret == 0)
        return MEOW_SHELL_RUNNING;

    if (ret > 0) {
        *exit_code = WIFEXITED(stat) ? WEXITSTATUS(stat) : MEOW_SHELL_ERR_EXEC;
        return MEOW_SHELL_OK;
    }

    last_errno = errno;
    return MEOW_SHELL_ERR_NO_PROC;
}

int meow_shell_kill(int pid, int sig)
{
    if (kill(pid, 0) < 0) {
        last_errno = errno;
        return MEOW_SHELL_ERR_NO_PROC;
    }

    if (killpg(pid, sig) < 0) {
        last_errno = errno;
        return MEOW_SHELL_ERR_NO_PROC;
    }

    return MEOW_SHELL_OK;
}
