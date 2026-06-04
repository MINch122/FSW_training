/**
 * @file meow_file.h
 * @author ryu@yonsei.ac.kr
 * @brief MEOW core: file I/O and operations.
 * 2026 Astrodynamics & Control Lab. Yonsei Univ.
 */
#ifndef _MEOW_FILE_H_
#define _MEOW_FILE_H_

#include <stdint.h>
#include <stddef.h>
#include <time.h>


/* return codes */
typedef enum {
    MEOW_FILE_OK        = 0,
    MEOW_FILE_ERR_NULL  = -1,
    MEOW_FILE_ERR_OPEN  = -2,
    MEOW_FILE_ERR_READ  = -3,
    MEOW_FILE_ERR_WRITE = -4,
    MEOW_FILE_ERR_STAT  = -5,
    MEOW_FILE_ERR_OP    = -6, /* unlink / rename / mkdir / truncate */
} meow_file_ret_t;


#define MEOW_FILE_PREVIEW_LEN   16

#define COPY_BUF_SIZE           2048  /* internal meow_file_copy buffer */

/* meow_file_write flags (combinable with |) */
#define MEOW_FILE_WRITE_APPEND  0x01  /* append to the end and ignore offset */
#define MEOW_FILE_WRITE_FSYNC   0x02  /* flush to storage before returning   */


typedef struct {
    size_t   size;
    time_t   mtime;     /* last modification                 */
    time_t   ctime;     /* last status change (not creation) */
    uint32_t mode;      /* permission bits, e.g. 0755        */
    int      is_dir;
    uint8_t  preview[MEOW_FILE_PREVIEW_LEN];
    size_t   preview_len;
} meow_file_stat_t;

typedef struct {
    uint64_t total_bytes;
    uint64_t avail_bytes;  /* available to non-root */
} meow_disk_stat_t;


/* -------------------------------------------------------------------------
 * I/O
 * ---------------------------------------------------------------------- */

/**
 * @brief Read up to buf_size bytes from path starting at offset.
 *
 * @details
 *      - Uses O_RDONLY + lseek + read. offset = 0 reads from the start.
 *      - *bytes_read == 0 means the file is empty or offset is at or
 *            beyond EOF.
 *
 * @param path              File to read.
 * @param offset            Byte offset from the start of the file.
 * @param[out] buf          Destination buffer.
 * @param buf_size          Capacity of @a buf in bytes.
 * @param[out] bytes_read   Number of bytes actually read.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if a pointer argument is NULL.
 *          MEOW_FILE_ERR_OPEN if the open() fails.
 *          MEOW_FILE_ERR_READ if lseek() or read() fails.
 */
int meow_file_read(const char* path,
                   size_t offset,
                   uint8_t* buf,
                   size_t buf_size,
                   size_t* bytes_read);

/**
 * @brief Write data to a file, creating it if necessary.
 *
 * @details
 *      - Uses O_WRONLY + O_CREAT + optional O_APPEND, plus lseek if not appending.
 *      - If O_APPEND is not set, data is written at the specified offset.
 *      - If MEOW_FILE_WRITE_FSYNC is set, fsync() is called for successful writes.
 *      - Partial writes (written > 0 but < size) are considered errors, returning
 *            MEOW_FILE_ERR_WRITE (which is extremely rare on a local filesystem).
 *
 * @param path    Destination file path.
 * @param data    Bytes to write.
 * @param size    Number of bytes to write.
 * @param offset  Byte offset from the start of the file. Ignored when APPEND is set.
 * @param flags   Combination of MEOW_FILE_WRITE_* flags (not the oflag to open()).
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if a pointer argument is NULL.
 *          MEOW_FILE_ERR_OPEN if the open() fails.
 *          MEOW_FILE_ERR_WRITE if lseek() or write() fails.
 */
int meow_file_write(const char* path,
                    const uint8_t* data,
                    size_t size,
                    size_t offset,
                    int flags);

/* -------------------------------------------------------------------------
 * File operations
 * ---------------------------------------------------------------------- */

/**
 * @brief Delete a file.
 * 
 * @details
 *      - Uses unlink() instead of remove(). Directories are never deleted.
 *
 * @param path  File to remove.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if @a path is NULL.
 *          MEOW_FILE_ERR_OP if unlink() fails.
 */
int meow_file_remove(const char* path);

