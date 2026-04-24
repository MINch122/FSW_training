/**
 * @file s5lab.c
 * @author ryu@yonsei.ac.kr
 * @brief Sapienza S5Lab Transceiver Module driver.
 * @version 0.1
 * @date 2026-03-20
 * 
 * Yonsei ACL, 2026
 */
#define _GNU_SOURCE   // strnlen
#include "s5lab.h"
#include "s5lab_types_internal.h"
#include "csp/csp_endian.h"

#include <string.h>
#include <stdlib.h>

int s5lab_init(void)
{
    // Nothing to implement currently.
    return S5LAB_OK;
}


int s5lab_get_specific_line(uint16_t line_no,
                            s5lab_rep_get_line_t* rep,
                            uint16_t rep_size_cap,
                            uint16_t* rep_size)
{
    s5lab_cmd_get_specific_line_t cmd;

    if (!rep)
        return S5LAB_ERR_NULL_PTR;

    if (rep_size_cap < sizeof(*rep))
        return S5LAB_ERR_REPBUF_TOO_SMALL;

    cmd.line_no = line_no;
    cmd.type = 0x00;

    return s5lab_transaction_unsized(S5LAB_PORT_LOG,
                                     &cmd, sizeof(cmd),
                                     rep, rep_size_cap,
                                     rep_size, s5lab_csp_get_timeout());
}


int s5lab_get_multiple_lines(uint16_t line_start,
                             uint16_t line_stop,
                             s5lab_reply_handler_t handler,
                             void* user_data)
{
    s5lab_cmd_get_multiple_lines_t cmd;

    if (!handler)
        return S5LAB_ERR_NULL_PTR;

    if (line_stop < line_start)
        return S5LAB_ERR_INVALID_PARAM;

    cmd.type = 0x01;
    cmd.line_start = line_start;
    cmd.line_stop = line_stop;

    return s5lab_transaction_multiple(S5LAB_PORT_LOG,
                                      line_stop - line_start + 1,
                                      &cmd, sizeof(cmd),
                                      s5lab_csp_get_timeout(),
                                      handler, user_data);
}


int s5lab_get_latest_line(s5lab_rep_get_line_t* rep,
                          uint16_t rep_size_cap,
                          uint16_t* rep_size)
{
    s5lab_cmd_get_latest_line_t cmd = { .type = 0x03 };

    if (!rep)
        return S5LAB_ERR_NULL_PTR;

    if (rep_size_cap < sizeof(*rep))
        return S5LAB_ERR_REPBUF_TOO_SMALL;

    return s5lab_transaction_unsized(S5LAB_PORT_LOG,
                                     &cmd, sizeof(cmd),
                                     rep, rep_size_cap,
                                     rep_size, s5lab_csp_get_timeout());
}


int s5lab_get_latest_n_lines(uint16_t n_lines,
                             s5lab_reply_handler_t handler,
                             void* user_data)
{
    s5lab_cmd_get_latest_n_lines_t cmd;

    if (!handler)
        return S5LAB_ERR_NULL_PTR;

    cmd.type = 0x04;
    cmd.n_lines = n_lines;

    return s5lab_transaction_multiple(S5LAB_PORT_LOG,
                                      n_lines,
                                      &cmd, sizeof(cmd),
                                      s5lab_csp_get_timeout(),
                                      handler, user_data);
}


int s5lab_clear_all_lines(void)
{
    s5lab_cmd_clear_all_lines_t cmd = { .type = 0x05, .code = 0xB00B };
    return s5lab_transaction(S5LAB_PORT_LOG,
                             &cmd, sizeof(cmd),
                             NULL, 0, s5lab_csp_get_timeout());
}


int s5lab_get_single_entry(uint16_t entry_index,
                           s5lab_rep_get_single_entry_t* rep,
                           uint16_t rep_buf_size,
                           uint16_t* rep_size)
{
    s5lab_cmd_get_single_entry_t cmd;

    if (!rep)
        return S5LAB_ERR_NULL_PTR;

    if (rep_buf_size < sizeof(*rep))
        return S5LAB_ERR_REPBUF_TOO_SMALL;

    cmd.type = 0x00;
    cmd.entry_index = entry_index;

    return s5lab_transaction_unsized(S5LAB_PORT_SCH,
                                     &cmd, sizeof(cmd),
                                     rep, rep_buf_size, rep_size, s5lab_csp_get_timeout());
}


