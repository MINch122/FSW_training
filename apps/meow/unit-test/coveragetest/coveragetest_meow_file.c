/**
 * Coverage tests for meow_file.c — pure POSIX, uses real temp files.
 */

#include "utassert.h"
#include "uttest.h"

#include "meow_file.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

#define TMP_A  "/tmp/meow_ut_a"
#define TMP_B  "/tmp/meow_ut_b"

static void cleanup(void)
{
    unlink(TMP_A);
    unlink(TMP_B);
}

static const uint8_t HELLO[] = "hello world";
static const size_t  HELLO_LEN = 11;

/* -------------------------------------------------------------------------
 * meow_file_write
 * ---------------------------------------------------------------------- */

void Test_FileWrite_NullArgs(void)
{
    uint8_t d = 0;
    UtAssert_INT32_EQ(meow_file_write(NULL, &d, 1, 0, 0),        MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_write(TMP_A, NULL, 1, 0, 0),     MEOW_FILE_ERR_NULL);
}

void Test_FileWrite_Basic(void)
{
    cleanup();
    UtAssert_INT32_EQ(meow_file_write(TMP_A, HELLO, HELLO_LEN, 0, 0), MEOW_FILE_OK);

    uint8_t buf[32];
    size_t  n = 0;
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 0, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, HELLO_LEN);
    UtAssert_MemCmpValue(buf, 'h', 1, "first byte");
    cleanup();
}

void Test_FileWrite_NoTruncate(void)
{
    cleanup();
    /* write 8 bytes, then overwrite first 2 — remaining 6 must survive */
    meow_file_write(TMP_A, (const uint8_t*)"ABCDEFGH", 8, 0, 0);
    UtAssert_INT32_EQ(meow_file_write(TMP_A, (const uint8_t*)"XY", 2, 0, 0), MEOW_FILE_OK);

    uint8_t buf[8] = {0};
    size_t  n = 0;
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 0, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, 8);
    UtAssert_True(buf[0] == 'X' && buf[1] == 'Y' &&
                  buf[2] == 'C' && buf[7] == 'H', "no truncate");
    cleanup();
}

void Test_FileWrite_Append(void)
{
    cleanup();
    UtAssert_INT32_EQ(meow_file_write(TMP_A, (const uint8_t*)"AB", 2, 0, 0), MEOW_FILE_OK);
    UtAssert_INT32_EQ(meow_file_write(TMP_A, (const uint8_t*)"CD", 2, 0, MEOW_FILE_WRITE_APPEND),
                      MEOW_FILE_OK);

    uint8_t buf[8] = {0};
    size_t  n = 0;
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 0, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, 4);
    UtAssert_True(buf[0] == 'A' && buf[1] == 'B' && buf[2] == 'C' && buf[3] == 'D',
                  "append result");
    cleanup();
}

void Test_FileWrite_Fsync(void)
{
    cleanup();
    UtAssert_INT32_EQ(meow_file_write(TMP_A, HELLO, HELLO_LEN, 0, MEOW_FILE_WRITE_FSYNC),
                      MEOW_FILE_OK);
    cleanup();
}

void Test_FileWrite_WithOffset(void)
{
    cleanup();
    /* write "ABCDEFGH", then patch bytes 4-5 with "XY" in place */
    meow_file_write(TMP_A, (const uint8_t*)"ABCDEFGH", 8, 0, 0);
    UtAssert_INT32_EQ(meow_file_write(TMP_A, (const uint8_t*)"XY", 2, 4, 0), MEOW_FILE_OK);

    uint8_t buf[8] = {0};
    size_t  n = 0;
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 0, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, 8);
    UtAssert_True(buf[0] == 'A' && buf[3] == 'D' &&
                  buf[4] == 'X' && buf[5] == 'Y' && buf[7] == 'H',
                  "offset patch");
    cleanup();
}

void Test_FileWrite_AppendIgnoresOffset(void)
{
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"AB", 2, 0, 0);
    /* non-zero offset with APPEND — offset must be ignored, data goes to end */
    UtAssert_INT32_EQ(meow_file_write(TMP_A, (const uint8_t*)"CD", 2, 99, MEOW_FILE_WRITE_APPEND),
                      MEOW_FILE_OK);

    uint8_t buf[8] = {0};
    size_t  n = 0;
    meow_file_read(TMP_A, 0, buf, sizeof(buf), &n);
    UtAssert_UINT32_EQ(n, 4);
    UtAssert_True(buf[2] == 'C' && buf[3] == 'D', "append ignores offset");
    cleanup();
}

