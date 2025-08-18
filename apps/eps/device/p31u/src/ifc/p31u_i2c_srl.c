#include "p31u.h"

// #include "cfe_srl_handle.h"
#include <string.h>


#define P31U_I2C_TX_SIZE_MAX    58
#define P31U_I2C_RX_SIZE_MAX    131


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

    // status = CFE_SRL_TransactionI2C(handle, tx, txSize + 1, rxBuf, rxSize + 2, Addr);
status = 0;
    ec = rxBuf[1];

    /**
     * Early return if the Error Code is not OK.
     */
    if (ec != 0)
        return - (ec << 8);

    /**
     * If the transaction failed that would probably be EREMOTEIO (121)
     * - the slave not ready to send data.
     */
    if (status != 0)
        return P31U_ERR_XFER;

    /**
     * Reply[0] = port.
     */
    if (rxBuf[0] != port)
        return P31U_ERR_PORT;

    if (rx && rxSize > 0)
        memcpy(rx, rxBuf + 2, rxSize);

    return P31U_OK;
}
