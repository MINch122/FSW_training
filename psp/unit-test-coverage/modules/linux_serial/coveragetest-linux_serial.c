/***********************************************************************
 *  Copyright (c) 2025, Yonsei University as represented by the
 *  Department of Satellite Systems (DSS) & Astrodynamic & Control Lab (ACL)
 *  All rights reserved. This software was created at DSS
 *  
 *  Author : Hyeok-jin Kweon
 * 
 *  \file coveragetest-linux_serial.c
 *
 ***********************************************************************/

/**
 * \file
 * \ingroup  modules
 *
 * Coverage test for linux serial module implementation
 */
#include "utassert.h"
#include "utstubs.h"
#include "uttest.h"

#include "cfe_psp.h"
#include "cfe_psp_config.h"
#include "cfe_psp_module.h"
#include "iodriver_serial_io.h"

#include "PCS_unistd.h"
#include "PCS_fcntl.h"
#include "PCS_ioctl.h"
#include "PCS_sys_socket.h"
#include "PCS_stdlib.h"
#include "PCS_string.h"

#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/if.h>

/*
 * Macro to add a test case to the list of tests to execute
 */
#define ADD_TEST(test) UtTest_Add(test, ResetTest, NULL, #test)

void ResetTest(void)
{
    UT_ResetState(0);
}

extern int32 linux_serial_write(int FD, void *data, size_t size);
extern int32 linux_serial_read(int FD, void *data, size_t size);
extern int32 linux_serial_close(int FD);

extern int32 linux_serial_open(const char *dev, int opt);
extern int32 linux_serial_opensocket(const char *dev);
extern int32_t linux_serial_open_dispatch(uint16_t SubchannelId, const char *Arg);

extern int32_t linux_serial_write_i2c(CFE_PSP_IODriver_SerialXfer_t *arg);
extern int32_t linux_serial_write_spi(CFE_PSP_IODriver_SerialXfer_t *arg);
extern int32_t linux_serial_write_can(CFE_PSP_IODriver_SerialXfer_t *arg);
extern int32_t linux_serial_write_uart(CFE_PSP_IODriver_SerialXfer_t *arg);
extern int32_t linux_serial_write_dispatch(uint16_t SubchannelId, void *Arg);

