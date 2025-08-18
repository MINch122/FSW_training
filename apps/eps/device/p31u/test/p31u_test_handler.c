#include "p31u.h"
#include "p31u_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>

typedef struct {
    const char* argname;
    int max;
    int min;
    bool rangeCheck;
} arg_entry_t;

typedef enum {
    PARSE_OK = 0,
    PARSE_NO_DIGITS,
} parse_ret_t;

static int parse_args(const char* arg,
                      const arg_entry_t* entry,
                      int count,
                      long* parsed)
{
    char* end;

    if (!arg || !entry || count < 0 || !parsed) {
        fprintf(stderr, "CRITICAL: invalid input to parse_args!\n");
        exit(1);
    }

    for (int i = 0; i < count; ++i) {

        errno = 0;
        long tmp = strtol(arg, &end, 10);

        if (end == arg || (*end != '\0' && !isspace(*end))) {
            fprintf(stderr,
                    "empty or non-digit input for argument \"%s\"\n",
                    entry[i].argname);
            return P31U_TEST_ERR_ARG;
        }

        if (errno != 0) {
            fprintf(stderr,
                    "conversion error for argument \"%s\": %s (%d)\n",
                    entry[i].argname, strerror(errno), errno);
            return P31U_TEST_ERR_STR2L;
        }
    
        if (entry[i].rangeCheck) {
            if (tmp > entry[i].max) {
                fprintf(stderr,
                        "argument \"%s\" input %ld is out of range (max %d)\n",
                        entry[i].argname, tmp, entry[i].max);
                return P31U_TEST_ERR_ARG_RANGE;
            }

            if (tmp < entry[i].min) {
                fprintf(stderr,
                        "argument \"%s\" input %ld is out of range (min %d)\n",
                        entry[i].argname, tmp, entry[i].min);
                return P31U_TEST_ERR_ARG_RANGE;
            }
        }

        parsed[i] = tmp;
        arg = end;
    }

    return 0;
}

static void print_hk_all(const void* all)
{
    const p31u_hk_t* hk = all;
    printf("vboost[mV]:   %4d, %4d, %4d\n",
            hk->vboost[0], hk->vboost[1], hk->vboost[2]);
    printf("vbatt[mV]:    %4d\n", hk->vbatt);
    printf("curin[mA]:    %4d, %4d, %4d\n",
            hk->curin[0], hk->curin[1], hk->curin[2]);
    printf("cursun[mA]:   %4d\n", hk->cursun);
    printf("cursys[mA]:   %4d\n", hk->cursys);
    printf("reserved1:    %4d\n", hk->reserved1);
    printf("curout[mA]:   %4d, %4d, %4d, %4d, %4d, %4d\n",
            hk->curout[0], hk->curout[1], hk->curout[2],
            hk->curout[3], hk->curout[4], hk->curout[5]);
    printf("output:       %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d\n",
            hk->output[0], hk->output[1], hk->output[2], hk->output[3],
            hk->output[4], hk->output[5], hk->output[6], hk->output[7]);
    printf("out_on_delta[s]: %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d\n", 
            hk->output_on_delta[0], hk->output_on_delta[1],
            hk->output_on_delta[2], hk->output_on_delta[3],
            hk->output_on_delta[4], hk->output_on_delta[5],
            hk->output_on_delta[6], hk->output_on_delta[7]);
    printf("out_off_delta[s]:  %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d\n",
            hk->output_off_delta[0], hk->output_off_delta[1],
            hk->output_off_delta[2], hk->output_off_delta[3],
            hk->output_off_delta[4], hk->output_off_delta[5],
            hk->output_off_delta[6], hk->output_off_delta[7]);
    printf("latchup:      %4d, %4d, %4d, %4d, %4d, %4d\n",
            hk->latchup[0], hk->latchup[1], hk->latchup[2],
            hk->latchup[3], hk->latchup[4], hk->latchup[5]);
    printf("wdt_i2c_time_left[s]: %4u\n", hk->wdt_i2c_time_left);
    printf("wdt_gnd_time_left[s]: %4u\n", hk->wdt_gnd_time_left);
    printf("wdt_csp_pings_left:   %4d, %4d\n",
            hk->wdt_csp_pings_left[0], hk->wdt_csp_pings_left[1]);
    printf("counter_wdt_i2c:      %4u\n", hk->counter_wdt_i2c);
    printf("counter_wdt_gnd:      %4u\n", hk->counter_wdt_gnd);
    printf("counter_wdt_csp:      %4u, %4u\n",
            hk->counter_wdt_csp[0], hk->counter_wdt_csp[1]);
    printf("counter_boot:         %4u\n", hk->counter_boot);
    printf("temp[degC]:   %2d, %2d, %2d, %2d, %2d, %2d\n",
            hk->temp[0], hk->temp[1], hk->temp[2],
            hk->temp[3],hk->temp[4], hk->temp[5]);
    printf("bootcause:    %4d\n", hk->bootcause);
    printf("battmode:     %4d\n", hk->battmode);
    printf("pptmode:      %4d\n", hk->pptmode);
}

