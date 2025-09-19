#include "p31u.h"
#include "eps_app.h"


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
    static uint8_t txBuf[P31U_I2C_TX_SIZE_MAX + 1] = {0,};
    static uint8_t rxBuf[P31U_I2C_RX_SIZE_MAX + 2] = {0,};
    uint16_t totTxLen, totRxLen;
    CFE_SRL_IO_Param_t Params = {0, };

    /**
     * Tx and Rx buffers are static; needs a size check.
     */
    if (txSize > P31U_I2C_TX_SIZE_MAX ||
        rxSize > P31U_I2C_RX_SIZE_MAX)
            return P31U_ERR_SIZE;

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
    totRxLen = rxSize + 2;

    Params.TxData = txBuf;
    Params.TxSize = totTxLen;
    Params.RxData = rxBuf;
    Params.RxSize = totRxLen;
    Params.Addr = EPS_I2C_ADDR;
    Params.Timeout = 100; // If combined transaction is wanted, delete this
    Params.Interval = 40;

    status = CFE_SRL_ApiRead(EPS_AppData.Handle, &Params);
    /**
     * If the transaction failed that would probably be EREMOTEIO (121)
     * - the slave not ready to send data.
     */
    if (status != 0)
        return P31U_ERR_XFER;
    
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
