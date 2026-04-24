/**
 * @file s5lab.h
 * @author ryu@yonsei.ac.kr
 * @brief Sapienza S5Lab Transceiver Module driver.
 * @version 0.1
 * @date 2026-03-20
 * 
 * Yonsei ACL, 2026
 */
#ifndef _S5LAB_H_
#define _S5LAB_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "s5lab_config.h"


/**
 * @brief Driver return codes.
 */
typedef enum {
    S5LAB_OK                    =  0,  /* success */
    S5LAB_ERR_INVALID_PARAM     = -1,  /* invalid parameter */
    S5LAB_ERR_NULL_PTR          = -2,  /* null passed to nonnull parameter */
    S5LAB_ERR_REPBUF_TOO_SMALL  = -3,  /* provided reply buffer too small */
    S5LAB_ERR_PAYLOAD_TOO_LARGE = -4,  /* command size exceeds CSP buffer capacity */
    S5LAB_ERR_NOMEM             = -5,  /* malloc failure */

    S5LAB_ERR_CSP_BUFFER        = -6,  /* no CSP buffer available for the command */
    S5LAB_ERR_CONN              = -7,  /* CSP connection failure */
    S5LAB_ERR_SEND              = -10, /* failed to send the command */
    S5LAB_ERR_TIMEOUT           = -11, /* no reply received within timeout */
    S5LAB_ERR_REPLY_SIZE        = -12, /* reply size mismatch */

    S5LAB_ERR_UNKNOWN           = -255,
} s5lab_error_t;


/**
 * @brief Custom CSP ports.
 */
typedef enum {
    S5LAB_PORT_GENERAL  = S5LAB_CONF_CSP_PORT_GENERAL,
    S5LAB_PORT_LOG      = S5LAB_CONF_CSP_PORT_LOG,
    S5LAB_PORT_SCH      = S5LAB_CONF_CSP_PORT_SCH,
    S5LAB_PORT_ROUTE    = S5LAB_CONF_CSP_PORT_ROUTING,
    S5LAB_PORT_PAR      = S5LAB_CONF_CSP_PORT_PARAMETERS,
    S5LAB_PORT_SHELL    = S5LAB_CONF_CSP_PORT_SHELL,
} s5lab_ports_t;


/**
 * @brief Initialize the S5Lab transceiver driver.
 * @return 0 on success, negative on error.
 */
int s5lab_init(void);


/* ════════════════════════════════════════════════════════════════════════
 *  Generic Transactions
 * ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Perform a CSP transaction with the S5Lab transceiver.
 *        Assumes a fixed reply size defined by the caller. Use
 *        s5lab_transaction_unsized() for variable-lengthed replies.
 *        This is a generic transaction function - not intended for userspace.
 * 
 * @param port      CSP destination port. See s5lab_ports_t.
 * @param tx        Command payload.
 * @param tx_len    Length of @a tx.
 * @param rx        Reply buffer.
 * @param rx_len    Size of the expected reply.
 * @param timeout_ms  Transaction timeout in milliseconds.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_SEND: failed to send the command,
 *         S5LAB_ERR_TIMEOUT: no reply received within timeout,
 *         S5LAB_ERR_REPLY_SIZE: reply size mismatch,
 *         S5LAB_ERR_CSP_BUFFER: too large @a tx_len or no CSP buffer available,
 *         S5LAB_ERR_CONN: CSP connection failure.
 */
int s5lab_transaction(uint8_t port,
                      const void* tx,
                      uint16_t tx_len,
                      void* rx,
                      uint16_t rx_len,
                      uint16_t timeout_ms);