static void print_hk_vi(const void* vi)
{
    const p31u_hk_vi_t* hk = vi;
    printf("vboost[mV]:       %4d, %4d, %4d\n",
            hk->vboost[0], hk->vboost[1], hk->vboost[2]);
    printf("vbatt[mV]:        %4d\n", hk->vbatt);
    printf("curin[mA]:        %4d, %4d, %4d\n",
            hk->curin[0], hk->curin[1], hk->curin[2]);
    printf("cursun[mA]:       %4d\n", hk->cursun);
    printf("cursys[mA]:       %4d\n", hk->cursys);
}

static void print_hk_out(const void* out)
{
    const p31u_hk_out_t* hk = out;
    printf("curout[mA]:       %4d, %4d, %4d, %4d, %4d, %4d\n",
        hk->curout[0], hk->curout[1], hk->curout[2],
        hk->curout[3], hk->curout[4], hk->curout[5]);
    printf("output:           %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d\n",
        hk->output[0], hk->output[1], hk->output[2], hk->output[3],
        hk->output[4], hk->output[5], hk->output[6], hk->output[7]);
    printf("output_on_delta[s]:  %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d\n",
        hk->output_on_delta[0], hk->output_on_delta[1],
        hk->output_on_delta[2], hk->output_on_delta[3],
        hk->output_on_delta[4], hk->output_on_delta[5],
        hk->output_on_delta[6], hk->output_on_delta[7]);
    printf("output_off_delta[s]: %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d\n",
        hk->output_off_delta[0], hk->output_off_delta[1],
        hk->output_off_delta[2], hk->output_off_delta[3],
        hk->output_off_delta[4], hk->output_off_delta[5],
        hk->output_off_delta[6], hk->output_off_delta[7]);
    printf("latchup:          %4d, %4d, %4d, %4d, %4d, %4d\n",
        hk->latchup[0], hk->latchup[1], hk->latchup[2],
        hk->latchup[3], hk->latchup[4], hk->latchup[5]); 
}

static void print_hk_wdt(const void* wdt)
{
    const p31u_hk_wdt_t* hk = wdt;
    printf("wdt_i2c_time_left[s]: %4u\n", hk->wdt_i2c_time_left);
    printf("wdt_gnd_time_left[s]: %4u\n", hk->wdt_gnd_time_left);
    printf("wdt_csp_pings_left:   %4d, %4d\n",
        hk->wdt_csp_pings_left[0], hk->wdt_csp_pings_left[1]);
    printf("counter_wdt_i2c:      %4u\n", hk->counter_wdt_i2c);
    printf("counter_wdt_gnd:      %4u\n", hk->counter_wdt_gnd);
    printf("counter_wdt_csp:      %4u, %4u\n",
        hk->counter_wdt_csp[0], hk->counter_wdt_csp[1]);
}

static void print_hk_basic(const void* basic)
{
    const p31u_hk_basic_t* hk = basic;
    printf("counter_boot:         %4u\n", hk->counter_boot);
    printf("temp[degC]:       %4d, %4d, %4d, %4d, %4d, %4d\n",
        hk->temp[0], hk->temp[1], hk->temp[2],
        hk->temp[3], hk->temp[4], hk->temp[5]);
    printf("bootcause:        %4d\n", hk->bootcause);
    printf("battmode:         %4d\n", hk->battmode);
    printf("pptmode:          %4d\n", hk->pptmode);
    printf("reserved2:        %4d\n", hk->reserved2);
}

int handler_gethk(void* args)
{
    int ret;
    long intargs[1];

    uint8_t type;
    int size;
    void (*printer)(const void*);

    arg_entry_t entries =
    {
        .argname = "type",
        .rangeCheck = true,
        .min = 0,
        .max = 4
    };

    union {
        p31u_hk_t all;
        p31u_hk_vi_t vi;
        p31u_hk_out_t out;
        p31u_hk_wdt_t wdt;
        p31u_hk_basic_t basic;
        p31u_hkparam_t old;
    } hk;

    ret = parse_args(args, &entries, 1, intargs);
    if (ret != 0)
        return ret;
    
    type = intargs[0];
    
    switch (type) {
    case 0:
        size = sizeof(hk.all);
        printer = print_hk_all;
        break;
    case 1:
        size = sizeof(hk.vi);
        printer = print_hk_vi;
        break;
    case 2:
        size = sizeof(hk.out);
        printer = print_hk_out;
        break;
    case 3:
        size = sizeof(hk.wdt);
        printer = print_hk_wdt;
        break;
    case 4:
        size = sizeof(hk.basic);
        printer = print_hk_basic;
        break;
    default:
        return P31U_TEST_ERR_ARG_INVAL;
    }
    
    ret = p31u_gethk(&hk, type, size);
    if (ret == P31U_OK)
        printer(&hk);

    return ret;
}

