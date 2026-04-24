/**
 * @file s5lab_csp.c
 * @author ryu@yonsei.ac.kr
 * @brief S5Lab CSP transaction layer. 
 * @version 0.1
 * @date 2026-03-21
 * 
 * Yonsei ACL, 2026
 */
#include "s5lab.h"
#include <csp/csp.h>


static volatile uint8_t s5lab_csp_node = S5LAB_CONF_CSP_DEFAULT_NODE;
static volatile uint16_t s5lab_csp_timeout_ms = S5LAB_CONF_CSP_DEFAULT_TIMEOUT;


void s5lab_csp_set_node(uint8_t node)
{
    s5lab_csp_node = node;
}


void s5lab_csp_set_timeout(uint16_t timeout_ms)
{
    s5lab_csp_timeout_ms = timeout_ms;
}


uint8_t s5lab_csp_get_node(void)
{
    return s5lab_csp_node;
} 


uint16_t s5lab_csp_get_timeout(void)
{
    return s5lab_csp_timeout_ms;
}


bool s5lab_csp_payload_fits(uint16_t payload_len)
{
    return payload_len <= csp_buffer_size() - CSP_BUFFER_PACKET_OVERHEAD;
}


typedef struct {
    int (*handler)(int index,
                   void* reply,
                   uint16_t reply_size,
                   void* user_data);
    void* user_data;
} multiple_reply_context_t;


int s5lab_csp_send(uint8_t port,
                   const void* tx,
                   uint16_t tx_len,
                   uint16_t timeout_ms,
                   csp_conn_t** conn_out)
{
    csp_packet_t* pkt;
    csp_conn_t* conn;

    conn = csp_connect(CSP_PRIO_NORM, s5lab_csp_node, port, timeout_ms, CSP_O_NONE);
    if (!conn)
        return S5LAB_ERR_CONN;
    
    pkt = csp_buffer_get(tx_len);
    if (!pkt) {
        csp_close(conn);
        return S5LAB_ERR_CSP_BUFFER;
    }

    if (tx)
        memcpy(pkt->data, tx, tx_len);

    pkt->length = tx_len;
    if (!csp_send(conn, pkt, timeout_ms)) {
        csp_buffer_free(pkt);
        csp_close(conn);
        return S5LAB_ERR_SEND;
    }

    if (conn_out)
        *conn_out = conn;
    else
        csp_close(conn);
    
    return S5LAB_OK;
}


int s5lab_transaction(uint8_t port,
                      const void* tx,
                      uint16_t tx_len,
                      void* rx,
                      uint16_t rx_len,
                      uint16_t timeout_ms)
{
    int ret;
    csp_packet_t* pkt;
    csp_conn_t* conn;

    ret = s5lab_csp_send(port, tx, tx_len, timeout_ms, &conn);
    if (ret != S5LAB_OK)
        return ret;

    if (!rx || rx_len == 0) {
        csp_close(conn);
        return S5LAB_OK;
    }

    pkt = csp_read(conn, timeout_ms);
    if (!pkt) {
        csp_close(conn);
        return S5LAB_ERR_TIMEOUT;
    }
    
    if (pkt->length != rx_len) {
        csp_buffer_free(pkt);
        csp_close(conn);
        return S5LAB_ERR_REPLY_SIZE;
    }

    if (rx)
        memcpy(rx, pkt->data, rx_len);

    csp_buffer_free(pkt);
    csp_close(conn);
    return S5LAB_OK;
}


int s5lab_transaction_unsized(uint8_t port,
                              const void* tx,
                              uint16_t tx_len,
                              void* rx,
                              uint16_t rx_buf_len,
                              uint16_t* rx_len,
                              uint16_t timeout_ms)
{
    int ret;
    csp_conn_t* conn;
    csp_packet_t* pkt;

    ret = s5lab_csp_send(port, tx, tx_len, timeout_ms, &conn);
    if (ret != S5LAB_OK)
        return ret;

    if (!rx || rx_len == 0) {
        csp_close(conn);
        return S5LAB_OK;
    }

    pkt = csp_read(conn, timeout_ms);
    if (!pkt) {
        csp_close(conn);
        return S5LAB_ERR_TIMEOUT;
    }
    
    uint16_t copy_len = pkt->length < rx_buf_len ? pkt->length : rx_buf_len;
    if (rx)
        memcpy(rx, pkt->data, copy_len);

    if (rx_len)
        *rx_len = pkt->length;

    csp_buffer_free(pkt);
    csp_close(conn);
    return S5LAB_OK;
}


int s5lab_transaction_multiple(uint8_t port,
                               int nrep,
                               const void* tx,
                               uint16_t tx_len,
                               uint16_t timeout_ms,
                               s5lab_reply_handler_t handler,
                               void* user_data)
{
    int ret;
    csp_conn_t* conn;

    if (!handler)
        return S5LAB_ERR_NULL_PTR;

    if (nrep < 0)
        return S5LAB_ERR_INVALID_PARAM;

    ret = s5lab_csp_send(port, tx, tx_len, timeout_ms, &conn);
    if (ret != S5LAB_OK)
        return ret;

    for (int i = 0; i < nrep; i++) {
        csp_packet_t* pkt = csp_read(conn, timeout_ms);
        if (!pkt) {
            ret = S5LAB_ERR_TIMEOUT;
            break;
        }

        ret = handler(i, pkt->data, pkt->length, user_data);
        csp_buffer_free(pkt);

        if (ret != 0)
            break;
    }

    /* The packet has been freed however it reached here. */
    csp_close(conn);
    return ret;
}