extern int32_t linux_serial_read_i2c(CFE_PSP_IODriver_SerialXfer_t *Arg);
extern int32_t linux_serial_read_spi(CFE_PSP_IODriver_SerialXfer_t *Arg);
int32_t linux_serial_read_can2(CFE_PSP_IODriver_SerialXfer_t *Arg) {
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
        StatusCode = linux_serial_read(Arg->FD, &frame, sizeof(struct can_frame));
        if (StatusCode != CFE_PSP_SUCCESS) {
            // LINUX_SERIAL_INCREASE_RXERR(Arg->FD);
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
    // LINUX_SERIAL_INCREASE_RXOPS(Arg->FD);
    // LINUX_SERIAL_INCREASE_RXCNT(Arg->FD, Arg->Params.RxSize);

    return StatusCode; // Guaranteed to `CFE_PSP_SUCCESS`
}
int32_t linux_serial_read_uart2(CFE_PSP_IODriver_SerialXfer_t *Arg) {
    int32 StatusCode;
    
    if (Arg->Params.TxData && Arg->Params.TxSize) {
        StatusCode = PCS_ioctl(Arg->FD, PCS_TCFLSH, 2);
        if (StatusCode < 0) {
            // LINUX_SERIAL_INCREASE_SETUPERR(Arg->FD);
            return CFE_PSP_IODriver_SERIAL_IOCTL_ERROR;
        }

        StatusCode = linux_serial_write(Arg->FD, Arg->Params.TxData, Arg->Params.TxSize);
        if (StatusCode != CFE_PSP_SUCCESS) {
            // LINUX_SERIAL_INCREASE_TXERR(Arg->FD);
            return StatusCode;
        }
        // LINUX_SERIAL_INCREASE_TXOPS(Arg->FD);
        // LINUX_SERIAL_INCREASE_TXCNT(Arg->FD, Arg->Params.TxSize);
        OS_TaskDelay(Arg->Params.Interval);
    }

    StatusCode = linux_serial_read(Arg->FD, Arg->Params.RxData, Arg->Params.RxSize);
    if (StatusCode != CFE_PSP_SUCCESS) {
        // LINUX_SERIAL_INCREASE_RXERR(Arg->FD);
    }
    else {
        // LINUX_SERIAL_INCREASE_RXOPS(Arg->FD);
        // LINUX_SERIAL_INCREASE_RXCNT(Arg->FD, Arg->Params.TxSize);
    }
    return StatusCode;
    
}
extern int32_t linux_serial_read_dispatch(uint16_t SubchannelId, void *Arg);

extern int32_t linux_serial_config_i2c(CFE_PSP_IODriver_Serial_cfg_t *cfg);
extern int32_t linux_serial_config_spi(CFE_PSP_IODriver_Serial_cfg_t *cfg);
extern int32_t linux_serial_config_can(CFE_PSP_IODriver_Serial_cfg_t *cfg);
extern int32_t linux_serial_config_uart(CFE_PSP_IODriver_Serial_cfg_t *cfg);
extern int32_t linux_serial_config_dispatch(uint16_t SubchannelId, void *Arg);

extern int32_t linux_serial_close_dispatch(uint16_t SubchannelId, void *Arg);

extern int32_t linux_serial_DevCmd(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg);
extern int32_t linux_serial_DevMutex(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg);


uint8_t TempTxBuf = {0xFF};
uint8_t TempRxBuf = {0xFF};
CFE_PSP_IODriver_SerialXfer_t UT_Xfer = {.FD = 3,
                                         .Params.TxData = &TempTxBuf,
                                         .Params.TxSize = sizeof(TempTxBuf),
                                         .Params.RxData = &TempRxBuf,
                                         .Params.RxSize = sizeof(TempRxBuf),
                                         .Params.Addr = 0x23,
                                         .Params.Timeout = 100,
                                         .Params.Interval = 100};

#define DEFAULT_DEV_FILE    "/dev/i2c-0"
void Test_linux_serial_open(void) {
    UtAssert_INT32_GTEQ(linux_serial_open(DEFAULT_DEV_FILE, PCS_O_RDWR), CFE_PSP_SUCCESS);

    int32 forced_ret_val = -1;
    UT_SetDeferredRetcode(UT_KEY(PCS_open), 1, forced_ret_val);
    UtAssert_INT32_EQ(linux_serial_open(DEFAULT_DEV_FILE, PCS_O_RDWR), CFE_PSP_IODriver_SERIAL_OPEN_ERROR);
}

void Test_linux_serial_socket(void) {
    int32 default_ret_fd = 23;

    UT_SetDefaultReturnValue(UT_KEY(PCS_socket), default_ret_fd);
    
    /* Test nominal case */
    UtAssert_INT32_GTEQ(linux_serial_opensocket(DEFAULT_DEV_FILE), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_ioctl, 1);

    int32 forced_ret_val = -1;
    UT_SetDeferredRetcode(UT_KEY(PCS_socket), 1, forced_ret_val);
    UtAssert_INT32_EQ(linux_serial_opensocket(DEFAULT_DEV_FILE), CFE_PSP_IODriver_SERIAL_SOCKET_ERROR);

    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 1, forced_ret_val);
    UtAssert_INT32_EQ(linux_serial_opensocket(DEFAULT_DEV_FILE), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);

    UT_SetDeferredRetcode(UT_KEY(PCS_bind), 1, forced_ret_val);
    UtAssert_INT32_EQ(linux_serial_opensocket(DEFAULT_DEV_FILE), CFE_PSP_IODriver_SERIAL_BIND_ERROR);
}


void Test_linux_serial_write(void) {
    /* Nominal case */
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, UT_Xfer.Params.TxSize);
    UtAssert_INT32_EQ(linux_serial_write(UT_Xfer.FD, UT_Xfer.Params.TxData, UT_Xfer.Params.TxSize), CFE_PSP_SUCCESS);

    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, -1);
    UtAssert_INT32_EQ(linux_serial_write(UT_Xfer.FD, UT_Xfer.Params.TxData, UT_Xfer.Params.TxSize), CFE_PSP_IODriver_SERIAL_WRITE_ERROR);

    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, UT_Xfer.Params.TxSize -1);
    UtAssert_INT32_EQ(linux_serial_write(UT_Xfer.FD, UT_Xfer.Params.TxData, UT_Xfer.Params.TxSize), CFE_PSP_IODriver_SERIAL_PARTIAL_WRITE_ERROR);
}

