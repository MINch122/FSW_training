/**
 * @file ftpnew.c
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief FTP-NEW server implementation.
 *        FTP-NEW stands for "F***ing Tired of Patching this
 *            Nonsensical Extensions and Wrappers for FTP."
 * @version 0.1
 * @date 2026-06-01
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 */
#include "ftpnew.h"
#include "ftpnew_types_internal.h"

#include "csp/csp.h"
#include <gs/ftp/types.h>
#if defined(GS_FTP_INTERNAL_USE)
  #undef GS_FTP_INTERNAL_USE
#endif
#define GS_FTP_INTERNAL_USE 1
  #include <gs/ftp/internal/types.h>
#undef  GS_FTP_INTERNAL_USE
#include "csp/csp_endian.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <gs/util/time.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/reboot.h>
#include <linux/reboot.h>

#define CHUNK_DONE      "+"
#define CHUNK_MISSING   "-"

typedef enum {
    FTP_IDLE     = 0,
    FTP_DOWNLOAD = 1,
    FTP_UPLOAD   = 2,
} ftp_state_t;

typedef struct {
    uint8_t state;
    csp_conn_t* conn;
    uint32_t timeout;
    uint32_t interpacket_delay;
} ftp_server_state_t;  

typedef struct {
    FILE* fp;
    FILE* map_fp;
    char* map_buf;

    char filename[GS_FTP_PATH_LENGTH];
    size_t size;
    uint32_t crc32;

    uint16_t chunk_size;
    uint32_t chunks;
} ftp_upload_file_state_t;

typedef struct {
    FILE* fp;

    char filename[GS_FTP_PATH_LENGTH];
    size_t size;
    uint32_t mem_addr;
    uint32_t mem_size;

    uint16_t chunk_size;
    uint32_t chunks;

    bool is_udp;
} ftp_download_file_state_t;

/**
 * Handler thread csp_read() timeout.
 */
static volatile uint32_t timeout_ms = 6000;

/**
 * Delay between download data packets. Used for UDP downloads only in order
 * to prevent packet overruns from the client side.
 */
static volatile uint32_t interpacket_delay_ms = 10;

/**
 * @brief Calculate open file CRC, from @a mem_addr to @a mem_addr + @a size.
 */
static uint32_t calc_crc32(FILE* fp, uint32_t mem_addr, uint32_t size)
{
    uint32_t crc = ftp_crc32_init();
    char buf[512];
    size_t bytes;

    if (fseek(fp, mem_addr, SEEK_SET) != 0) {
        return 0; /* error */
    }
    while (size > 0) {
        size_t to_read = sizeof(buf);
        if (to_read > size) {
            to_read = size;
        }
        bytes = fread(buf, 1, to_read, fp);
        if (bytes == 0) {
            if (feof(fp)) {
                break;
            }
            else {
                return 0; /* error */
            }
        }
        crc = ftp_crc32_update(crc, buf, bytes);
        size -= bytes;
    }

    return ftp_crc32_finalize(crc);
}

/**
 * @brief Handle a done packet (ftp_done_t).
 *        Marks the end of an upload or download transaction. Upon receiving
 *        this packet, the server thread terminates after cleanup.
 * 
 * @details
 *      - Action:
 *        1) Closes open file descriptors (target/map).
 *        2) Removes map file (upload only).
 *        3) Frees memory map (upload only).
 * 
 *      - Error conditions (server-side):
 *        1) Invalid state (FTP_IDLE).
 */
static ftp_ret_t ftp_handler_done(csp_packet_t* request,
                                  ftp_server_state_t* server_state,
                                  void* file_state)
{
    ftp_ret_t ret = FTP_OK;

    if (!request || !server_state || !file_state) {
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_handler_done!\n");
        if (request)
            csp_buffer_free(request);
        return FTP_ERR_INVAL;
    }

    ftp_debug_info("done received\n");

    if (server_state->state != FTP_DOWNLOAD &&
        server_state->state != FTP_UPLOAD) {
        ftp_debug_error("got done but state (%d) is not download or upload\n",
                        server_state->state);
        ret = FTP_ERR_BUSY;
        goto done_out;
    }

    /* Clean up file state. */
    if (server_state->state == FTP_DOWNLOAD) {
        ftp_download_file_state_t* download_state = file_state;
        if (download_state->fp) {
            fclose(download_state->fp);
            download_state->fp = NULL;
        }
    }
    else {
        ftp_upload_file_state_t* upload_state = file_state;
        if (upload_state->fp) {
            fclose(upload_state->fp);
            upload_state->fp = NULL;
        }
        if (upload_state->map_buf) {
            free(upload_state->map_buf);
            upload_state->map_buf = NULL;
        }
        if (upload_state->map_fp) {
            fclose(upload_state->map_fp);
            upload_state->map_fp = NULL;
            /* Remove map.*/
            char map_filename[GS_FTP_PATH_LENGTH + 5];
            snprintf(map_filename, sizeof(map_filename), "%s.map", upload_state->filename);
            remove(map_filename);
        }
    }
  
done_out:

    server_state->state = FTP_IDLE;
    csp_buffer_free(request);
    return ret;
}

/**
 * @brief Handle a download request packet (ftp_download_request_t).
 *        Download sequence 1 of 3.
 * 
 * @details
 *      - Action:
 *        1) Change state to FTP_DOWNLOAD.
 *        2) Open the requested file.
 *        3) Calculate download size min(mem_size, file_size - mem_addr).
 *        4) Calculate CRC32 of the file.
 *        5) Send a download reply (ftp_download_reply_t).
 * 
 *      - Error conditions (server-side):
 *        1) Invalid state (not FTP_IDLE).
 *        2) Failed to open file.
 *        3) Invalid chunk size (0).
 * 
 *      - Error conditions (client-side, all followed by a done packet):
 *        1) Reply size not 1 + sizeof(ftp_download_reply_t).
 *        2) Reply type not FTP_DOWNLOAD_REPLY.
 *        3) Reply return code not FTP_OK.
 *        4) Client file init failed.
 * 
 *      - Done request does not remove the client map (so that it may resume
 *        later), unless it was sent because the download is complete.
 * 
 *      - If successful, expects a status reply (ftp_status_reply_t) for the
 *        next transaction.
 */
