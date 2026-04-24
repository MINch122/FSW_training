/*
 *  Copyright (c) 2015, United States government as represented by the
 *  administrator of the National Aeronautics Space Administration.
 *  All rights reserved. This software was created at NASA Glenn
 *  Research Center pursuant to government contracts.
 */

/**
 * \file
 *
 * I/O adapter for serial
 */

#ifndef CFE_PSP_IODRIVER_SERIAL_IO_H
#define CFE_PSP_IODRIVER_SERIAL_IO_H

/* Include all base definitions */
#include "iodriver_base.h"
#include <sys/types.h>

/**
 * \brief Parameter payload for write, read commands.
 * \note Supported Protocol : I2C, SPI, CAN, UART
 * Higher layer (i.e. cFE SRL) use this structure for Xfer
 */
typedef struct {
    /* Pointer of Tx Data buffer */
    void *TxData;
    /* Size of Tx Data */
    size_t TxSize;
    /* Pointer of Rx Data buffer - No need for Write */
    void *RxData;
    /* Size of Rx Size - No need for Write */
    size_t RxSize;
    /**
     * Used for ApiRead - Not used in SPI
     * Unit : millisec
     */
    uint32_t Timeout;
    /**
     * Used for I2C, CAN
     * I2C : Slave Addr
     * CAN : CAN frame ID
     */
    uint32_t Addr;
    /**
     * Used for ApiRead
     * The parameter determine the **time interval** between `Write` -> `Read`
     * Unit : millisec (Exceptionally, usec in SPI)
     * In I2C, if this parameter is none 0, Do atomic transaction. If not, Do combined transaction
     * 
     */
    uint32_t Interval;

    /**
     * Read Bytes from ApiRead - Not used in I2C, SPI
     */
    ssize_t ReadBytes;
} CFE_PSP_IODriver_SerialXferParam_t;

/**
 * \brief TRx struct for I/O
 * \note Used in `LINUX_SERIAL_WRITE_SUBSYS` and `LINUX_SERIAL_READ_SUBSYS`
 */
typedef struct {
    CFE_PSP_IODriver_SerialXferParam_t Params;
    int32_t FD;
} CFE_PSP_IODriver_SerialXfer_t;


/**
 * Protocol config struct
 * Used in `LINUX_SERIAL_CONFIG_SUBSYS`
 * Higher layer should use `CFE_PSP_IODriver_Serial_cfg_t` for protocol abstraction
 */
/* I2C config struct */
typedef struct {
    bool tenbit;
    bool pec_en;
    uint8_t retries;
} CFE_PSP_I2C_cfg_t; // size 3

/* SPI config struct */
typedef struct {
    uint8_t mode;
    uint8_t bpw;
    uint32_t speed;

    uint8_t Padding[2];
} CFE_PSP_SPI_cfg_t; // size 8

/* CAN config struct */
typedef struct {
    uint32_t id;
    uint32_t mask;
    bool is_ext;

    uint8_t Padding[3];
} CFE_PSP_CAN_filter_t; // size 12

typedef struct {
    CFE_PSP_CAN_filter_t *filter;
    uint8_t filter_num;

    bool loopback_en;
    bool recv_own_msgs;
    bool recv_err_frame;
    bool timestamp_en; /* @deprecated by toolchain kernel restriction */

    uint8_t Padding[3];
} CFE_PSP_CAN_cfg_t; // size 16

/* UART (+ RS-*) config struct */
typedef struct {
    uint32_t baud;
    uint8_t databits; // 5 ~ 8
    uint8_t parity; // none(0), even(1), odd(2)
    uint8_t stopbits; // 1, 2

    uint8_t Padding[1];
} CFE_PSP_UART_cfg_t; // size 8

/**
 * \brief Unified configuration structure
 * \note Higher layer should use this structure
 * Also, carefully select the protocol want to configure
 */
typedef struct {
    uint8_t FD;
    union {
        CFE_PSP_I2C_cfg_t i2c;
        CFE_PSP_SPI_cfg_t spi;
        CFE_PSP_CAN_cfg_t can;
        CFE_PSP_UART_cfg_t uart;
    } cfg;
} __attribute__((packed)) CFE_PSP_IODriver_Serial_cfg_t; // size 17. if not packed, 24

