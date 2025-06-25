/**
 * @file
 *   Specification for the CFE Event Services (CFE_SB) command function codes
 *
 * @note
 *   This file should be strictly limited to the command/function code (CC)
 *   macro definitions.  Other definitions such as enums, typedefs, or other
 *   macros should be placed in the msgdefs.h or msg.h files.
 */

#ifndef CFE_SRL_FCNCODES_H
#define CFE_SRL_FCNCODES_H

#define CFE_SRL_NOOP_CC                     0
#define CFE_SRL_RESET_COUNTERS_CC           1

#define CFE_SRL_RESET_HANDLE_COUNTERS_CC    2
#define CFE_SRL_GET_HANDLE_STATUS_CC        3
#define CFE_SRL_INIT_HANDLE_CC              4
#define CFE_SRL_CLOSE_HANDLE_CC             5

#endif /* CFE_SRL_FCNCODES_H */