void Test_linux_serial_read(void) {
    /* Nominal case */
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, UT_Xfer.Params.RxSize);
    UtAssert_INT32_EQ(linux_serial_read(UT_Xfer.FD, UT_Xfer.Params.RxData, UT_Xfer.Params.RxSize), CFE_PSP_SUCCESS);

    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, -1);
    UtAssert_INT32_EQ(linux_serial_read(UT_Xfer.FD, UT_Xfer.Params.RxData, UT_Xfer.Params.RxSize), CFE_PSP_IODriver_SERIAL_READ_ERROR);

    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, UT_Xfer.Params.RxSize -1);
    UtAssert_INT32_EQ(linux_serial_read(UT_Xfer.FD, UT_Xfer.Params.RxData, UT_Xfer.Params.RxSize), CFE_PSP_IODriver_SERIAL_PARTIAL_READ_ERROR);
}

void Test_linux_serial_close(void) {
    UtAssert_INT32_EQ(linux_serial_close(UT_Xfer.FD), CFE_PSP_SUCCESS);

    int32_t forced_ret_val = -1;
    UT_SetDeferredRetcode(UT_KEY(PCS_close), 1, forced_ret_val);
    UtAssert_INT32_EQ(linux_serial_close(UT_Xfer.FD), CFE_PSP_IODriver_SERIAL_CLOSE_ERROR);
}

/*----------------------End of basic function---------------------*/

/*---------------------Write function test--------------------------*/
void Test_linux_serial_write_i2c(void) {
    CFE_PSP_IODriver_SerialXfer_t i2c_Xfer = {.FD = 3,
                                         .Params.TxData = &TempTxBuf,
                                         .Params.TxSize = sizeof(TempTxBuf),
                                         .Params.RxData = &TempRxBuf,
                                         .Params.RxSize = sizeof(TempRxBuf),
                                         .Params.Addr = 0x23,
                                         .Params.Timeout = 100,
                                         .Params.Interval = 100};
    /* Mominal combined case */
    UtAssert_INT32_EQ(linux_serial_write_i2c(&i2c_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_ioctl, 1);

    /* Error in combined case */
    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 1, -1);
    UtAssert_INT32_EQ(linux_serial_write_i2c(&i2c_Xfer), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 2); /* call count cumulated */

    /* Nominal separate case */
    i2c_Xfer.Params.TxSize = 4;
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, i2c_Xfer.Params.TxSize);
    UtAssert_INT32_EQ(linux_serial_write_i2c(&i2c_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_ioctl, 4);

    /* Error in separate case */
    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 1, -1);
    UtAssert_INT32_EQ(linux_serial_write_i2c(&i2c_Xfer), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 5);

    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, -1);
    UtAssert_INT32_EQ(linux_serial_write_i2c(&i2c_Xfer), CFE_PSP_IODriver_SERIAL_WRITE_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 7);
    UtAssert_STUB_COUNT(PCS_write, 2);

    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, i2c_Xfer.Params.TxSize - 1);
    UtAssert_INT32_EQ(linux_serial_write_i2c(&i2c_Xfer), CFE_PSP_IODriver_SERIAL_PARTIAL_WRITE_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 9);
    UtAssert_STUB_COUNT(PCS_write, 3);
}

void Test_linux_serial_write_spi(void) {
    CFE_PSP_IODriver_SerialXfer_t spi_Xfer = {.FD = 3,
                                         .Params.TxData = &TempTxBuf,
                                         .Params.TxSize = sizeof(TempTxBuf),
                                         .Params.RxData = &TempRxBuf,
                                         .Params.RxSize = sizeof(TempRxBuf),
                                         .Params.Addr = 0x23,
                                         .Params.Timeout = 100,
                                         .Params.Interval = 100};

    
    /* Mominal combined case */
    UtAssert_INT32_EQ(linux_serial_write_spi(&spi_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_ioctl, 1);

    /* Error in combined case */
    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 1, -1);
    UtAssert_INT32_EQ(linux_serial_write_spi(&spi_Xfer), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 2);
}

