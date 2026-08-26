/*******************************************************************************************
 * @file io_csp_ftp.c
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief Server-side FTP backend implementation.
 * 
 * @version 2.0
 * 
 * @date 2023-01-26
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#include "io_crc32.h"
#include "ftp_tools.h"
#include "io_csp_ftp_types.h"
#include "io_csp_ftp_config.h"

#include "cfe.h"

#include <csp/csp.h>
#include <gs/ftp/client.h>
#include <csp/csp_endian.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#ifndef IO_CSP_FTP_DEBUG
#define CSP_PORT_FTP        19 // originanally 16.
#else
enum CSP_PORTS {
    CSP_PORT_FTP            = 19,
    CSP_PORT_FTP_BBB        = 17,
};
#endif


/* FTP transfer global spec. */
static io_csp_ftp_transfer_spec tspec;
static bool ignore_length_check = false;


/* Server task. */
static int32 io_csp_ftp_init_server(csp_socket_t** server_sock);
int32 io_csp_ftp_server_start(uint32* run_status);
static void io_csp_ftp_server_loop(csp_socket_t* socket, uint32* run_status);


/* GS FTP packet handlers. */
static io_csp_ftp_ret ftp_handler_download_request(ftp_download_request_t* request);
static io_csp_ftp_ret ftp_handler_upload_request(ftp_upload_request_t* request);
static io_csp_ftp_ret ftp_handler_data(ftp_data_t* data);
static io_csp_ftp_ret ftp_handler_status_request(void);
static io_csp_ftp_ret ftp_handler_status_reply(ftp_status_reply_t* status);
static io_csp_ftp_ret ftp_handler_crc_request(void);
static io_csp_ftp_ret ftp_handler_abort(void);
static io_csp_ftp_ret ftp_handler_done(void);
static io_csp_ftp_ret ftp_handler_list_request(ftp_list_request_t* request);
static io_csp_ftp_ret ftp_handler_copy_request(ftp_copy_request_t* request);
#ifdef COMBINED_FILE_CONTROL_HANDLER
static io_csp_ftp_ret ftp_handler_fsctl_request(ftp_packet_t* request);
#else
static io_csp_ftp_ret ftp_handler_move_request(ftp_move_request_t* request);
static io_csp_ftp_ret ftp_handler_remove_request(ftp_remove_request_t* request);
static io_csp_ftp_ret ftp_handler_mkdir_request(ftp_mkdir_request_t* request);
static io_csp_ftp_ret ftp_handler_rmdir_request(ftp_rmdir_request_t* request);
#endif
#if 0 /* ! NOT SUPPORTED ! */
static io_csp_ftp_ret ftp_handler_zip_request(void);
static io_csp_ftp_ret ftp_handler_mkfs_request(void);
#endif
/* Custom handlers (MIMAN). */
// static io_csp_ftp_ret ftp_handler_restart_server_task(ftp_miman_server_restart_request_t* request);
static io_csp_ftp_ret ftp_handler_request_recent_retcodes(void);
static io_csp_ftp_ret ftp_handler_set_timeout(ftp_miman_timeout_request_t* request);
static io_csp_ftp_ret ftp_handler_request_file_cksum(ftp_miman_cksum_request_t* request);
static io_csp_ftp_ret ftp_handler_filespec_cleanup(void);
static io_csp_ftp_ret ftp_handler_read_chunk_request(ftp_miman_read_chunk_request_t* request);
static io_csp_ftp_ret ftp_handler_write_chunk_request(ftp_miman_write_chunk_request_t* request);
static io_csp_ftp_ret ftp_handler_ignore_length_check_request(void);
static io_csp_ftp_ret ftp_handler_killme(ftp_miman_killme_request_t* request);


/* Static helpers. */
static gs_ftp_return_t ftp_helper_init_upload(void);
static gs_ftp_return_t ftp_helper_read_upload_status(ftp_status_reply_t* status);
static gs_ftp_return_t ftp_helper_read_chunk(void* data, const io_csp_ftp_filespec* filespec, unsigned chunk, unsigned chunk_size, unsigned len);
static gs_ftp_return_t ftp_helper_write_chunk(const void* data, const io_csp_ftp_filespec* filespec, unsigned chunk, unsigned chunk_size, unsigned len, bool update_map);
static gs_ftp_return_t ftp_helper_filecopy(const char* from, const char* to);
static gs_ftp_return_t ftp_helper_filespec_cleanup(void);
static gs_ftp_return_t ftp_helper_conn_timeout(void);
// static gs_ftp_return_t ftp_helper_send_nosup_reply(uint8_t packet_type);



/* Chunk status markers */
static const char * const packet_ok      = IO_CSP_CHUNK_MARKER_OK;
static const char * const packet_missing = IO_CSP_CHUNK_MARKER_NO;

