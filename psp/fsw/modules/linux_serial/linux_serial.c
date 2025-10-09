/***********************************************************************
 *  Copyright (c) 2025, Yonsei University as represented by the
 *  Department of Satellite Systems (DSS) & Astrodynamic & Control Lab (ACL)
 *  All rights reserved. This software was created at DSS
 *  
 *  Author : Hyeok-jin Kweon
 * 
 *  \file linux_serial.c
 *
 ***********************************************************************/
/*
 * NOTE: This relies on the Linux Kernel I/O system
 * Documented here: 
 */

 /************************************************************************
 * Includes
 ************************************************************************/
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/if.h>

#include <linux/spi/spidev.h>

#include "cfe_psp.h"
#include "cfe_psp_module.h"
#include "osapi.h"

#include "iodriver_impl.h"
#include "iodriver_serial_io.h"

/********************************************************************
 * Local Defines
 ********************************************************************/
#define GETFD(cfg) cfg->FD
#define GETCFG(protocol) cfg->cfg.protocol

/**
 * Counter table size
 * Each entry store the various I/O counters
 * If data interface (i.e. file descriptor) want to be counted
 * is too many, enlarge this value
 */ 
#define LINUX_SERIAL_COUNTER_TABLE_SIZE     20
#define LINUX_SERIAL_COUNTER_ENTRY_NOT_USED ((int)-1)

/* Counter entry enum for distinguish member */
typedef enum {
    LINUX_SERIAL_COUNTER_TABLE_TXOPS,
    LINUX_SERIAL_COUNTER_TABLE_RXOPS,
    LINUX_SERIAL_COUNTER_TABLE_TXCNT,
    LINUX_SERIAL_COUNTER_TABLE_RXCNT,
    LINUX_SERIAL_COUNTER_TABLE_TXERR,       /* \brief Error during write */
    LINUX_SERIAL_COUNTER_TABLE_RXERR,       /* \brief Error during read */
    LINUX_SERIAL_COUNTER_TABLE_SETUPERR,    /* <\brief Error during setup phase */
} linux_serial_cnt_mem_t;

/* Mutex definition */
#define LINUX_SERIAL_NO_MUTEX_HASH     ((int32_t)-1)

/* kernel restriction */
#define LINUX_SERIAL_AT91_MAX_TX_SIZE  3 /* <\brief kernel restriction about maximum write size. Should be modified depends on the specific kernel */

/* Subsystem Definition */
#define LINUX_SERIAL_OPEN_SUBSYS    CFE_PSP_IODriver_OPEN_SUBSYSTEM
#define LINUX_SERIAL_WRITE_SUBSYS   CFE_PSP_IODriver_WRITE_SUBSYSTEM
#define LINUX_SERIAL_READ_SUBSYS    CFE_PSP_IODriver_READ_SUBSYSTEM
#define LINUX_SERIAL_CONFIG_SUBSYS  CFE_PSP_IODriver_CONFIG_SUBSYSTEM
#define LINUX_SERIAL_CLOSE_SUBSYS   CFE_PSP_IODriver_CLOSE_SUBSYSTEM

static void linux_serial_Init(uint32_t local_module_id);
static int32_t linux_serial_open_dispatch(uint16_t SubChannelId, const char *Arg);
static int32_t linux_serial_write_dispatch(uint16_t SubchannelId, void *Arg);
static int32_t linux_serial_read_dispatch(uint16_t SubchannelId, void *Arg);
static int32_t linux_serial_config_dispatch(uint16_t SubchannelId, void *Arg);
static int32_t linux_serial_close_dispatch(uint16_t SubchannelId, void *Arg);

static int32_t linux_serial_DevCmd(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg);
static int32_t linux_serial_DevMutex(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg);


/********************************************************************
 * Global Data
 ********************************************************************/
/* linux_serial device command that is called by iodriver to start up linux_serial */
CFE_PSP_IODriver_API_t linux_serial_DevApi = {.DeviceCommand = linux_serial_DevCmd,
                                              .DeviceMutex = linux_serial_DevMutex};

CFE_PSP_MODULE_DECLARE_IODEVICEDRIVER(linux_serial);



static CFE_PSP_IODriver_Serial_cnt_t counter_table[LINUX_SERIAL_COUNTER_TABLE_SIZE];

/***********************************************************************
 * Global Functions
 ***********************************************************************/
void linux_serial_Init(uint32_t local_module_id) {
    /* Initialize the counter table */
    for (uint8_t i = 0; i < LINUX_SERIAL_COUNTER_TABLE_SIZE; i++) {
        /* `FD == -1` indicate that this entry is currently not used */
        counter_table[i].FD = LINUX_SERIAL_COUNTER_ENTRY_NOT_USED;
        memset(&counter_table[i].cnts, 0, sizeof(CFE_PSP_IODriver_Serial_cnt_t));
    }
    return;
}
/***********************************************************************
 * Util Functions
 ***********************************************************************/
/*--------------------Counter table handling function-----------------------*/
static void linux_serial_allocate_cnt_entry(int FD) {
    for (uint8_t i = 0; i < LINUX_SERIAL_COUNTER_TABLE_SIZE; i++) {
        if (counter_table[i].FD == LINUX_SERIAL_COUNTER_ENTRY_NOT_USED) {
            counter_table[i].FD = FD;
            return;
        }
    }
    return;
}

static CFE_PSP_IODriver_Serial_cnt_t *linux_serial_get_cnt_entry(int FD) {
    for (uint8_t i = 0; i < LINUX_SERIAL_COUNTER_TABLE_SIZE; i++) {
        if (counter_table[i].FD == FD)
            return &counter_table[i];
    }
    return NULL;
}