static ftp_ret_t ftp_handler_download_request(csp_packet_t* packet,
                                              ftp_server_state_t* server_state,
                                              ftp_download_file_state_t* file_state)
{
    ftp_ret_t ret;

    if (!packet || !server_state || !file_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_handler_download_request!\n");
        return FTP_ERR_INVAL;
    }

    ftp_packet_t* req = (ftp_packet_t*) packet->data;

    ftp_debug_info("download request received\n");

    if (server_state->state != FTP_IDLE) {
        ftp_debug_error("got a download request but state is not idle (%d)", server_state->state);
        ret = FTP_ERR_BUSY;
        goto downreq_out;
    }

    server_state->state = FTP_DOWNLOAD;

    strncpy(file_state->filename, req->down.path, sizeof(file_state->filename));
    file_state->filename[sizeof(file_state->filename) - 1] = 0;

    FILE* fp = fopen(file_state->filename, "r");
    if (!fp) {
        ftp_debug_error("failed to open file %s: %s\n",
                        file_state->filename, strerror(errno));
        ret = FTP_ERR_NOENT;
        goto downreq_out;
    }
    else {
        struct stat statbuf;
        if (fstat(fileno(fp), &statbuf) != 0) {
            ftp_debug_error("failed to get file size for %s: %s\n",
                            file_state->filename, strerror(errno));
            fclose(fp);
            ret = FTP_ERR_IO;
            goto downreq_out;
        }
        file_state->size = statbuf.st_size;

        file_state->chunk_size = csp_ntoh16(req->down.chunk_size);
        if (file_state->chunk_size == 0) {
            ftp_debug_error("invalid chunk size 0\n");
            fclose(fp);
            ret = FTP_ERR_INVAL;
            goto downreq_out;
        }
    
        file_state->mem_size = csp_ntoh32(req->down.mem_size);
        file_state->mem_addr = csp_ntoh32(req->down.mem_addr);
        if (file_state->mem_addr > file_state->size) {
            ftp_debug_error("mem addr %u exceeds file size %zu\n",
                            file_state->mem_addr, file_state->size);
            fclose(fp);
            ret = FTP_ERR_NOSPC;
            goto downreq_out;
        }

        /**
         * Nonzero mem_size = use specified size. Zero mem_size = "don't care".
         */
        file_state->size -= file_state->mem_addr;
        if (file_state->mem_size > 0 && file_state->mem_size < file_state->size)
            file_state->size = file_state->mem_size;
    
        file_state->chunks = (file_state->size + file_state->chunk_size - 1)
                                / file_state->chunk_size;

        ftp_debug_info("file opened successfully, size %zu, chunks %u\n",
                        file_state->size, file_state->chunks);

        ret = FTP_OK;
    }

    file_state->fp = fp;
    file_state->is_udp = ((csp_conn_flags(server_state->conn) & CSP_FRDP) == 0);

downreq_out:

    req->type = FTP_DOWNLOAD_REPLY;
    req->downrep.ret = ret;
    req->downrep.size = csp_hton32(file_state->size);
    if (ret == FTP_OK) {
        /**
         * Real CRC validation is done by the CRC request handler at the end.
         * This "early" CRC is just for the client's info callback reference.
         */
        req->downrep.crc32 = calc_crc32(fp, file_state->mem_addr, file_state->size);
    }
    packet->length = sizeof(req->type) + sizeof(req->downrep);
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending downrep reply\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }

    return ret;
}

static int read_data(FILE* fp, void* data, size_t offset, size_t size)
{
    if (fseek(fp, offset, SEEK_SET) != 0) {
        ftp_debug_error("error seeking to offset %zu: %s\n", offset, strerror(errno));
        return FTP_ERR_IO;
    }

    size_t read = fread(data, 1, size, fp);
    if (read != size) {
        if (feof(fp)) {
            ftp_debug_error("unexpected end of file while reading: expected %zu bytes, got %zu\n",
                            size, read);
        }
        else {
            ftp_debug_error("error reading data: expected %zu bytes, got %zu: %s\n",
                            size, read, strerror(errno));
        }
        return FTP_ERR_IO;
    }

    return FTP_OK;
}

/**
 * @brief Handle a status reply packet (ftp_status_reply_t).
 *        Download sequence 2 of 3.
 * 
 * @details
 *      - Action:
 *        1) Iterate over the entries in the status reply.
 *        2) For each entry, read the indicated chunk from the file and send 
 *           it back in a data packet (ftp_data_t). If reading the chunk fails,
 *           send a data packet with an error code.
 * 
 *      - Error conditions (server-side):
 *        1) Invalid state (not FTP_DOWNLOAD).
 *        2) Invalid chunk number (greater than total chunks).
 *        3) Failed to get a CSP buffer for data packets.
 *        4) Failed to read chunk.
 *        5) Failed to send data packet.
 * 
 *      - Error conditions (client-side, all followed by a done packet):
 *        1) Packet type FTP_ERR_IO (indicating chunk read error).
 *        2) Client-side chunk/map write error.
 *        3) Data packet timeout.
 *        * Invalid chunk number does not terminate the trx but silently drops.
 * 
 *      - Client-side does not check the reply length. The data size is assumed
 *        to follow the chunk size agreed in the download request.
 * 
 *      - Done request does not remove the client map (so that it may resume
 *        later), unless it was sent because the download is complete.
 * 
 *      - If successful, expects a CRC request (ftp_crc_request_t) for the
 *        next transaction.
 */
