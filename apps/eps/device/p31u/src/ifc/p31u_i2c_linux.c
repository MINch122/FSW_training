/**
 * @file p31u_i2c_linux.h
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief Test purpose linux i2c interface.
 * Last modified: 2025-07-27
 * Astrodynamics & Control Lab. 2025.
 */

#include "p31u.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>


#define P31U_I2C_TX_SIZE_MAX    58
#define P31U_I2C_RX_SIZE_MAX    131


static int fd;
static uint8_t addr;

int i2c_init(const char* dev, uint8_t address)
{
    fd = open(dev, O_RDWR);
    if (fd < 0) {
        fprintf(stderr,
                "failed to open %s: %s\n",
                dev,
                strerror(errno));
        return -1;
    }

    // if (ioctl(fd, I2C_SLAVE, address) < 0) {
    //     fprintf(stderr,
    //             "failed to set slave addr %u: %s\n",
    //             address,
    //             strerror(errno));
    //     return -1;
    // }

    // addr = address;

    printf("i2c dev %s initialized.\n",
           dev);
    return 0;
}


#ifdef P31U_I2C_LINUX_USE_MBUS

int p31u_transaction(uint8_t port,
                     const void* tx,
                     uint16_t txSize,
                     void* rx,
                     uint16_t rxSize)
{
    int status;
    uint8_t ec;
	struct i2c_msg msg[2];
    struct i2c_rdwr_ioctl_data i2c;

    static uint8_t txBuf[P31U_I2C_TX_SIZE_MAX + 1];
    static uint8_t rxBuf[P31U_I2C_RX_SIZE_MAX + 2];

    if (txSize > P31U_I2C_TX_SIZE_MAX ||
        rxSize > P31U_I2C_RX_SIZE_MAX)
            return P31U_ERR_SIZE;

    memset(rxBuf, 0, sizeof(rxBuf));

    txBuf[0] = port;

    if (tx && txSize > 0)
        memcpy(txBuf + 1, tx, txSize);
    
    txSize++;
    rxSize += 2;

    printf("tx: ");
    for (int i = 0; i < txSize; ++i)
        printf("%02X ", ((uint8_t*) txBuf)[i]);
    printf("\n");

    msg[0].addr   = addr;
    msg[0].buf    = txBuf;
    msg[0].len    = txSize;
    msg[0].flags  = 0;
    msg[1].addr  = addr;
    msg[1].buf   = rxBuf;
    msg[1].len   = rxSize;
    msg[1].flags = I2C_M_RD;

    i2c.nmsgs = 2;
    i2c.msgs = msg;

    if (ioctl(fd, I2C_RDWR, &i2c) < 0) {
        fprintf(stderr,
                "ioctl xfer err: %s\n",
                strerror(errno));
        return P31U_ERR_XFER;
	}

    printf("rx: ");
    for (int i = 0; i < rxSize; ++i)
        printf("%02X ", ((uint8_t*) rxBuf)[i]);
    printf("\n");

    memcpy(rx, rxBuf + 2, rxSize - 2);

    return P31U_OK;
}

#else

int p31u_transaction(uint8_t port,
                     const void* tx,
                     uint16_t txSize,
                     void* rx,
                     uint16_t rxSize)
{
    int status;
    uint8_t ec;
    static uint8_t txBuf[P31U_I2C_TX_SIZE_MAX + 1];
    static uint8_t rxBuf[P31U_I2C_RX_SIZE_MAX + 2];
    uint16_t totTxLen, totRxLen;

    /**
     * Tx and Rx buffers are static; needs a size check.
     */
    if (txSize > P31U_I2C_TX_SIZE_MAX ||
        rxSize > P31U_I2C_RX_SIZE_MAX)
            return P31U_ERR_SIZE;

    memset(rxBuf, 0, sizeof(rxBuf));

    /**
     * Always send a port number even with no tx data.
     */
    txBuf[0] = port;

    if (tx && txSize > 0)
        memcpy(txBuf + 1, tx, txSize);

    /**
     * Append the port size.
     */
    totTxLen = txSize + 1;

    status = write(fd, txBuf, totTxLen);
    if (status != totTxLen) {
        fprintf(stderr,
                "write err: %u/%u written (%s)\n",
                status < 0 ? 0 : status,
                totTxLen,
                strerror(errno));
        return P31U_ERR_WRITE;
    }

    printf("tx: ");
    for (int i = 0; i < totTxLen; ++i)
        printf("%02X ", ((const uint8_t*) txBuf)[i]);
    printf("\n");

    /**
     * Append the port and EC sizes.
     */
    totRxLen = rxSize + 2;

    status = read(fd, rxBuf, totRxLen);
    if (status != totRxLen) {
        fprintf(stderr,
                "read err: %u/%u read (%s)\n",
                status < 0 ? 0 : status,
                totRxLen,
                strerror(errno));
        return P31U_ERR_READ;
    }

    printf("rx: ");
    for (int i = 0; i < totRxLen; ++i)
        printf("%02X ", ((const uint8_t*) rxBuf)[i]);
    printf("\n");

    ec = rxBuf[1];

    /**
     * Early return if the Error Code is not OK.
     */
    if (ec != 0)
        return - (ec << 8);

    /**
     * Reply[0] = port.
     */
    if (rxBuf[0] != port)
        return P31U_ERR_PORT;

    if (rx && rxSize > 0)
        memcpy(rx, rxBuf + 2, rxSize);

    return P31U_OK;
}

#endif