/**
 * @brief Perform a CSP transaction with the S5Lab transceiver, allowing
 *        variable reply sizes. Use s5lab_transaction() for fixed-size replies.
 *        This is a generic transaction function - not intended for userspace.
 * 
 * @param port          CSP destination port. See s5lab_ports_t.
 * @param tx            Command payload.
 * @param tx_len        Length of @a tx.
 * @param rx            Reply buffer.
 * @param rx_buf_len    Size of the buffer. Truncates replies larger than this size.
 * @param[out] rx_len_out  Actual size of the reply. Null allowed.
 * @param timeout_ms    Transaction timeout in milliseconds.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_SEND: failed to send the command,
 *         S5LAB_ERR_TIMEOUT: no reply received within timeout,
 *         S5LAB_ERR_CSP_BUFFER: too large @a tx_len or no CSP buffer available,
 *         S5LAB_ERR_CONN: CSP connection failure.
 */
int s5lab_transaction_unsized(uint8_t port,
                              const void* tx,
                              uint16_t tx_len,
                              void* rx,
                              uint16_t rx_buf_len,
                              uint16_t* rx_len,
                              uint16_t timeout_ms);

/**
 * @brief Callback type for handling replies in a multiple-reply transaction.
 * 
 * @param index        Reply index, starting from 0 for the first reply.
 * @param reply_data   Pointer to the reply data (CSP packet data member).
 * @param reply_len    Length of the reply data (CSP packet length member).
 * @param user_data    User-defined data passed to the handler.
 * @return 0 to continue receiving replies, nonzero to abort the transaction.
 */
typedef int (*s5lab_reply_handler_t)(uint16_t index,
                                     void* reply_data,
                                     uint16_t reply_len,
                                     void* user_data);

/**
 * @brief Perform a CSP transaction that expects sequential replies, invoking
 *        @a handler for each reply received. Transaction ends when either:
 *        1) all @a nrep replies have been received,
 *        2) no reply is received within @a timeout_ms or,
 *        3) @a handler returns a nonzero value.
 *
 * @details
 *     - The handler accesses the data section of the reply CSP packet directly.
 *     - The packet is freed right after the handler execution, and must not 
 *         be accessed afterwards.
 *     - Timeout is applied to individual replies, not the overall transaction.
 *         e.g., transaction could last 3*timeout_ms if 3 replies are expected.
 *     - If @a handler returns nonzero, the transaction is aborted and the
 *         this function returns that value.
 *     - Assumes unsized replies. Actual reply size is passed to the handler.
 * 
 * @param port          CSP destination port. See s5lab_ports_t.
 * @param tx            Command payload.
 * @param tx_len        Length of @a tx. 
 * @param timeout_ms    Timeout for each individual reply.
 * @param handler       Callback function invoked for each reply.
 * @param user_data     User-defined data passed to the handler.
 * @return S5LAB_OK: success (all @a nrep replies received),
 *         S5LAB_ERR_SEND: failed to send the command,
 *         S5LAB_ERR_TIMEOUT: no reply received within timeout,
 *         S5LAB_ERR_CSP_BUFFER: too large @a tx_len or no CSP buffer available,
 *         S5LAB_ERR_CONN: CSP connection failure.
 */
int s5lab_transaction_multiple(uint8_t port,
                               int nrep,
                               const void* tx,
                               uint16_t tx_len,
                               uint16_t timeout_ms,
                               s5lab_reply_handler_t handler,
                               void* user_data);

 
/**
 * @brief Set the destination CSP node the transactions will be sent to.
 *        (Host-side only, has no effect on the S5Lab transceiver itself).
 */
void s5lab_csp_set_node(uint8_t node);


/**
 * @brief Set timeout in milliseconds for all subsequent transactions.
 */
void s5lab_csp_set_timeout(uint16_t timeout_ms);


/**
 * @brief Get the currently configured destination CSP node.
 */
uint8_t s5lab_csp_get_node(void);


/**
 * @brief Get the currently configured transaction timeout in milliseconds.
 */
uint16_t s5lab_csp_get_timeout(void);


/**
 * @brief Check if a given command can fit in a CSP packet with the current buffer size.
 * 
 * @param payload_len Length of the command in bytes.
 */
