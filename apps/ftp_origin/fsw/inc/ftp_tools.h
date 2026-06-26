/*******************************************************************************************
 * @file ftp_tools.h
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief Miscellaneous debugging tools.
 * 
 * @version 1.0
 * 
 * @date 2022-03-22
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#ifndef _FTP_TOOLS_H_
#define _FTP_TOOLS_H_

#include "cfe.h"
#include "io_csp_ftp_config.h"

typedef enum {
    FTP_LOG_INFO        = 1,
    FTP_LOG_ERROR       = 2,
    FTP_LOG_NORMAL      = 3,
} ftp_log_type;

#define IO_CSP_FTP_DEBUG
#ifdef IO_CSP_FTP_DEBUG
#define ftp_debug(str, mode, ...)         ftp_debug_impl(str, __FUNCTION__, mode, ##__VA_ARGS__)
#define ftp_debug_normal(str, ...)        ftp_debug_impl(str, __FUNCTION__, FTP_LOG_NORMAL, ##__VA_ARGS__)
#define ftp_debug_error(str, ...)         ftp_debug_impl(str, __FUNCTION__, FTP_LOG_ERROR, ##__VA_ARGS__)
#define ftp_debug_info(str, ...)          ftp_debug_impl(str, __FUNCTION__, FTP_LOG_INFO, ##__VA_ARGS__)
#else
#define ftp_debug(str, mode, ...)         do {} while (0)
#define ftp_debug_normal(str, ...)        do {} while (0)
#define ftp_debug_error(str, ...)         do {} while (0)
#define ftp_debug_info(str, ...)          do {} while (0)
#endif

void ftp_debug_impl(const char* str, const char* caller, ftp_log_type mode, ...);

#endif