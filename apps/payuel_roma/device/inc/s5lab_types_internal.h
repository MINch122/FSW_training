/**
 * @file s5lab_types_internal.h
 * @author ryu@yonsei.ac.kr
 * @brief Additional command-reply structures not needed by users.
 * @version 0.1
 * @date 2026-03-20
 * 
 * Yonsei ACL, 2026
 */
#ifndef _S5LAB_TYPES_INTERNAL_H_
#define _S5LAB_TYPES_INTERNAL_H_

#include <stdint.h>


/* ════════════════════════════════════════════════════════════════════════
 *  LOG (port 16)
 * ════════════════════════════════════════════════════════════════════════ */

typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint16_t line_no;
} s5lab_cmd_get_specific_line_t;

typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint16_t line_start;
    uint16_t line_stop;
} s5lab_cmd_get_multiple_lines_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
} s5lab_cmd_get_latest_line_t;

typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint16_t n_lines;
} s5lab_cmd_get_latest_n_lines_t;

typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint16_t code;
} s5lab_cmd_clear_all_lines_t;


/* ════════════════════════════════════════════════════════════════════════
 *  SCHEDULE (port 17)
 * ════════════════════════════════════════════════════════════════════════ */

typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint16_t entry_index;
} s5lab_cmd_get_single_entry_t;


typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint16_t start_index;
    uint16_t end_index;
} s5lab_cmd_get_multiple_entries_t;


typedef struct __attribute__((packed)) {
    uint8_t  type;
    uint64_t time_ms;
    uint32_t repeat_every_ms;
    int8_t   repeat_max;
    uint8_t  command[];
} s5lab_cmd_add_entry_t;

typedef struct __attribute__((packed)) {
    uint8_t error_code;
} s5lab_rep_add_entry_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t entry_index;
} s5lab_cmd_remove_entry_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_remove_entry_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
} s5lab_cmd_get_used_slots_t;

typedef struct __attribute__((packed)) {
    uint8_t slots;
} s5lab_rep_get_used_slots_t;

/* ════════════════════════════════════════════════════════════════════════
 *  ROUTING (port 18)
 * ════════════════════════════════════════════════════════════════════════ */

typedef struct __attribute__((packed)) {
    uint8_t type;
} s5lab_cmd_set_route_default_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_set_route_default_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
} s5lab_cmd_reset_route_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_reset_route_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
} s5lab_cmd_load_route_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_load_route_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
} s5lab_cmd_save_route_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
} s5lab_cmd_send_route_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_send_route_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    char    route[];
} s5lab_cmd_set_route_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_set_route_t;

/* ════════════════════════════════════════════════════════════════════════
 *  PARAMETERS (port 19)
 * ════════════════════════════════════════════════════════════════════════ */

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t table;
    uint8_t param;
} s5lab_cmd_par_get_t;

typedef struct __attribute__((packed)) {
    int32_t value;
} s5lab_rep_par_get_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t table;
    uint8_t param;
    int32_t value;
} s5lab_cmd_par_set_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_par_set_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t table;
} s5lab_cmd_par_defaults_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_par_defaults_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t table;
} s5lab_cmd_par_save_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_par_save_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t table;
} s5lab_cmd_par_restore_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_par_restore_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t table;
} s5lab_cmd_par_load_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_par_load_t;


typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t table;
    uint8_t enable;
} s5lab_cmd_par_set_oob_t;

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_par_set_oob_t;


/* ════════════════════════════════════════════════════════════════════════
 *  REMOTE TERMINAL (port 20)
 * ════════════════════════════════════════════════════════════════════════ */

// No s5lab_cmd_send_command_t. The payload is just a command string of variable length.

typedef struct __attribute__((packed)) {
    uint8_t result;
} s5lab_rep_send_command_t;


#endif