static ftp_ret_t ftp_handler_status_reply(csp_packet_t* packet,
                                          ftp_server_state_t* server_state,
                                          ftp_download_file_state_t* file_state)
{
    ftp_ret_t ret = FTP_OK;
    ftp_status_reply_t status;

    ftp_debug_info("status reply received\n");

    if (!packet || !server_state || !file_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_handler_status_reply!\n");
        return FTP_ERR_INVAL;
    }

    if (server_state->state != FTP_DOWNLOAD) {
        ftp_debug_error("got a status reply but state is not download (%d)", server_state->state);
        ret = FTP_ERR_BUSY;
        goto statusrep_out;
    }

    ftp_packet_t* rep = (ftp_packet_t*) packet->data;

    /**
     * We'll reuse the request packet to send data. Make a local copy of the
     * status reply.
     */
    memcpy(&status, &rep->statusrep, sizeof(status));
    status.complete = csp_ntoh32(status.complete);
    status.total = csp_ntoh32(status.total);
    status.entries = csp_ntoh16(status.entries);

    ftp_debug_info("status: %u/%u chunks complete, %u entries\n",
                    status.complete, status.total, status.entries);

    for (uint16_t i = 0; i < status.entries; ++i) {
        uint32_t next = csp_ntoh32(status.entry[i].next);
        uint32_t count = csp_ntoh32(status.entry[i].count);
        ftp_debug_info("entry %u: next %u, count %u\n", i, next, count);

        for (uint32_t j = 0; j < count; ++j) {
            uint32_t chunk = next + j;
            if (chunk >= file_state->chunks) {
                ftp_debug_error("invalid chunk %u (total chunks %u)\n",
                                chunk, file_state->chunks);
                ret = FTP_ERR_INVAL;
                goto statusrep_out;
            }

            size_t offset = file_state->mem_addr + chunk * file_state->chunk_size;
            size_t size = chunk == file_state->chunks - 1   ?
                          file_state->size % file_state->chunk_size :
                          file_state->chunk_size;
            if (size == 0) /* If the file size is an exact multiple of the chunk size */
                size = file_state->chunk_size;

            size_t data_packet_size = sizeof(rep->type) + sizeof(rep->data.chunk) + size;

            if (!packet) {
                packet = csp_buffer_get(data_packet_size);
                if (!packet) {
                    ftp_debug_error("error getting CSP buffer for data packet\n");
                    return FTP_ERR_NOMEM;
                }
            }
            rep = (ftp_packet_t*) packet->data;

            ret = read_data(file_state->fp, rep->data.bytes, offset, size);
            if (ret != FTP_OK) {
                ftp_debug_error("error reading chunk %u: %d\n", chunk, ret);
                goto statusrep_out;
            }

            rep->type = FTP_DATA;
            /* Chunk is always LE (original GomSpace code blunder) */
            rep->data.chunk = csp_htole32(chunk);
            packet->length = data_packet_size;
            if (!csp_send(server_state->conn, packet, server_state->timeout)) {
                ftp_debug_error("error sending data packet\n");
                ret = FTP_ERR_IO;
                goto statusrep_out;
            }
            packet = NULL;

            if (file_state->is_udp) {
                /* Inter-packet delay for UDP transfers to avoid overwhelming the client. */
                struct timespec deadline;
                clock_gettime(CLOCK_MONOTONIC, &deadline);
                deadline.tv_sec += server_state->interpacket_delay / 1000;
                deadline.tv_nsec += (server_state->interpacket_delay % 1000) * 1000000;
                if (deadline.tv_nsec >= 1000000000) {
                    deadline.tv_sec += 1;
                    deadline.tv_nsec -= 1000000000;
                }
                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL);
            }
        }
    }

statusrep_out:
    /**
     * There's no ret field in the data packet. The client end seems to 
     * expect an error code in the type field if reading fails...
     */
    if (ret != FTP_OK) {
        if (!packet) {
            packet = csp_buffer_get(sizeof(rep->type));
            if (!packet) {
                ftp_debug_error("error getting CSP buffer for error code packet\n");
                return FTP_ERR_NOMEM;
            }
        }
        rep = (ftp_packet_t*) packet->data;
        /* Overload the type byte with the error code; client decodes as int8_t. */
        rep->type = ret;
        packet->length = sizeof(rep->type);
        if (!csp_send(server_state->conn, packet, server_state->timeout)) {
            ftp_debug_error("error sending error code packet\n");
            csp_buffer_free(packet);
            return FTP_ERR_IO;
        }
    }

    if (packet)
        csp_buffer_free(packet);

    return ret;
}

static int init_upload(const char* filename,
                       uint32_t chunks,
                       FILE** fp_out,
                       FILE** map_fp_out,
                       char** map_buf_out)
{
    if (!filename || !fp_out || !map_fp_out || !map_buf_out)
        return FTP_ERR_INVAL;

    bool new_file = false;
    char map_filename[GS_FTP_PATH_LENGTH + 5];
    snprintf(map_filename, sizeof(map_filename), "%s.map", filename);

    /**
     * Open or create the local target file.
     */
    FILE* fp = fopen(filename, "r+");
    if (!fp) {
        /**
         * Target file does not exist. Create new one.
         */
        fp = fopen(filename, "w+");
        if (!fp) {
            ftp_debug_error("failed to create local target %s: %s\n",
                            filename, strerror(errno));
            return FTP_ERR_NOENT;
        }
        new_file = true;
    }


    char* map_buf = NULL;
    FILE* map_fp  = NULL;

    if (chunks) {
        /**
        * Allocate a bitmap in memory.
        */
        map_buf = malloc(chunks);
        if (!map_buf) {
            ftp_debug_error("failed to allocate map buffer\n");
            fclose(fp);
            return FTP_ERR_NOMEM;
        }
    
        /**
         * Try opening an existing bitmap file.
         */
        map_fp = fopen(map_filename, "r+");
        if (!map_fp) {
            if (!new_file) {
                /**
                 * Error: target file already exists but bitmap doesn't.
                 */
                ftp_debug_error("local target %s exists but bitmap doesn't\n", filename);
                fclose(fp);
                free(map_buf);
                return FTP_ERR_EXISTS;
            }

            /**
             * Bitmap file does not exist. Create new one and clear it.
             */
            map_fp = fopen(map_filename, "w+");
            if (!map_fp) {
                ftp_debug_error("failed to create bitmap file %s: %s\n",
                                map_filename, strerror(errno));
                fclose(fp);
                free(map_buf);
                return FTP_ERR_NOENT;
            }

            for (uint32_t i = 0; i < chunks; ++i) {
                if (fwrite(CHUNK_MISSING, 1, 1, map_fp) != 1) {
                    ftp_debug_error("error clearing bitmap file (chunk %d): %s\n",
                                    i, strerror(errno));
                    fclose(fp);
                    fclose(map_fp);
                    free(map_buf);
                    return FTP_ERR_IO;
                }
                map_buf[i] = *CHUNK_MISSING;
            }

            fflush(map_fp);
            fsync(fileno(map_fp));
        }
        else {
            /**
             * Bitmap file exists. Load it to memory.
             */
            size_t read = fread(map_buf, 1, chunks, map_fp);
            if (read != chunks) {
                ftp_debug_error("error reading bitmap file: expected %u bytes, got %zu\n",
                                chunks, read);
                fclose(fp);
                fclose(map_fp);
                free(map_buf);
                return FTP_ERR_IO;
            }
        }
    }

    /* Populate file state. */
    *fp_out = fp;
    *map_fp_out = map_fp;
    *map_buf_out = map_buf;

    return FTP_OK;
}