bool s5lab_csp_payload_fits(uint16_t payload_len);


/* ════════════════════════════════════════════════════════════════════════
 *  LOG (port 16)
 * ════════════════════════════════════════════════════════════════════════ */

 /**
  * @brief Get Line reply template.
  */
typedef struct __attribute__((packed)) {
    uint16_t line_no;
    uint8_t  info;
    uint8_t  len;
    uint32_t time_s;
    uint16_t time_ms;
    uint16_t incr;
    char     line[];
} s5lab_rep_get_line_t;


/**
 * @brief Get a specific line in the log.
 * 
 *   FOR SAFETY, THE LINE GETTERS DO NOT NULL-TERMINATE THE REPLY LOGS.
 *   IN CASE THE DEVICE REPLIES WITH NO NULL CHARACTERS, IT IS THE USER'S
 *   RESPONSIBILITY TO ADD ONE IF NEEDED.
 * 
 * @param line_no       Line number to retrieve.
 * @param[out] rep      Reply data buffer.
 * @param rep_size_cap  Size of the reply buffer.
 * @param[out] rep_size Actual size of the reply.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a rep is null,
 *         S5LAB_ERR_REPBUF_TOO_SMALL: reply buffer too small,
 *         otherwise, s5lab_transaction_unsized() return codes.
 */
int s5lab_get_specific_line(uint16_t line_no,
                            s5lab_rep_get_line_t* rep,
                            uint16_t rep_size_cap,
                            uint16_t* rep_size);


/**
 * @brief Get multiple lines from line start to line stop.
 *      Lines are sent sequentially and handled by the @a handler callback.
 *      See s5lab_transaction_multiple() for detailed transaction behavior.
 * 
 *   FOR SAFETY, THE LINE GETTERS DO NOT NULL-TERMINATE THE REPLY LOGS.
 *   IN CASE THE DEVICE REPLIES WITH NO NULL CHARACTERS, IT IS THE USER'S
 *   RESPONSIBILITY TO ADD ONE IF NEEDED.
 * 
 * @param line_start    First line number (inclusive).
 * @param line_stop     Last line number (inclusive?).
 * @param handler       Callback function invoked for each reply.
 * @param user_data     User-defined data passed to the handler.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a handler is null,
 *         S5LAB_ERR_INVALID_PARAM: @a line_stop < @a line_start,
 *         otherwise, s5lab_transaction_multiple() return codes
 *         or the handler's return value.
 */
int s5lab_get_multiple_lines(uint16_t line_start,
                             uint16_t line_stop,
                             s5lab_reply_handler_t handler,
                             void* user_data);

/**
 * @brief Retrieve the latest log line.
 * 
 *   FOR SAFETY, THE LINE GETTERS DO NOT NULL-TERMINATE THE REPLY LOGS.
 *   IN CASE THE DEVICE REPLIES WITH NO NULL CHARACTERS, IT IS THE USER'S
 *   RESPONSIBILITY TO ADD ONE IF NEEDED.
 * 
 * @param[out] rep      Reply data buffer.
 * @param rep_size_cap  Size of the reply buffer.
 * @param[out] rep_size Actual size of the reply.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a rep is null,
 *         S5LAB_ERR_REPBUF_TOO_SMALL: reply buffer too small,
 *         otherwise, s5lab_transaction_unsized() return codes.
 */
int s5lab_get_latest_line(s5lab_rep_get_line_t* rep,
                          uint16_t rep_size_cap,
                          uint16_t* rep_size);

/**
 * @brief Retrieve the latest N log lines.
 * 
 *   FOR SAFETY, THE LINE GETTERS DO NOT NULL-TERMINATE THE REPLY LOGS.
 *   IN CASE THE DEVICE REPLIES WITH NO NULL CHARACTERS, IT IS THE USER'S
 *   RESPONSIBILITY TO ADD ONE IF NEEDED.
 * 
 * @param n_lines       Number of lines to retrieve.
 * @param handler       Callback function invoked for each reply.
 * @param user_data     User-defined data passed to the handler.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a handler is null,
 *         otherwise, s5lab_transaction_multiple() return codes
 *         or the handler's return value.
 */