void Test_linux_serial_write_can(void) {
    CFE_PSP_IODriver_SerialXfer_t can_Xfer = {.FD = 3,
                                        .Params.TxData = &TempTxBuf,
                                        .Params.TxSize = sizeof(TempTxBuf),
                                        .Params.RxData = &TempRxBuf,
                                        .Params.RxSize = sizeof(TempRxBuf),
                                        .Params.Addr = 0x23,
                                        .Params.Timeout = 100,
                                        .Params.Interval = 100};

    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, 16); /* struct can_frame size 16 */
    UtAssert_INT32_EQ(linux_serial_write_can(&can_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_write, 1);

    /* Test segmented packet */
    uint8_t Buf[10] = {0,};
    can_Xfer.Params.TxData = Buf;
    can_Xfer.Params.TxSize = 10;
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, 16);
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 2, 16);
    UtAssert_INT32_EQ(linux_serial_write_can(&can_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_write, 3);

    /* Error in 1st pakcet tx */
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, -1);
    UtAssert_INT32_EQ(linux_serial_write_can(&can_Xfer), CFE_PSP_IODriver_SERIAL_WRITE_ERROR);
    UtAssert_STUB_COUNT(PCS_write, 4);
}

void Test_linux_serial_write_uart(void) {
    CFE_PSP_IODriver_SerialXfer_t uart_Xfer = {.FD = 3,
                                        .Params.TxData = &TempTxBuf,
                                        .Params.TxSize = sizeof(TempTxBuf),
                                        .Params.RxData = &TempRxBuf,
                                        .Params.RxSize = sizeof(TempRxBuf),
                                        .Params.Addr = 0x23,
                                        .Params.Timeout = 100,
                                        .Params.Interval = 100};

    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, uart_Xfer.Params.TxSize);
    UtAssert_INT32_EQ(linux_serial_write_uart(&uart_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_write, 1);

    /* Error */
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, -1);
    UtAssert_INT32_EQ(linux_serial_write_uart(&uart_Xfer), CFE_PSP_IODriver_SERIAL_WRITE_ERROR);
    UtAssert_STUB_COUNT(PCS_write, 2);
}
/*--------------------------End of Write function test--------------------------*/

/*-------------------------Read function test--------------------------*/
void Test_linux_serial_read_i2c(void) {
    CFE_PSP_IODriver_SerialXfer_t i2c_Xfer = {.FD = 3,
                                        .Params.TxData = &TempTxBuf,
                                        .Params.TxSize = sizeof(TempTxBuf),
                                        .Params.RxData = &TempRxBuf,
                                        .Params.RxSize = sizeof(TempRxBuf),
                                        .Params.Addr = 0x23,
                                        .Params.Timeout = 100,
                                        .Params.Interval = 100};
                                        
    /* Nominal separate transaction */
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, i2c_Xfer.Params.TxSize);
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, i2c_Xfer.Params.RxSize);
    UtAssert_INT32_EQ(linux_serial_read_i2c(&i2c_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_write, 1);
    UtAssert_STUB_COUNT(OS_TaskDelay, 1);
    UtAssert_STUB_COUNT(PCS_read, 1);

    /* Error in separate transaction */
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, -1);
    UtAssert_INT32_EQ(linux_serial_read_i2c(&i2c_Xfer), CFE_PSP_IODriver_SERIAL_WRITE_ERROR);
    UtAssert_STUB_COUNT(PCS_write, 2);
    UtAssert_STUB_COUNT(PCS_read, 1);

    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, i2c_Xfer.Params.TxSize);
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, -1);
    UtAssert_INT32_EQ(linux_serial_read_i2c(&i2c_Xfer), CFE_PSP_IODriver_SERIAL_READ_ERROR);
    UtAssert_STUB_COUNT(PCS_write, 3);
    UtAssert_STUB_COUNT(OS_TaskDelay, 2);
    UtAssert_STUB_COUNT(PCS_read, 2);

    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 1, -1);
    UtAssert_INT32_EQ(linux_serial_read_i2c(&i2c_Xfer), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 7);
    UtAssert_STUB_COUNT(PCS_write, 3);
    UtAssert_STUB_COUNT(PCS_read, 2);

    /* Nominal combined transaction */
    i2c_Xfer.Params.Interval = 0;
    UtAssert_INT32_EQ(linux_serial_read_i2c(&i2c_Xfer), CFE_PSP_SUCCESS);

    /* Error in combined */
    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 1, -1);
    UtAssert_INT32_EQ(linux_serial_read_i2c(&i2c_Xfer), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);

}