static io_csp_ftp_ret ftp_handler_download_request(ftp_download_request_t* request) {

    csp_packet_t* reply;
    ftp_packet_t* downrep;
    size_t downrep_len = sizeof(downrep->type) + sizeof(downrep->downrep);

    ftp_debug_info("download request received\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a download request but transfer status is NOT idle (%d)\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(downrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = downrep_len;
    downrep = (ftp_packet_t*) reply->data;

    /* Start download sequence. */
    tspec.transfer_status = IO_CSP_FTP_DOWNLOAD;

    /* Store request info to the global spec. */
    request->chunk_size = csp_ntoh16(request->chunk_size);
    request->mem_addr = csp_ntoh32(request->mem_addr);
    request->mem_size = csp_ntoh32(request->mem_size);
    request->path[sizeof(request->path) - 1] = 0;
    memcpy(&tspec.downspec, request, sizeof(tspec.downspec));

    /* Open the target file. */
    if (!(tspec.filespec.fp = fopen(tspec.downspec.path, "r"))) {
        ftp_debug_error("unable to open file %s: %s\n", tspec.downspec.path, strerror(errno));
        downrep->downrep.ret = GS_FTP_RET_IO;
    }
    else {
        /* Read file size. */
        struct stat statbuf;
        fstat(fileno(tspec.filespec.fp), &statbuf);
        tspec.filespec.size = statbuf.st_size;
        tspec.filespec.chunks = (tspec.filespec.size + tspec.downspec.chunk_size - 1) / tspec.downspec.chunk_size;

        downrep->downrep.size = csp_hton32(tspec.filespec.size);
        downrep->downrep.crc32 = csp_hton32(io_crc_get_file_crc32(tspec.filespec.fp));
        downrep->downrep.ret = GS_FTP_RET_OK;
    }

    downrep->type = FTP_DOWNLOAD_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("error sending downrep reply\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}

static io_csp_ftp_ret ftp_handler_upload_request(ftp_upload_request_t* request) {

    csp_packet_t* reply;
    ftp_packet_t* uprep;
    size_t uprep_len = sizeof(uprep->type) + sizeof(uprep->uprep);

    ftp_debug_info("upload request received\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got an upload request but transfer status is NOT idle (%d)\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(uprep_len))) {
        return IO_CSP_FTP_RET_NOBUF;
    }

    reply->length = uprep_len;
    uprep = (ftp_packet_t*) reply->data;

    /* Start upload sequence. */
    tspec.transfer_status = IO_CSP_FTP_UPLOAD;

    /* Store request info to the global spec. */
    request->size = csp_ntoh32(request->size);
    request->crc32 = csp_ntoh32(request->crc32);
    request->chunk_size = csp_ntoh16(request->chunk_size);
    request->mem_addr = csp_ntoh32(request->mem_addr);
    request->path[sizeof(request->path) - 1] = 0;
    memcpy(&tspec.upspec, request, sizeof(tspec.upspec));
    memcpy(tspec.filespec.filename, tspec.upspec.path, sizeof(tspec.filespec.filename));

    ftp_debug_normal("upload local path %s, file size %d, chunk size %d\n", 
                    tspec.upspec.path, tspec.upspec.size, tspec.upspec.chunk_size);

    /* Send reply. */
    uprep->uprep.ret = ftp_helper_init_upload();
    uprep->type = FTP_UPLOAD_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("error sending downrep reply\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}



static io_csp_ftp_ret ftp_handler_data(ftp_data_t* data) {

    unsigned datalen;

    if (tspec.transfer_status != IO_CSP_FTP_UPLOAD) {
        ftp_debug_error("got a data packet but transfer status is NOT upload\n");
        return IO_CSP_FTP_RET_STATUS;
    }
    
    /* Chunk number is always in LE (see GS client APIs). */
    data->chunk = csp_letoh32(data->chunk);
    if (data->chunk >= tspec.filespec.chunks) {
        ftp_debug_error("received out-of-range chunk (%d/%d).\n", data->chunk, tspec.filespec.chunks);
        return IO_CSP_FTP_RET_RANGE;
    }
    
    datalen = data->chunk + 1 == tspec.filespec.chunks ? tspec.upspec.size % tspec.upspec.chunk_size : tspec.upspec.chunk_size;
    ftp_debug_normal("data packet received: %d/%d, datalen %d\n", data->chunk + 1, tspec.filespec.chunks, datalen);

    if (ftp_helper_write_chunk(data->bytes, &tspec.filespec, data->chunk, tspec.upspec.chunk_size, datalen, true) != GS_FTP_RET_OK) {
        ftp_debug_error("chunk write error (%d/%d), datalen %d.\n", data->chunk, tspec.filespec.chunks, datalen);
        return IO_CSP_FTP_RET_CHUNK_WRITE;
    }

    return IO_CSP_FTP_RET_OK;

}



static io_csp_ftp_ret ftp_handler_status_request(void) {
    
    csp_packet_t* reply;
    ftp_packet_t* statusrep;
    size_t statusrep_len = sizeof(statusrep->type) + sizeof(statusrep->statusrep);

    ftp_debug_info("status request received\n");
    if (tspec.transfer_status != IO_CSP_FTP_UPLOAD) {
        ftp_debug_error("got a status request but transfer status is NOT upload (%d)\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(statusrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = statusrep_len;
    statusrep = (ftp_packet_t*) reply->data;

    /* Read file status. */
    memset(&statusrep->statusrep, 0, sizeof(statusrep->statusrep));
    statusrep->statusrep.ret = ftp_helper_read_upload_status(&statusrep->statusrep);
    /* Handle the byte order. */
    for (int i = 0; i < statusrep->statusrep.entries; i++) {
        statusrep->statusrep.entry[i].count = csp_hton32(statusrep->statusrep.entry[i].count);
        statusrep->statusrep.entry[i].next = csp_hton32(statusrep->statusrep.entry[i].next);
    }
    statusrep->statusrep.entries = csp_hton16(statusrep->statusrep.entries);
    statusrep->statusrep.total = csp_hton32(statusrep->statusrep.total);
    statusrep->statusrep.complete = csp_hton32(statusrep->statusrep.complete);

    /* Send reply. */
    statusrep->type = FTP_STATUS_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("status reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}



static io_csp_ftp_ret ftp_handler_status_reply(ftp_status_reply_t* status) {

    csp_packet_t* packet;
    ftp_packet_t* data;
    unsigned pktlen;

    ftp_debug_info("status reply received\n");
    if (tspec.transfer_status != IO_CSP_FTP_DOWNLOAD) {
        ftp_debug_error("got a download reply but transfer status is NOT download (%d)\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    status->total = csp_ntoh32(status->total);
    status->complete = csp_ntoh32(status->complete);
    status->entries = csp_ntoh16(status->entries);
    ftp_debug_normal("total chunks %d, completed %d\n", status->total, status->complete);

    /* Start data stage. */
    for (unsigned i = 0; i < status->entries; ++i) {
        status->entry[i].count = csp_ntoh32(status->entry[i].count);
        status->entry[i].next = csp_ntoh32(status->entry[i].next);
        ftp_debug_normal("entry %d, count %d, next %d\n", i, status->entry[i].count, status->entry[i].next);
    
        /* Read a chunk and send it out. */
        for (unsigned j = 0; j < status->entry[i].count; ++j) {
    
            uint32 chunk = status->entry[i].next + j;
            // remain = tspec.filespec.size - chunk * tspec.downspec.chunk_size;
            unsigned datalen = chunk + 1 < tspec.filespec.chunks ? 
                               tspec.downspec.chunk_size : tspec.filespec.size % tspec.downspec.chunk_size;
            pktlen = sizeof(data->type) + sizeof(data->data.chunk) + datalen;

            if (!(packet = csp_buffer_get(pktlen))) {
                ftp_debug_error("no CSP buffer available.\n");
                return IO_CSP_FTP_RET_NOBUF;
            }
            packet->length = pktlen;

            data = (ftp_packet_t*) packet->data;
            data->data.chunk = csp_hton32(chunk);

            data->type = FTP_DATA;
            if (ftp_helper_read_chunk(data->data.bytes, &tspec.filespec, 
                                                    chunk, tspec.downspec.chunk_size, datalen) != GS_FTP_RET_OK) {
                if (!csp_send(tspec.conn, packet, 60000)) {
                    csp_buffer_free(packet);
                }
                ftp_debug_error("read error on chunk %d.\n", chunk);
                return IO_CSP_FTP_RET_CHUNK_READ;
            }

            if (!csp_send(tspec.conn, packet, 60000)) {
                csp_buffer_free(packet);
                ftp_debug_error("data send error on chunk %d.\n", chunk);
                return IO_CSP_FTP_RET_SEND;
            }
        }
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_crc_request(void) {

    csp_packet_t* reply;
    ftp_packet_t* crcrep;
    size_t crcrep_len = sizeof(crcrep->type) + sizeof(crcrep->crcrep);

    ftp_debug_info("CRC request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_DOWNLOAD && 
        tspec.transfer_status != IO_CSP_FTP_UPLOAD) {
        ftp_debug_error("got a CRC request but transfer status IS idle\n");
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(crcrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = crcrep_len;
    crcrep = (ftp_packet_t*) reply->data;

    crcrep->crcrep.crc = io_crc_get_file_crc32(tspec.filespec.fp);
    ftp_debug_normal("CRC value 0x%08X\n", crcrep->crcrep.crc);
    crcrep->crcrep.ret = GS_FTP_RET_OK;
    crcrep->crcrep.crc = csp_hton32(crcrep->crcrep.crc);

    /* Send reply. */
    crcrep->type = FTP_CRC_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("crc reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_abort(void) {
    return ftp_handler_done();
}

static io_csp_ftp_ret ftp_handler_done(void) {

    // char mapname[GS_FTP_PATH_LENGTH + 10];

    ftp_debug_info("abort (DONE) request received.\n");
    if (tspec.transfer_status == IO_CSP_FTP_IDLE) {
        ftp_debug_error("got an abort request but transfer status IS idle.\n");
        return IO_CSP_FTP_RET_STATUS;
    }

    // snprintf(mapname, sizeof(mapname), "%s.map", tspec.filespec.filename);
    // remove(mapname);

    return ftp_helper_filespec_cleanup() == GS_FTP_RET_OK ? IO_CSP_FTP_RET_OK : IO_CSP_FTP_RET_INVAL;
}


static io_csp_ftp_ret ftp_handler_list_request(ftp_list_request_t* request) {

    csp_packet_t* reply;
    ftp_packet_t* listrep;
    size_t listrep_len = sizeof(listrep->type) + sizeof(listrep->listrep);
    size_t listent_len = sizeof(listrep->type) + sizeof(listrep->listent);

    struct dirent* ent;
    uint16_t count = 0;

    ftp_debug_info("list request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a list request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Entry count reply. */
    if (!(reply = csp_buffer_get(listrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = listrep_len;
    listrep = (ftp_packet_t*) reply->data;

    request->path[sizeof(request->path) - 1] = 0;
    if ((tspec.filespec.dp = opendir(request->path)) == NULL) {
        listrep->listrep.ret = GS_FTP_RET_IO;
    }
    else {
        while ((ent = readdir(tspec.filespec.dp))) {
            /* Skip non-regular files. deleted !ent->d_name ||*/
            if ( ent->d_name[0] == 0 || !strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }
            ++count;
        }
        listrep->listrep.ret = GS_FTP_RET_OK;
        listrep->listrep.entries = csp_hton16(count);
    }

    listrep->type = FTP_LIST_REPLY;
    if (!csp_send(tspec.conn, reply, 30000)) {
        ftp_debug_error("list reply send error.\n");
        csp_buffer_free(reply);
        closedir(tspec.filespec.dp);
        return IO_CSP_FTP_RET_SEND;
    }

    if (!tspec.filespec.dp) {
        return IO_CSP_FTP_RET_ERROR;
    }

    /* Rewind and recurse again for entry replies. */
    rewinddir(tspec.filespec.dp);
    for (int i = 0; i < count; ++i) {

        /* Entry reply. */
        if (!(reply = csp_buffer_get(listent_len))) {
            ftp_debug_error("no CSP buffer available.\n");
            return IO_CSP_FTP_RET_NOBUF;
        }
        reply->length = listent_len;
        listrep = (ftp_packet_t*) reply->data;

        while ((ent = readdir(tspec.filespec.dp))) {
            struct stat st;
            /*  deleted !ent->d_name || */
            if (ent->d_name[0] == 0 || !strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }
            memcpy(listrep->listent.path, ent->d_name, sizeof(listrep->listent.path));
            if (stat(ent->d_name, &st) < 0) {
                listrep->listent.size = 0;
                listrep->listent.type = 0xFF;
            }
            else {
                listrep->listent.size = csp_hton32(st.st_size);
                listrep->listent.type = S_ISDIR(st.st_mode) ? GS_FTP_LIST_DIR : GS_FTP_LIST_FILE;
            }
            break;
        }
        listrep->listent.entry = csp_hton16(i);

        listrep->type = FTP_LIST_ENTRY;
        if (!csp_send(tspec.conn, reply, 30000)) {
            ftp_debug_error("list entry send error (entry %d/%d).\n", i, count);
            csp_buffer_free(reply);
            closedir(tspec.filespec.dp);
            tspec.filespec.dp = NULL;
            return IO_CSP_FTP_RET_SEND;
        }

    }

    closedir(tspec.filespec.dp);
    tspec.filespec.dp = NULL;
    return IO_CSP_FTP_RET_OK;
}


#ifdef COMBINED_FILE_CONTROL_HANDLER



#else

static io_csp_ftp_ret ftp_handler_move_request(ftp_move_request_t* request) {

    csp_packet_t* reply;
    ftp_packet_t* moverep;
    size_t moverep_len = sizeof(moverep->type) + sizeof(moverep->moverep);

    ftp_debug_info("move request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a move request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    /* It's a small reply but we'll need a CSP buffer for send() anyway. */
    if (!(reply = csp_buffer_get(moverep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }

    reply->length = moverep_len;
    moverep = (ftp_packet_t*) reply->data;

    /* Ensure null termination. */
    request->from[sizeof(request->from) - 1] = 0;
    request->to[sizeof(request->to) - 1] = 0;
    /* Move. */
    moverep->moverep.ret = rename(request->from, request->to) == 0 ? GS_FTP_RET_OK : GS_FTP_RET_NOENT;

    /* Send reply. */
    moverep->type = FTP_MOVE_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("move reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_remove_request(ftp_remove_request_t* request) {

    csp_packet_t* reply;
    ftp_packet_t* removerep;
    size_t removerep_len = sizeof(removerep->type) + sizeof(removerep->removerep);

    ftp_debug_info("remove request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a remove request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(removerep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = removerep_len;
    removerep = (ftp_packet_t*) reply->data;

    /* Remove. */
    request->path[sizeof(request->path) - 1] = 0;
    removerep->removerep.ret = remove(request->path) == 0 ? GS_FTP_RET_OK : GS_FTP_RET_NOENT;

    /* Send reply. */
    removerep->type = FTP_REMOVE_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("remove reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_copy_request(ftp_copy_request_t* request) {
    csp_packet_t* reply;
    ftp_packet_t* copyrep;
    size_t copyrep_len = sizeof(copyrep->type) + sizeof(copyrep->copyrep);

    ftp_debug_info("copy request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a copy request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(copyrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = copyrep_len;
    copyrep = (ftp_packet_t*) reply->data;

    request->from[sizeof(request->from) - 1] = 0;
    request->to[sizeof(request->to) - 1] = 0;
    copyrep->copyrep.ret = ftp_helper_filecopy(request->from, request->to);

    /* Send reply. */
    copyrep->type = FTP_COPY_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("remove reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;
}


static io_csp_ftp_ret ftp_handler_mkdir_request(ftp_mkdir_request_t* request) {

    csp_packet_t* reply;
    ftp_packet_t* mkdirrep;
    size_t mkdirrep_len = sizeof(mkdirrep->type) + sizeof(mkdirrep->mkdirrep);

    ftp_debug_info("mkdir request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a mkdir request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(mkdirrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }

    reply->length = mkdirrep_len;
    mkdirrep = (ftp_packet_t*) reply->data;

    /* Ensure null termination. */
    request->path[sizeof(request->path) - 1] = 0;
    /* Make directory. */
    mkdirrep->mkdirrep.ret = mkdir(request->path, request->mode) == 0 ? GS_FTP_RET_OK : GS_FTP_RET_NOENT;

    /* Send reply. */
    mkdirrep->type = FTP_MKDIR_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("mkdir reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}



static io_csp_ftp_ret ftp_handler_rmdir_request(ftp_rmdir_request_t* request) {

    csp_packet_t* reply;
    ftp_packet_t* rmdirrep;
    size_t rmdirrep_len = sizeof(rmdirrep->type) + sizeof(rmdirrep->rmdirrep);

    ftp_debug_info("rmdir request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a rmdir request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(rmdirrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }

    reply->length = rmdirrep_len;
    rmdirrep = (ftp_packet_t*) reply->data;

    /* Remove directory. */
    request->path[sizeof(request->path) - 1] = 0;
    rmdirrep->rmdirrep.ret = rmdir(request->path) == 0 ? GS_FTP_RET_OK : GS_FTP_RET_NOENT;

    /* Send reply. */
    rmdirrep->type = FTP_RMDIR_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("rmdir reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}

#endif


#if 0
static io_csp_ftp_ret ftp_handler_copy_request(void) {
    return ftp_helper_send_nosup_reply(FTP_COPY_REPLY) == GS_FTP_RET_OK ? IO_CSP_FTP_RET_OK : IO_CSP_FTP_RET_SEND;
}
static io_csp_ftp_ret ftp_handler_zip_request(void) {
    return ftp_helper_send_nosup_reply(FTP_ZIP_REPLY) == GS_FTP_RET_OK ? IO_CSP_FTP_RET_OK : IO_CSP_FTP_RET_SEND;
}
static io_csp_ftp_ret ftp_handler_mkfs_request(void) {
    return ftp_helper_send_nosup_reply(FTP_MKFS_REPLY) == GS_FTP_RET_OK ? IO_CSP_FTP_RET_OK : IO_CSP_FTP_RET_SEND;
}
#endif


// static io_csp_ftp_ret ftp_handler_restart_server_task(ftp_miman_server_restart_request_t* request) {
//     return IO_CSP_FTP_RET_OK;

// }


static io_csp_ftp_ret ftp_handler_set_timeout(ftp_miman_timeout_request_t* request) {
    
    csp_packet_t* reply;
    ftp_miman_packet_t* timeoutrep;
    size_t timeoutrep_len = sizeof(timeoutrep->type) + sizeof(timeoutrep->timeoutrep);

    ftp_debug_info("set timeout request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a set timeout request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    if (!(reply = csp_buffer_get(timeoutrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = timeoutrep_len;
    timeoutrep = (ftp_miman_packet_t*) reply->data;

    tspec.timeout = csp_ntoh32(request->timeout);

    timeoutrep->type = FTP_MIMAN_SET_TIMEOUT_REPLY;
    timeoutrep->timeoutrep.curr_timeout = csp_hton32(tspec.timeout);
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("read chunk reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;
}


static io_csp_ftp_ret ftp_handler_read_chunk_request(ftp_miman_read_chunk_request_t* request) {
    
    gs_ftp_return_t ret;
    io_csp_ftp_filespec* fspec;
    csp_packet_t* reply;
    ftp_miman_packet_t* readrep;
    size_t readrep_len = sizeof(readrep->type) + sizeof(readrep->readrep.ret) + sizeof(readrep->readrep.entry);

    ftp_debug_info("read chunk request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a read chunk request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    request->chunk_size = csp_ntoh16(request->chunk_size);
    if (request->chunk_size > sizeof(readrep->readrep.bytes)) {
        ftp_debug_error("chunk size is too large.\n");
        return IO_CSP_FTP_RET_INVAL;
    }
    readrep_len += request->chunk_size;

    request->size = csp_ntoh32(request->size);
    request->offset = csp_ntoh32(request->offset);
    if (request->offset < 0 || request->size < 0) {
        ftp_debug_error("invalid size/offset.\n");
        ret = GS_FTP_RET_INVAL;
    }
    else {
        if ((fspec = malloc(sizeof(*fspec)))) {
            request->path[sizeof(request->path) - 1] = '\0';
            if ((fspec->fp = fopen(request->path, "r"))) {

                if (request->size < request->chunk_size) {
                    request->chunk_size = request->size;
                }

                for (int i = 0; i < (request->size + request->chunk_size - 1) / request->chunk_size; ++i) {
                    if (!(reply = csp_buffer_get(readrep_len))) {
                        ftp_debug_error("no CSP buffer available.\n");
                        return IO_CSP_FTP_RET_NOBUF;
                    }
                    reply->length = readrep_len;
                    readrep = (ftp_miman_packet_t*) reply->data;
                    readrep->type = FTP_MIMAN_READ_CHUNK_REPLY;
                    readrep->readrep.entry = i;
                    readrep->readrep.ret = ftp_helper_read_chunk(readrep->readrep.bytes, fspec, request->offset + i * request->chunk_size, 1, request->chunk_size);

                    if (!csp_send(tspec.conn, reply, 1000)) {
                        ftp_debug_error("read chunk reply send error\n");
                        csp_buffer_free(reply);
                        return IO_CSP_FTP_RET_SEND;
                    }
                }
                fclose(fspec->fp);
                return GS_FTP_RET_OK;
            }
            else {
                ftp_debug_error("could not open %s: %s.\n", request->path, strerror(errno));
                ret = GS_FTP_RET_IO;
            }
            free(fspec);
        }
        else {
            ftp_debug_error("malloc error.\n");
            ret = GS_FTP_RET_NOMEM;
        }
    }

    if (!(reply = csp_buffer_get(readrep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = sizeof(readrep->type) + sizeof(readrep->readrep.ret);
    readrep = (ftp_miman_packet_t*) reply->data;
    readrep->type = FTP_MIMAN_READ_CHUNK_REPLY;
    readrep->readrep.ret = ret;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("read chunk reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_write_chunk_request(ftp_miman_write_chunk_request_t* request) {

    io_csp_ftp_filespec* fspec;
    csp_packet_t* reply;
    ftp_miman_packet_t* writerep;
    size_t writerep_len = sizeof(writerep->type) + sizeof(writerep->writerep);

    ftp_debug_info("write chunk request received.\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a write chunk request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    if (!(reply = csp_buffer_get(writerep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }

    reply->length = writerep_len;
    writerep = (ftp_miman_packet_t*) reply->data;

    if (request->offset < 0 || request->size < 0 || request->size > sizeof(request->bytes)) {
        writerep->writerep.ret = GS_FTP_RET_INVAL;
    }
    else {
        if ((fspec = malloc(sizeof(*fspec)))) {
            request->path[sizeof(request->path) - 1] = '\0';
            if ((fspec->fp = fopen(request->path, "r+"))) {
                writerep->writerep.ret = ftp_helper_write_chunk(request->bytes, fspec, request->offset, 1, request->size, false);
                writerep->writerep.err = errno;
                fclose(fspec->fp);
            }
            else {
                writerep->writerep.ret = GS_FTP_RET_IO;
            }
            free(fspec);
        }
        else {
            writerep->writerep.ret = GS_FTP_RET_NOMEM;
        }
    }

    writerep->type = FTP_MIMAN_WRITE_CHUNK_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("read chunk reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}



static io_csp_ftp_ret ftp_handler_ignore_length_check_request(void) {
    uint8_t reply[2];

    ftp_debug_info("ignlenchk request received.\n");
    ignore_length_check = true;

    reply[0] = FTP_MIMAN_IGNORE_LENGTH_CHECK_REPLY;
    reply[1] = ignore_length_check;

    if (csp_transaction_persistent(tspec.conn, 1000, reply, 2, NULL, 0) <= 0) {
        return IO_CSP_FTP_RET_SEND;
    }
    return IO_CSP_FTP_RET_OK;
}



static io_csp_ftp_ret ftp_handler_killme(ftp_miman_killme_request_t* request) {

    csp_packet_t* reply;
    ftp_miman_packet_t* killmerep;
    size_t killmerep_len = sizeof(killmerep->type) + sizeof(killmerep->killmerep);

    ftp_debug_info("killme request received.\n");
    request->magic = csp_ntoh32(request->magic);
    if (request->magic == 0xDEADFACE) {

        /* Exit immediately without a reply. We don't want to risk anything. */
        exit(1);  // Replace me with the PSP exit entry!

            /* Last resort. */
        /*Avada*/;;;abort();++(*((int*)0));;;/*Kedavra*/

        /* If that shit fails there's no hope. */
        if ((reply = csp_buffer_get(killmerep_len)) == NULL) {
            ftp_debug_error("no CSP buffer available.\n");
            return IO_CSP_FTP_RET_NOBUF;
        }
        reply->length = killmerep_len;
        killmerep = (ftp_miman_packet_t*) reply->data;
        killmerep->type = FTP_MIMAN_KILL_ME_REPLY;
        killmerep->killmerep.ret = MM_FTP_RET_KILL_FAIL;

        if (!csp_send(tspec.conn, reply, 10000)) {
            csp_buffer_free(reply);
            return IO_CSP_FTP_RET_SEND;
        }
        return IO_CSP_FTP_RET_ERROR;

    }

    /* Invalid magic number. */
    if ((reply = csp_buffer_get(killmerep_len)) == NULL) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = killmerep_len;
    killmerep = (ftp_miman_packet_t*) reply->data;
    killmerep->type = FTP_MIMAN_KILL_ME_REPLY;
    killmerep->killmerep.ret = MM_FTP_RET_MAGIC;

    if (!csp_send(tspec.conn, reply, 10000)) {
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_ERROR;
}

static io_csp_ftp_ret ftp_handler_request_recent_retcodes(void) {

    csp_packet_t* reply;
    ftp_miman_packet_t* retcoderep;
    size_t retcoderep_len = sizeof(retcoderep->type) + sizeof(retcoderep->retcoderep);

    ftp_debug_info("retcode request received\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a retcode request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    if ((reply = csp_buffer_get(retcoderep_len)) == NULL) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = retcoderep_len;
    retcoderep = (ftp_miman_packet_t*) reply->data;

// FIXME: DELETE THE OFLOW TEST. USELESS!
    if (tspec.retcodes.pos >= IO_CSP_FTP_RETCODE_LEN) {
        tspec.retcodes.pos = 0;
        retcoderep->retcoderep.ret = MM_FTP_RET_RETCODE_OFLOW;
    }
    else {
        retcoderep->retcoderep.ret = GS_FTP_RET_OK;
    }

    for (int i = 0; i < IO_CSP_FTP_RETCODE_LEN; ++i) {
        int j = (IO_CSP_FTP_RETCODE_LEN - i + tspec.retcodes.pos) % IO_CSP_FTP_RETCODE_LEN;
        retcoderep->retcoderep.type[i] = tspec.retcodes.type[j];
        retcoderep->retcoderep.retcode[i] = tspec.retcodes.ret[j];
    }

    retcoderep->type = FTP_MIMAN_RETCODE_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_request_file_cksum(ftp_miman_cksum_request_t* request) {

    FILE* fp;
    csp_packet_t* reply;
    ftp_miman_packet_t* cksumrep;
    size_t cksumrep_len = sizeof(cksumrep->type) + sizeof(cksumrep->cksumrep);

    ftp_debug_info("checksum request received\n");
    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        ftp_debug_error("got a cksum request but transfer status is NOT idle (%d).\n", tspec.transfer_status);
        return IO_CSP_FTP_RET_STATUS;
    }

    if ((reply = csp_buffer_get(cksumrep_len)) == NULL) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = cksumrep_len;
    cksumrep = (ftp_miman_packet_t*) reply->data;

    request->path[sizeof(request->path) - 1] = '\0';
    if ((fp = fopen(request->path, "r")) == NULL) {
        cksumrep->cksumrep.ret = GS_FTP_RET_IO;
    }
    else {
        cksumrep->cksumrep.ret = GS_FTP_RET_OK;
        cksumrep->cksumrep.crc32 = csp_hton32(io_crc_get_file_crc32(fp));
    }

    cksumrep->type = FTP_MIMAN_CHECKSUM_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_filespec_cleanup(void) {

    csp_packet_t* reply;
    ftp_miman_packet_t* cleanuprep;
    size_t cleanuprep_len = sizeof(cleanuprep->type) + sizeof(cleanuprep->cleanuprep);

    ftp_debug_info("cleanup request received\n");

    if (!(reply = csp_buffer_get(cleanuprep_len))) {
        ftp_debug_error("no CSP buffer available.\n");
        return IO_CSP_FTP_RET_NOBUF;
    }
    reply->length = cleanuprep_len;
    cleanuprep = (ftp_miman_packet_t*) reply->data;

    cleanuprep->cleanuprep.ret = ftp_helper_filespec_cleanup();
    tspec.transfer_status = IO_CSP_FTP_IDLE;

    cleanuprep->type = FTP_MIMAN_FSPEC_CLEANUP_REPLY;
    if (!csp_send(tspec.conn, reply, 1000)) {
        ftp_debug_error("read chunk reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_SEND;
    }

    return IO_CSP_FTP_RET_OK;
}


static gs_ftp_return_t ftp_helper_init_upload(void) {

    bool file_was_there = true;
    char mapname[GS_FTP_PATH_LENGTH + 10];

    ftp_helper_filespec_cleanup();

    /* Try opening the local target file. Create new if didn't exist. */
    sprintf(mapname, "%s.map", tspec.filespec.filename);
    if ((tspec.filespec.fp = fopen(tspec.filespec.filename, "r+")) == NULL) {
        if ((tspec.filespec.fp = fopen(tspec.filespec.filename, "w+")) == NULL) {
            ftp_debug_error("cannot open file %s: %s\n", tspec.filespec.filename, strerror(errno));
            return GS_FTP_RET_IO;
        }
        file_was_there = false;
    }

    tspec.filespec.chunks = (tspec.upspec.size + tspec.upspec.chunk_size - 1) / tspec.upspec.chunk_size;
    if ((tspec.filespec.mapbuf = malloc(tspec.filespec.chunks)) == NULL) {
        fclose(tspec.filespec.fp);
        tspec.filespec.fp = NULL;
        return GS_FTP_RET_NOSPC;
    }

    /* Now try opening its bitmap. */
    if ((tspec.filespec.mapfp = fopen(mapname, "r+")) == NULL) {

        /* We can't have target file without a bitmap. */
        if (file_was_there) {
            fclose(tspec.filespec.fp);
            tspec.filespec.fp = NULL;
            free(tspec.filespec.mapbuf);
            tspec.filespec.mapbuf = NULL;
            ftp_debug_error("local target %s exists but bitmap doesn't\n", tspec.filespec.filename);
            return GS_FTP_RET_EXISTS;
        }

        /* Create a new bitmap and clear it. */
        if ((tspec.filespec.mapfp = fopen(mapname, "w+")) == NULL) {
            ftp_debug_error("cannot create map %s: %s\n", tspec.filespec.filename, strerror(errno));
            return GS_FTP_RET_IO;
        }

        for (int i = 0; i < tspec.filespec.chunks; ++i) {
            if (fwrite(packet_missing, 1, 1, tspec.filespec.mapfp) != 1) {
                ftp_debug_error("error clearing map: %s\n", strerror(errno));
                fclose(tspec.filespec.fp);
                tspec.filespec.fp = NULL;
                fclose(tspec.filespec.mapfp);
                tspec.filespec.mapfp = NULL;
                free(tspec.filespec.mapbuf);
                tspec.filespec.mapbuf = NULL;
                return GS_FTP_RET_IO;
            }
            tspec.filespec.mapbuf[i] = *packet_missing;
        }

        fflush(tspec.filespec.mapfp);
        fsync(fileno(tspec.filespec.mapfp));

    }
    else {
        /* The map exists. */
        if (fread(tspec.filespec.mapbuf, 1, tspec.filespec.chunks, tspec.filespec.mapfp) != tspec.filespec.chunks) {
            ftp_debug_error("could not read from existing map %s.\n", mapname);
            fclose(tspec.filespec.fp);
            tspec.filespec.fp = NULL;
            fclose(tspec.filespec.mapfp);
            tspec.filespec.mapfp = NULL;
            free(tspec.filespec.mapbuf);
            tspec.filespec.mapbuf = NULL;
            return GS_FTP_RET_IO;
        }
    }

    return GS_FTP_RET_OK;

}


static gs_ftp_return_t ftp_helper_read_upload_status(ftp_status_reply_t* status) {

    unsigned count_in_entry = 0;
    unsigned next = 0;
    bool s;

    if (!tspec.filespec.mapbuf) {
        return GS_FTP_RET_INVAL;
    }

    for (unsigned i = 0; i < tspec.filespec.chunks; ++i) {
        if ((s = (tspec.filespec.mapbuf[i] == *packet_ok))) {
            status->complete++;
        }
        if (status->entries < GS_FTP_STATUS_CHUNKS) {
            if (!s) {
                if (!count_in_entry) {
                    next = i;
                }
                count_in_entry++;
            }
            if (count_in_entry > 0 && (s || i == tspec.filespec.chunks - 1)) {
                status->entry[status->entries].next = next;
                status->entry[status->entries].count = count_in_entry;
                status->entries++;
            }
        }
    }

    status->entries = status->entries;
    status->complete = status->complete;
    status->total = tspec.filespec.chunks;

    return GS_FTP_RET_OK;

}


static gs_ftp_return_t ftp_helper_read_chunk(void* data, const io_csp_ftp_filespec* filespec,
                                             unsigned chunk, unsigned chunk_size, unsigned datalen) {

    if (!data || !filespec) {
        return GS_FTP_RET_INVAL;
    }
    if (!filespec->fp) {
        return GS_FTP_RET_INVAL;
    }
    if (datalen > GS_FTP_MAX_CHUNK_SIZE) {
        return GS_FTP_RET_NOSPC;
    }
    if (ftell(filespec->fp) != chunk * chunk_size) {
        if (fseek(filespec->fp, chunk * chunk_size, SEEK_SET) != 0) {
            return GS_FTP_RET_IO;
        }
    }
    if (fread(data, 1, datalen, filespec->fp) != datalen) {
        return GS_FTP_RET_IO;
    }
    return GS_FTP_RET_OK;

}


static gs_ftp_return_t ftp_helper_write_chunk(const void* data, const io_csp_ftp_filespec* filespec,
                                              unsigned chunk, unsigned chunk_size, unsigned datalen, bool update_map) {

    if (!data || !filespec) {
        return GS_FTP_RET_INVAL;
    }

    if (!filespec->fp) {
        return GS_FTP_RET_INVAL;
    }

    if (update_map && (!filespec->mapfp || !filespec->mapbuf)) {
        return GS_FTP_RET_INVAL;
    }

    if (ftell(filespec->fp) != chunk * chunk_size) {
        if (fseek(filespec->fp, chunk * chunk_size, SEEK_SET) != 0) {
            return GS_FTP_RET_IO;
        }
    }
    if (fwrite(data, 1, datalen, filespec->fp) != datalen) {
        return GS_FTP_RET_IO;
    }

    fflush(filespec->fp);
    fsync(fileno(filespec->fp));

    if (update_map) {

        if (ftell(filespec->mapfp) != chunk) {
            if (fseek(filespec->mapfp, chunk, SEEK_SET) != 0) {
                return GS_FTP_RET_IO;
            }
        }
        if (fwrite(packet_ok, 1, 1, filespec->mapfp) != 1) {
            return GS_FTP_RET_IO;
        }

        filespec->mapbuf[chunk] = *packet_ok;

        fflush(filespec->mapfp);
        fsync(fileno(filespec->mapfp));

    }

    return GS_FTP_RET_OK;

}


// static gs_ftp_return_t ftp_helper_send_nosup_reply(uint8_t packet_type) {
//     csp_packet_t* packet = csp_buffer_get(sizeof(gs_ftp_type_t) + sizeof(uint8_t));
//     packet->data[0] = packet_type;
//     packet->data[1] = GS_FTP_RET_NOTSUP;
//     if (!csp_send(tspec.conn, packet, tspec.timeout)) {
//         csp_buffer_free(packet);
//         return GS_FTP_RET_IO;
//     }
//     return GS_FTP_RET_OK;
// }


static gs_ftp_return_t ftp_helper_filecopy(const char* from, const char* to) {

    int fdfrom, fdto;
    int bytes;
    char buf[IO_CSP_FTP_COPY_BUFSIZ];

    if ((fdfrom = open(from, O_RDONLY)) < 0) {
        ftp_debug_error("unable to open source file %s: %s\n", from, strerror(errno));
        return GS_FTP_RET_IO;
    }

    if ((fdto = open(to, O_CREAT | O_WRONLY)) < 0) {
        ftp_debug_error("unable to open dest file %s: %s\n", to, strerror(errno));
        close(fdfrom);
        return GS_FTP_RET_IO;
    }

    while ((bytes = read(fdfrom, buf, sizeof(buf))) > 0) {
        if (write(fdto, buf, bytes) != bytes) {
            close(fdfrom);
            close(fdto);
            remove(to);
            return GS_FTP_RET_IO;
        }
    }

    fsync(fdto);
    close(fdfrom);
    close(fdto);

    return GS_FTP_RET_OK;

}



static gs_ftp_return_t ftp_helper_filespec_cleanup(void) {

    if (tspec.filespec.fp) {
        fclose(tspec.filespec.fp);
        tspec.filespec.fp = NULL;
    }

    if (tspec.filespec.mapfp) {
        fclose(tspec.filespec.mapfp);
        tspec.filespec.mapfp = NULL;
    }

    if (tspec.filespec.mapbuf) {
        free(tspec.filespec.mapbuf);
        tspec.filespec.mapbuf = NULL;
    }

    if (tspec.filespec.dp) {
        closedir(tspec.filespec.dp);
        tspec.filespec.dp = NULL;
    }

    return GS_FTP_RET_OK;

}


static gs_ftp_return_t ftp_helper_conn_timeout(void) {
    ftp_debug_info("timeout handler called.\n");
    ftp_helper_filespec_cleanup();
    tspec.transfer_status = IO_CSP_FTP_IDLE;
    return GS_FTP_RET_OK;
}



static int32 io_csp_ftp_init_server(csp_socket_t** server_sock_out) {

    csp_socket_t* server_sock;

    if (!server_sock_out) {
        return -1;
    }

    *server_sock_out = NULL;

    memset(&tspec, 0, sizeof(tspec));
    tspec.transfer_status = IO_CSP_FTP_IDLE;
    tspec.timeout = IO_CSP_FTP_READ_DEFAULT_TIMEOUT;

    if ((server_sock = csp_socket(0)) == NULL) {
        ftp_debug_error("ftp socket creat error\n");
        return -1;
    }

    if (csp_bind(server_sock, CSP_PORT_FTP) != 0) {
        ftp_debug_error("bind error for ftp port %d\n", CSP_PORT_FTP);
        csp_close(server_sock);
        return -1;
    }
    
    if (csp_listen(server_sock, 10) < 0) {
        ftp_debug_error("listen error for ftp port %d\n", CSP_PORT_FTP);
        csp_close(server_sock);
        return -1;
    }

    *server_sock_out = server_sock;
    ftp_debug_info("FTP server listening on port %d\n", CSP_PORT_FTP);
    return 0;

}

int32 io_csp_ftp_server_start(uint32* run_status) {

    csp_socket_t* server_sock;
    int32 status;

    status = io_csp_ftp_init_server(&server_sock);
    if (status != 0) {
        return status;
    }

    io_csp_ftp_server_loop(server_sock, run_status);

    if (tspec.conn) {
        csp_close(tspec.conn);
        tspec.conn = NULL;
    }

    ftp_helper_filespec_cleanup();
    csp_close(server_sock);

    return 0;

}


#define LENGTH_CHECK(len, key)      (ignore_length_check || ((len) >= sizeof(((ftp_packet_t*)0)->type) + sizeof(((ftp_packet_t*)0)->key)))
#define LENGTH_CHECK_SHORT(len)     (ignore_length_check || ((len) >= sizeof(((ftp_packet_t*)0)->type)))


static void io_csp_ftp_server_loop(csp_socket_t* socket, uint32* run_status) {

    csp_packet_t*       packet;
    ftp_packet_t*       ftp_packet;
    io_csp_ftp_ret      ret;
    uint8_t type;
    bool out;

    while (CFE_ES_RunLoop(run_status) == true) {

        /* Do we wait forever? */
        if (!(tspec.conn = csp_accept(socket, IO_CSP_FTP_ACCEPT_TIMEOUT)))
            continue;

        ftp_debug_normal("incoming connection from node %d\n", csp_conn_src(tspec.conn));
        while (CFE_ES_RunLoop(run_status) == true) {

            /* The adjustable timeout applies only on read(). CSP does not support send timeout in this version. */
            if ((packet = csp_read(tspec.conn, tspec.timeout)) == NULL) {
                ftp_debug_normal("timeout reached during main read().\n");
                goto no_read;
            }

            ftp_packet = (ftp_packet_t*) packet->data;
            type = ftp_packet->type;
            out = false;

            /* None of these handlers free the incoming packet. */
            switch (type) {
                case FTP_DOWNLOAD_REQUEST:
                    ret = LENGTH_CHECK(packet->length, down) 
                        ? ftp_handler_download_request(&ftp_packet->down) 
                        : MM_FTP_RET_PKT_LEN;
                    break;
                case FTP_STATUS_REPLY:
                    ret = LENGTH_CHECK(packet->length, statusrep) 
                        ? ftp_handler_status_reply(&ftp_packet->statusrep)
                        : MM_FTP_RET_PKT_LEN;
                    break;
                case FTP_UPLOAD_REQUEST:
                    ret = LENGTH_CHECK(packet->length, up) 
                        ? ftp_handler_upload_request(&ftp_packet->up)
                        : MM_FTP_RET_PKT_LEN;
                    break;
                case FTP_STATUS_REQUEST:
                    ret = LENGTH_CHECK_SHORT(packet->length)
                        ? ftp_handler_status_request()
                        : MM_FTP_RET_PKT_LEN;
                    break;
                case FTP_DATA:
                    ret = ftp_handler_data(&ftp_packet->data);
                    break;
                case FTP_CRC_REQUEST:
                    ret = LENGTH_CHECK_SHORT(packet->length)
                        ? ftp_handler_crc_request()
                        : MM_FTP_RET_PKT_LEN;
                    out = true;
                    break;
                case FTP_DONE:
                    ret = ftp_handler_done();
                    out = true;
                    break;
                case FTP_ABORT:
                    ret = ftp_handler_abort();
                    out = true;
                    break;
                case FTP_LIST_REQUEST:
                    ret = LENGTH_CHECK(packet->length, list) 
                        ? ftp_handler_list_request(&ftp_packet->list)
                        : MM_FTP_RET_PKT_LEN;
                    out = true;
                    break;
                case FTP_MOVE_REQUEST:
                    ret = LENGTH_CHECK(packet->length, move) 
                        ? ftp_handler_move_request(&ftp_packet->move)
                        : MM_FTP_RET_PKT_LEN;
                    out = true;
                    break;
                case FTP_REMOVE_REQUEST:
                    ret = LENGTH_CHECK(packet->length, remove) 
                        ? ftp_handler_remove_request(&ftp_packet->remove)
                        : MM_FTP_RET_PKT_LEN;
                    out = true;
                    break;
                case FTP_COPY_REQUEST:
                    ret = LENGTH_CHECK(packet->length, copy) 
                        ? ftp_handler_copy_request(&ftp_packet->copy)
                        : MM_FTP_RET_PKT_LEN;
                    out = true;
                    break;
                case FTP_MKDIR_REQUEST:
                    ret = LENGTH_CHECK(packet->length, mkdir) 
                        ? ftp_handler_mkdir_request(&ftp_packet->mkdir)
                        : MM_FTP_RET_PKT_LEN;
                    out = true;
                    break;
                case FTP_RMDIR_REQUEST:
                    ret = LENGTH_CHECK(packet->length, rmdir) 
                        ? ftp_handler_rmdir_request(&ftp_packet->rmdir)
                        : MM_FTP_RET_PKT_LEN;
                    out = true;
                    break;

                /* Custom handlers. */
                case FTP_MIMAN_RETCODE_REQUEST:
                    ret = ftp_handler_request_recent_retcodes();
                    out = true;
                    break;
                case FTP_MIMAN_SET_TIMEOUT_REQUEST:
                    ret = ftp_handler_set_timeout(&((ftp_miman_packet_t*) ftp_packet)->timeout);
                    out = true;
                    break;
                case FTP_MIMAN_CHECKSUM_REQUEST:
                    ret = ftp_handler_request_file_cksum(&((ftp_miman_packet_t*) ftp_packet)->cksum);
                    out = true;
                    break;
                case FTP_MIMAN_FSPEC_CLEANUP_REQUEST:
                    ret = ftp_handler_filespec_cleanup();
                    out = true;
                    break;
                case FTP_MIMAN_READ_CHUNK_REQUEST:
                    ret = ftp_handler_read_chunk_request(&((ftp_miman_packet_t*) ftp_packet)->read);
                    out = true;
                    break;
                case FTP_MIMAN_WRITE_CHUNK_REQUEST:
                    ret = ftp_handler_write_chunk_request(&((ftp_miman_packet_t*) ftp_packet)->write);
                    out = true;
                    break;
                case FTP_MIMAN_IGNORE_LENGTH_CHECK_REQUEST:
                    ret = ftp_handler_ignore_length_check_request();
                    out = true;
                    break;
                // case FTP_MIMAN_SERVER_RESTART_REQUEST:
                //     ret = ftp_handler_restart_server_task(&((ftp_miman_packet_t*) ftp_packet)->restart);
                //     break;
                case FTP_MIMAN_KILL_ME_REQUEST:
                    ret = ftp_handler_killme(&((ftp_miman_packet_t*) ftp_packet)->killme);
                    out = true;
                    break;

                default:
                    ftp_debug_error("Invalid ftp packet type: %d\n", ftp_packet->type);
                    ret = IO_CSP_FTP_RET_INVAL;
                    break;
            }
            csp_buffer_free(packet);

            /* Mark the retcodes. */
            tspec.retcodes.type[tspec.retcodes.pos] = type;
            tspec.retcodes.ret[tspec.retcodes.pos] = ret;
            tspec.retcodes.pos = (tspec.retcodes.pos + 1) % IO_CSP_FTP_RETCODE_LEN;

            /* Abort consecutive reads if needed. */
            if (out || ret != IO_CSP_FTP_RET_OK)
                break;
        }
no_read:
        if (tspec.conn) {
            csp_close(tspec.conn);
            tspec.conn = NULL;
        }
        ftp_helper_conn_timeout();

    }

}