/**
 * @brief Handle an upload request packet (ftp_upload_request_t).
 *        Upload sequence 1 of 4.
 * 
 * @details
 *      - Action:
 *        1) Change state to FTP_UPLOAD.
 *        2) Store filename, size and chunk size in file state.
 *        3) Open the target file and its bitmap, allocate bitmap in memory.
 *        4) Send an upload reply (ftp_upload_reply_t).
 * 
 *      - Error conditions (server-side):
 *        1) Invalid state (not FTP_IDLE).
 *        2) Invalid chunk size (0).
 *        4) Failed to send upload reply.
 * 
 *      - Error conditions (client-side, all followed by a done packet):
 *        1) Reply size not 1 + sizeof(ftp_upload_reply_t).
 *        2) Reply type not FTP_UPLOAD_REPLY.
 *        3) Reply return code not FTP_OK.
 * 
 *      - If successful, expects a status request (ftp_status_request_t)
 *        for the next transaction.
 */
static ftp_ret_t ftp_handler_upload_request(csp_packet_t* packet,
                                            ftp_server_state_t* server_state,
                                            ftp_upload_file_state_t* file_state)
{
    ftp_ret_t ret;

    if (!packet || !server_state || !file_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_handler_upload_request!\n");
        return FTP_ERR_INVAL;
    }

    ftp_packet_t* req = (ftp_packet_t*) packet->data;
    
    if (server_state->state != FTP_IDLE) {
        ftp_debug_error("got an upload request but state is not idle (%d)",
                        server_state->state);
        ret = FTP_ERR_BUSY;
        goto upreq_out;
    }

    server_state->state = FTP_UPLOAD;

    /**
     * Store the upload context to file state. Ignore mem_addr for RAM backend.
     */
    file_state->chunk_size = csp_ntoh16(req->up.chunk_size);
    if (file_state->chunk_size == 0) {
        ftp_debug_error("invalid chunk size 0\n");
        ret = FTP_ERR_INVAL;
        goto upreq_out;
    }
    strncpy(file_state->filename, req->up.path, sizeof(file_state->filename));
    file_state->filename[sizeof(file_state->filename) - 1] = 0;
    file_state->size = csp_ntoh32(req->up.size);
    file_state->chunks = (file_state->size + file_state->chunk_size - 1)
                        / file_state->chunk_size;

    ftp_debug_info("upload request received, path %s, size %u, chunk size %u\n",
                    file_state->filename, file_state->size,
                    file_state->chunk_size);

    ret = init_upload(file_state->filename,
                      file_state->chunks,
                      &file_state->fp,
                      &file_state->map_fp,
                      &file_state->map_buf);

upreq_out:

    req->type = FTP_UPLOAD_REPLY;
    req->uprep.ret = ret;
    packet->length = sizeof(req->type) + sizeof(req->uprep);
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending uprep reply\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }

    return ret;
}

/**
 * @brief Handle a status request packet (ftp_status_request_t).
 *        Upload sequence 2 of 4.
 * 
 * @details
 *      - Action:
 *        1) Iterate over the bitmap in memory.
 *        2) Prepare a status reply (ftp_status_reply_t) including count of
 *           complete chunks and entries for contiguous incomplete chunk
 *           sequences.
 *        3) Send the status reply.
 * 
 *      - Error conditions (server-side):
 *        1) Invalid state (not FTP_UPLOAD).
 *        2) Failed to send status reply.
 * 
 *      - Error conditions (client-side, all followed by a done packet):
 *        1) Reply type not FTP_STATUS_REPLY.
 *        2) Reply return code not FTP_OK.
 * 
 *      - Client-side does not check the reply length. Screw it. We're
 *        sending sizeof(ftp_status_reply_t) anyway.
 * 
 *      - If successful, expects a status request (ftp_status_request_t)
 *        for the next transaction.
 */
static ftp_ret_t ftp_handler_status_request(csp_packet_t* packet,
                                            ftp_server_state_t* server_state,
                                            ftp_upload_file_state_t* file_state)
{
    ftp_ret_t ret;

    if (!packet || !server_state || !file_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_handler_status_request!\n");
        return FTP_ERR_INVAL;
    }

    ftp_packet_t* req = (ftp_packet_t*) packet->data;

    ftp_debug_info("status request received\n");

    if (server_state->state != FTP_UPLOAD) {
        ftp_debug_error("got a status request but state is not upload (%d)",
                        server_state->state);
        ret = FTP_ERR_BUSY;
        goto statusreq_out;
    }

    /**
     * Status request has no content. Prepare the reply.
     */
    memset(&req->statusrep, 0, sizeof(req->statusrep));

    /**
     * map_buf in memory has been already populated by init_upload().
     */
    uint16_t entries = 0;
    uint32_t count = 0;
    uint32_t next = 0;
    for (uint32_t i = 0; i < file_state->chunks; ++i) {
        if (file_state->map_buf[i] == *CHUNK_DONE) {
            /**
             * Chunk +.
             * 1) increment complete count.
             * 2) if this marks the end of a contiguous incomplete chunk sequence,
             *    add an entry for the sequence.
             */
            req->statusrep.complete++;
            if (count > 0 && entries < GS_FTP_STATUS_CHUNKS) {
                req->statusrep.entry[entries].next = csp_hton32(next);
                req->statusrep.entry[entries].count = csp_hton32(count);
                entries++;
                count = 0;
            }
        }
        else {
            /**
             * Chunk -. We're in an incomplete chunk sequence entry.
             * 1) If this is the first chunk of the sequence, mark the start index.
             * 2) Increment count for this entry.
             */
            if (count == 0)
                next = i;

            count++;
            
            if (i == file_state->chunks - 1) {
                /* Last chunk. If we're in an incomplete chunk sequence, add an entry for it. */
                if (entries < GS_FTP_STATUS_CHUNKS) {
                    req->statusrep.entry[entries].next = csp_hton32(next);
                    req->statusrep.entry[entries].count = csp_hton32(count);
                    entries++;
                }
            }
        }
    }
    
    req->statusrep.entries  = csp_hton16(entries);
    req->statusrep.complete = csp_hton32(req->statusrep.complete);
    req->statusrep.total    = csp_hton32(file_state->chunks);
    ret = FTP_OK;

statusreq_out:

    req->type = FTP_STATUS_REPLY;
    req->statusrep.ret = ret;
    packet->length = sizeof(req->type) + sizeof(req->statusrep);
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending status reply\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }

    return ret;
}

