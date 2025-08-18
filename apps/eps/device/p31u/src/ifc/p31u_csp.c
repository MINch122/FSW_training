/**
 * @file p31u_csp.c
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief GomSpace NanoPower P31u Power System interface layer (CSP mode).
 * Last modified: 2025-07-25
 * Astrodynamics & Control Lab. 2024. ryu@yonsei.ac.kr
 */
#include "p31u.h"

#include <csp/csp.h>
#include <stdatomic.h>

static atomic_uint p31u_csp_timeout = P31U_CSP_DEFAULT_TRANS_TIMEOUT;
static uint8_t p31u_csp_node        = P31U_CSP_DEFAULT_NODE;

void p31u_csp_set_transaction_timeout(uint32_t timeout) {
    p31u_csp_timeout = timeout;
}

void p31u_csp_set_node(uint8_t node) {
    p31u_csp_node = node;
}

int p31u_transaction(uint8_t port,
                     const void* tx,
                     uint16_t txSize,
                     void* rx,
                     uint16_t rxSize)
{
    int status;

    /**
     * It is possible that both @a tx and @a rx are null,
     * which would be a simple "no-op" sent to the @a port.
     * So we don't do any pointer sanity checks here.
     */
    status = csp_transaction(CSP_PRIO_HIGH,
                             p31u_csp_node,
                             port,
                             p31u_csp_timeout,
                             (void*)tx,
                             txSize,
                             rx,
                             rxSize);

    /**
     * If tx only, csp_transaction() should return 1 on success.
     */
    if (!rx || rxSize == 0)
        return status == 1 ? P31U_OK : P31U_ERR_WRITE;

    /**
     * Else, should've returned @a rxSize.
     */
    if (status == rxSize)
        return P31U_OK;

    /**
     * Else did read something but not completely.
     */
    if (status > 0)
        return P31U_ERR_READ;

    /**
     * If returned 0 when a reply was expected, there's no way to determine
     * where it went wrong between send and read.
     */
    return P31U_ERR_XFER;
}