int s5lab_get_latest_n_lines(uint16_t n_lines,
                             s5lab_reply_handler_t handler,
                             void* user_data);

/**
 * @brief Clear all log lines.
 * @return S5LAB_OK: success,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_clear_all_lines(void);


/* ════════════════════════════════════════════════════════════════════════
 *  SCHEDULE (port 17)
 * ════════════════════════════════════════════════════════════════════════ */


typedef struct __attribute__((packed)) {
    uint8_t  sign;
    uint8_t  entry_index;
    uint64_t time_ms; 
    uint32_t repeat_every_ms;
    uint8_t  repeat_max;
    uint8_t  command[];
} s5lab_rep_get_single_entry_t;

/**
 * @brief Get one specific schedule entry
 * @param entry_index   Index of the entry to retrieve.
 * @param[out] rep      Reply data buffer.
 * @param rep_buf_size  Size of the reply buffer.
 * @param[out] rep_size Actual size of the reply.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: null pointer passed,
 *         S5LAB_ERR_REPBUF_TOO_SMALL: reply buffer too small,
 *         otherwise, s5lab_transaction_unsized() return codes.
 */
int s5lab_get_single_entry(uint16_t entry_index,
                           s5lab_rep_get_single_entry_t* rep,
                           uint16_t rep_buf_size,
                           uint16_t* rep_size);

/**
 * @brief Retrieve multiple schedule entries.
 *        Schedule entries are s5lab_rep_get_single_entry_t replies sent
 *        sequentially. The handler callback is invoked for each entry received.
 *        See s5lab_transaction_multiple() for detailed transaction behavior.
 *        
 *      1) The return value of the handler matters. If the handler returns
 *              non-zero, the transaction is aborted and this function returns
 *              with the handler's return value.
 * 
 *      2) The handler is responsible for validating the reply size and content.
 *
 * @param start_index  First entry index (inclusive).
 * @param end_index    Last entry index (inclusive).
 * @param handler      Callback function to handle each retrieved entry.
 * @param user_data    User data pointer passed to the handler. Null allowed.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a handler is null,
 *         S5LAB_ERR_INVALID_PARAM: @a end_index < @a start_index,
 *         otherwise, s5lab_transaction() return codes or the handler's return value.
 */
int s5lab_get_multiple_entries(uint16_t start_index,
                               uint16_t end_index,
                               s5lab_reply_handler_t handler,
                               void* user_data);