static int write_data(FILE* fp, const void* data, size_t offset, size_t size)
{
    if (!fp || !data)
        return FTP_ERR_INVAL;

    if (fseek(fp, offset, SEEK_SET) != 0) {
        ftp_debug_error("error seeking to offset %zu: %s\n", offset, strerror(errno));
        return FTP_ERR_IO;
    }

    size_t written = fwrite(data, 1, size, fp);
    if (written != size) {
        ftp_debug_error("error writing chunk %zu: expected to write %zu bytes, actually wrote %zu bytes: %s\n",
                        offset, size, written, strerror(errno));
        return FTP_ERR_IO;
    }

    fflush(fp);
    fsync(fileno(fp));

    return FTP_OK;
}

/**
 * @brief Handle uplink data stage (ftp_data_t).
 *        Upload sequence 3 of 4.
 * 
 * @details
 *      - Action:
 *        1) Write the chunk to the target file at the correct offset.
 * 
 *      - Error conditions (server-side):
 *        1) Invalid state (not FTP_UPLOAD).
 *        2) Chunk number exceeds total chunks.
 *        3) Chunk write error.
 *        4) Bitmap file update error.
 * 
 *      - Error conditions (client-side):
 *        1) None (the client does not expect any reply).
 * 
 *      - If successful, expects a CRC request (ftp_crc_request_t)
 *        for the next transaction.
 */
static ftp_ret_t ftp_handler_data(csp_packet_t* packet,
                                  ftp_server_state_t* server_state,
                                  ftp_upload_file_state_t* file_state)
{
    ftp_ret_t ret;

    if (!packet || !server_state || !file_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_handler_data!\n");
        return FTP_ERR_INVAL;
    }

    ftp_packet_t* req = (ftp_packet_t*) packet->data;

    if (server_state->state != FTP_UPLOAD) {
        ftp_debug_error("got a data packet but state is not upload (%d)", server_state->state);
        ret = FTP_ERR_BUSY;
        goto data_out;
    }

    /* Chunk number is always LE (original GomSpace code blunder). */
    req->data.chunk = csp_letoh32(req->data.chunk);

    if (req->data.chunk >= file_state->chunks) {
        ftp_debug_error("chunk number %u exceeds total chunks %u\n",
                        req->data.chunk, file_state->chunks);
        ret = FTP_ERR_INVAL;
        goto data_out;
    }

    size_t offset = req->data.chunk * file_state->chunk_size;
    size_t size = (offset + file_state->chunk_size > file_state->size)
                    ? (file_state->size - offset)
                    : file_state->chunk_size;
    
    if (packet->length < sizeof(req->type) + sizeof(req->data.chunk) + size) {
        ftp_debug_error("invalid data packet length %u for chunk %u (expected %zu)\n",
                        packet->length, req->data.chunk,
                        sizeof(req->type) + sizeof(req->data.chunk) + size);
        ret = FTP_ERR_INVAL;
        goto data_out;
    }

    if (write_data(file_state->fp, req->data.bytes, offset, size) != FTP_OK) {
        ftp_debug_error("error writing chunk %u\n", req->data.chunk);
        ret = FTP_ERR_IO;
        goto data_out;
    }

    // ftp_debug_info("data packet received: chunk %u, size %zu\n", req->data.chunk, size);

    /* Update bitmap on disk. */
    if (fseek(file_state->map_fp, req->data.chunk, SEEK_SET) != 0) {
        ftp_debug_error("error seeking bitmap file to chunk %u: %s\n",
                        req->data.chunk, strerror(errno));
        ret = FTP_ERR_IO;
        goto data_out;
    }
    if (fwrite(CHUNK_DONE, 1, 1, file_state->map_fp) != 1) {
        ftp_debug_error("error writing bitmap file for chunk %u: %s\n",
                        req->data.chunk, strerror(errno));
        ret = FTP_ERR_IO;
        goto data_out;
    }

    fflush(file_state->map_fp);
    fsync(fileno(file_state->map_fp));

    /* Update bitmap in memory. */
    file_state->map_buf[req->data.chunk] = *CHUNK_DONE;
    ret = FTP_OK;

data_out:

    /* No reply for data packets (always free). */
    csp_buffer_free(packet);
    return ret;
}

/**
 * @brief Handle a CRC request packet (ftp_crc_request_t).
 *        Download sequence 3 of 3, Upload sequence 4 of 4.
 * 
 * @details
 *      - Action:
 *        1) If in upload state, calculate CRC32 of the whole file.
 *        2) If in download state, calculate CRC32 of the file considering the
 *           offset and size specified in the download request.
 *        3) Send CRC reply (ftp_crc_reply_t).
 * 
 *      - Error conditions (server-side):
 *        1) Invalid state (neither FTP_DOWNLOAD nor FTP_UPLOAD).
 *        2) I/O error.
 *        3) Failed to send CRC reply.
 * 
 *      - Error conditions (client-side, all followed by a done packet):
 *        1) Packet type not FTP_CRC_REPLY.
 *        2) Reply size not 1 + sizeof(ftp_crc_reply_t).
 *        3) Reply return code not FTP_OK.
 *        4) Client-side CRC mismatch.
 * 
 *      - Whether upload or download, this handler only sends the local file
 *        CRC. The comparison is always done on the client side.
 */