static void linux_serial_increase_cnt(int FD, linux_serial_cnt_mem_t member, int val) {
    CFE_PSP_IODriver_Serial_cnt_t *entry = linux_serial_get_cnt_entry(FD);
    if (entry == NULL) return;

    switch (member)
    {
        case LINUX_SERIAL_COUNTER_TABLE_TXOPS:
            entry->cnts.TxOps += val;
            break;
        case LINUX_SERIAL_COUNTER_TABLE_RXOPS:
            entry->cnts.RxOps += val;
            break;
        case LINUX_SERIAL_COUNTER_TABLE_TXCNT:
            entry->cnts.TxCnt += val;
            break;
        case LINUX_SERIAL_COUNTER_TABLE_RXCNT:
            entry->cnts.RxCnt += val;
            break;
        case LINUX_SERIAL_COUNTER_TABLE_TXERR:
            entry->cnts.TxErr += val;
            break;
        case LINUX_SERIAL_COUNTER_TABLE_RXERR:
            entry->cnts.RxErr += val;
            break;
        case LINUX_SERIAL_COUNTER_TABLE_SETUPERR:
            entry->cnts.SetupErr += val;
            break;
        default:
            break;
    }
    entry->cnts.__errno = errno;
}
#define LINUX_SERIAL_INCREASE_TXOPS(FD) \
    linux_serial_increase_cnt(FD, LINUX_SERIAL_COUNTER_TABLE_TXOPS, 1)
#define LINUX_SERIAL_INCREASE_RXOPS(FD) \
    linux_serial_increase_cnt(FD, LINUX_SERIAL_COUNTER_TABLE_RXOPS, 1)
#define LINUX_SERIAL_INCREASE_TXCNT(FD, bytes) \
    linux_serial_increase_cnt(FD, LINUX_SERIAL_COUNTER_TABLE_TXCNT, bytes)
#define LINUX_SERIAL_INCREASE_RXCNT(FD, bytes) \
    linux_serial_increase_cnt(FD, LINUX_SERIAL_COUNTER_TABLE_RXCNT, bytes)
#define LINUX_SERIAL_INCREASE_TXERR(FD) \
    linux_serial_increase_cnt(FD, LINUX_SERIAL_COUNTER_TABLE_TXERR, 1)
#define LINUX_SERIAL_INCREASE_RXERR(FD) \
    linux_serial_increase_cnt(FD, LINUX_SERIAL_COUNTER_TABLE_RXERR, 1)
#define LINUX_SERIAL_INCREASE_SETUPERR(FD) \
    linux_serial_increase_cnt(FD, LINUX_SERIAL_COUNTER_TABLE_SETUPERR, 1)

static void linux_serial_free_cnt_entry(int FD) {
    for (uint8_t i = 0; i < LINUX_SERIAL_COUNTER_TABLE_SIZE; i++) {
        if (counter_table[i].FD == FD) {
            counter_table[i].FD = LINUX_SERIAL_COUNTER_ENTRY_NOT_USED;
            memset(&counter_table[i].cnts, 0, sizeof(CFE_PSP_IODriver_Serial_cnt_Payload_t));
            return;
        }
    }
    return;
}

static void linux_serial_clear_cnt_entry(int FD) {
    for (uint8_t i = 0; i < LINUX_SERIAL_COUNTER_TABLE_SIZE; i++) {
        if (counter_table[i].FD == FD) {
            memset(&counter_table[i].cnts, 0, sizeof(CFE_PSP_IODriver_Serial_cnt_Payload_t));
            return;
        }
    }
    return;
}

/**
 * \brief Return the counter (and recent errno) of specific file descriptor
 * \param arg [in, out] Will be casted to `CFE_PSP_IODriver_Serial_cnt_t`. Member `FD` is input. Counters and errno of specific FD.
 * Caller should substitute the struct to `const`
 * \return StatusCode. Only `0`(`CFE_PSP_SUCCESS`) is success.
 */
static int32 linux_serial_return_counter(void *arg) {
    CFE_PSP_IODriver_Serial_cnt_t *cnt = (CFE_PSP_IODriver_Serial_cnt_t *)arg;
    for (uint8_t i = 0; i < LINUX_SERIAL_COUNTER_TABLE_SIZE; i++) {
        if (counter_table[i].FD == cnt->FD) {
            cnt->cnts = counter_table[i].cnts;
            return CFE_PSP_SUCCESS;
        }
    }
    OS_printf("failed.\n");
    return CFE_PSP_IODriver_SERIAL_ERROR;
}
/*------------------End of Counter table handling function--------------------*/

