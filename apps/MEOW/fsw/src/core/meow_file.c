/**
 * @file meow_file.c
 * @author ryu@yonsei.ac.kr
 * @brief MEOW core: file I/O and operations.
 * 2026 Astrodynamics & Control Lab. Yonsei Univ.
 */
#include "meow_file.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/statvfs.h>

static int last_errno = 0;

int meow_file_errno(void) { return last_errno; }

/* -------------------------------------------------------------------------
 * I/O
 * ---------------------------------------------------------------------- */

int meow_file_read(const char* path,
                   size_t offset,
                   uint8_t* buf,
                   size_t buf_size,
                   size_t* bytes_read)
{
    if (!path || !buf || !bytes_read)
        return MEOW_FILE_ERR_NULL;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_OPEN;
    }

    if (offset > 0 && lseek(fd, (off_t)offset, SEEK_SET) < 0) {
        last_errno = errno;
        close(fd);
        return MEOW_FILE_ERR_READ;
    }

    ssize_t n = read(fd, buf, buf_size);
    int saved = errno;
    close(fd);
    if (n < 0) {
        last_errno = saved;
        return MEOW_FILE_ERR_READ;
    }

    *bytes_read = (size_t)n;
    return MEOW_FILE_OK;
}

int meow_file_write(const char* path,
                    const uint8_t* data,
                    size_t size,
                    size_t offset,
                    int flags)
{
    int ret;

    if (!path || !data)
        return MEOW_FILE_ERR_NULL;

    int open_flags = O_WRONLY | O_CREAT;
    if (flags & MEOW_FILE_WRITE_APPEND)
        open_flags |= O_APPEND;

    int fd = open(path, open_flags, 0644);
    if (fd < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_OPEN;
    }

    if (!(flags & MEOW_FILE_WRITE_APPEND) && offset > 0) {
        if (lseek(fd, (off_t)offset, SEEK_SET) < 0) {
            last_errno = errno;
            close(fd);
            return MEOW_FILE_ERR_WRITE;
        }
    }

    ssize_t written = write(fd, data, size);

    if (written == (ssize_t)size) {
        if (flags & MEOW_FILE_WRITE_FSYNC)
            fsync(fd);
        ret = MEOW_FILE_OK;
    }
    else {
        last_errno = errno;
        ret = MEOW_FILE_ERR_WRITE;
    }

    close(fd);
    return ret;
}

/* -------------------------------------------------------------------------
 * File operations
 * ---------------------------------------------------------------------- */

int meow_file_remove(const char* path)
{
    if (!path)
        return MEOW_FILE_ERR_NULL;

    if (unlink(path) < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_OP;
    }

    return MEOW_FILE_OK;
}

int meow_file_copy(const char* src, const char* dst)
{
    if (!src || !dst)
        return MEOW_FILE_ERR_NULL;

    struct stat ss, ds;
    if (stat(src, &ss) < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_STAT;
    }

    if (stat(dst, &ds) == 0 &&
        ss.st_ino == ds.st_ino && ss.st_dev == ds.st_dev)
        return MEOW_FILE_ERR_OP;  /* src and dst are the same file */

    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_OPEN;
    }

    int dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, ss.st_mode & 0777);
    if (dst_fd < 0) {
        last_errno = errno;
        close(src_fd);
        return MEOW_FILE_ERR_OPEN;
    }

    uint8_t buf[COPY_BUF_SIZE];
    ssize_t n;
    int ret = MEOW_FILE_OK;

    while ((n = read(src_fd, buf, sizeof(buf))) > 0)
        if (write(dst_fd, buf, (size_t)n) != n) {
            last_errno = errno;
            ret = MEOW_FILE_ERR_WRITE;
            break;
        }

    if (n < 0) {
        last_errno = errno;
        ret = MEOW_FILE_ERR_READ;
    }

    close(src_fd);
    close(dst_fd);
    if (ret != MEOW_FILE_OK)
        unlink(dst);  /* remove partial destination */

    return ret;
}