int s5lab_get_multiple_entries(uint16_t start_index,
                               uint16_t end_index,
                               s5lab_reply_handler_t handler,
                               void* user_data)
{
    s5lab_cmd_get_multiple_entries_t cmd;

    if (!handler)
        return S5LAB_ERR_NULL_PTR;
    
    if (end_index < start_index)
        return S5LAB_ERR_INVALID_PARAM;

    cmd.type = 0x01;
    cmd.start_index = start_index;
    cmd.end_index = end_index;

    return s5lab_transaction_multiple(S5LAB_PORT_SCH,
                                      end_index - start_index + 1,
                                      &cmd, sizeof(cmd),
                                      s5lab_csp_get_timeout(),
                                      handler, user_data);
}


int s5lab_add_entry(uint64_t time_ms,
                    uint32_t repeat_every_ms,
                    int8_t repeat_max,
                    const uint8_t* command,
                    uint16_t command_len,
                    uint8_t* error_code)
{
    s5lab_cmd_add_entry_t* cmd;
    s5lab_rep_add_entry_t rep;

    if (!command)
        return S5LAB_ERR_NULL_PTR;  

    if (command_len > UINT16_MAX - sizeof(*cmd) ||
        !s5lab_csp_payload_fits(sizeof(*cmd) + command_len))
        return S5LAB_ERR_PAYLOAD_TOO_LARGE;

    cmd = malloc(sizeof(*cmd) + command_len);
    if (!cmd)
        return S5LAB_ERR_NOMEM;

    cmd->type = 0x03;
    cmd->time_ms = time_ms;
    cmd->repeat_every_ms = repeat_every_ms;
    cmd->repeat_max = repeat_max;
    memcpy(cmd->command, command, command_len); // TODO: Is this command a string? Should be null-terminated if so?

    int ret = s5lab_transaction(S5LAB_PORT_SCH,
                                cmd, sizeof(*cmd) + command_len,
                                &rep, sizeof(rep), s5lab_csp_get_timeout());

    if (ret == S5LAB_OK && error_code) {
        *error_code = rep.error_code;
    }

    free(cmd);
    return ret;
}


int s5lab_remove_entry(uint8_t entry_index, uint8_t* result)
{
    s5lab_cmd_remove_entry_t cmd = {0}; 
    cmd.type = 0x04;
    cmd.entry_index = entry_index;
    s5lab_rep_remove_entry_t rep;

    int ret = s5lab_transaction(S5LAB_PORT_SCH,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());

    if (ret == S5LAB_OK && result) {
        *result = rep.result;
    }
    return ret;
}


int s5lab_get_used_slots(uint8_t* slots)
{
    s5lab_cmd_get_used_slots_t cmd = { .type = 0x05 };
    s5lab_rep_get_used_slots_t rep;

    if (!slots)
        return S5LAB_ERR_NULL_PTR;

    int ret = s5lab_transaction(S5LAB_PORT_SCH,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());
    if (ret == S5LAB_OK) {
        *slots = rep.slots;
    }
    return ret;
}


static int s5lab_route_control(uint8_t control_type, uint8_t* result)
{
    uint8_t reply;

    if (!result)
        return S5LAB_ERR_NULL_PTR;

    int ret = s5lab_transaction(S5LAB_PORT_ROUTE,
                                &control_type, sizeof(control_type),
                                &reply, sizeof(reply), s5lab_csp_get_timeout());
    if (ret == S5LAB_OK) {
        *result = reply;
    }
    return ret;
}


int s5lab_set_route_default(uint8_t* result)
{
    return s5lab_route_control(0x00, result);
}


int s5lab_reset_route(uint8_t* result)
{
    return s5lab_route_control(0x01, result);
}


int s5lab_load_route(uint8_t* result)
{
    return s5lab_route_control(0x02, result);
}


int s5lab_save_route(char* saved_route, uint16_t route_buf_size)
{
    s5lab_cmd_save_route_t cmd = { .type = 0x03 };

    if (!saved_route)
        return S5LAB_ERR_NULL_PTR;

    return s5lab_transaction_unsized(S5LAB_PORT_ROUTE,
                                     &cmd, sizeof(cmd),
                                     saved_route, route_buf_size, NULL, s5lab_csp_get_timeout());
}


int s5lab_send_route(uint8_t* result)
{
    return s5lab_route_control(0x04, result);
}


