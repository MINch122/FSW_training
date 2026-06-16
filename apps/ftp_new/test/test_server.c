/**
 * @file test_server.c
 * @brief Server-side test runner.
 *
 * Initializes CSP as SERVER_ADDR, opens vcan0, sets up the default route,
 * starts the router task, then hands control to ftp_server_start() (in
 * ../server/src/ftpnew.c). ftp_server_start() loops on csp_accept and
 * spawns one ftp_handler_thread per accepted connection.
 *
 * The server stays up until killed via SIGINT or SIGTERM (run_tests.sh
 * sends one when the client finishes).
 */
#include "test_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#define SERVER_WORKDIR "."

static void on_signal(int sig)
{
    (void)sig;
    /* The handler threads do the real work; the listener never returns.
     * Let the OS reap us on the signal -- nothing we can flush gracefully. */
    _exit(0);
}

static int prep_workdir(void)
{
    const char *dir = getenv("FTPNEW_TEST_WORKDIR");
    if (!dir)
        dir = SERVER_WORKDIR;

    if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
        perror("mkdir server workdir");
        return -1;
    }
    if (chdir(dir) != 0) {
        perror("chdir server workdir");
        return -1;
    }
    printf("[server] workdir = %s\n", dir);
    return 0;
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    signal(SIGINT,  on_signal);
    signal(SIGTERM, on_signal);

    if (prep_workdir() != 0)
        return 1;

    if (test_csp_init_node(SERVER_ADDR) != 0) {
        fprintf(stderr, "[server] CSP init failed\n");
        return 1;
    }

    printf("[server] listening on CSP port %u\n", FTP_PORT);

    /* Blocks forever in the accept loop. */
    int rc = ftp_server_start();
    fprintf(stderr, "[server] ftp_server_start returned %d -- exiting\n", rc);

    test_csp_teardown();
    return rc;
}