int handler_gethk_all(void* args)
{
    int ret;
    p31u_hk_t hk;
    (void) args;

    ret = p31u_gethk_all(&hk);

    if (ret == P31U_OK)
        print_hk_all(&hk);

    return ret;
}

int handler_gethk_vi(void* args)
{
    int ret;
    p31u_hk_vi_t hk;
    (void) args;

    ret = p31u_gethk_vi(&hk);

    if (ret == P31U_OK)
        print_hk_vi(&hk);
    
    return ret;
}

int handler_gethk_out(void* args)
{
    int ret;
    p31u_hk_out_t hk;
    (void) args;

    ret = p31u_gethk_out(&hk);

    if (ret == P31U_OK)
        print_hk_out(&hk);

    return ret;
}

int handler_gethk_wdt(void* args)
{
    int ret;
    p31u_hk_wdt_t hk;
    (void) args;

    ret = p31u_gethk_wdt(&hk);

    if (ret == P31U_OK)
        print_hk_wdt(&hk);

    return ret;
}

int handler_gethk_basic(void* args)
{
    int ret;
    p31u_hk_basic_t hk;
    (void) args;

    ret = p31u_gethk_basic(&hk);

    if (ret == P31U_OK)
        print_hk_basic(&hk);

    return ret;
}

// int handler_gethk_old(void* args)
// {
//     int ret;
//     p31u_hkparam_t hk;
//     (void) args;

//     ret = p31u_gethk_old(&hk);

//     if (ret == P31U_OK)
//         print_hk_old(&hk);

//     return ret;
// }


int handler_set_outputs(void* args)
{
    int ret;
    int argcnt = 1;
    long intargs[1];

    uint8_t mask;

    arg_entry_t entries[1] =
    {
        {.argname = "channel",
         .rangeCheck = true,
         .min = 0,
         .max = UINT8_MAX
        }
    };

    ret = parse_args(args, entries, argcnt, intargs);
    if (ret != 0)
        return ret;

    for (int i = 0; i < argcnt; ++i) {
        printf("%s: %ld. ", entries[i].argname, intargs[i]);
    }
    printf("\n");

    mask = intargs[0];
    return p31u_set_outputs(mask);
}

int handler_set_output_single(void* args)
{
    int ret;
    int argcnt = 3;
    long intargs[3];

    uint8_t channel, value;
    uint16_t delay;

    arg_entry_t entries[3] =
    {
        {.argname = "channel",
         .rangeCheck = true,
         .min = 0,
         .max = UINT8_MAX
        },
        {.argname = "value",
         .rangeCheck = true,
         .min = 0,
         .max = UINT8_MAX
        },
        {.argname = "delay",
         .rangeCheck = true,
         .min = 0,
         .max = UINT16_MAX
        },
    };

    ret = parse_args(args, entries, argcnt, intargs);
    if (ret != 0)
        return ret;

    for (int i = 0; i < argcnt; ++i) {
        printf("%s: %ld. ", entries[i].argname, intargs[i]);
    }
    printf("\n");
    
    channel = intargs[0];
    value = intargs[1];
    delay = intargs[2];

    return p31u_set_output_single(channel, value, delay);
}

int handler_set_pv_volt(void* args)
{
    int ret;
    int argcnt = 3;
    long intargs[3];

    int16_t v1, v2, v3;

    arg_entry_t entries[3] =
    {
        {.argname = "V1",
         .rangeCheck = true,
         .min = INT16_MIN,
         .max = INT16_MAX
        },
        {.argname = "V2",
         .rangeCheck = true,
         .min = INT16_MIN,
         .max = INT16_MAX
        },
        {.argname = "V3",
         .rangeCheck = true,
         .min = INT16_MIN,
         .max = INT16_MAX
        },
    };

    ret = parse_args(args, entries, argcnt, intargs);
    if (ret != 0)
        return ret;

    for (int i = 0; i < argcnt; ++i) {
        printf("%s: %ld. ", entries[i].argname, intargs[i]);
    }
    printf("\n");
    
    v1 = intargs[0];
    v2 = intargs[1];
    v3 = intargs[2];

    return p31u_set_pv_volt(v1, v2, v3);
}

