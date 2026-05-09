#ifndef _LTRX_IFC_H_
#define _LTRX_IFC_H_

#include "ltrx.h"


/* Max read size if rx_size is -1.
   This should be consistent with the CSP buffer settings.
*/
#define LTRX_SERIAL_READ_MAX 512

/**
 * Simulated serial interface for testing without hardware.
 */
#define LTRX_SERIAL_SIMULATED 1


#define LTRX_SERIAL_DEV "/dev/ttyS1"


#if LTRX_SERIAL_SIMULATED
    /**
     * Simulated device context.
     */
    typedef struct {
        void *expect_tx;
        size_t expect_tx_len;
        void *response;
        size_t response_len;
        uint32_t delay_ms;
    } sim_device_t;

    bool sim_device_set_response(const void *response, size_t response_len, size_t offset);
    bool sim_device_set_expect(const void *expect_tx, size_t expect_tx_len, size_t offset);
    void sim_device_set_delay(uint32_t delay_ms);
#endif


int ltrx_serial_init(const char* dev, int baud);


int32_t ltrx_serial_transaction(const void* tx_buf,
                                uint16_t tx_len,
                                void* rx_buf,
                                int32_t rx_size,
                                uint16_t timeout_ms);

#endif