/* -------------------------------------------------------------------------
 * meow_file_read
 * ---------------------------------------------------------------------- */

void Test_FileRead_NullArgs(void)
{
    uint8_t buf[8];
    size_t  n;
    UtAssert_INT32_EQ(meow_file_read(NULL, 0, buf, sizeof(buf), &n),   MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 0, NULL, sizeof(buf), &n), MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 0, buf, sizeof(buf), NULL),MEOW_FILE_ERR_NULL);
}

void Test_FileRead_NoExist(void)
{
    uint8_t buf[8];
    size_t  n;
    UtAssert_INT32_EQ(meow_file_read("/tmp/meow_ut_no_such_file", 0, buf, sizeof(buf), &n),
                      MEOW_FILE_ERR_OPEN);
}

void Test_FileRead_WithOffset(void)
{
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"ABCDEFGH", 8, 0, 0);

    uint8_t buf[8] = {0};
    size_t  n = 0;
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 4, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, 4);
    UtAssert_True(buf[0] == 'E', "offset read");
    cleanup();
}

void Test_FileRead_AtEOF(void)
{
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"X", 1, 0, 0);

    uint8_t buf[8];
    size_t  n = 99;
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 1, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, 0);
    cleanup();
}

/* -------------------------------------------------------------------------
 * meow_file_remove
 * ---------------------------------------------------------------------- */

void Test_FileRemove_NullArg(void)
{
    UtAssert_INT32_EQ(meow_file_remove(NULL), MEOW_FILE_ERR_NULL);
}

void Test_FileRemove_Success(void)
{
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"x", 1, 0, 0);
    UtAssert_INT32_EQ(meow_file_remove(TMP_A), MEOW_FILE_OK);
    UtAssert_INT32_EQ(meow_file_remove(TMP_A), MEOW_FILE_ERR_OP); /* already gone */
}

/* -------------------------------------------------------------------------
 * meow_file_copy
 * ---------------------------------------------------------------------- */

void Test_FileCopy_NullArgs(void)
{
    UtAssert_INT32_EQ(meow_file_copy(NULL, TMP_B),   MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_copy(TMP_A, NULL),   MEOW_FILE_ERR_NULL);
}

void Test_FileCopy_SameFile(void)
{
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"hi", 2, 0, 0);
    UtAssert_INT32_EQ(meow_file_copy(TMP_A, TMP_A), MEOW_FILE_ERR_OP);
    cleanup();
}

void Test_FileCopy_Success(void)
{
    cleanup();
    meow_file_write(TMP_A, HELLO, HELLO_LEN, 0, 0);
    UtAssert_INT32_EQ(meow_file_copy(TMP_A, TMP_B), MEOW_FILE_OK);

    uint8_t buf[32];
    size_t  n = 0;
    meow_file_read(TMP_B, 0, buf, sizeof(buf), &n);
    UtAssert_UINT32_EQ(n, HELLO_LEN);
    UtAssert_MemCmp(buf, HELLO, HELLO_LEN, "copy content matches");
    cleanup();
}

/* -------------------------------------------------------------------------
 * meow_file_move
 * ---------------------------------------------------------------------- */

void Test_FileMove_NullArgs(void)
{
    UtAssert_INT32_EQ(meow_file_move(NULL, TMP_B), MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_move(TMP_A, NULL), MEOW_FILE_ERR_NULL);
}

