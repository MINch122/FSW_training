/**
 * @file
 *   FTP Application Private Config Definitions
 *
 * This provides default values for configurable items that are internal
 * to this module and do NOT affect the interface(s) of this module.  Changes
 * to items in this file only affect the local module and will be transparent
 * to external entities that are using the public interface(s).
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef FTP_INTERNAL_CFG_H
#define FTP_INTERNAL_CFG_H

#define FTP_PIPE_DEPTH      32

/* File Chunk Size. This value indicate Only real file data. Any kind of header & tail is not included */
#define FTP_MAX_CHUNK_SIZE          128

/* \deprecated Not used */
#define FTP_MAX_CHUNK_DATA_SIZE     (FTP_MAX_CHUNK_SIZE - OS_MAX_PATH_LEN)

/* Delay between chunk */
#define FTP_CHUNK_SLEEP_MS  50


#endif