static ftp_ret_t ftp_handler_crc_request(csp_packet_t* packet,
                                         ftp_server_state_t* server_state,
                                         void* file_state)
{
    ftp_ret_t ret;

    if (!packet || !server_state || !file_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null arguments to ftp_handler_crc_request!\n");
        return FTP_ERR_INVAL;
    }

    ftp_packet_t* req = (ftp_packet_t*) packet->data;

    ftp_debug_info("CRC request received\n");

    if (server_state->state != FTP_DOWNLOAD && server_state->state != FTP_UPLOAD) {
        ftp_debug_error("got a CRC request but state is not download or upload (%d)", server_state->state);
        ret = FTP_ERR_BUSY;
        goto crc_out;
    }

    /**
     * For download, consider the offset and size specified in the request.
     * For upload, it's always the whole file.
     */
    if (server_state->state == FTP_DOWNLOAD) {
        ftp_download_file_state_t* down_file_state = (ftp_download_file_state_t*) file_state;
        req->crcrep.crc = csp_hton32(calc_crc32(down_file_state->fp,
                                                down_file_state->mem_addr,
                                                down_file_state->size));
    }
    else {
        ftp_upload_file_state_t* up_file_state = (ftp_upload_file_state_t*) file_state;
        req->crcrep.crc = csp_hton32(calc_crc32(up_file_state->fp, 0, up_file_state->size));
    }
    
    ret = FTP_OK;

crc_out:
    req->type = FTP_CRC_REPLY;
    req->crcrep.ret = ret;
    packet->length = sizeof(req->type) + sizeof(req->crcrep);
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending error code packet\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }
    return ret;
}


#define FTP_MAX_LOG_ENTRIES 100

typedef struct {
    uint32_t count;
    ftp_log_entry_t entries[FTP_MAX_LOG_ENTRIES];
} ftp_result_log_t;

static ftp_result_log_t result_log;

void ftp_append_result(ftp_ret_t ret, uint8_t type)
{
    int index = result_log.count % FTP_MAX_LOG_ENTRIES;

    /* Pull the handler's most-recent debug message into the entry's
     * message field BEFORE we emit any debug output of our own (which
     * would overwrite the per-thread capture buffer). */
    const char* msg = ftp_debug_get_last_message();
    strncpy(result_log.entries[index].message, msg,
            sizeof(result_log.entries[index].message) - 1);
    result_log.entries[index].message[sizeof(result_log.entries[index].message) - 1] = 0;
    // struct timespec ts;
    // clock_gettime(CLOCK_REALTIME, &ts);
    result_log.entries[index].tag.csp_time = gs_time_uptime();
    result_log.entries[index].tag.index    = (uint16_t)result_log.count;
    result_log.entries[index].tag.ret      = ret;
    result_log.entries[index].tag.type     = type;
    result_log.count++;
}

static ftp_ret_t ftp_ext_handler_log_request(csp_packet_t* packet,
                                             ftp_server_state_t* server_state)
{
    if (!packet || !server_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_ext_handler_log_request!\n");
        return FTP_ERR_INVAL;
    }

    ftp_ext_packet_t* req = (ftp_ext_packet_t*) packet->data;

    ftp_debug_info("log request received\n");

    uint16_t index = csp_ntoh16(req->log.index);
    uint16_t reply_size = sizeof(req->type) + sizeof(req->logrep.ret);
    uint32_t available = (result_log.count < FTP_MAX_LOG_ENTRIES)
                         ? result_log.count : FTP_MAX_LOG_ENTRIES;

    switch (req->log.action) {
    case 0:
        /* Get the @a index most recent log tags, oldest-first. */
        if (index > available)
            index = available;
        if (index > FTP_EXT_LOG_REPLY_MAX_TAGS)
            index = FTP_EXT_LOG_REPLY_MAX_TAGS;
        for (uint16_t i = 0; i < index; ++i) {
            uint16_t log_index = (result_log.count + FTP_MAX_LOG_ENTRIES - index + i) % FTP_MAX_LOG_ENTRIES;
            req->logrep.tags[i].index    = csp_hton16(result_log.entries[log_index].tag.index);
            req->logrep.tags[i].csp_time = csp_hton32(result_log.entries[log_index].tag.csp_time);
            req->logrep.tags[i].type     = result_log.entries[log_index].tag.type;
            req->logrep.tags[i].ret      = result_log.entries[log_index].tag.ret;
        }
        req->logrep.ret = FTP_OK;
        req->logrep.count = (uint16_t)result_log.count;
        reply_size += sizeof(req->logrep.count) + index * sizeof(ftp_log_tag_t);
        break;

    case 1: {
        req->logrep.count = (uint16_t)result_log.count;
        reply_size += sizeof(req->logrep.count);

        ftp_log_entry_t* src = NULL;
        for (uint32_t i = 0; i < available; i++) {
            if (result_log.entries[i].tag.index == index) {
                src = &result_log.entries[i];
                break;
            }
        }
        if (!src) {
            req->logrep.ret = FTP_ERR_NOSPC;
            /* no payload -- entry not in the ring */
        }
        else {
            req->logrep.log.tag.index    = csp_hton16(src->tag.index);
            req->logrep.log.tag.csp_time = csp_hton32(src->tag.csp_time);
            req->logrep.log.tag.type     = src->tag.type;
            req->logrep.log.tag.ret      = src->tag.ret;
            strncpy(req->logrep.log.message, src->message,
                    sizeof(req->logrep.log.message) - 1);
            req->logrep.log.message[sizeof(req->logrep.log.message) - 1] = 0;

            req->logrep.ret = FTP_OK;
            reply_size += sizeof(req->logrep.log);
        }
    }
    break;

    case 2:
        /* clears log */
        memset(&result_log, 0, sizeof(result_log));
        req->logrep.ret = FTP_OK;
        req->logrep.count = 0;
        reply_size += sizeof(req->logrep.count);
        break;

    default:
        ftp_debug_error("invalid log action %u\n", req->log.action);
        req->logrep.ret = FTP_ERR_INVAL;
    }

    req->type = FTP_EXT_LOG_REPLY;
    packet->length = reply_size;
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending log reply\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }

    return FTP_OK;
}

static ftp_ret_t ftp_ext_shell_cmd(csp_packet_t* packet,
                                   ftp_server_state_t* server_state)
{
    if (!packet || !server_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_ext_shell_cmd!\n");
        return FTP_ERR_INVAL;
    }

    ftp_ext_packet_t* req = (ftp_ext_packet_t*) packet->data;
    req->shell.cmd[sizeof(req->shell.cmd) - 1] = 0;

    ftp_debug_info("shell command request received: %s\n", req->shell.cmd);

    ftp_ret_t ret = FTP_OK;
    int32_t   sysret = -1;

    pid_t pid = fork();
    if (pid < 0) {
        ftp_debug_error("fork failed: %s\n", strerror(errno));
        ret = FTP_ERR_IO;
    }
    else if (pid == 0) {
        /* Child: replace with /bin/sh. The execl below should never return. */
        execl("/bin/sh", "sh", "-c", req->shell.cmd, (char*) NULL);
        ftp_debug_error("execl failed: %s\n", strerror(errno));
        _exit(127); /* "command not found" convention */
    }
    else {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            ftp_debug_error("waitpid failed: %s\n", strerror(errno));
            ret = FTP_ERR_IO;
        }
        else if (WIFEXITED(status))
            sysret = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            sysret = 128 + WTERMSIG(status);
        else
            ret = FTP_ERR_IO; /* stopped/continued -- not what we waited for */
    }

    req->type = FTP_EXT_SHELL_REPLY;
    req->shellrep.ret    = ret;
    req->shellrep.sysret = csp_hton32((uint32_t)sysret);
    packet->length = sizeof(req->type) + sizeof(req->shellrep);
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending shell command reply\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }

    return ret;
}