void Test_linux_serial_read_spi(void) {
    CFE_PSP_IODriver_SerialXfer_t spi_Xfer = {.FD = 3,
                                        .Params.TxData = &TempTxBuf,
                                        .Params.TxSize = sizeof(TempTxBuf),
                                        .Params.RxData = &TempRxBuf,
                                        .Params.RxSize = sizeof(TempRxBuf),
                                        .Params.Addr = 0x23,
                                        .Params.Timeout = 100,
                                        .Params.Interval = 100};
    /* Nominal case */
    UtAssert_INT32_EQ(linux_serial_read_spi(&spi_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_ioctl, 1);

    /* Error case */
    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 1, -1);
    UtAssert_INT32_EQ(linux_serial_read_spi(&spi_Xfer), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 2);
}

void Test_linux_serial_read_can(void) {
    CFE_PSP_IODriver_SerialXfer_t can_Xfer = {.FD = 3,
                                        .Params.TxData = &TempTxBuf,
                                        .Params.TxSize = sizeof(TempTxBuf),
                                        .Params.RxData = &TempRxBuf,
                                        .Params.RxSize = sizeof(TempRxBuf),
                                        .Params.Addr = 0x23,
                                        .Params.Timeout = 100,
                                        .Params.Interval = 100};
    /* Nominal case */
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, sizeof(struct can_frame));
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, sizeof(struct can_frame));
    UtAssert_INT32_EQ(linux_serial_read_can2(&can_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_write, 1);
    UtAssert_STUB_COUNT(PCS_read, 1);
    UtAssert_STUB_COUNT(OS_TaskDelay, 1);

    /* Error in write */
    UT_ResetState(0);
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, sizeof(struct can_frame)-1);
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, sizeof(struct can_frame));
    UtAssert_INT32_EQ(linux_serial_read_can2(&can_Xfer), CFE_PSP_IODriver_SERIAL_PARTIAL_WRITE_ERROR);
    UtAssert_STUB_COUNT(PCS_write, 1);
    UtAssert_STUB_COUNT(PCS_read, 0);
    UtAssert_STUB_COUNT(OS_TaskDelay, 0);

    /* Error in read */
    UT_ResetState(0);
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, sizeof(struct can_frame));
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, sizeof(struct can_frame)-1);
    UtAssert_INT32_EQ(linux_serial_read_can2(&can_Xfer), CFE_PSP_IODriver_SERIAL_PARTIAL_READ_ERROR);
    UtAssert_STUB_COUNT(PCS_write, 1);
    UtAssert_STUB_COUNT(PCS_read, 1);
    UtAssert_STUB_COUNT(OS_TaskDelay, 1);
}

