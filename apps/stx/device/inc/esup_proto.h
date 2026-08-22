/**
 * @file esup_proto.h
 * @brief Device-independent constants of the EnduroSat User Protocol (ESUP).
 * 
 * Defines the minimal, shared ESUP frame layout and the command codes.
 * Everything device-specific lives in a per-device header, NOT here.
 */
#ifndef ESUP_PROTO_H
#define ESUP_PROTO_H

#include <stdint.h>

/*
 * Frame header magic.
 */
#define ESUP_MAGIC_0 0x45U /* 'E' */
#define ESUP_MAGIC_1 0x53U /* 'S' */
#define ESUP_MAGIC_2 0x55U /* 'U' */
#define ESUP_MAGIC_3 0x50U /* 'P' */

/* Field lengths in bytes. */
#define ESUP_SIZE_HEADER     4U  /** Header magic. */
#define ESUP_SIZE_MODULE_ID  2U  /** Module ID. */
#define ESUP_SIZE_DATA_LEN   2U  /** Data Length field. */
#define ESUP_SIZE_CMD_STATUS 2U  /** Command Status field. */
#define ESUP_SIZE_COMMAND    2U  /** Command field. */
#define ESUP_SIZE_TYPE       2U  /** Type field. */
#define ESUP_SIZE_CRC        4U  /** CRC-32 field. */

/* Byte offsets of each field from the start of a frame. */
#define ESUP_OFST_HEADER     0U   /** Offset of the header magic. */
#define ESUP_OFST_MODULE_ID  4U   /** Offset of the Module ID. */
#define ESUP_OFST_DATA_LEN   6U   /** Offset of the Data Length field. */
#define ESUP_OFST_CMD_STATUS 8U   /** Offset of the Command Status field. */
#define ESUP_OFST_COMMAND    10U  /** Offset of the Command field. */
#define ESUP_OFST_TYPE       12U  /** Offset of the Type field. */
#define ESUP_OFST_DATA       14U  /** Offset of the Data field (CRC follows it). */

/**
 * Bytes from the start of the header up to and including the Type field. This
 * is the fixed part that precedes the variable Data field, and together with
 * the Data field it forms the CRC input range.
 */
#define ESUP_SIZE_PREDATA    14U

/** Fixed overhead of a frame excluding Data and padding: pre-data + CRC. */
#define ESUP_SIZE_OVERHEAD   18U

/** Maximum size of the Data field in bytes. */
#define ESUP_DATA_MAX       1472U

/** On-wire length alignment: every frame is padded with zeros to a multiple. */
#define ESUP_ALIGN          16U

/**
 * Maximum on-wire frame length in bytes: align_up(ESUP_SIZE_OVERHEAD +
 * ESUP_DATA_MAX, ESUP_ALIGN) = 1504.
 */
#define ESUP_FRAME_MAX      1504U

/*
 * CRC-32 parameters.
 */
#define ESUP_CRC32_POLY   0xEDB88820U
#define ESUP_CRC32_INIT   0x00000000U
#define ESUP_CRC32_XOROUT 0x00000000U

/**
 * @brief Command Status field values.
 */
typedef enum {
    ESUP_STATUS_NONE               = 0x0000, /** Field insignificant (host commands). */
    ESUP_STATUS_ACK                = 0x0005, /** Command received and accepted. */
    ESUP_STATUS_NOT_ACK            = 0x0006, /** Rejected (usually bad parameters). */
    ESUP_STATUS_BUSY               = 0x0007, /** That command is currently executing. */
    ESUP_STATUS_NCE                = 0x0008, /** No command for execution (expired / wrong selector). */
    ESUP_STATUS_STACK_FULL         = 0x0009, /** No free container cell of the required kind. */
    ESUP_STATUS_TEMP_NOT_ACCEPTED  = 0x0010  /** Container full; retry later. VERIFY (0x0010 vs 0x000A). */
} esup_status_code_t;

/**
 * @brief The ESUP command codes.
 */
typedef enum {
    ESUP_CMD_GET              = 0x0100, /** Read a parameter by Type. */
    ESUP_CMD_SET              = 0x0101, /** Write a parameter by Type. */
    ESUP_CMD_DIR              = 0x0102, /** List root-directory files. */
    ESUP_CMD_DIR_NEXT         = 0x0103, /** Continue a file listing. */
    ESUP_CMD_DELETE_FILE      = 0x0104, /** Delete one named file. */
    ESUP_CMD_DELETE_ALL_FILES = 0x0105, /** Delete all non-service files. */
    ESUP_CMD_CREATE_FILE      = 0x0106, /** Create a file, returns a handle. */
    ESUP_CMD_WRITE_FILE       = 0x0107, /** Write a data packet using a handle. */
    ESUP_CMD_OPEN_FILE        = 0x0108, /** Open a file, returns handle and length. */
    ESUP_CMD_READ_FILE        = 0x0109, /** Read a data packet using a handle. */
    ESUP_CMD_QUICK_FORMAT     = 0x010C, /** Quick-format the SD card (FAT32); 12-20 s. */
    ESUP_CMD_UPDATE_FW        = 0x0112, /** Apply an on-card firmware update; auto-restarts. */
    ESUP_CMD_SAFE_SHUTDOWN    = 0x0113, /** Persist state and halt the processor. */
    ESUP_CMD_GET_RESULTS      = 0x0114  /** Poll for a stored result (engine-issued). */
} esup_command_code_t;

/** @brief Type value for commands that carry no meaningful Type field. */
#define ESUP_TYPE_NONE 0x0000U

/**
 * @brief Command-execution status OK (first byte of a result's Data field).
 */
#define ESUP_EXEC_OK 0x00U

#endif /* ESUP_PROTO_H */
