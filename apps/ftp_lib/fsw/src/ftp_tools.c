/*******************************************************************************************
 * @file ftp_tools.c
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
#include "ftp_tools.h"
#include <stdio.h>
#include <stdarg.h>


void ftp_debug_impl(const char* str, const char* caller, ftp_log_type mode, ...) {

    uint8 color;
    va_list va;
    
    /* Set color code. */
    switch (mode) {
        case FTP_LOG_INFO:
            color = 32; break;
        case FTP_LOG_ERROR:
            color = 31; break;
        default:
            color = 0; break;
    }

    /* Change the output color. */
    printf("\x1b[%um", color);

    /* Put the calling function name. */
    printf("%s: ", caller);

    /* Print the string. */
    va_start(va, mode);
    vprintf(str, va);
    va_end(va);

    /* Reset the color code and flush. */
    printf("\x1b[0m");
    fflush(stdout);

}