int meow_file_move(const char* src, const char* dst)
{
    if (!src || !dst)
        return MEOW_FILE_ERR_NULL;

    if (rename(src, dst) == 0)
        return MEOW_FILE_OK;

    if (errno != EXDEV) {
        last_errno = errno;
        return MEOW_FILE_ERR_OP;
    }

    /* Copy-remove fallback when rename() fails. */
    int ret = meow_file_copy(src, dst);
    if (ret != MEOW_FILE_OK)
        return ret;

    if (unlink(src) < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_OP;
    }

    return MEOW_FILE_OK;
}

/* -------------------------------------------------------------------------
 * Info
 * ---------------------------------------------------------------------- */

int meow_file_stat(const char* path, meow_file_stat_t* out)
{
    if (!path || !out)
        return MEOW_FILE_ERR_NULL;

    struct stat st;
    if (stat(path, &st) < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_STAT;
    }

    out->size    = (size_t)st.st_size;
    out->mtime   = st.st_mtime;
    out->ctime   = st.st_ctime;
    out->mode    = (uint32_t)(st.st_mode & 07777);
    out->is_dir  = S_ISDIR(st.st_mode);

    out->preview_len = 0;
    if (S_ISREG(st.st_mode)) {
        int fd = open(path, O_RDONLY);
        if (fd >= 0) {
            ssize_t n = read(fd, out->preview, MEOW_FILE_PREVIEW_LEN);
            if (n > 0)
                out->preview_len = (size_t)n;
            close(fd);
        }
    }

    return MEOW_FILE_OK;
}

/* -------------------------------------------------------------------------
 * Extras
 * ---------------------------------------------------------------------- */

int meow_file_truncate(const char* path)
{
    if (!path)
        return MEOW_FILE_ERR_NULL;

    if (truncate(path, 0) < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_OP;
    }

    return MEOW_FILE_OK;
}

int meow_file_tail(const char* path,
                   size_t n_bytes,
                   uint8_t* buf,
                   size_t buf_size,
                   size_t* bytes_read)
{
    if (!path || !buf || !bytes_read)
        return MEOW_FILE_ERR_NULL;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_OPEN;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size < 0) {
        last_errno = errno;
        close(fd);
        return MEOW_FILE_ERR_READ;
    }

    off_t read_size = (off_t)(n_bytes < buf_size ? n_bytes : buf_size);
    off_t offset    = file_size > read_size ? file_size - read_size : 0;

    if (lseek(fd, offset, SEEK_SET) < 0) {
        last_errno = errno;
        close(fd);
        return MEOW_FILE_ERR_READ;
    }

    ssize_t n = read(fd, buf, (size_t)read_size);
    int saved = errno;
    close(fd);
    if (n < 0) {
        last_errno = saved;
        return MEOW_FILE_ERR_READ;
    }

    *bytes_read = (size_t)n;
    return MEOW_FILE_OK;
}


static uint32_t crc32_step(uint32_t crc, uint8_t byte)
{
    crc ^= byte;
    for (int i = 0; i < 8; i++)
        crc = (crc >> 1) ^ (0xEDB88320u & -(uint32_t)(crc & 1u));
    return crc;
}

int meow_file_checksum(const char* path, uint32_t* crc_out)
{
    if (!path || !crc_out)
        return MEOW_FILE_ERR_NULL;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_OPEN;
    }

    uint32_t crc = 0xFFFFFFFFu;
    uint8_t  buf[512];
    ssize_t  n;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++)
            crc = crc32_step(crc, buf[i]);
    }
    int saved = errno;
    close(fd);

    if (n < 0) {
        last_errno = saved;
        return MEOW_FILE_ERR_READ;
    }
    *crc_out = ~crc;
    return MEOW_FILE_OK;
}

int meow_disk_stat(const char* path, meow_disk_stat_t* out)
{
    if (!path || !out)
        return MEOW_FILE_ERR_NULL;

    struct statvfs sv;
    if (statvfs(path, &sv) < 0) {
        last_errno = errno;
        return MEOW_FILE_ERR_STAT;
    }

    out->total_bytes = (uint64_t)sv.f_blocks * sv.f_frsize;
    out->avail_bytes = (uint64_t)sv.f_bavail * sv.f_frsize;
    return MEOW_FILE_OK;
}