static ftp_ret_t ftp_ext_handler_ram_write(csp_packet_t* packet,
                                           ftp_server_state_t* server_state)
{
    ftp_ret_t ret;

    if (!packet || !server_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_ext_handler_ram_write!\n");
        return FTP_ERR_INVAL;
    }

    ftp_ext_packet_t* req = (ftp_ext_packet_t*) packet->data;

    ftp_debug_info("RAM write request received\n");

    uint32_t addr = csp_ntoh32(req->write.addr);
    uint32_t size = csp_ntoh32(req->write.size);
    if (size > sizeof(req->write.bytes)) {
        ftp_debug_error("RAM write size %u exceeds max %zu\n",
                        size, sizeof(req->write.bytes));
        ret = FTP_ERR_NOSPC;
    }
    else {
        memcpy((void*)(uintptr_t)addr, req->write.bytes, size);
        ret = FTP_OK;
    }

    req->type = FTP_EXT_RAM_WRITE_REPLY;
    req->writerep.ret = ret;
    packet->length = sizeof(req->type) + sizeof(req->writerep);
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending RAM write reply\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }

    return ret;
}

static ftp_ret_t ftp_ext_handler_ram_read(csp_packet_t* packet,
                                          ftp_server_state_t* server_state)
{
    if (!packet || !server_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_ext_handler_ram_read!\n");
        return FTP_ERR_INVAL;
    }

    ftp_ext_packet_t* req = (ftp_ext_packet_t*) packet->data;

    ftp_debug_info("RAM read request received\n");

    uint32_t addr = csp_ntoh32(req->read.addr);
    uint32_t size = csp_ntoh32(req->read.size);
    if (size > sizeof(req->readrep.bytes)) {
        ftp_debug_error("RAM read size %u exceeds max %zu\n",
                        size, sizeof(req->readrep.bytes));
        size = sizeof(req->readrep.bytes);
    }
    
    memcpy(req->readrep.bytes, (const void*)(uintptr_t)addr, size);

    req->type = FTP_EXT_RAM_READ_REPLY;
    packet->length = sizeof(req->type) + size;
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending RAM read reply\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }

    return FTP_OK;
}

static ftp_ret_t ftp_ext_handler_kill(csp_packet_t* packet,
                                      ftp_server_state_t* server_state)
{
    ftp_ret_t ret;

    if (!packet || !server_state) {
        if (packet)
            csp_buffer_free(packet);
        ftp_debug_error("(CRITICAL) Null argument(s) to ftp_ext_handler_kill!\n");
        return FTP_ERR_INVAL;
    }

    ftp_ext_packet_t* req = (ftp_ext_packet_t*) packet->data;
    uint32_t magic = csp_ntoh32(req->kill.magic);
    int status = csp_ntoh32(req->kill.status);

    ftp_debug_info("kill request received\n");

    if (magic == FTP_EXT_KILL_MAGIC_EXIT) {
        _exit(status);
        ret = FTP_ERR_IO;
    }
    else if (magic == FTP_EXT_KILL_MAGIC_REBOOT) {
        ret = reboot(LINUX_REBOOT_CMD_RESTART);
        if (ret != 0)
            ftp_debug_error("error rebooting system: returned %d\n", ret);
    }
    else {
        ftp_debug_error("kill request with unrecognized magic 0x%08x\n", magic);
        ret = FTP_ERR_INVAL;
    }

    req->type = FTP_EXT_KILL_REPLY;
    req->killrep.ret = (int32_t)csp_hton32((uint32_t)ret);
    packet->length = sizeof(req->type) + sizeof(req->killrep);
    if (!csp_send(server_state->conn, packet, server_state->timeout)) {
        ftp_debug_error("error sending kill reply\n");
        csp_buffer_free(packet);
        return FTP_ERR_IO;
    }

    return ret;
}

static void cleanup_file_states(ftp_upload_file_state_t* upload_file_state,
                                ftp_download_file_state_t* download_file_state)
{
    if (upload_file_state) {
        if (upload_file_state->fp)
            fclose(upload_file_state->fp);
        if (upload_file_state->map_fp)
            fclose(upload_file_state->map_fp);
        if (upload_file_state->map_buf)
            free(upload_file_state->map_buf);
    }

    if (download_file_state) {
        if (download_file_state->fp)
            fclose(download_file_state->fp);
    }
}