int handler_set_pv_auto(void* args)
{
    int ret;
    int argcnt = 1;
    long intargs[1];

    uint8_t mode;

    arg_entry_t entries[1] =
    {
        {.argname = "mode",
         .rangeCheck = true,
         .min = 0,
         .max = UINT8_MAX
        }
    };

    ret = parse_args(args, entries, argcnt, intargs);
    if (ret != 0)
        return ret;

    for (int i = 0; i < argcnt; ++i) {
        printf("%s: %ld. ", entries[i].argname, intargs[i]);
    }
    printf("\n");
    
    mode = intargs[0];
    return p31u_set_pv_auto(mode);
}

int handler_set_heater(void* args)
{
    int ret;
    int argcnt = 3;
    long intargs[3];

    uint8_t cmd, heater, mode;
    p31u_reply_set_heater reply;

    arg_entry_t entries[3] =
    {
        {.argname = "cmd",
         .rangeCheck = true,
         .min = 0,
         .max = UINT8_MAX
        },
        {.argname = "heater",
         .rangeCheck = true,
         .min = 0,
         .max = UINT8_MAX
        },
        {.argname = "mode",
         .rangeCheck = true,
         .min = 0,
         .max = UINT8_MAX
        }
    };

    ret = parse_args(args, entries, argcnt, intargs);
    if (ret != 0)
        return ret;

    for (int i = 0; i < argcnt; ++i) {
        printf("%s: %ld. ", entries[i].argname, intargs[i]);
    }
    printf("\n");
    
    cmd = intargs[0];
    heater = intargs[1];
    mode = intargs[2];

    ret =  p31u_set_heater(cmd, heater, mode, &reply);
    if (ret == P31U_OK) {
        printf("bp4_heater: %d\n"
               "onboard_heater: %d\n",
               reply.bp4_heater, reply.onboard_heater);
    }

    return ret;
}

int handler_reset_counters(void* args)
{
    (void) args;
    return p31u_reset_counters();
}

int handler_reset_wdt(void* args)
{
    (void) args;
    return p31u_reset_wdt();
}

int handler_hard_reset(void* args)
{
    (void) args;
    return p31u_hard_reset();
}

int handler_get_config(void* args)
{
    int ret;
    p31u_config_t conf;
    (void) args;

    ret = p31u_get_config(&conf);
    if (ret == P31U_OK) {
        printf("pptmode: %"PRIu8"\r\n",
                conf.ppt_mode);
        printf("battheater: %"PRIu8", low: %"PRIi8", high: %"PRIi8"\r\n",
              conf.battheater_mode, conf.battheater_low, conf.battheater_high);
        for (int i = 0; i < 3; i++)
            printf("vboost[%u]: %"PRIu16"\r\n",
                   i, conf.vboost[i]);
        for (int i = 0; i < 8; i++)
            printf("output[%u]: ondelay: %"PRIu16", offdelay: %"PRIu16","
                   "normal mode: %"PRIu8", safe mode %"PRIu8"\r\n",
                   i,
                   conf.output_initial_on_delay[i],
                   conf.output_initial_off_delay[i],
                   conf.output_normal_value[i],
                   conf.output_safe_value[i]);
    }

    return ret;
}

int handler_get_config2(void* args)
{
    int ret;
    p31u_config2_t conf;
    (void) args;

    ret = p31u_get_config2(&conf);
    if (ret == P31U_OK) {
        printf("Batt max volt      %"PRIu16" mV\r\n",conf.batt_maxvoltage);
        printf("Batt normal volt   %"PRIu16" mV\r\n",conf.batt_normalvoltage);
        printf("Batt safe volt     %"PRIu16" mV\r\n",conf.batt_safevoltage);
        printf("Batt critical volt %"PRIu16" mV\r\n",conf.batt_criticalvoltage);
    }

    return ret;
}

int handler_config_cmd(void* args)
{
    int ret;
    int argcnt = 1;
    long intargs[1];

    uint8_t cmd;

    arg_entry_t entries[1] =
    {
        {.argname = "cmd",
         .rangeCheck = true,
         .min = 0,
         .max = UINT8_MAX
        }
    };

    ret = parse_args(args, entries, argcnt, intargs);
    if (ret != 0)
        return ret;

    for (int i = 0; i < argcnt; ++i) {
        printf("%s: %ld. ", entries[i].argname, intargs[i]);
    }
    printf("\n");
    
    cmd = intargs[0];
    return p31u_config_cmd(cmd);
}
