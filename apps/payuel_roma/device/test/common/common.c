#include "common.h"

#include <csp/csp.h>
#include <csp/drivers/can_socketcan.h>

#include <stdio.h>

#define CLR_RESET  "\033[0m"
#define CLR_RED    "\033[31m"
#define CLR_GREEN  "\033[32m"
#define CLR_YELLOW "\033[33m"
#define CLR_CYAN   "\033[36m"
#define CLR_BWHITE "\033[97m"
#define CLR_DIM    "\033[2m"
#define CLR_MAGENTA "\033[35m"
#define CLR_BOLD   "\033[1m"

int init_csp(uint8_t node, uint16_t buffer_size, uint16_t nbuffers)
{
    csp_conf_t csp_conf = {
        .address = node,
        .hostname = "s5lab_tester",
        .model = "none",
        .revision = "0.1",
        .conn_max = 5,
        .conn_queue_length = 5,
        .fifo_length = 10,
        .port_max_bind = 25,
        .rdp_max_window = 10,
        .buffers = nbuffers,
        .buffer_data_size = buffer_size,
        .conn_dfl_so = 0,
    };

    int ret = csp_init(&csp_conf);
    if (ret != CSP_ERR_NONE) {
        fprintf(stderr, "Failed to initialize CSP: %d\n", ret);
        return ret;
    }

    csp_debug_set_level(CSP_WARN, true);
    csp_debug_set_level(CSP_ERROR, true);
    csp_debug_set_level(CSP_INFO, true);
    // csp_debug_set_level(CSP_PACKET, true);

    return csp_route_start_task(1024, 10);
}

int init_can_iface(const char* bus, uint8_t dst_node)
{
    csp_iface_t* iface = csp_can_socketcan_init(bus, 1000000, true);
    if (!iface) {
        fprintf(stderr, "failed to open CAN bus <%s>\n", bus);
        return -1;
    }

    return csp_route_set(dst_node, iface, CSP_NODE_MAC);
}


void log_summary(int passed, int total)
{
    bool all_passed = (passed == total);
    const char *color = all_passed ? CLR_GREEN CLR_BOLD : CLR_RED CLR_BOLD;

    printf("\n%s%d/%d tests passed%s\n", color, passed, total, CLR_RESET);
}

void LOG_PASS(const char *fmt, ...)
{
    printf("%s%s%s ", CLR_GREEN, "PASS", CLR_RESET);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    putchar('\n');
}

void LOG_FAIL(const char *fmt, ...)
{
    printf("%s%s%s ", CLR_RED, "FAIL", CLR_RESET);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    putchar('\n');
}
