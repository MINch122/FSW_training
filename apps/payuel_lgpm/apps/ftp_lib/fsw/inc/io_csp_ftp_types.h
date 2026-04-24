/*******************************************************************************************
 * @file io_csp_ftp_types.c
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief FTP custom data types.
 * 
 * @version 2.0
 * 
 * @date 2023-01-11
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#ifndef _IO_CSP_FTP_TYPEDEFS_H_
#define _IO_CSP_FTP_TYPEDEFS_H_


#include "cfe.h"
#include "io_csp_ftp_config.h"

#include <stdio.h>
#include <dirent.h>

#include <csp/csp_types.h>
#include <gs/ftp/types.h>
#define GS_FTP_INTERNAL_USE 1
#include <gs/ftp/internal/types.h>
// #undef GS_FTP_INTERNAL_USE


typedef enum __attribute__((packed)) {
    IO_CSP_FTP_IDLE         = 0,
    IO_CSP_FTP_UPLOAD       = 1,
    IO_CSP_FTP_DOWNLOAD     = 2,
} io_csp_ftp_trans_direction;


typedef enum __attribute__((packed)) {
    IO_CSP_FTP_RET_OK           = 0,
    IO_CSP_FTP_RET_ERROR        = 1,
    IO_CSP_FTP_RET_NULL         = 2,
    IO_CSP_FTP_RET_INVAL        = 3,
    IO_CSP_FTP_RET_RANGE        = 4,

    IO_CSP_FTP_RET_STATUS       = 10,

    IO_CSP_FTP_RET_SEND         = 20,
    IO_CSP_FTP_RET_READ         = 21,
    IO_CSP_FTP_RET_TRANS        = 21,

    IO_CSP_FTP_RET_NOBUF        = 30,
    IO_CSP_FTP_RET_CHUNK_WRITE  = 31,
    IO_CSP_FTP_RET_CHUNK_READ   = 32,
} io_csp_ftp_ret;


typedef struct {
    FILE*   fp;
    FILE*   mapfp;
    DIR*    dp;
    char    filename[GS_FTP_PATH_LENGTH];
    char*   mapbuf;
    uint32_t  size;
    uint32_t  chunks;
} io_csp_ftp_filespec;


typedef struct {
    unsigned pos;
    uint8_t type[IO_CSP_FTP_RETCODE_LEN];
    uint8_t ret[IO_CSP_FTP_RETCODE_LEN];
} io_csp_ftp_retcodes;


typedef struct {
    io_csp_ftp_filespec     filespec;
    ftp_upload_request_t    upspec;
    ftp_download_request_t  downspec;
    csp_conn_t*             conn;
    uint32_t                timeout;
    gs_ftp_backend_type_t   backend;
    io_csp_ftp_trans_direction  transfer_status;
    io_csp_ftp_retcodes     retcodes;
} io_csp_ftp_transfer_spec;


typedef enum __attribute__ ((packed)) {
    FTP_MIMAN_RETCODE_REQUEST           = 100,
    FTP_MIMAN_RETCODE_REPLY             = 101,
    FTP_MIMAN_SET_TIMEOUT_REQUEST       = 102,
    FTP_MIMAN_SET_TIMEOUT_REPLY         = 103,
    FTP_MIMAN_CHECKSUM_REQUEST          = 104,
    FTP_MIMAN_CHECKSUM_REPLY            = 105,
    FTP_MIMAN_FSPEC_CLEANUP_REQUEST     = 106,
    FTP_MIMAN_FSPEC_CLEANUP_REPLY       = 107,
    FTP_MIMAN_READ_CHUNK_REQUEST        = 108,
    FTP_MIMAN_READ_CHUNK_REPLY          = 109,
    FTP_MIMAN_WRITE_CHUNK_REQUEST       = 110,
    FTP_MIMAN_WRITE_CHUNK_REPLY         = 111,
    // FTP_MIMAN_SERVER_RESTART_REQUEST    = 112,
    // FTP_MIMAN_SERVER_RESTART_REPLY      = 113,
    FTP_MIMAN_IGNORE_LENGTH_CHECK_REQUEST = 114,
    FTP_MIMAN_IGNORE_LENGTH_CHECK_REPLY = 115,
    FTP_MIMAN_KILL_ME_REQUEST           = 120,
    FTP_MIMAN_KILL_ME_REPLY             = 121,
    FTP_MIMAN_SYSCMD_REQUEST           = 122,
    FTP_MIMAN_SYSCMD_REPLY             = 123,
    FTP_MIMAN_REPLY_UNKNOWN             = 200,
} ftp_miman_type;


typedef enum __attribute__ ((packed)) {
    MM_FTP_RET_PKT_LEN          = 100,
    MM_FTP_RET_RETCODE_OFLOW    = 103,
    MM_FTP_RET_MAGIC            = 104,
    MM_FTP_RET_KILL_FAIL        = 105,
} ftp_miman_return_t;


typedef struct __attribute__ ((packed)) {
    uint8_t ret;    /* retcode for THIS operation. */
    uint8_t type[IO_CSP_FTP_RETCODE_LEN];
    uint8_t retcode[IO_CSP_FTP_RETCODE_LEN];
} ftp_miman_retcode_reply_t;


