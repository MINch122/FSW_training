/**
 * @file ltrx_test_serial.c
 * @author ryu@yonsei.ac.kr
 * @brief A simple LTRX device unit test suite.
 * @version 1.0
 * @date 2026-03-12
 * 
 * ACL Yonsei, 2026
 */
#include "ltrx_ifc.h"
#include "ltrx_test_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

static int test_failed = 0;
static int test_passed = 0;

#define SUMMARY() \
    do { \
        log_summary(test_passed, test_passed + test_failed); \
        exit(test_failed > 0 ? 1 : 0); \
    } while (0)

#define TEST_EQ(expr, expected) \
    do { \
        int64_t result = (expr); \
        if (result == (expected)) { \
            LOG_PASS(#expr); \
            test_passed++; \
        } else { \
            LOG_FAIL(#expr ": expected %" PRId64 ", got %" PRId64, (int64_t)(expected), (int64_t)result); \
            test_failed++; \
        } \
    } while (0)

#define TEST_NEQ(expr, not_expected) \
    do { \
        int64_t result = (expr); \
        if (result != (not_expected)) { \
            LOG_PASS(#expr); \
            test_passed++; \
        } else { \
            LOG_FAIL(#expr ": expected %" PRId64 ", got %" PRId64, (int64_t)(not_expected), (int64_t)result); \
            test_failed++; \
        } \
    } while (0)


int main(int argc, char *argv[])
{
    const char *dev;
    bool mock = false;
    int baud;

    log_init();

    if (argc < 3)
    {
#if LTRX_SERIAL_SIMULATED
        printf("Running on simulated serial interface...\n");
        mock = true;
        dev = NULL; // Let the sim init create a pty
        baud = 115200;
#else
        (void) mock;
        printf("Usage: %s <serial_device> <baud_rate>\n", argv[0]);
        return 1;
#endif
    }
    else {
        dev = argv[1];
        baud = atoi(argv[2]);
    }

    LTRX_BeaconCmdHeader_t header;
    uint8_t payload[256];

    TEST_EQ(ltrx_serial_init(dev, baud), LTRX_SUCCESS);

    LTRX_RegisterTransport(ltrx_serial_transaction);

    TEST_EQ(LTRX_BuildCommandHeader(&header, 0x01, 0x02, 0x03, sizeof(payload), 0xC0FFEE), LTRX_SUCCESS);
    TEST_EQ(header.FromID, 0x01);
    TEST_EQ(header.ToID, 0x02);
    TEST_EQ(header.TypeID, 0x03);
    TEST_EQ(header.Length, sizeof(payload));
    TEST_EQ(header.CRC, 0xC0FFEE);

    TEST_EQ(LTRX_SendCommand(&header, payload, sizeof(payload)), LTRX_SUCCESS);
    TEST_EQ(LTRX_SendCommand(&header, NULL, 0), LTRX_SUCCESS);
    TEST_EQ(LTRX_SendCommand(&header, NULL, sizeof(payload)), LTRX_ERROR_NULL_PTR);


    LTRX_MessageHeader_Payload_t offer_parsed;
#if LTRX_SERIAL_SIMULATED
    LTRX_BeaconCmdHeader_t offer_header = {
        .FromID = 0x02,
        .ToID = 0x01,
        .TypeID = LTRX_BEACON_CMD_OFFER_RECEIVE_MSG,
        .Length = sizeof(LTRX_MessageHeader_Payload_t),
        .CRC = 0,
    };
    LTRX_MessageHeader_Payload_t offer_payload = {
        .MessageID = 0x12345678,
        .MessageLength = 12,
        .MessageCRC = 0xDEADBEEF,
    };
    if (mock) {
        sim_device_set_expect(NULL, 0, 0);
        sim_device_set_response(&offer_header, sizeof(offer_header), 0);
        sim_device_set_response(&offer_payload, sizeof(offer_payload), sizeof(offer_header));
    }
#endif

    TEST_EQ(LTRX_ReceiveOfferMessage(&offer_parsed), LTRX_SUCCESS);

#if LTRX_SERIAL_SIMULATED
    if (mock) {
        TEST_EQ(offer_parsed.MessageID, offer_payload.MessageID);
        TEST_EQ(offer_parsed.MessageLength, offer_payload.MessageLength);
        TEST_EQ(offer_parsed.MessageCRC, offer_payload.MessageCRC);
    }
#endif

    TEST_EQ(LTRX_ConfirmReadyForMessage(), LTRX_SUCCESS);

    SUMMARY();
    return 0;
}