void Test_FileMove_SameFs(void)
{
    cleanup();
    meow_file_write(TMP_A, HELLO, HELLO_LEN, 0, 0);
    UtAssert_INT32_EQ(meow_file_move(TMP_A, TMP_B), MEOW_FILE_OK);

    /* src gone, dst present */
    uint8_t buf[32];
    size_t  n = 0;
    UtAssert_INT32_EQ(meow_file_read(TMP_A, 0, buf, sizeof(buf), &n), MEOW_FILE_ERR_OPEN);
    UtAssert_INT32_EQ(meow_file_read(TMP_B, 0, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, HELLO_LEN);
    cleanup();
}

/* -------------------------------------------------------------------------
 * meow_file_stat
 * ---------------------------------------------------------------------- */

void Test_FileStat_NullArgs(void)
{
    meow_file_stat_t s;
    UtAssert_INT32_EQ(meow_file_stat(NULL, &s),   MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_stat(TMP_A, NULL), MEOW_FILE_ERR_NULL);
}

void Test_FileStat_NoExist(void)
{
    meow_file_stat_t s;
    UtAssert_INT32_EQ(meow_file_stat("/tmp/meow_ut_no_such_file", &s), MEOW_FILE_ERR_STAT);
}

void Test_FileStat_RegularFile(void)
{
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"ABCDEF", 6, 0, 0);

    meow_file_stat_t s;
    memset(&s, 0, sizeof(s));
    UtAssert_INT32_EQ(meow_file_stat(TMP_A, &s), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(s.size, 6);
    UtAssert_True(s.mtime > 0, "mtime set");
    UtAssert_True(s.is_dir == 0, "not a dir");
    UtAssert_True(s.preview_len == 6, "preview_len");
    UtAssert_True(s.preview[0] == 'A', "preview[0]");
    cleanup();
}

/* -------------------------------------------------------------------------
 * meow_file_truncate
 * ---------------------------------------------------------------------- */

void Test_FileTruncate_NullArg(void)
{
    UtAssert_INT32_EQ(meow_file_truncate(NULL), MEOW_FILE_ERR_NULL);
}

void Test_FileTruncate_Success(void)
{
    cleanup();
    meow_file_write(TMP_A, HELLO, HELLO_LEN, 0, 0);
    UtAssert_INT32_EQ(meow_file_truncate(TMP_A), MEOW_FILE_OK);

    meow_file_stat_t s;
    meow_file_stat(TMP_A, &s);
    UtAssert_UINT32_EQ(s.size, 0);
    cleanup();
}

/* -------------------------------------------------------------------------
 * meow_file_tail
 * ---------------------------------------------------------------------- */

void Test_FileTail_NullArgs(void)
{
    uint8_t buf[8];
    size_t  n;
    UtAssert_INT32_EQ(meow_file_tail(NULL, 4, buf, sizeof(buf), &n),   MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_tail(TMP_A, 4, NULL, sizeof(buf), &n), MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_tail(TMP_A, 4, buf, sizeof(buf), NULL),MEOW_FILE_ERR_NULL);
}

void Test_FileTail_LastFourBytes(void)
{
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"ABCDEFGH", 8, 0, 0);

    uint8_t buf[8] = {0};
    size_t  n = 0;
    UtAssert_INT32_EQ(meow_file_tail(TMP_A, 4, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, 4);
    UtAssert_True(buf[0] == 'E' && buf[3] == 'H', "tail content");
    cleanup();
}

void Test_FileTail_RequestMoreThanFile(void)
{
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"AB", 2, 0, 0);

    uint8_t buf[8] = {0};
    size_t  n = 0;
    UtAssert_INT32_EQ(meow_file_tail(TMP_A, 100, buf, sizeof(buf), &n), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(n, 2);
    cleanup();
}

/* -------------------------------------------------------------------------
 * meow_file_checksum
 * ---------------------------------------------------------------------- */

void Test_FileChecksum_NullArgs(void)
{
    uint32_t crc;
    UtAssert_INT32_EQ(meow_file_checksum(NULL, &crc),   MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_file_checksum(TMP_A, NULL),  MEOW_FILE_ERR_NULL);
}

void Test_FileChecksum_KnownVector(void)
{
    /* CRC-32/ISO-HDLC of "123456789" is 0xCBF43926 */
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"123456789", 9, 0, 0);

    uint32_t crc = 0;
    UtAssert_INT32_EQ(meow_file_checksum(TMP_A, &crc), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(crc, 0xCBF43926u);
    cleanup();
}

void Test_FileChecksum_EmptyFile(void)
{
    /* CRC-32 of empty = ~0xFFFFFFFF = 0x00000000 */
    cleanup();
    meow_file_write(TMP_A, (const uint8_t*)"", 0, 0, 0);

    uint32_t crc = 0xDEADBEEFu;
    UtAssert_INT32_EQ(meow_file_checksum(TMP_A, &crc), MEOW_FILE_OK);
    UtAssert_UINT32_EQ(crc, 0x00000000u);
    cleanup();
}

/* -------------------------------------------------------------------------
 * meow_disk_stat
 * ---------------------------------------------------------------------- */

void Test_DiskStat_NullArgs(void)
{
    meow_disk_stat_t s;
    UtAssert_INT32_EQ(meow_disk_stat(NULL, &s),    MEOW_FILE_ERR_NULL);
    UtAssert_INT32_EQ(meow_disk_stat("/tmp", NULL), MEOW_FILE_ERR_NULL);
}

void Test_DiskStat_Success(void)
{
    meow_disk_stat_t s = {0};
    UtAssert_INT32_EQ(meow_disk_stat("/tmp", &s), MEOW_FILE_OK);
    UtAssert_True(s.total_bytes > 0, "total_bytes > 0");
    UtAssert_True(s.avail_bytes <= s.total_bytes, "avail <= total");
}

/* -------------------------------------------------------------------------
 * Test registration
 * ---------------------------------------------------------------------- */

void UtTest_Setup(void)
{
    UtTest_Add(Test_FileWrite_NullArgs,            NULL, NULL, "FileWrite_NullArgs");
    UtTest_Add(Test_FileWrite_Basic,               NULL, NULL, "FileWrite_Basic");
    UtTest_Add(Test_FileWrite_NoTruncate,          NULL, NULL, "FileWrite_NoTruncate");
    UtTest_Add(Test_FileWrite_Append,              NULL, NULL, "FileWrite_Append");
    UtTest_Add(Test_FileWrite_Fsync,               NULL, NULL, "FileWrite_Fsync");
    UtTest_Add(Test_FileWrite_WithOffset,          NULL, NULL, "FileWrite_WithOffset");
    UtTest_Add(Test_FileWrite_AppendIgnoresOffset, NULL, NULL, "FileWrite_AppendIgnoresOffset");

    UtTest_Add(Test_FileRead_NullArgs,             NULL, NULL, "FileRead_NullArgs");
    UtTest_Add(Test_FileRead_NoExist,              NULL, NULL, "FileRead_NoExist");
    UtTest_Add(Test_FileRead_WithOffset,           NULL, NULL, "FileRead_WithOffset");
    UtTest_Add(Test_FileRead_AtEOF,                NULL, NULL, "FileRead_AtEOF");

    UtTest_Add(Test_FileRemove_NullArg,            NULL, NULL, "FileRemove_NullArg");
    UtTest_Add(Test_FileRemove_Success,            NULL, NULL, "FileRemove_Success");

    UtTest_Add(Test_FileCopy_NullArgs,             NULL, NULL, "FileCopy_NullArgs");
    UtTest_Add(Test_FileCopy_SameFile,             NULL, NULL, "FileCopy_SameFile");
    UtTest_Add(Test_FileCopy_Success,              NULL, NULL, "FileCopy_Success");

    UtTest_Add(Test_FileMove_NullArgs,             NULL, NULL, "FileMove_NullArgs");
    UtTest_Add(Test_FileMove_SameFs,               NULL, NULL, "FileMove_SameFs");

    UtTest_Add(Test_FileStat_NullArgs,             NULL, NULL, "FileStat_NullArgs");
    UtTest_Add(Test_FileStat_NoExist,              NULL, NULL, "FileStat_NoExist");
    UtTest_Add(Test_FileStat_RegularFile,          NULL, NULL, "FileStat_RegularFile");

    UtTest_Add(Test_FileTruncate_NullArg,          NULL, NULL, "FileTruncate_NullArg");
    UtTest_Add(Test_FileTruncate_Success,          NULL, NULL, "FileTruncate_Success");

    UtTest_Add(Test_FileTail_NullArgs,             NULL, NULL, "FileTail_NullArgs");
    UtTest_Add(Test_FileTail_LastFourBytes,        NULL, NULL, "FileTail_LastFourBytes");
    UtTest_Add(Test_FileTail_RequestMoreThanFile,  NULL, NULL, "FileTail_RequestMoreThanFile");

    UtTest_Add(Test_FileChecksum_NullArgs,         NULL, NULL, "FileChecksum_NullArgs");
    UtTest_Add(Test_FileChecksum_KnownVector,      NULL, NULL, "FileChecksum_KnownVector");
    UtTest_Add(Test_FileChecksum_EmptyFile,        NULL, NULL, "FileChecksum_EmptyFile");

    UtTest_Add(Test_DiskStat_NullArgs,             NULL, NULL, "DiskStat_NullArgs");
    UtTest_Add(Test_DiskStat_Success,              NULL, NULL, "DiskStat_Success");
}
