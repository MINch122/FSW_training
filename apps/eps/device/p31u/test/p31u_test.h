#ifndef _P31U_TEST_H_
#define _P31U_TEST_H_

#define P31U_TEST_ERR_ARG        -1000
#define P31U_TEST_ERR_STR2L      -1001
#define P31U_TEST_ERR_ARG_RANGE  -1002
#define P31U_TEST_ERR_ARG_INVAL  -1003
#define P31U_TEST_ERR_UNKNOWN_FN -1004

typedef enum {
    GET_HK = 1,
    GETHK_ALL,
    GETHK_VI,
    GETHK_OUT,
    GETHK_WDT,
    GETHK_BASIC,
    GETHK_OLD,

    SET_OUTPUTS,
    SET_OUTPUT_SINGLE,
    SET_PV_VOLT,
    SET_PV_AUTO,
    SET_HEATER,

    RESET_COUNTERS,
    RESET_WDT,
    HARD_RESET,

    CONFIG_CMD,
    GET_CONFIG,
    SET_CONFIG,
    CONFIG2_CMD,
    GET_CONFIG2,
    SET_CONFIG2,
    SET_CONFIG3,
} func_enum;

typedef struct {
    func_enum index;
    const char* name;
    const char* help;
    const char* usage;
    int (*handler)(void*);
} cmd_entry_t;

int handler_gethk(void* args);
int handler_gethk_all(void* args);
int handler_gethk_vi(void* args);
int handler_gethk_out(void* args);
int handler_gethk_wdt(void* args);
int handler_gethk_basic(void* args);
// int handler_gethk_old(void* args);
int handler_set_outputs(void* args);
int handler_set_output_single(void* args);
int handler_set_pv_volt(void* args);
int handler_set_pv_auto(void* args);
int handler_set_heater(void* args);
int handler_reset_counters(void* args);
int handler_reset_wdt(void* args);
int handler_hard_reset(void* args);
int handler_get_config(void* args);
int handler_get_config2(void* args);
int handler_config_cmd(void* args);


#endif