void Test_linux_serial_read_uart(void) {
    CFE_PSP_IODriver_SerialXfer_t uart_Xfer = {.FD = 3,
                                        .Params.TxData = &TempTxBuf,
                                        .Params.TxSize = sizeof(TempTxBuf),
                                        .Params.RxData = &TempRxBuf,
                                        .Params.RxSize = sizeof(TempRxBuf),
                                        .Params.Addr = 0x23,
                                        .Params.Timeout = 100,
                                        .Params.Interval = 100};
    /* Nominal case */
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, uart_Xfer.Params.TxSize);
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, uart_Xfer.Params.RxSize);
    UtAssert_INT32_EQ(linux_serial_read_uart2(&uart_Xfer), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_write, 1);
    UtAssert_STUB_COUNT(PCS_read, 1);
    UtAssert_STUB_COUNT(OS_TaskDelay, 1);

    /* Error in write */
    UT_ResetState(0);
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, 0);
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, uart_Xfer.Params.RxSize);
    UtAssert_INT32_EQ(linux_serial_read_uart2(&uart_Xfer), CFE_PSP_IODriver_SERIAL_PARTIAL_WRITE_ERROR);
    UtAssert_STUB_COUNT(PCS_write, 1);
    UtAssert_STUB_COUNT(PCS_read, 0);
    UtAssert_STUB_COUNT(OS_TaskDelay, 0);

    /* Error in read */
    UT_ResetState(0);
    UT_SetDeferredRetcode(UT_KEY(PCS_write), 1, uart_Xfer.Params.TxSize);
    UT_SetDeferredRetcode(UT_KEY(PCS_read), 1, 0);
    UtAssert_INT32_EQ(linux_serial_read_uart2(&uart_Xfer), CFE_PSP_IODriver_SERIAL_PARTIAL_READ_ERROR);
    UtAssert_STUB_COUNT(PCS_write, 1);
    UtAssert_STUB_COUNT(PCS_read, 1);
    UtAssert_STUB_COUNT(OS_TaskDelay, 1);
}


void Test_linux_serial_config_i2c(void) {
    CFE_PSP_IODriver_Serial_cfg_t cfg = {0,};

    UtAssert_INT32_EQ(linux_serial_config_i2c(&cfg), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_ioctl, 3);

    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 2, -1);
    UtAssert_INT32_EQ(linux_serial_config_i2c(&cfg), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 5);
}

void Test_linux_serial_config_spi(void) {
    CFE_PSP_IODriver_Serial_cfg_t cfg = {0,};

    UtAssert_INT32_EQ(linux_serial_config_spi(&cfg), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_ioctl, 3);

    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 2, -1);
    UtAssert_INT32_EQ(linux_serial_config_spi(&cfg), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 5);
}

void Test_linux_serial_config_can(void) {
    CFE_PSP_IODriver_Serial_cfg_t cfg = {0,};

    UtAssert_INT32_EQ(linux_serial_config_can(&cfg), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_setsockopt, 3);

    UT_SetDeferredRetcode(UT_KEY(PCS_setsockopt), 2, -1);
    UtAssert_INT32_EQ(linux_serial_config_can(&cfg), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_setsockopt, 5);
}

void Test_linux_serial_config_uart(void) {
    CFE_PSP_IODriver_Serial_cfg_t cfg = {0,};

    UtAssert_INT32_EQ(linux_serial_config_uart(&cfg), CFE_PSP_SUCCESS);
    UtAssert_STUB_COUNT(PCS_ioctl, 2);

    UT_SetDeferredRetcode(UT_KEY(PCS_ioctl), 2, -1);
    UtAssert_INT32_EQ(linux_serial_config_uart(&cfg), CFE_PSP_IODriver_SERIAL_IOCTL_ERROR);
    UtAssert_STUB_COUNT(PCS_ioctl, 4);
}

/*--------------------------End of read function test--------------------------*/

void UtTest_Setup(void) {
    ADD_TEST(Test_linux_serial_open);
    ADD_TEST(Test_linux_serial_socket);
    ADD_TEST(Test_linux_serial_write);
    ADD_TEST(Test_linux_serial_read);
    ADD_TEST(Test_linux_serial_close);

    ADD_TEST(Test_linux_serial_write_i2c);
    ADD_TEST(Test_linux_serial_write_spi);
    ADD_TEST(Test_linux_serial_write_can);
    ADD_TEST(Test_linux_serial_write_uart);

    ADD_TEST(Test_linux_serial_read_i2c);
    ADD_TEST(Test_linux_serial_read_spi);
    ADD_TEST(Test_linux_serial_read_can);
    ADD_TEST(Test_linux_serial_read_uart);

    ADD_TEST(Test_linux_serial_config_i2c);
    ADD_TEST(Test_linux_serial_config_spi);
    ADD_TEST(Test_linux_serial_config_can);
    ADD_TEST(Test_linux_serial_config_uart);
}