void* ftp_handler_thread(void* arg)
{
    ftp_ret_t ret;
    uint8_t   type;
    ftp_server_state_t server_state;
    ftp_upload_file_state_t upload_file_state;
    ftp_download_file_state_t download_file_state;
    ftp_ext_packet_t* ext_req;

    server_state.state = FTP_IDLE;   
    server_state.conn = (csp_conn_t*) arg;
    server_state.timeout = timeout_ms;
    server_state.interpacket_delay = interpacket_delay_ms;

    memset(&upload_file_state, 0, sizeof(upload_file_state));
    memset(&download_file_state, 0, sizeof(download_file_state));

    while (1) {
        csp_packet_t* packet = csp_read(server_state.conn, server_state.timeout);
        if (!packet) {
            ftp_debug_error("[tid %lu] read timeout\n", pthread_self());
            ret = FTP_ERR_TIMEOUT;
            type = FTP_TYPE_NONE;
            goto handler_thread_out;
        }
    
        type = ((ftp_packet_t*) packet->data)->type;
        switch (type) {
        case FTP_DOWNLOAD_REQUEST:  /* Download sequence 1 of 3. */
            ret = ftp_handler_download_request(packet, &server_state, &download_file_state);
            break;

        case FTP_STATUS_REPLY:      /* Download sequence 2 of 3. */
            ret = ftp_handler_status_reply(packet, &server_state, &download_file_state);
            break;

        case FTP_UPLOAD_REQUEST:    /* Upload sequence 1 of 3. */
            ret = ftp_handler_upload_request(packet, &server_state, &upload_file_state);
            break;

        case FTP_STATUS_REQUEST:    /* Upload sequence 2 of 3. */
            ret = ftp_handler_status_request(packet, &server_state, &upload_file_state);
            break;

        case FTP_DATA:              /* Upload sequence 3 of 4. */
            ret = ftp_handler_data(packet, &server_state, &upload_file_state);
            break;

        case FTP_CRC_REQUEST:       /* Download sequence 3 of 3, Upload sequence 4 of 4. */
        {
            void *file_state = (server_state.state == FTP_DOWNLOAD)
                            ? (void*) &download_file_state
                            : (void*) &upload_file_state;
            ret = ftp_handler_crc_request(packet, &server_state, file_state);
        }
        break;

        case FTP_DONE:              /* Cleanup. */
        {
            void* file_state = (server_state.state == FTP_DOWNLOAD)
                                ? (void*) &download_file_state
                                : (void*) &upload_file_state;
            ret = ftp_handler_done(packet, &server_state, file_state);
            goto handler_thread_out;
        }
        break;

        case FTP_EXT_LOG_REQUEST:
            ret = ftp_ext_handler_log_request(packet, &server_state);
            goto handler_thread_out;

        case FTP_EXT_TIMEOUT_SET:
            ftp_debug_info("timeout set request received\n");
            ext_req = (ftp_ext_packet_t*) packet->data;
            timeout_ms = csp_ntoh32(ext_req->timeout.timeout);
            uint32_t pkt_delay = csp_ntoh32(ext_req->timeout.interpacket_delay);
            interpacket_delay_ms = pkt_delay > 10000 ? 10000 : pkt_delay;
            /* fallthrough */
        case FTP_EXT_TIMEOUT_GET:
            if (type == FTP_EXT_TIMEOUT_GET)
                ftp_debug_info("timeout get request received\n");
            ext_req = (ftp_ext_packet_t*) packet->data;
            ext_req->type = FTP_EXT_TIMEOUT_REPLY;
            ext_req->timeoutrep.timeout = csp_hton32(timeout_ms);
            ext_req->timeoutrep.interpacket_delay = csp_hton32(interpacket_delay_ms);
            packet->length = sizeof(ext_req->type) + sizeof(ext_req->timeoutrep);
            if (!csp_send(server_state.conn, packet, server_state.timeout)) {
                ftp_debug_error("error sending timeout reply\n");
                csp_buffer_free(packet);
                ret = FTP_ERR_IO;
            }
            else
                ret = FTP_OK;
            goto handler_thread_out;

        case FTP_EXT_SHELL_CMD:
            ret = ftp_ext_shell_cmd(packet, &server_state);
            goto handler_thread_out;

        case FTP_EXT_CSP_BUFS:
            ftp_debug_info("CSP buffers request received\n");
            ext_req = (ftp_ext_packet_t*) packet->data;
            ext_req->type = FTP_EXT_CSP_BUFS_REPLY;
            ext_req->buffersrep.count = csp_hton32(csp_buffer_remaining());
            ext_req->buffersrep.size  = csp_hton32(csp_buffer_size());
            packet->length = sizeof(ext_req->type) + sizeof(ext_req->buffersrep);
            if (!csp_send(server_state.conn, packet, server_state.timeout)) {
                ftp_debug_error("error sending CSP buffers reply\n");
                csp_buffer_free(packet);
                ret = FTP_ERR_IO;
            }
            else
                ret = FTP_OK;
            goto handler_thread_out;

        case FTP_EXT_RAM_WRITE:
            ret = ftp_ext_handler_ram_write(packet, &server_state);
            goto handler_thread_out;

        case FTP_EXT_RAM_READ:
            ret = ftp_ext_handler_ram_read(packet, &server_state);
            goto handler_thread_out;

        case FTP_EXT_KILL:
            ret = ftp_ext_handler_kill(packet, &server_state);
            goto handler_thread_out;

        case FTP_EXT_PING:
            /* echo */
            ftp_debug_info("ping received\n");
            ext_req = (ftp_ext_packet_t*) packet->data;
            ext_req->type = FTP_EXT_PING_REPLY;
            if (!csp_send(server_state.conn, packet, server_state.timeout)) {
                ftp_debug_error("error sending ping echo\n");
                csp_buffer_free(packet);
                ret = FTP_ERR_IO;
            }
            else
                ret = FTP_OK;
            goto handler_thread_out;

        default:
            ftp_debug_error("received packet with unknown type %u\n", type);
            csp_buffer_free(packet);
            ret = FTP_ERR_NOTSUP;
            goto handler_thread_out;
        }
        if (ret != FTP_OK)
            ftp_append_result(ret, type);
    }

handler_thread_out:

    csp_close(server_state.conn);
    /* cleanup for timeout exit */
    cleanup_file_states(&upload_file_state, &download_file_state);

    if (ret != FTP_OK)
        ftp_append_result(ret, type);

    pthread_exit(NULL);
    return NULL;
}

int ftp_server_start(void)
{
    csp_socket_t* socket = csp_socket(CSP_SO_NONE);
    if (socket == NULL) {
        ftp_debug_error("error creating CSP socket\n");
        return -1;
    }

    if (csp_bind(socket, 9) != CSP_ERR_NONE) {
        ftp_debug_error("error binding CSP socket\n");
        csp_close(socket);
        return -1;
    }

    if (csp_listen(socket, 10) != CSP_ERR_NONE) {
        ftp_debug_error("error listening on CSP socket\n");
        csp_close(socket);
        return -1;
    }

    while (1) {
        csp_conn_t* conn = csp_accept(socket, CSP_MAX_DELAY);
        if (conn == NULL) {
            ftp_debug_error("error accepting connection\n");
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, ftp_handler_thread, conn) != 0) {
            ftp_debug_error("error creating thread for new connection\n");
            csp_close(conn);
            continue;
        }
        /* Connection closed by handler thread */
        pthread_detach(tid);
    }

    /* should never reach here */
    csp_close(socket);
    return 0;
}