/**
 * @brief Copy src to dst, preserving permission bits.
 *
 * @details
 *      - Performs an inode check before copying; returns MEOW_FILE_ERR_OP
 *            if src and dst resolve to the same file.
 *      - On write failure, the partial destination file is removed.
 *      - Does not preserve ownership or timestamps.
 *      - Copies even if @a dst already exists, truncating it first, as
 *            normal cp would. To check for existence, stat @a dst first.
 *
 * @param src  Source file path.
 * @param dst  Destination file path.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if a pointer argument is NULL.
 *          MEOW_FILE_ERR_STAT if src cannot be stat'd.
 *          MEOW_FILE_ERR_OPEN if src or dst cannot be opened.
 *          MEOW_FILE_ERR_READ if reading src fails mid-copy.
 *          MEOW_FILE_ERR_WRITE if writing dst fails mid-copy.
 *          MEOW_FILE_ERR_OP if src and dst are the same file.
 */
int meow_file_copy(const char* src, const char* dst);

/**
 * @brief Move src to dst.
 *
 * @details
 *      - Tries rename() first. On EXDEV (cross-filesystem), falls back to
 *            meow_file_copy() followed by unlink(src).
 *
 * @param src  Source file path.
 * @param dst  Destination file path.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if a pointer argument is NULL.
 *          MEOW_FILE_ERR_OP if rename() fails for a reason other than EXDEV.
 *          See meow_file_copy() return codes for cross-filesystem fallback.
 */
int meow_file_move(const char* src, const char* dst);

/* -------------------------------------------------------------------------
 * Info
 * ---------------------------------------------------------------------- */

/**
 * @brief Stat a file or directory, including a short content preview.
 *
 * @details
 *      - For regular files, fills out->preview with up to MEOW_FILE_PREVIEW_LEN
 *            bytes from the start of the file. out->preview_len is 0 for
 *            directories or if the preview read fails.
 *
 * @param path      File or directory to stat.
 * @param[out] out  Filled on MEOW_FILE_OK.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if a required pointer argument is NULL.
 *          MEOW_FILE_ERR_STAT if stat() fails.
 */
int meow_file_stat(const char* path, meow_file_stat_t* out);

/* -------------------------------------------------------------------------
 * Extras
 * ---------------------------------------------------------------------- */

/**
 * @brief Zero out file contents without removing it (truncate to length 0).
 * 
 * @details
 *        - This is useful for clearing files without tempering with other
 *              tasks' open file descriptors (e.g., logs).
 *
 * @param path  File to truncate.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if path is NULL.
 *          MEOW_FILE_ERR_OP if truncate() fails.
 */
int meow_file_truncate(const char* path);

/**
 * @brief Read the last n_bytes of a file into buf.
 *
 * @details
 *      - Seeks to max(0, file_size − min(n_bytes, buf_size)) before reading.
 *      - Useful for tailing log files without reading the entire file first.
 *
 * @param path              File to read.
 * @param n_bytes           Number of bytes to read from the end.
 * @param[out] buf          Destination buffer.
 * @param buf_size          Capacity of buf in bytes.
 * @param[out] bytes_read   Number of bytes actually read.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if a required pointer argument is NULL.
 *          MEOW_FILE_ERR_OPEN if the file cannot be opened.
 *          MEOW_FILE_ERR_READ if lseek or read fails.
 */
int meow_file_tail(const char* path,
                   size_t n_bytes,
                   uint8_t* buf,
                   size_t buf_size,
                   size_t* bytes_read);

/**
 * @brief Compute the CRC-32 (IEEE 802.3) of an entire file.
 *
 * @param path          File to checksum.
 * @param[out] crc_out  CRC-32 value on MEOW_FILE_OK.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if a required pointer argument is NULL.
 *          MEOW_FILE_ERR_OPEN if the file cannot be opened.
 *          MEOW_FILE_ERR_READ if a read error occurs during the pass.
 */
int meow_file_checksum(const char* path, uint32_t* crc_out);

/**
 * @brief Query filesystem space for the filesystem containing path.
 *
 * @param path      Any path on the target filesystem.
 * @param[out] out  Filled on MEOW_FILE_OK.
 * @return  MEOW_FILE_OK on success.
 *          MEOW_FILE_ERR_NULL if a required pointer argument is NULL.
 *          MEOW_FILE_ERR_STAT if statvfs() fails.
 */
int meow_disk_stat(const char* path, meow_disk_stat_t* out);

/**
 * @brief Return the system errno from the most recent failed meow_file_* call.
 *        Call this only when a meow_file_* function returns an error.
 */
int meow_file_errno(void);

#endif /* MEOW_FILE_H */
