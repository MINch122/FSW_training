/**
 * @file simdev_csp_server.c
 * @author ryu@yonsei.ac.kr
 * @brief I just really don't wanna go to the clean room!
 * @version 0.1
 * @date 2026-03-24
 * 
 * Yonsei ACL, 2026
 */
#include <csp/csp.h>
#include <csp/drivers/can_socketcan.h>

#include "s5lab_types_internal.h"
#include "s5lab.h"

#include "common.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define SIMDEV_NODE_HOST 3
#define SIMDEV_NODE_SELF 10
#define SIMDEV_LOG_MAX_LENGTH 200
#define SIMDEV_MAX_LOGS 100
#define SIMDEV_CSP_CAN_INTERFACE "vcan0"

  
typedef union {
    uint8_t type;

    s5lab_cmd_get_specific_line_t get_specific_line;
    s5lab_cmd_get_multiple_lines_t get_multiple_lines;
    s5lab_cmd_get_latest_line_t get_latest_line;
    s5lab_cmd_get_latest_n_lines_t get_latest_n_lines;
    s5lab_cmd_clear_all_lines_t clear_all_lines;

    s5lab_cmd_get_single_entry_t get_single_entry;
    s5lab_cmd_get_multiple_entries_t get_multiple_entries;
    s5lab_cmd_add_entry_t add_entry;
    s5lab_cmd_remove_entry_t remove_entry;
    s5lab_cmd_get_used_slots_t get_used_slots;

    s5lab_cmd_set_route_default_t set_route_default;
    s5lab_cmd_reset_route_t reset_route;
    s5lab_cmd_load_route_t load_route;
    s5lab_cmd_save_route_t save_route;
    s5lab_cmd_send_route_t send_route;
    s5lab_cmd_set_route_t set_route;

    s5lab_cmd_par_get_t par_get;
    s5lab_cmd_par_set_t par_set;
    s5lab_cmd_par_defaults_t par_defaults;
    s5lab_cmd_par_save_t par_save;
    s5lab_cmd_par_restore_t par_restore;
    s5lab_cmd_par_load_t par_load;
    s5lab_cmd_par_set_oob_t par_set_oob;

    char send_command[256];
} s5lab_cmd_t;

typedef struct {
    uint16_t line_no;
    uint8_t  info;
    uint8_t  len;
    uint32_t time_s;
    uint16_t time_ms;
    uint16_t incr;
    char     line[SIMDEV_LOG_MAX_LENGTH];
} log_entry_t;

typedef struct {
    int nlogs;
    log_entry_t logs[SIMDEV_MAX_LOGS];
} log_storage_t;

static log_storage_t log_storage;

static bool copy_log(s5lab_rep_get_line_t* rep, uint16_t line)
{
    if (line >= SIMDEV_MAX_LOGS || line >= log_storage.nlogs) {
        return false;
    }
    rep->line_no = log_storage.logs[line].line_no;
    rep->info = log_storage.logs[line].info;
    rep->len = log_storage.logs[line].len;
    rep->time_s = log_storage.logs[line].time_s;
    rep->time_ms = log_storage.logs[line].time_ms;
    rep->incr = log_storage.logs[line].incr++;
    strncpy(rep->line, log_storage.logs[line].line, sizeof(log_storage.logs[line].line) - 1);
    rep->line[csp_buffer_size() - sizeof(csp_packet_t) - sizeof(*rep) - 1] = '\0';
    return true;
}

static void add_log(const char* log)
{
    if (log_storage.nlogs < SIMDEV_MAX_LOGS) {
        printf("Adding log [#%d]: %s\n", log_storage.nlogs, log);
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        log_entry_t* entry = &log_storage.logs[log_storage.nlogs];
        entry->line_no = log_storage.nlogs;
        entry->info = 0;
        entry->len = strlen(log);
        entry->time_s = now.tv_sec;
        entry->time_ms = now.tv_nsec / 1000000;
        entry->incr = 0;
        strncpy(entry->line, log, sizeof(entry->line) - 1);
        entry->line[sizeof(entry->line) - 1] = '\0';
        log_storage.nlogs++;
    }
}

static void clear_logs(void)
{
    log_storage.nlogs = 0;
}

