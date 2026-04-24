/**
 * @file s5lab_test_sim.c
 * @author ryu@yonsei.ac.kr
 * @brief I just really don't wanna go to the clean room!
 * @version 0.1
 * @date 2026-03-24
 * 
 * Yonsei ACL, 2026
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <csp/csp.h>
#include <csp/drivers/can_socketcan.h>

#include "s5lab.h"

#include "common.h"

static int nlog_handler(uint16_t index, void* reply_data, uint16_t reply_len, void* user_data)
{
    s5lab_rep_get_line_t* rep = reply_data;
    (void)index; (void)user_data; (void)reply_len;
    if (!reply_data)
        return -99;
    rep->line[rep->len] = '\0';
    printf("    Received line %u (len %3u): %s\n", rep->line_no, rep->len, rep->line);
    return 0;
}


int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    void* cmd_buffer = malloc(256);
    if (!cmd_buffer) {
        fprintf(stderr, "Failed to allocate command buffer\n");
        return -1;
    }

    if (init_csp(HOST_NODE_ADDRESS, HOST_BUFFER_SIZE, 10) != CSP_ERR_NONE ||
        init_can_iface("vcan0", DEVICE_NODE_ADDRESS) != CSP_ERR_NONE)
        return -1;


    printf("Host simulator started on address %d\n", csp_get_address());
    printf("Destination (device) node is %d\n", s5lab_csp_get_node());


//== CSP SERVICE TESTS =====================================================================

    {
        TEST_NEQ(csp_ping(s5lab_csp_get_node(), 1000, 1, 0), -1);
        TEST_NEQ(csp_ping(s5lab_csp_get_node(), 1000, 128, 0), -1);
        TEST_EQ(csp_ping(s5lab_csp_get_node(), 1000, 500, 0), -1); // Should fail.
    }

//== LOG COMMAND TESTS =====================================================================
    
    {
        uint16_t rx_len;
        s5lab_rep_get_line_t* rep = cmd_buffer;

        TEST_EQ(s5lab_get_latest_line(rep, 256, &rx_len), S5LAB_OK);
        rep->line[rep->len] = '\0';
        printf("    Latest line of length %d: %s\n", rx_len, rep->line);

        TEST_EQ(s5lab_get_specific_line(1, rep, 256, &rx_len), S5LAB_OK);
        rep->line[rep->len] = '\0';
        printf("    Line 1 of length %d: %s\n", rx_len, rep->line);

        TEST_EQ(s5lab_get_multiple_lines(1, 4, nlog_handler, NULL), S5LAB_OK);
        
        TEST_EQ(s5lab_get_latest_n_lines(7, nlog_handler, NULL), S5LAB_OK);

        TEST_EQ(s5lab_clear_all_lines(), S5LAB_OK);
    }

//== CSP SETTINGS ==========================================================================

    {
        uint16_t timeout = 1234;
        s5lab_csp_set_timeout(timeout);
        TEST_EQ(s5lab_csp_get_timeout(), timeout);
        s5lab_csp_set_timeout(1000);

        uint8_t dummy_node = 2;
        s5lab_csp_set_node(dummy_node);
        TEST_EQ(s5lab_csp_get_node(), dummy_node);
        TEST_EQ(csp_ping(s5lab_csp_get_node(), 1000, 1, 0), -1); // Ping should fail.
        s5lab_csp_set_node(DEVICE_NODE_ADDRESS);
        TEST_NEQ(csp_ping(s5lab_csp_get_node(), 1000, 1, 0), -1); // Ping should work.
    }


//== SUMMARY =============================================================================

    SUMMARY();

    return 0;
}