/*-----------------------------Basic I/O function-----------------------------*/
int32 linux_serial_write(int FD, void *data, size_t size) {
    ssize_t wrbyte;

    wrbyte = write(FD, data, size);
    if (wrbyte < 0) {
        return CFE_PSP_IODriver_SERIAL_WRITE_ERROR;
    }
    
    else if (wrbyte != size) {
        return CFE_PSP_IODriver_SERIAL_PARTIAL_WRITE_ERROR;
    }
    
    return CFE_PSP_SUCCESS;
}
int32 linux_serial_read(int FD, void *data, size_t size) {
    ssize_t rdbyte;

    rdbyte = read(FD, data, size);
    if (rdbyte < 0) {
        return CFE_PSP_IODriver_SERIAL_READ_ERROR;
    }
    
    else if (rdbyte != size) {
        return CFE_PSP_IODriver_SERIAL_PARTIAL_READ_ERROR;
    }

    return CFE_PSP_SUCCESS;
}
int32 linux_serial_poll_read(int FD, void *Data, size_t size, uint32_t Timeout) {
    struct pollfd FDS[1];

    FDS[0] = (struct pollfd) {
        .fd = FD,
        .events = POLLIN
    };

    int ret = poll(FDS, 1, Timeout);

    if (ret > 0 && FDS[0].revents & POLLIN) {  // When Read Event Occured,
        return linux_serial_read(FD, Data, size);
    }

    else if (ret == 0) return CFE_PSP_IODriver_SERIAL_TIMEOUT_ERROR; // timeout
    
    else if (ret < 0) return CFE_PSP_IODriver_SERIAL_ERROR; // error

    else if (FDS[0].revents & (POLLERR | POLLHUP | POLLNVAL))
        return CFE_PSP_IODriver_SERIAL_ERROR; // error
    
    else return CFE_PSP_IODriver_SERIAL_ERROR; // error
}
int32 linux_serial_close(int FD) {
    int32 StatusCode;

    StatusCode = close(FD);
    if (StatusCode == 0) {
        linux_serial_free_cnt_entry(FD);
        StatusCode = CFE_PSP_SUCCESS;
    }
    else StatusCode = CFE_PSP_IODriver_SERIAL_CLOSE_ERROR;

    return StatusCode;
}
/*--------------------------End of Basic I/O function--------------------------*/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_serial_open()                                 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int32 linux_serial_open(const char *dev, int opt) {
    int32 StatusCode;

    StatusCode = open(dev, opt);
    if (StatusCode < 0) {
        StatusCode = CFE_PSP_IODriver_SERIAL_OPEN_ERROR;
    }
    linux_serial_allocate_cnt_entry(StatusCode);

    return StatusCode; // return file descriptor
}
int32 linux_serial_opensocket(const char *dev) {
    int StatusCode;
    int sock;
    struct ifreq ifr = {0};
    struct sockaddr_can addr = {0};

    sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sock < 0) return CFE_PSP_IODriver_SERIAL_SOCKET_ERROR;

    strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    StatusCode = ioctl(sock, SIOCGIFINDEX, &ifr); // Get interface index number
    if (StatusCode < 0) {
        linux_serial_close(sock);
        return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    StatusCode = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (StatusCode < 0) {
        linux_serial_close(sock);
        return CFE_PSP_IODriver_SERIAL_BIND_ERROR;
    }

    // If there is no error, return socket descriptor
    linux_serial_allocate_cnt_entry(StatusCode);

    return sock;
}
int32_t linux_serial_open_dispatch(uint16_t SubchannelId, const char *Arg) {
    int32 StatusCode; // Positive for FD (file descriptor), Nagative for error

    switch (SubchannelId)
    {
    case CFE_PSP_IODriver_SERIAL_I2C_SUBCH:
    case CFE_PSP_IODriver_SERIAL_SPI_SUBCH:
        StatusCode = linux_serial_open(Arg, O_RDWR);
        break;
    case CFE_PSP_IODriver_SERIAL_CAN_SUBCH:
        StatusCode = linux_serial_opensocket(Arg);
        break;
    case CFE_PSP_IODriver_SERIAL_UART_SUBCH:
        StatusCode = linux_serial_open(Arg, O_RDWR | O_NOCTTY); // <\brief not used the control terminal
        break;
    default:
        /* Unsupported type */
        StatusCode = CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR;
    }
    return StatusCode;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_serial_write()                                */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int32_t linux_serial_write_i2c(CFE_PSP_IODriver_SerialXfer_t *arg) {
    int32_t StatusCode;
    int32_t size = arg->Params.TxSize;

    if (size > LINUX_SERIAL_AT91_MAX_TX_SIZE) {
        StatusCode = ioctl(arg->FD, I2C_SLAVE, arg->Params.Addr);
        if (StatusCode < 0) {
            LINUX_SERIAL_INCREASE_SETUPERR(arg->FD);
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }

        StatusCode = ioctl(arg->FD, I2C_TIMEOUT, (arg->Params.Timeout + 9)/10u);
        if (StatusCode < 0) {
            LINUX_SERIAL_INCREASE_SETUPERR(arg->FD);
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }

        StatusCode = linux_serial_write(arg->FD, arg->Params.TxData, arg->Params.TxSize);
        if (StatusCode != CFE_PSP_SUCCESS) {
            LINUX_SERIAL_INCREASE_TXERR(arg->FD);
        }
        else {
            LINUX_SERIAL_INCREASE_TXOPS(arg->FD);
            LINUX_SERIAL_INCREASE_TXCNT(arg->FD, arg->Params.TxSize);
        }
    }
    else {
        struct i2c_msg msg[1] = {0,};
        struct i2c_rdwr_ioctl_data pkt = {.msgs = msg,
                                          .nmsgs = 0};
        msg[pkt.nmsgs ++] = (struct i2c_msg) {
            .addr = arg->Params.Addr,
            .flags = 0,
            .len = arg->Params.TxSize,
            .buf = (uint8_t *)arg->Params.TxData
        };

        StatusCode = ioctl(arg->FD, I2C_RDWR, &pkt);
        if (StatusCode < 0) {
            LINUX_SERIAL_INCREASE_TXERR(arg->FD);
            StatusCode = CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }
        else {
            LINUX_SERIAL_INCREASE_TXOPS(arg->FD);
            LINUX_SERIAL_INCREASE_TXCNT(arg->FD, arg->Params.TxSize);
            StatusCode = CFE_PSP_SUCCESS; // explicit substitution
        }
    }

    return StatusCode;
}
int32_t linux_serial_write_spi(CFE_PSP_IODriver_SerialXfer_t *arg) {
    int32_t StatusCode;
    struct spi_ioc_transfer xfer[1] = {0};
    uint8_t nmsg = 0;
    
    xfer->tx_buf = (uint64_t)(uintptr_t)arg->Params.TxData;
    xfer->len = arg->Params.TxSize;
    nmsg ++;

    StatusCode = ioctl(arg->FD, SPI_IOC_MESSAGE(nmsg), xfer);
    if (StatusCode < 0) {
        LINUX_SERIAL_INCREASE_TXERR(arg->FD);
        StatusCode = CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
    }
    else {
        LINUX_SERIAL_INCREASE_TXOPS(arg->FD);
        LINUX_SERIAL_INCREASE_TXCNT(arg->FD, arg->Params.TxSize);
        StatusCode = CFE_PSP_SUCCESS;
    }

    return StatusCode;
}
int32_t linux_serial_write_can(CFE_PSP_IODriver_SerialXfer_t *arg) {
    int32_t StatusCode;
    struct can_frame frame;
    size_t Size = arg->Params.TxSize;

    size_t TotBytes = 0; // Total Tx bytes till now
    size_t WrBytes; // Write bytes at this very time
    while (TotBytes < Size) {
        // Configure Frame
        WrBytes = (Size - TotBytes >= CAN_MAX_DLEN) ? CAN_MAX_DLEN : (Size - TotBytes);
        frame.can_id = arg->Params.Addr | CAN_EFF_FLAG;
        frame.can_dlc = WrBytes;
        memcpy(frame.data, ((uint8_t *)arg->Params.TxData) + TotBytes, WrBytes);

        // Write
        StatusCode = linux_serial_write(arg->FD, &frame, sizeof(frame));
        if (StatusCode != CFE_PSP_SUCCESS) {
            LINUX_SERIAL_INCREASE_TXERR(arg->FD);
            return StatusCode;
        }
        TotBytes += WrBytes;
    }
    LINUX_SERIAL_INCREASE_TXOPS(arg->FD);
    LINUX_SERIAL_INCREASE_TXCNT(arg->FD, arg->Params.TxSize);

    return StatusCode;
}
int32_t linux_serial_write_uart(CFE_PSP_IODriver_SerialXfer_t *arg) {
    int32_t StatusCode;

    StatusCode = linux_serial_write(arg->FD, arg->Params.TxData, arg->Params.TxSize);
    if (StatusCode != CFE_PSP_SUCCESS)
        LINUX_SERIAL_INCREASE_TXERR(arg->FD);
    else {
        LINUX_SERIAL_INCREASE_TXOPS(arg->FD);
        LINUX_SERIAL_INCREASE_TXCNT(arg->FD, arg->Params.TxSize);
    }
    return StatusCode;
}

/// @brief 
/// @param SubchannelId 
/// @param Arg Casted to `CFE_PSP_IODriver_SerialXfer_t`
/// @return 
int32_t linux_serial_write_dispatch(uint16_t SubchannelId, void *Arg) {
    int32 StatusCode;

    switch (SubchannelId)
    {
    case CFE_PSP_IODriver_SERIAL_I2C_SUBCH:
        StatusCode = linux_serial_write_i2c((CFE_PSP_IODriver_SerialXfer_t *)Arg);
        break;
    case CFE_PSP_IODriver_SERIAL_SPI_SUBCH:
        StatusCode = linux_serial_write_spi((CFE_PSP_IODriver_SerialXfer_t *)Arg);
        break;
    case CFE_PSP_IODriver_SERIAL_UART_SUBCH:
        StatusCode = linux_serial_write_uart((CFE_PSP_IODriver_SerialXfer_t *)Arg);
        break;
    case CFE_PSP_IODriver_SERIAL_CAN_SUBCH:
        StatusCode = linux_serial_write_can((CFE_PSP_IODriver_SerialXfer_t *)Arg);
        break;
    default:
        /* Unsupported type */
        StatusCode = CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR; // Higher layer should handle this
        break;
    }

    return StatusCode;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_serial_read()                                 */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int32_t linux_serial_read_i2c(CFE_PSP_IODriver_SerialXfer_t *Arg) {
    int32_t StatusCode = 0; 

    if (Arg->Params.Interval) { // If interval param used, do atomic transaction
        StatusCode = ioctl(Arg->FD, I2C_SLAVE, Arg->Params.Addr);
        if (StatusCode < 0) {
            LINUX_SERIAL_INCREASE_SETUPERR(Arg->FD);
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }

        StatusCode = ioctl(Arg->FD, I2C_TIMEOUT, (Arg->Params.Timeout + 9)/10u);
        if (StatusCode < 0) {
            LINUX_SERIAL_INCREASE_SETUPERR(Arg->FD);
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }

        StatusCode = linux_serial_write(Arg->FD, Arg->Params.TxData, Arg->Params.TxSize);
        if (StatusCode != CFE_PSP_SUCCESS) {
            LINUX_SERIAL_INCREASE_TXERR(Arg->FD);
            return StatusCode;
        }
        LINUX_SERIAL_INCREASE_TXOPS(Arg->FD);
        LINUX_SERIAL_INCREASE_TXCNT(Arg->FD, Arg->Params.TxSize);
        OS_TaskDelay(Arg->Params.Interval);

        StatusCode = linux_serial_read(Arg->FD, Arg->Params.RxData, Arg->Params.RxSize);
        if (StatusCode != CFE_PSP_SUCCESS) {
            LINUX_SERIAL_INCREASE_RXERR(Arg->FD);
            return StatusCode;
        }
        LINUX_SERIAL_INCREASE_RXOPS(Arg->FD);
        LINUX_SERIAL_INCREASE_RXCNT(Arg->FD, Arg->Params.RxSize);
        
    }
    else { // If interval param not used, do combined transaction
        struct i2c_msg msg[2] = {0,};
        struct i2c_rdwr_ioctl_data pkt = {.msgs = msg,
                                          .nmsgs = 0};

        if (Arg->Params.TxData && Arg->Params.TxSize) { // If Tx data exist,
            msg[pkt.nmsgs ++] = (struct i2c_msg) {
                .addr = Arg->Params.Addr,
                .flags = 0, // Write flag
                .len = Arg->Params.TxSize,
                .buf = (uint8_t *)Arg->Params.TxData
            };
        }
        if (Arg->Params.RxData && Arg->Params.RxSize) { // If Rx data exist,
            msg[pkt.nmsgs ++] = (struct i2c_msg) {
                .addr = Arg->Params.Addr,
                .flags = I2C_M_RD,
                .len = Arg->Params.RxSize,
                .buf = (uint8_t *)Arg->Params.RxData
            };
        }

        if (pkt.nmsgs == 0)
            return CFE_PSP_SUCCESS; // Do nothing

        /**
         * Do transaction
         * If ioctl return -1 by interrupt signal, try again
         */
        do {
            StatusCode = ioctl(Arg->FD, I2C_RDWR, &pkt);
        } while (StatusCode < 0 && errno == EINTR);

        if (StatusCode < 0) {
            LINUX_SERIAL_INCREASE_TXERR(Arg->FD);
            LINUX_SERIAL_INCREASE_RXERR(Arg->FD);
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }
        else {
            LINUX_SERIAL_INCREASE_TXOPS(Arg->FD);
            LINUX_SERIAL_INCREASE_TXCNT(Arg->FD, Arg->Params.TxSize);
            LINUX_SERIAL_INCREASE_RXOPS(Arg->FD);
            LINUX_SERIAL_INCREASE_RXCNT(Arg->FD, Arg->Params.RxSize);
            return CFE_PSP_SUCCESS;
        }
    }

    return StatusCode;
}
int32_t linux_serial_read_spi(CFE_PSP_IODriver_SerialXfer_t *Arg) {
    int32 StatusCode;
    struct spi_ioc_transfer xfer[2] = {0,};
    uint8_t nmsg = 0;

    if (Arg->Params.TxData && Arg->Params.TxSize) {
        xfer[nmsg].tx_buf = (uint64_t)(uintptr_t)Arg->Params.TxData;
        xfer[nmsg].len = Arg->Params.TxSize;
        nmsg ++;
    }
    if (Arg->Params.RxData && Arg->Params.RxSize) {
        xfer[nmsg].rx_buf = (uint64_t)(uintptr_t)Arg->Params.RxData;
        xfer[nmsg].len = Arg->Params.RxSize;
        nmsg ++;
    }

    StatusCode = ioctl(Arg->FD, SPI_IOC_MESSAGE(nmsg), xfer);
    if (StatusCode < 0) {
        LINUX_SERIAL_INCREASE_TXERR(Arg->FD);
        LINUX_SERIAL_INCREASE_RXERR(Arg->FD);
        return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
    }
    else {
        LINUX_SERIAL_INCREASE_TXOPS(Arg->FD);
        LINUX_SERIAL_INCREASE_TXCNT(Arg->FD, Arg->Params.TxSize);
        LINUX_SERIAL_INCREASE_RXOPS(Arg->FD);
        LINUX_SERIAL_INCREASE_RXCNT(Arg->FD, Arg->Params.RxSize);
        return CFE_PSP_SUCCESS;
    }

    return StatusCode;
}
int32_t linux_serial_read_can(CFE_PSP_IODriver_SerialXfer_t *Arg) {
    int32 StatusCode;
    struct can_frame frame;
    size_t size = Arg->Params.RxSize;

    if (Arg->Params.TxData && Arg->Params.TxSize) {
        StatusCode = linux_serial_write_can(Arg);
        if (StatusCode != CFE_PSP_SUCCESS) {
            return StatusCode;
        }
        /* At this step, there is no need to increase counter. `write_can` already do */
        OS_TaskDelay(Arg->Params.Interval);
    }

    size_t TotBytes = 0; // Total Rx bytes till now
    size_t RdBytes; // Read bytes at this very time
    while (TotBytes < size) {
        RdBytes = (size - TotBytes >= CAN_MAX_DLEN) ? CAN_MAX_DLEN : (size - TotBytes);
        // Poll Read
        StatusCode = linux_serial_poll_read(Arg->FD, &frame, sizeof(struct can_frame), Arg->Params.Timeout);
        if (StatusCode != CFE_PSP_SUCCESS) {
            LINUX_SERIAL_INCREASE_RXERR(Arg->FD);
            return StatusCode;
        }

        uint32_t RxID = 0;
        if (frame.can_id & CAN_EFF_FLAG) RxID = frame.can_id & CAN_EFF_MASK;
        else RxID = frame.can_id & CAN_SFF_MASK;
        OS_printf("InComing CAN Frame ID: %u\n", RxID);

        if (RdBytes != frame.can_dlc) {
            OS_printf("%s: CAN read length NOT matched!\n", __func__);
        }
        memcpy((uint8_t *)Arg->Params.RxData + TotBytes, frame.data, RdBytes);
        TotBytes += RdBytes;
    }
    LINUX_SERIAL_INCREASE_RXOPS(Arg->FD);
    LINUX_SERIAL_INCREASE_RXCNT(Arg->FD, Arg->Params.RxSize);

    return StatusCode; // Guaranteed to `CFE_PSP_SUCCESS`
}
int32_t linux_serial_read_uart(CFE_PSP_IODriver_SerialXfer_t *Arg) {
    int32 StatusCode;
    
    if (Arg->Params.TxData && Arg->Params.TxSize) {
        StatusCode = ioctl(Arg->FD, TCFLSH, TCIOFLUSH);
        if (StatusCode < 0) {
            LINUX_SERIAL_INCREASE_SETUPERR(Arg->FD);
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }

        StatusCode = linux_serial_write(Arg->FD, Arg->Params.TxData, Arg->Params.TxSize);
        if (StatusCode != CFE_PSP_SUCCESS) {
            LINUX_SERIAL_INCREASE_TXERR(Arg->FD);
            return StatusCode;
        }
        LINUX_SERIAL_INCREASE_TXOPS(Arg->FD);
        LINUX_SERIAL_INCREASE_TXCNT(Arg->FD, Arg->Params.TxSize);
        OS_TaskDelay(Arg->Params.Interval);
    }

    StatusCode = linux_serial_poll_read(Arg->FD, Arg->Params.RxData, Arg->Params.RxSize, Arg->Params.Timeout);
    if (StatusCode != CFE_PSP_SUCCESS) {
        LINUX_SERIAL_INCREASE_RXERR(Arg->FD);
    }
    else {
        LINUX_SERIAL_INCREASE_RXOPS(Arg->FD);
        LINUX_SERIAL_INCREASE_RXCNT(Arg->FD, Arg->Params.TxSize);
    }
    return StatusCode;
}


/// @brief 
/// @param SubchannelId 
/// @param Arg Casted to `CFE_PSP_IODriver_SerialXfer_t`
/// @return 
int32_t linux_serial_read_dispatch(uint16_t SubchannelId, void *Arg) {
    int32_t StatusCode;

    switch (SubchannelId)
    {
    case CFE_PSP_IODriver_SERIAL_I2C_SUBCH:
        StatusCode = linux_serial_read_i2c((CFE_PSP_IODriver_SerialXfer_t *)Arg);
        break;
    case CFE_PSP_IODriver_SERIAL_SPI_SUBCH:
        StatusCode = linux_serial_read_spi((CFE_PSP_IODriver_SerialXfer_t *)Arg);
        break;
    case CFE_PSP_IODriver_SERIAL_UART_SUBCH:
        StatusCode = linux_serial_read_uart((CFE_PSP_IODriver_SerialXfer_t *)Arg);
        break;
    case CFE_PSP_IODriver_SERIAL_CAN_SUBCH:
        StatusCode = linux_serial_read_can((CFE_PSP_IODriver_SerialXfer_t *)Arg);
        break;
    default:
        /* Unsupported type */
        StatusCode = CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR;
        break;
    }

    return StatusCode;
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_serial_config()                               */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
int32_t linux_serial_config_i2c(CFE_PSP_IODriver_Serial_cfg_t *cfg) {
    int32_t StatusCode;
    int FD = GETFD(cfg);
    CFE_PSP_I2C_cfg_t i2c = GETCFG(i2c);

    /* 10 bit addressing */
    StatusCode = ioctl(FD, I2C_TENBIT, i2c.tenbit ? 1: 0);
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    /* SMBus PEC */
    StatusCode = ioctl(FD, I2C_PEC, i2c.pec_en ? 1: 0);
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    /* Driver level retries */
    StatusCode = ioctl(FD, I2C_RETRIES, (unsigned long)i2c.retries);
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    return CFE_PSP_SUCCESS;
}
int32_t linux_serial_config_spi(CFE_PSP_IODriver_Serial_cfg_t *cfg) {
    int32_t StatusCode;
    int FD = GETFD(cfg);
    CFE_PSP_SPI_cfg_t spi = GETCFG(spi);

    StatusCode = ioctl(FD, SPI_IOC_WR_MODE, &spi.mode);
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    StatusCode = ioctl(FD, SPI_IOC_WR_MAX_SPEED_HZ, &spi.speed);
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    StatusCode = ioctl(FD, SPI_IOC_WR_BITS_PER_WORD, &spi.bpw);
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    return CFE_PSP_SUCCESS;

}
int32_t linux_serial_config_can(CFE_PSP_IODriver_Serial_cfg_t *cfg) {
    int32_t StatusCode;
    int FD = GETFD(cfg);
    CFE_PSP_CAN_cfg_t can = GETCFG(can);

    /* if filter exist, set all filter*/
    if (can.filter && can.filter_num) { 
        size_t n = can.filter_num;
        struct can_filter *entry = calloc(n, sizeof(*entry));

        for (size_t i = 0; i < n; i++) {
            uint32_t id = can.filter->id;
            uint32_t mask = can.filter->mask;

            if (can.filter->is_ext) {
                id   &= CAN_EFF_MASK;
                mask &= CAN_EFF_MASK;

                id   |= CAN_EFF_FLAG;
                mask |= CAN_EFF_FLAG;
            }
            else {
                id   &= CAN_SFF_MASK;
                mask &= CAN_SFF_MASK;
            }
            entry[i].can_id = id;
            entry[i].can_mask = mask;

            can.filter ++;
        }
        StatusCode = setsockopt(FD, SOL_CAN_RAW, CAN_RAW_FILTER, entry, n * sizeof(*entry));
        free(entry);
        if (StatusCode < 0) {   
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }
    }
    else {
        StatusCode = setsockopt(FD, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
        if (StatusCode < 0) {
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }
    }

    int val;
    /* loopback option */
    val = can.loopback_en ? 1 : 0;
    StatusCode = setsockopt(FD, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &val, sizeof(val));
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    /* receive own msgs */
    val = can.recv_own_msgs ? 1 : 0;
    StatusCode = setsockopt(FD, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &val, sizeof(val));
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    /* time stamp */
    // val = can.timestamp_en ? 1 : 0;
    // StatusCode = setsockopt(FD, SOL_SOCKET, SO_TIMESTAMP, &val, sizeof(val));
    // if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    /* receive err frame */
    if (can.recv_err_frame) {
        can_err_mask_t errmask = CAN_ERR_MASK; // allow all error
        StatusCode = setsockopt(FD, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &errmask, sizeof(errmask));
        if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
    }
    
    return CFE_PSP_SUCCESS;
}
int32_t linux_serial_config_uart(CFE_PSP_IODriver_Serial_cfg_t *cfg) {
    int32_t StatusCode;
    struct termios2 tio2;
    
    int FD = GETFD(cfg);
    CFE_PSP_UART_cfg_t uart = GETCFG(uart);

    StatusCode = ioctl(FD, TCGETS2, &tio2);
    if (StatusCode < 0) return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;

    /**
     * Baud Rate Setting
     */
    tio2.c_cflag &= ~CBAUD;
    tio2.c_cflag |= BOTHER;
    tio2.c_ispeed = uart.baud;
    tio2.c_ospeed = uart.baud;

    /**
     * Flag setting
     */
    // Control Flag
    tio2.c_cflag |= (CLOCAL | CREAD); // Local connection, Enable Read
    tio2.c_cflag &= ~(PARENB | PARODD | CSTOPB); // Parity bit off, 1 Stop bit

    /* Parity bit */
    tio2.c_cflag &= ~(PARENB | PARODD); // none
    if (uart.parity == 1) tio2.c_cflag |= PARENB; // even
    else if (uart.parity == 2) tio2.c_cflag |= (PARENB | PARODD); // odd

    /* Data bit */
    tio2.c_cflag &= ~CSIZE; // Clear Data bit num
    switch (uart.databits) {
        case 5 : tio2.c_cflag |= CS5; break;
        case 6 : tio2.c_cflag |= CS6; break;
        case 7 : tio2.c_cflag |= CS7; break;
        default: tio2.c_cflag |= CS8; break;
    }

    /* Stop bit */
    if (uart.stopbits == 2) tio2.c_cflag |= CSTOPB;
    else tio2.c_cflag &= ~CSTOPB;

    // Local Flag
    tio2.c_lflag &= ~(ISIG | ICANON); // Neglect Terminal signal (like SIGINT), Read by character
    tio2.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL); // Echo off
    
    // Input Flag - Applied to OBC's input data
    tio2.c_iflag &= ~(INPCK | ISTRIP); // Parity check off, Masking(8 bit to 7 bit by Mask 0x7F) off
    tio2.c_iflag &= ~(IXON | IXOFF | IXANY); // Flow Control off
    tio2.c_iflag &= ~(ICRNL | INLCR | IGNCR); // input `\r` <-> `\n` off, ignore `\r` off

    // Output Flag - Applied to OBC's output data
    tio2.c_oflag &= ~(OCRNL | ONLCR | ONOCR | ONLRET | OPOST); // Post Process off & output `\r` <-> `\n` off

    StatusCode = ioctl(FD, TCSETS2, &tio2);
    if (StatusCode < 0) {
        return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
    }

    return CFE_PSP_SUCCESS;
}


/// @brief 
/// @param SubchannelId 
/// @param Arg [in] Casted to `CFE_PSP_IODriver_Serial_cfg_t`
/// @return 
int32_t linux_serial_config_dispatch(uint16_t SubchannelId, void *Arg) {
    int32_t StatusCode;
    CFE_PSP_IODriver_Serial_cfg_t *cfg = (CFE_PSP_IODriver_Serial_cfg_t *)Arg;

    switch (SubchannelId)
    {
    case CFE_PSP_IODriver_SERIAL_I2C_SUBCH:
        StatusCode = linux_serial_config_i2c(cfg);
        break;
    case CFE_PSP_IODriver_SERIAL_SPI_SUBCH:
        StatusCode = linux_serial_config_spi(cfg);
        break;
    case CFE_PSP_IODriver_SERIAL_CAN_SUBCH:
        StatusCode = linux_serial_config_can(cfg);
        break;
    case CFE_PSP_IODriver_SERIAL_UART_SUBCH:
        StatusCode = linux_serial_config_uart(cfg);
        break;
    
    default:
        /* Unsupported type */
        StatusCode = CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR;
        break;
    }
    if (StatusCode != CFE_PSP_SUCCESS) {
        LINUX_SERIAL_INCREASE_SETUPERR(cfg->FD);
    }

    return StatusCode;
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_serial_close()                                */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/// @brief 
/// @param SubchannelId 
/// @param Arg Casted to `CFE_PSP_IODriver_SerialXfer_t`
/// @return 
int32_t linux_serial_close_dispatch(uint16_t SubchannelId, void *Arg) {
    int32_t StatusCode;
    CFE_PSP_IODriver_SerialXfer_t *x = (CFE_PSP_IODriver_SerialXfer_t *)Arg;

    switch (SubchannelId)
    {
    case CFE_PSP_IODriver_SERIAL_I2C_SUBCH:
    case CFE_PSP_IODriver_SERIAL_SPI_SUBCH:
    case CFE_PSP_IODriver_SERIAL_CAN_SUBCH:
    case CFE_PSP_IODriver_SERIAL_UART_SUBCH:
        StatusCode = linux_serial_close(x->FD);
        break;
    default:
        /* Unsupported type */
        StatusCode = CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR;
        break;
    }

    return StatusCode;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_serial_DevCmd()                               */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/**
 * \brief Main entry point for API.
 *
 * This function is called through iodriver to invoke the linux_serial module.
 *
 * \par Assumptions, External Events, and Notes:
 *          None
 *
 * \param[in] CommandCode  The CFE_PSP_IODriver_xxx command.
 * \param[in] SubsystemId  The monitor subsystem identifier
 * \param[in] SubchannelId The monitor subchannel identifier
 * \param[in] Arg          The arguments for the corresponding command.
 *
 * \returns Status code
 * \retval Positive value if successful, negative value for error.
 */
int32_t linux_serial_DevCmd(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg) {
    
    int32 StatusCode;

    StatusCode = CFE_PSP_ERROR_NOT_IMPLEMENTED;
    switch (SubsystemId) {
        case LINUX_SERIAL_OPEN_SUBSYS:
            /* invoke `open()` */
            /* This subsystem returns FD for success, negative for error */
            StatusCode = linux_serial_open_dispatch(SubchannelId, Arg.ConstStr);
            break;

        case LINUX_SERIAL_WRITE_SUBSYS:
            /* invoke `write()` */
            StatusCode = linux_serial_write_dispatch(SubchannelId, Arg.Vptr);
            break;

        case LINUX_SERIAL_READ_SUBSYS:
            /* invoke `read()` */
            StatusCode = linux_serial_read_dispatch(SubchannelId, Arg.Vptr);
            break;
        case LINUX_SERIAL_CONFIG_SUBSYS:
            switch (CommandCode)
            {
                case CFE_PSP_IODriver_SET_CONFIGURATION:
                    /* invoke `ioctl()` or `termios kind` */
                    StatusCode = linux_serial_config_dispatch(SubchannelId, Arg.Vptr);
                    break;
                case CFE_PSP_IODriver_SERIAL_IO_GET_CNTS:
                    /* invoke counter collect function */
                    StatusCode = linux_serial_return_counter(Arg.Vptr);
                    break;
                case CFE_PSP_IODriver_SERIAL_IO_CLEAR_CNTS:
                    /* invoke counters clear function */
                    linux_serial_clear_cnt_entry((int)Arg.U32);
                    StatusCode = CFE_PSP_SUCCESS;
                default:
                    /* do nothing */
                    StatusCode = CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR;
                    break;
            }
            break;
        case LINUX_SERIAL_CLOSE_SUBSYS:
            /* invoke `close()` */
            StatusCode = linux_serial_close_dispatch(SubchannelId, Arg.Vptr);
            break;

        default:
            /* do nothing */
            StatusCode = CFE_PSP_IODriver_SERIAL_INVALID_TYPE_ERROR;
            break;
    }

    return StatusCode;
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_serial_DevMutex()                             */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/**
 * \brief Hash function for lock table indexing.
 *
 * This function is called through iodriver to invoke the linux_serial hash calculation.
 *
 * \par Assumptions, External Events, and Notes:
 *          None
 *
 * \param[in] CommandCode  The CFE_PSP_IODriver_xxx command.
 * \param[in] SubsystemId  The monitor subsystem identifier
 * \param[in] SubchannelId The monitor subchannel identifier
 * \param[in] Arg          The arguments for the corresponding command.
 *
 * \returns Status code
 * \retval Positive value if use lock/unlock, negative value for no lock.
 */
int32_t linux_serial_DevMutex(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg) {

    int32_t hash = 0;
    /* Open Subsystem does not need the lock */
    if (SubsystemId == LINUX_SERIAL_OPEN_SUBSYS)
        hash = LINUX_SERIAL_NO_MUTEX_HASH;

    
    /* Other Subsystem need lock, hash is calculated by FD val */
    else if (Arg.Vptr) {
        if (SubsystemId == LINUX_SERIAL_CONFIG_SUBSYS) {
            if (CommandCode == CFE_PSP_IODriver_SET_CONFIGURATION) {
                const CFE_PSP_IODriver_Serial_cfg_t *cfg = (CFE_PSP_IODriver_Serial_cfg_t *)Arg.Vptr;
                hash = CFE_PSP_IODriver_HashMutex(hash, cfg->FD);    
            }
            else {
                const CFE_PSP_IODriver_Serial_cnt_t *cnt = (CFE_PSP_IODriver_Serial_cnt_t *)Arg.Vptr;
                hash = CFE_PSP_IODriver_HashMutex(hash, cnt->FD);    
            }
        }
        else {
            const CFE_PSP_IODriver_SerialXfer_t *x = (CFE_PSP_IODriver_SerialXfer_t *)Arg.Vptr;
            hash = CFE_PSP_IODriver_HashMutex(hash, x->FD);
        }
    }
    
    return hash;
}