typedef struct __attribute__ ((packed)) {
    uint32_t timeout;
} ftp_miman_timeout_request_t;

typedef struct __attribute__ ((packed)) {
    uint32_t curr_timeout;
} ftp_miman_timeout_reply_t;



typedef struct __attribute__ ((packed)) {
    char path[GS_FTP_PATH_LENGTH];
} ftp_miman_cksum_request_t;

typedef struct __attribute__ ((packed)) {
    uint8_t ret;
    uint32_t crc32;
} ftp_miman_cksum_reply_t;



typedef struct __attribute__ ((packed)) {
    uint8_t ret;
} ftp_miman_fspec_cleanup_reply_t;



typedef struct __attribute__ ((packed)) {
    uint32_t size;
    uint32_t offset;
    uint16_t chunk_size;
    char path[GS_FTP_PATH_LENGTH];
} ftp_miman_read_chunk_request_t;

typedef struct __attribute__ ((packed)) {
    uint8_t ret;
    uint8_t entry;
    uint8_t bytes[GS_FTP_MAX_CHUNK_SIZE];
} ftp_miman_read_chunk_reply_t;




typedef struct __attribute__ ((packed)) {
    uint32_t offset;
    uint16_t size;
    char path[GS_FTP_PATH_LENGTH];
    uint8_t bytes[GS_FTP_MAX_CHUNK_SIZE];
} ftp_miman_write_chunk_request_t;

typedef struct __attribute__ ((packed)) {
    uint8_t ret;
    int err;
} ftp_miman_write_chunk_reply_t;



typedef struct __attribute__ ((packed)) {
    uint8_t port;
    uint32_t timeout;
} ftp_miman_server_restart_request_t;

typedef struct __attribute__ ((packed)) {
    uint8_t ret;
} ftp_miman_server_restart_reply_t;



typedef struct __attribute__ ((packed)) {
    uint32_t magic;
} ftp_miman_killme_request_t;

typedef struct __attribute__ ((packed)) {
    uint8_t ret;
} ftp_miman_killme_reply_t;



typedef struct __attribute__ ((packed)) {
    char cmd[128];
    char stdout_dump[GS_FTP_PATH_LENGTH];
} ftp_miman_system_cmd_request_t;

typedef struct __attribute__ ((packed)) {
    uint8_t ret;
    uint8_t dumpret;
    int sysret;
} ftp_miman_system_cmd_reply_t;



typedef struct __attribute__ ((packed)) {

    uint8_t type;

    union {
        ftp_miman_retcode_reply_t retcoderep;

        ftp_miman_timeout_request_t timeout;
        ftp_miman_timeout_reply_t timeoutrep;

        ftp_miman_cksum_request_t cksum;
        ftp_miman_cksum_reply_t cksumrep;

        ftp_miman_fspec_cleanup_reply_t cleanuprep;

        ftp_miman_read_chunk_request_t read;
        ftp_miman_read_chunk_reply_t readrep;

        ftp_miman_write_chunk_request_t write;
        ftp_miman_write_chunk_reply_t writerep;

        ftp_miman_server_restart_request_t restart;
        ftp_miman_server_restart_reply_t restartrep;

        ftp_miman_killme_request_t killme;
        ftp_miman_killme_reply_t killmerep;

        ftp_miman_system_cmd_request_t syscmd;
        ftp_miman_system_cmd_reply_t syscmdrep;
    };

} ftp_miman_packet_t;


#endif