/**
 * @brief Add a new schedule entry.
 * @param time_ms           Dunno
 * @param repeat_every_ms   Dunno
 * @param repeat_max        Dunno
 * @param command           Command payload.
 * @param command_len       Length of the @a command.
 * @param[out] error_code   Returned error code is stored here. Null allowed.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a command is null,
 *         S5LAB_ERR_PAYLOAD_TOO_LARGE: @a command to long to fit in a packet,
 *         S5LAB_ERR_NOMEM: malloc failure,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_add_entry(uint64_t time_ms,
                    uint32_t repeat_every_ms,
                    int8_t repeat_max,
                    const uint8_t* command,
                    uint16_t command_len,
                    uint8_t* error_code);


/**
 * @brief Remove one entry without shifting
 * @param entry_index  Index of the entry to remove.
 * @param[out] result  Remove result. Null allowed.
 * @return S5LAB_OK: success,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_remove_entry(uint8_t entry_index, uint8_t* result);


/**
 * @brief Return number of active entries
 * @param[out] slots  Number of used slots returned here.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a slots is null,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_get_used_slots(uint8_t* slots);


/* ════════════════════════════════════════════════════════════════════════
 *  ROUTING (port 18)
 * ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Apply the default routing table (0/0 CAN).
 * @param[out] result  Returned result byte.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a result is null,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_set_route_default(uint8_t* result);


/**
 * @brief Reset the routing table to 1/5 CAN.
 * @param[out] result  Returned result byte.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a result is null,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_reset_route(uint8_t* result);

/**
 * @brief Load the routing table from FRAM.
 * @param[out] result  Returned result byte.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a result is null,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_load_route(uint8_t* result);

/**
 * @brief Save the current routing table to FRAM and return it.
 *        DOES NOT ENSURE NULL-TERMINATION OF THE ROUTE STRING.
 *        The caller must handle it properly.
 * @param[out] saved_route Route string reply buffer.
 * @param route_buf_size  Size of the route buffer in bytes.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a saved_route is null,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_save_route(char* saved_route, uint16_t route_buf_size);

/**
 * @brief Send the FRAM route via CSP.
 * @param[out] result  Returned result byte.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a result is null,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_send_route(uint8_t* result);


/**
 * @brief Set the routing table from a CSP-style CIDR rule string.
 * @param route        Route to set.
 * @param[out] result  Returned result byte.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a route or @a result is null,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_set_route(const char* route,
                    uint8_t* result);


/* ════════════════════════════════════════════════════════════════════════
 *  PARAMETERS (port 19)
 * ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Read a parameter value.
 * @param table  Parameter table index.
 * @param param  Parameter index within the table.
 * @param[out] value  Returned parameter value.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a value is null,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_par_get(uint8_t table, uint8_t param, int32_t* value);


/**
 * @brief Set a parameter value.
 * @param table   Parameter table index.
 * @param param   Parameter index within the table.
 * @param value   Value to set.
 * @param[out] result  Returned result byte. Null allowed.
 * @return S5LAB_OK: success,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_par_set(uint8_t table, uint8_t param,
                  int32_t value, uint8_t* result);


/**
 * @brief Restore all parameters in the table to defaults.
 * @param table   Parameter table index.
 * @param[out] result  Returned result byte. Null allowed.
 * @return S5LAB_OK: success,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_par_defaults(uint8_t table, uint8_t* result);


/**
 * @brief Save current table parameters to nonvolatile storage.
 * @param table   Parameter table index.
 * @param[out] result  Returned result byte. Null allowed.
 * @return S5LAB_OK: success,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_par_save(uint8_t table, uint8_t* result);


/**
 * @brief Restore table parameters from saved nonvolatile copy.
 * @param table   Parameter table index.
 * @param[out] result  Returned result byte. Null allowed.
 * @return S5LAB_OK: success,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_par_restore(uint8_t table, uint8_t* result);


/**
 * @brief Load the built-in parameter set for the table.
 * @param table   Parameter table index.
 * @param[out] result  Returned result byte. Null allowed.
 * @return S5LAB_OK: success,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_par_load(uint8_t table, uint8_t* result);



/**
 * @brief Enable or disable out-of-bounds parameter changes for a table.
 * @param table   Parameter table index.
 * @param enable  1 to enable, 0 to disable.
 * @param[out] result  Returned result byte. Null allowed.
 * @return S5LAB_OK: success,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_par_set_oob(uint8_t table, uint8_t enable, uint8_t* result);


/* ════════════════════════════════════════════════════════════════════════
 *  REMOTE TERMINAL (port 20)
 * ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Emulate typing a command into the terminal, to issue commands.
 * @param cmd     Null-terminated command string.
 * @param[out] result  Returned result byte. Null allowed.
 * @return S5LAB_OK: success,
 *         S5LAB_ERR_NULL_PTR: @a cmd is null,
 *         S5LAB_ERR_PAYLOAD_TOO_LARGE: @a cmd too long to fit in a packet,
 *         otherwise, s5lab_transaction() return codes.
 */
int s5lab_send_command(const char* cmd, uint8_t* result);


#endif