int s5lab_set_route(const char* route,
                    uint8_t* result)
{
    s5lab_cmd_set_route_t* cmd;
    s5lab_rep_set_route_t rep;

    if (!route || !result)
        return S5LAB_ERR_NULL_PTR;

    size_t route_len = strnlen(route, UINT16_MAX - sizeof(*cmd) - 1);
    if (!s5lab_csp_payload_fits(sizeof(*cmd) + route_len + 1))
         return S5LAB_ERR_PAYLOAD_TOO_LARGE;

    cmd = malloc(sizeof(*cmd) + route_len + 1);
    if (!cmd)
        return S5LAB_ERR_NOMEM;

    cmd->type = 0x05;
    memcpy(cmd->route, route, route_len); 
    cmd->route[route_len] = '\0';

    int ret = s5lab_transaction(S5LAB_PORT_ROUTE,
                                cmd, sizeof(*cmd) + route_len + 1,
                                &rep, sizeof(rep), s5lab_csp_get_timeout());
    if (ret == S5LAB_OK) {
        *result = rep.result;
    }
    free(cmd);
    return ret;
}


int s5lab_par_get(uint8_t table,
                  uint8_t param,
                  int32_t* value)
{
    s5lab_cmd_par_get_t cmd = { .type = 0x00, .table = table, .param = param };
    s5lab_rep_par_get_t rep;

    if (!value)
        return S5LAB_ERR_NULL_PTR;

    int ret = s5lab_transaction(S5LAB_PORT_PAR,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());
    if (ret == S5LAB_OK) {
        *value = csp_ntoh32(rep.value);
    }
    return ret;
}


int s5lab_par_set(uint8_t table,
                  uint8_t param,
                  int32_t value,
                  uint8_t* result)
{
    s5lab_cmd_par_set_t cmd = { .type = 0x01, .table = table, .param = param, .value = csp_hton32(value) };
    s5lab_rep_par_set_t rep;

    int ret = s5lab_transaction(S5LAB_PORT_PAR,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());

    if (ret == S5LAB_OK && result) {
        *result = rep.result;
    }
    return ret;
}


int s5lab_par_defaults(uint8_t table, uint8_t* result)
{
    s5lab_cmd_par_defaults_t cmd = { .type = 0x03, .table = table };
    s5lab_rep_par_defaults_t rep;

    int ret = s5lab_transaction(S5LAB_PORT_PAR,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());

    if (ret == S5LAB_OK && result) {
        *result = rep.result;
    }
    return ret;
}


int s5lab_par_save(uint8_t table, uint8_t* result)
{
    s5lab_cmd_par_save_t cmd = { .type = 0x04, .table = table };
    s5lab_rep_par_save_t rep;

    int ret = s5lab_transaction(S5LAB_PORT_PAR,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());

    if (ret == S5LAB_OK && result) {
        *result = rep.result;
    }
    return ret;
}


int s5lab_par_restore(uint8_t table, uint8_t* result)
{
    s5lab_cmd_par_restore_t cmd = { .type = 0x05, .table = table };
    s5lab_rep_par_restore_t rep;

    int ret = s5lab_transaction(S5LAB_PORT_PAR,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());

    if (ret == S5LAB_OK && result) {
        *result = rep.result;
    }
    return ret;
}


int s5lab_par_load(uint8_t table, uint8_t* result)
{
    s5lab_cmd_par_load_t cmd = { .type = 0x06, .table = table };
    s5lab_rep_par_load_t rep;

    int ret = s5lab_transaction(S5LAB_PORT_PAR,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());
    if (ret == S5LAB_OK && result) {
        *result = rep.result;
    }
    return ret;
}


int s5lab_par_set_oob(uint8_t table, uint8_t enable, uint8_t* result)
{
    s5lab_cmd_par_set_oob_t cmd = { .type = 0x07, .table = table, .enable = enable };
    s5lab_rep_par_set_oob_t rep;

    int ret = s5lab_transaction(S5LAB_PORT_PAR,
                                &cmd, sizeof(cmd),
                                &rep, sizeof(rep), s5lab_csp_get_timeout());
    if (ret == S5LAB_OK && result) {
        *result = rep.result;
    }
    return ret;
}


int s5lab_send_command(const char* cmd_str, uint8_t* result)
{
    s5lab_rep_send_command_t rep;

    if (!cmd_str)
        return S5LAB_ERR_NULL_PTR;

    size_t cmd_len = strnlen(cmd_str, UINT16_MAX - 1);
    if (cmd_len == UINT16_MAX - 1 ||
        !s5lab_csp_payload_fits(cmd_len + 1))
        return S5LAB_ERR_PAYLOAD_TOO_LARGE;

    int ret = s5lab_transaction(S5LAB_PORT_SHELL,
                                cmd_str, cmd_len + 1,
                                &rep, sizeof(rep), s5lab_csp_get_timeout());

    if (ret == S5LAB_OK && result) {
        *result = rep.result;
    }
    return ret;
}