typedef union {
    uint8_t bytes[24];
    struct {
        uint8_t FD;
        union {
            CFE_PSP_I2C_cfg_t i2c;
            CFE_PSP_SPI_cfg_t spi;
            CFE_PSP_CAN_cfg_t can;
            CFE_PSP_UART_cfg_t uart;
        } cfg;
    } Payload; // size 24
} CFE_PSP_IODriver_Serial_cfg2_t;


/**
 * \brief Serial handle counter table entry
 * \note This module will store the various counter,
 * and returned the entry to the requesting app
 * But, higher layer should not use directly. use 
 */
typedef struct {
    uint16_t TxOps, RxOps;  /* \brief Read/Write Operation counter. Only increased in success condition */
    uint16_t TxCnt, RxCnt;  /* \brief Read/Write Bytes counter. Only increased in success condition */
    uint16_t SetupErr, TxErr, RxErr;  /* \brief Read/Write Error counter. Only increased in error condition */
    int __errno;
} CFE_PSP_IODriver_Serial_cnt_Payload_t;


typedef struct {
    int FD;         /* \brief File descriptor */
    CFE_PSP_IODriver_Serial_cnt_Payload_t cnts;
} CFE_PSP_IODriver_Serial_cnt_t;

/**
 * Subchannel specific to serial oriented interfaces
 * These are indicate the specific communication protocol
 */
enum {
    CFE_PSP_IODriver_SERIAL_I2C_SUBCH,
    CFE_PSP_IODriver_SERIAL_SPI_SUBCH,
    CFE_PSP_IODriver_SERIAL_CAN_SUBCH,
    CFE_PSP_IODriver_SERIAL_UART_SUBCH, /* <\brief include `RS-*` series */

    CFE_PSP_IODriver_SERIAL_SUBCH_MAX,
};

/**
 * Opcodes specific to serial oriented interfaces
 * FIX: Right now these are the same as the packet interface and need to be changed.
 */
enum
{
    CFE_PSP_IODriver_SERIAL_IO_NOOP = CFE_PSP_IODriver_STREAM_IO_CLASS_BASE,

    CFE_PSP_IODriver_SERIAL_IO_GET_CNTS,    /* <\brief Get counters information to specific FD */
    CFE_PSP_IODriver_SERIAL_IO_CLEAR_CNTS,  /* <\brief Clear counters to specific FD */

    CFE_PSP_IODriver_SERIAL_IO_MAX
};

/**
 * Additional error codes specific to Serial I/O
 *
 * These are based from the CFE_PSP_IODriver_SERIAL_IO_CLASS_BASE so as to not conflict with other classes of I/O
 */
enum
{
    CFE_PSP_IODriver_SERIAL_ERROR_NONE = 0, // Use CFE_PSP_SUCCESS instead

    CFE_PSP_IODriver_SERIAL_ERROR_BASE = -(CFE_PSP_IODriver_SERIAL_IO_CLASS_BASE + 0xFFFF),
    CFE_PSP_IODriver_SERIAL_LENGTH_ERROR,
    CFE_PSP_IODriver_SERIAL_CRC_ERROR,

    CFE_PSP_IODriver_SERIAL_OPEN_ERROR,
    CFE_PSP_IODriver_SERIAL_SOCKET_ERROR,
    CFE_PSP_IODriver_SERIAL_BIND_ERROR,

    CFE_PSP_IODriver_SERIAL_CLOSE_ERROR,

    CFE_PSP_IODriver_SERIAL_ERROR,
    CFE_PSP_IODriver_SERIAL_TIMEOUT_ERROR,
    CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR,
    CFE_PSP_IODriver_SERIAL_WRITE_ERROR,
    CFE_PSP_IODriver_SERIAL_PARTIAL_WRITE_ERROR,
    CFE_PSP_IODriver_SERIAL_READ_ERROR,
    CFE_PSP_IODriver_SERIAL_PARTIAL_READ_ERROR,

    CFE_PSP_IODriver_SERIAL_IOCTL_ERROR,
};

#endif /* CFE_PSP_IODRIVER_SERIAL_IO_H */
