/**
 * @file meow_shell.h
 * @author ryu@yonsei.ac.kr
 * @brief MEOW core: shell command execution functions.
 * 2026 Astrodynamics & Control Lab. Yonsei Univ.
 */
#ifndef _MEOW_SHELL_H_
#define _MEOW_SHELL_H_

#include <stdint.h>


/* return codes */
typedef enum {
    MEOW_SHELL_OK           =  0,
    MEOW_SHELL_RUNNING      =  1, /* poll: worker still alive          */
    MEOW_SHELL_ERR_NULL_CMD = -1,
    MEOW_SHELL_ERR_FORK     = -2,
    MEOW_SHELL_ERR_REDIR    = -3,
    MEOW_SHELL_ERR_EXEC     = -4,
    MEOW_SHELL_ERR_TIMEOUT  = -5,
    MEOW_SHELL_ERR_NO_PROC  = -6,
    MEOW_SHELL_ERR_BLOCKED  = -7,
} meow_shell_ret_t;


typedef struct {
    int exit_code; /* WEXITSTATUS on clean exit, MEOW_SHELL_ERR_* otherwise */
    int pid;       /* child PID (async: live until polled; sync: reaped)    */
} meow_shell_result_t;


/**
 * @brief Execute a shell command, waiting for it to finish or timeout.
 *
 * @details
 *      - The command is executed as /bin/sh -c "cmd". Returns after the child
 *            is reaped or killed on timeout (i.e., this function blocks).
 *      - redir_path is opened O_TRUNC before fork. If open fails, returns
 *            MEOW_SHELL_ERR_REDIR immediately without spawning a child.
 *      - timeout_ms is converted to microseconds internally; values larger than
 *            UINT32_MAX / 1000 will overflow (do not use such large values).
 *      - MEOW_SHELL_OK only means the child was forked and reaped. Check
 *            result->exit_code for the command's actual exit status. MEOW_SHELL_OK
 *            does not imply the command itself succeeded.
 *      - Some dangerous cmds (e.g. "rm -rf /") are blocked by a pattern search.
 *            This is a simple parsing sanity check, not a foolproof security
 *            boundary (e.g. shell metacharacters like "$(rm) -rf /" bypass it).
 *            Use the _force() variant to skip the check entirely.
 *
 * @param cmd        Command line to execute.
 * @param redir_path Path to redirect stdout and stderr to. Must be non-NULL.
 * @param timeout_ms Timeout in milliseconds. 0 waits indefinitely.
 * @param[out] result Child PID and exit code on MEOW_SHELL_OK.
 * @return           MEOW_SHELL_OK: child reaped; inspect result->exit_code.
 *                   MEOW_SHELL_ERR_NULL_CMD: a required argument is NULL.
 *                   MEOW_SHELL_ERR_BLOCKED: command matched the blacklist.
 *                   MEOW_SHELL_ERR_REDIR: failed to open redir_path before fork.
 *                   MEOW_SHELL_ERR_FORK: fork() failed.
 *                   MEOW_SHELL_ERR_EXEC: waitpid() returned an unexpected error.
 *                   MEOW_SHELL_ERR_TIMEOUT: child killed after timeout_ms elapsed.
 */
int meow_shell_exec_sync(const char* cmd,
                         const char* redir_path,
                         uint32_t timeout_ms,
                         meow_shell_result_t* result);

/**
 * @brief Execute a shell command synchronously, bypassing the blacklist.
 *
 * @return See meow_shell_exec_sync().
 */
int meow_shell_exec_sync_force(const char* cmd,
                               const char* redir_path,
                               uint32_t timeout_ms,
                               meow_shell_result_t* result);

/**
 * @brief Launch a shell command without waiting for it to finish.
 *
 * @details
 *      - The command is executed as /bin/sh -c "cmd". Returns immediately after
 *            the child is forked.
 *      - redir_path is opened O_TRUNC before fork. If open fails, returns
 *            MEOW_SHELL_ERR_REDIR immediately without spawning a child.
 *      - On MEOW_SHELL_OK, *out_pid is the live child PID. Call meow_shell_poll()
 *            to reap the exit status when the child finishes.
 *      - Same blacklist rules as meow_shell_exec_sync() apply. Use the _force()
 *            variant to skip the check entirely.
 *
 * @param cmd         Command line to execute.
 * @param redir_path  Path to redirect stdout and stderr to. Must be non-NULL.
 * @param[out] out_pid Live child PID on MEOW_SHELL_OK.
 * @return            MEOW_SHELL_OK: child launched; use *out_pid with poll/kill.
 *                    MEOW_SHELL_ERR_NULL_CMD: a required argument is NULL.
 *                    MEOW_SHELL_ERR_BLOCKED: command matched the blacklist.
 *                    MEOW_SHELL_ERR_REDIR: failed to open redir_path before fork.
 *                    MEOW_SHELL_ERR_FORK: fork() failed.
 */
int meow_shell_exec_async(const char* cmd,
                          const char* redir_path,
                          int* out_pid);

/**
 * @brief Launch a shell command asynchronously, bypassing the blacklist.
 *
 * @return See meow_shell_exec_async().
 */
int meow_shell_exec_async_force(const char* cmd,
                                const char* redir_path,
                                int* out_pid);

/**
 * @brief Poll a child process for exit status without blocking.
 *        Reaps the child if it has exited, retrieving the exit_code.
 *
 * @param pid Child process ID to poll returned from async shell exec.
 * @param exit_code Pointer to store the exit code if the child has exited.
 * @return MEOW_SHELL_OK if the child has exited and exit_code is valid.
 *         MEOW_SHELL_RUNNING if the child is still running.
 *         MEOW_SHELL_ERR_NULL_CMD if exit_code is NULL.
 *         MEOW_SHELL_ERR_NO_PROC if the child process does not exist.
 */
int meow_shell_poll(int pid, int* exit_code);

/**
 * @brief Kill a child process launched by async shell exec.
 *
 * @param pid  Child process ID to kill returned from async shell exec.
 * @param sig  Signal to send to the child process.
 * @return MEOW_SHELL_OK if the signal was successfully sent.
 *         MEOW_SHELL_ERR_NO_PROC if the child process does not exist.
 */
int meow_shell_kill(int pid, int sig);

/**
 * @brief Return the system errno from the most recent failed meow_shell_* call.
 */
int meow_shell_errno(void);

#endif /* MEOW_SHELL_H */