static int handler_log(csp_conn_t* conn, csp_packet_t* packet)
{
    s5lab_cmd_t* cmd = (s5lab_cmd_t*)packet->data;
    uint16_t line_start, line_stop;
    
    switch (cmd->type) {
    case 0:
        printf("Get specific line: %u\n", cmd->get_specific_line.line_no);
        line_start = cmd->get_specific_line.line_no;
        line_stop = cmd->get_specific_line.line_no;
        break;
    case 1:
        printf("Get multiple lines: %u to %u\n", cmd->get_multiple_lines.line_start, cmd->get_multiple_lines.line_stop);
        line_start = cmd->get_multiple_lines.line_start;
        line_stop = cmd->get_multiple_lines.line_stop;
        break;
    case 3:
        printf("Get latest line\n");
        line_start = 0;
        line_stop = 0;
        break;
    case 4:
        printf("Get latest n lines: %u\n", cmd->get_latest_n_lines.n_lines);
        if (cmd->get_latest_n_lines.n_lines == 0) {
            printf("Invalid n_lines: 0\n");
            return -1;
        }
        line_start = 0;
        line_stop = cmd->get_latest_n_lines.n_lines - 1;
        break;
    case 5:
        printf("Clear all lines with code: %u\n", cmd->clear_all_lines.code);
        clear_logs();
        return 0;
    default:
        printf("Unknown command type: %u\n", cmd->type);
        return -1;
    }

    for (uint16_t i = line_start; i <= line_stop; i++) {
        if (copy_log((s5lab_rep_get_line_t*)packet->data, i)) {
            // Send the log entry back to the client
            packet->length = sizeof(s5lab_rep_get_line_t)
                            + strlen(((s5lab_rep_get_line_t*)packet->data)->line);
            // printf("    %s\n", ((s5lab_rep_get_line_t*)packet->data)->line);
            if (!csp_send(conn, packet, 0)) {
                printf("Failed to send log for line: %u\n", i);
                csp_buffer_free(packet);
                return -1;
            }
        } else {
            printf("Failed to copy log for line: %u\n", i);
            csp_buffer_free(packet);
            return -1;
        }
        packet = csp_buffer_get(sizeof(s5lab_rep_get_line_t) + SIMDEV_LOG_MAX_LENGTH);
        if (!packet) {
            printf("Failed to allocate packet for line: %u\n", i);
            return -1;
        }
    }

    csp_buffer_free(packet);
    return 0;
}


int main(void)
{
    if (init_csp(DEVICE_NODE_ADDRESS, DEVICE_BUFFER_SIZE, 10) != CSP_ERR_NONE ||
        init_can_iface("vcan0", HOST_NODE_ADDRESS) != CSP_ERR_NONE)
        return -1;

    csp_socket_t* socket = csp_socket(0);
    if (!socket) {
        printf("Failed to create socket\n");
        return -1;
    }

    if (csp_bind(socket, CSP_ANY) != CSP_ERR_NONE) {
        printf("Failed to bind to CSP_ANY\n");
        return -1;
    }

    if (csp_listen(socket, 5) != CSP_ERR_NONE) {
        printf("Failed to listen on socket\n");
        return -1;
    }

    printf("CSP server started on address: %u\n", csp_get_address());

    // test log entries
    add_log("Simulated log #1");
    add_log("Simulated log #2: Say my name.");
    add_log("Simulated log #3: I don't have a damn clue who the hell you are.");
    add_log("Simulated log #4: Yeah, you do.");
    add_log("Simulated log #5: I'm the cook. I'm the one who killed Gus Fring. That's right. Now say my name.");
    add_log("Simulated log #6: You're Heisenberg.");
    add_log("Simulated log #7: YOU'RE GODDAMN RIGHT.");

    while (1) {
        csp_conn_t* conn = csp_accept(socket, CSP_MAX_TIMEOUT);

        if (conn) {
            csp_packet_t* packet = csp_read(conn, CSP_MAX_TIMEOUT);
            if (!packet) {
                printf("Failed to read packet\n");
                csp_close(conn);
                continue;
            }
    
            printf("packet received on port: %u\n", csp_conn_dport(conn));
            switch (csp_conn_dport(conn)) {
            case S5LAB_PORT_LOG:
                handler_log(conn, packet);
                break;
            case S5LAB_PORT_GENERAL:
            case S5LAB_PORT_SCH:
            case S5LAB_PORT_ROUTE:
            case S5LAB_PORT_PAR:
            case S5LAB_PORT_SHELL:
                printf("Received packet on port: %u\n", csp_conn_dport(conn));
                csp_buffer_free(packet);
                break;
            default:
                csp_service_handler(conn, packet);
                break;
            }

            csp_close(conn);
        }
    }

    return 0;
}
