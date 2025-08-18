#define P31U_I2C_LINUX_USE_MBUS
#include "p31u.h"
#include "p31u_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

static cmd_entry_t cmds[] = {
    {
        .index = GET_HK,
        .handler = handler_gethk,
        .name = "gethk",
        .help = "Print selected housekeeping",
        .usage = "<index>"
    },

    {
        .index = GETHK_ALL,
        .handler = handler_gethk_all,
        .name = "gethk_all",
        .help = "Print all housekeeping",
        .usage = ""
    },

    {
        .index = GETHK_VI,
        .handler = handler_gethk_vi,
        .name = "gethk_vi",
        .help = "Print voltage input housekeeping",
        .usage = ""
    },

    {
        .index = GETHK_OUT,
        .handler = handler_gethk_out,
        .name = "gethk_out",
        .help = "Print output channel housekeeping",
        .usage = ""
    },

    {
        .index = GETHK_WDT,
        .handler = handler_gethk_wdt,
        .name = "gethk_wdt",
        .help = "Print watchdog timer housekeeping",
        .usage = ""
    },

    {
        .index = GETHK_BASIC,
        .handler = handler_gethk_basic,
        .name = "gethk_basic",
        .help = "Print basic housekeeping",
        .usage = ""
    },

    // {
    //     .index = GETHK_OLD,
    //     .handler = handler_gethk_old,
    //     .name = "gethk_old",
    //     .help = "Print backward compatible housekeeping",
    //     .usage = ""
    // },

    {
        .index = GETHK_OLD,
        .handler = handler_set_outputs,
        .name = "set_outputs",
        .help = "Set channel on/offs",
        .usage = "<mask>"
    },

    {
        .index = SET_OUTPUT_SINGLE,
        .handler = handler_set_output_single,
        .name = "set_output_single",
        .help = "Set single channel on/off after a seconds-delay",
        .usage = "<channel> <value> <delay>"
    },

    {
        .index = SET_PV_VOLT,
        .handler = handler_set_pv_volt,
        .name = "set_pv_volt",
        .help = "Set photovoltaic input voltages (PV mode 2 only)",
        .usage = "<V1> <V2> <V3>"
    },

    {
        .index = SET_PV_AUTO,
        .handler = handler_set_pv_auto,
        .name = "set_pv_auto",
        .help = "Set power tracking mode (0: dft, 1: MPPT, 2: fixed)",
        .usage = "<mode>"
    },

    {
        .index = SET_HEATER,
        .handler = handler_set_heater,
        .name = "set_heater",
        .help = "Set heater on/off",
        .usage = "<heater> <mode>"
    },

    {
        .index = RESET_COUNTERS,
        .handler = handler_reset_counters,
        .name = "reset_counters",
        .help = "Reset boot/watchdog counters",
        .usage = ""
    },

    {
        .index = RESET_WDT,
        .handler = handler_reset_wdt,
        .name = "reset_wdt",
        .help = "Reset watchdog timers",
        .usage = ""
    },

    {
        .index = HARD_RESET,
        .handler = handler_hard_reset,
        .name = "hard_reset",
        .help = "Perform hard-reset",
        .usage = ""
    },

    {
        .index = GET_CONFIG,
        .handler = handler_get_config,
        .name = "get_config",
        .help = "Get current config data",
        .usage = ""
    },

    {
        .index = GET_CONFIG2,
        .handler = handler_get_config2,
        .name = "get_config2",
        .help = "Get current config2 data",
        .usage = ""
    },

    {
        .index = CONFIG_CMD,
        .handler = handler_config_cmd,
        .name = "config_cmd",
        .help = "Restore config",
        .usage = "<cmd>"
    }
};

static void print_usage(const cmd_entry_t* cmd,
                        bool printHelp)
{
    if (printHelp)
        printf("%s: %s\n", cmd->name, cmd->help);
    printf("usage: %s %s\n\n", cmd->name, cmd->usage);
}

static char *trim(char *str)
{
    while (*str == ' ') str++;
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\n')) *end-- = '\0';
    return str;
}

static int parse_and_execute(char *input)
{
    int ret;

    if (!input || strlen(input) == 0)
        return 0;

    char* name = strtok(input, " "); 
    char* args = input + strlen(name) + 1;

    for (size_t i = 0; i < sizeof(cmds)/sizeof(cmds[0]); ++i)
        if (strcmp(cmds[i].name, name) == 0) {
            ret = cmds[i].handler(args);
            if (ret != P31U_OK) {
                if (ret == P31U_TEST_ERR_ARG)
                    print_usage(&cmds[i], false);
                else
                    printf("cmd %s error: returned %d\n", cmds[i].name, ret);
            }
            return ret;
        }

    fprintf(stderr, "Unknown function: %s\n", name);
    return -1;
}


char* cmd_generator(const char* input, int state) {
    static int index, len;
    const char* cmd;

    if (state == 0) {
        index = 0;
        len = strlen(input);
    }

    while ((cmd = cmds[index++].name) != NULL) {
        if (strncmp(cmd, input, len) == 0) {
            return strdup(cmd);
        }
    }

    return NULL;
}

char** cmd_autocomplete(const char* input, int s, int e) {
    (void)s; (void)e;
    return rl_completion_matches(input, cmd_generator);
}

int i2c_init(const char* dev, uint8_t address);

int main(void) {
    char *line;

    // if (i2c_init("/dev/i2c-0", 6) != 0)
    if (i2c_init("/dev/zero", 6) != 0)
        exit(1);
    
    rl_attempted_completion_function = cmd_autocomplete;

    while ((line = readline(">> ")) != NULL) {
        char *trimmed = trim(line);
        if (strlen(trimmed) > 0) {
            add_history(trimmed);
            parse_and_execute(trimmed);
        }
        free(line);
    }

    return 0;
}


