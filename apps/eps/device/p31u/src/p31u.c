/**
 * @file p31u.c
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief GomSpace NanoPower P31u Power System driver implementation.
 *        Reference: NanoPower P31u Manual Rev. 10
 * Last modified: 2025-07-27
 * Astrodynamics & Control Lab. 2024. ryu@yonsei.ac.kr
 */
#include "p31u.h"

#if P31U_USE_NTOH || P31U_USE_HTON
  #include <arpa/inet.h>
#endif

int p31u_set(uint8_t port,
             const void* data,
             int size)
{
    return p31u_transaction(port, data, size, NULL, 0);
}

int p31u_get(uint8_t port,
             void* data,
             int size)
{
    return p31u_transaction(port, NULL, 0, data, size);
}

#if P31U_USE_NTOH
void p31u_ntoh_hk_vi(p31u_hk_vi_t* hk)
{
    if (!hk)
        return;

    hk->vboost[0] = ntohs(hk->vboost[0]);
    hk->vboost[1] = ntohs(hk->vboost[1]);
    hk->vboost[2] = ntohs(hk->vboost[2]);
    hk->vbatt = ntohs(hk->vbatt);
    hk->curin[0] = ntohs(hk->curin[0]);
    hk->curin[1] = ntohs(hk->curin[1]);
    hk->curin[2] = ntohs(hk->curin[2]);
    hk->cursun = ntohs(hk->cursun);
    hk->cursys = ntohs(hk->cursys);
}

void p31u_ntoh_hk_out(p31u_hk_out_t* hk)
{
    if (!hk)
        return;

    for (int i = 0; i < 6; ++i) {
        hk->curout[i] = ntohs(hk->curout[i]);
        hk->latchup[i] = ntohs(hk->latchup[i]);
    }
    for (int i = 0; i < 8; ++i) {
        hk->output_on_delta[i] = ntohs(hk->output_on_delta[i]);
        hk->output_off_delta[i] = ntohs(hk->output_off_delta[i]);
    }
}

void p31u_ntoh_hk_wdt(p31u_hk_wdt_t* hk)
{
    if (!hk)
        return;

    hk->wdt_i2c_time_left = ntohl(hk->wdt_i2c_time_left);
    hk->wdt_gnd_time_left = ntohl(hk->wdt_gnd_time_left);
    hk->counter_wdt_i2c = ntohl(hk->counter_wdt_i2c);
    hk->counter_wdt_gnd = ntohl(hk->counter_wdt_gnd);
    hk->counter_wdt_csp[0] = ntohl(hk->counter_wdt_csp[0]);
    hk->counter_wdt_csp[1] = ntohl(hk->counter_wdt_csp[1]);
}

void p31u_ntoh_hk_basic(p31u_hk_basic_t* hk)
{
    if (!hk)
        return;

    hk->counter_boot = ntohl(hk->counter_boot);
    for (size_t i = 0; i < 6; ++i) {
        hk->temp[i] = ntohs(hk->temp[i]);
    }
}
#endif

int p31u_gethk(void* hk,
               uint8_t type,
               int size)
{
    if (!hk)
        return P31U_ERR_NULL;

    return p31u_transaction(P31U_PORT_HK,
                            &type,
                            1,
                            hk,
                            size);
}

int p31u_gethk_all(p31u_hk_t* hk)
{
#if P31U_USE_NTOH
    int status;
    status = p31u_gethk(hk, 0, sizeof(*hk));

    if (status == P31U_OK) {
        p31u_ntoh_hk_vi((p31u_hk_vi_t*)hk);
        p31u_ntoh_hk_out((p31u_hk_out_t*)hk->curout);
        p31u_ntoh_hk_wdt((p31u_hk_wdt_t*)&hk->wdt_i2c_time_left);
        p31u_ntoh_hk_basic((p31u_hk_basic_t*)&hk->counter_boot);
    }
    return status;
#else
    return p31u_gethk(hk, 0, sizeof(*hk));
#endif
}

int p31u_gethk_vi(p31u_hk_vi_t* hk)
{
#if P31U_USE_NTOH
    int status;
    status = p31u_gethk(hk, 1, sizeof(*hk));
    if (status == P31U_OK) {
        p31u_ntoh_hk_vi(hk);
    }
    return status;
#else
    return p31u_gethk(hk, 1, sizeof(*hk));
#endif
}

int p31u_gethk_out(p31u_hk_out_t* hk)
{
#if P31U_USE_NTOH
    int status;
    status = p31u_gethk(hk, 2, sizeof(*hk));
    if (status == P31U_OK) {
        p31u_ntoh_hk_out(hk);
    }
    return status;
#else
    return p31u_gethk(hk, 2, sizeof(*hk));
#endif
}

int p31u_gethk_wdt(p31u_hk_wdt_t* hk)
{
#if P31U_USE_NTOH
    int status;
    status = p31u_gethk(hk, 3, sizeof(*hk));
    if (status == P31U_OK) {
        p31u_ntoh_hk_wdt(hk);
    }
    return status;
#else
    return p31u_gethk(hk, 3, sizeof(*hk));
#endif
}

int p31u_gethk_basic(p31u_hk_basic_t* hk)
{
#if P31U_USE_NTOH
    int status;
    status = p31u_gethk(hk, 4, sizeof(*hk));
    if (status == P31U_OK) {
        p31u_ntoh_hk_basic(hk);
    }
    return status;
#else
    return p31u_gethk(hk, 4, sizeof(*hk));
#endif
}

int p31u_gethk_obsolete(p31u_hkparam_t* hk)
{
    return p31u_get(P31U_PORT_HK,
                    hk,
                    sizeof(*hk));
}

int p31u_set_outputs(uint8_t mask)
{
    return p31u_set(P31U_PORT_SET_OUTPUT,
                    &mask,
                    1);
}

int p31u_set_output_single(uint8_t channel,
                           uint8_t value,
                           int16_t delay)
{
#if P31U_USE_HTON
    delay = htons(delay);
#endif
    struct {
        uint8_t channel;
        uint8_t value;
        int16_t delay;
    } arg = {channel, value, delay};
    return p31u_set(P31U_PORT_SET_SINGLE_OUTPUT,
                    &arg,
                    sizeof(arg));
}

int p31u_set_pv_volt(int16_t voltage1,
                     int16_t voltage2,
                     int16_t voltage3)
{
#if P31U_USE_HTON
    voltage1 = htons(voltage1);
    voltage2 = htons(voltage2);
    voltage3 = htons(voltage3);
#endif
    int16_t arg[3] = {voltage1, voltage2, voltage3};
    return p31u_set(P31U_PORT_SET_PV_VOLT,
                    arg,
                    sizeof(arg));
}

int p31u_set_pv_auto(uint8_t mode)
{
    return p31u_set(P31U_PORT_SET_PV_AUTO,
                    &mode,
                    1);
}

int p31u_set_heater(uint8_t cmd,
                    uint8_t heater,
                    uint8_t mode,
                    p31u_reply_set_heater* reply)
{
    struct {
        uint8_t cmd;
        uint8_t heater;
        uint8_t mode;
    } arg = {cmd, heater, mode};
    return p31u_transaction(P31U_PORT_SET_HEATER,
                            &arg,
                            sizeof(arg),
                            reply,
                            sizeof(*reply));
}

int p31u_reset_counters(void)
{
    uint8_t magic = 0x42;
    return p31u_set(P31U_PORT_RESET_COUNTERS,
                    &magic,
                    1);
}

int p31u_reset_wdt(void)
{
    uint8_t magic = 0x78;
    return p31u_set(P31U_PORT_RESET_WDT,
                    &magic,
                    1);
}

int p31u_config_cmd(uint8_t cmd)
{
    return p31u_set(P31U_PORT_CONFIG_CMD,
                    &cmd,
                    1);
}

int p31u_get_config(p31u_config_t* config)
{
    if (!config)
        return P31U_ERR_NULL;

    return p31u_get(P31U_PORT_CONFIG_GET,
                    config,
                    sizeof(*config));
}

int p31u_set_config(p31u_req_const p31u_config_t* config)
{
    if (!config)
        return P31U_ERR_NULL;

#if P31U_USE_HTON
    for (int i = 0; i < 8; ++i) {
        config->output_initial_on_delay[i] 
            = htons(config->output_initial_on_delay[i]);
        config->output_initial_off_delay[i] 
            = htons(config->output_initial_off_delay[i]);
        config->vboost[0] = htons(config->vboost[0]);
        config->vboost[1] = htons(config->vboost[1]);
        config->vboost[2] = htons(config->vboost[2]);
    }
#endif
    return p31u_set(P31U_PORT_CONFIG_SET,
                    config,
                    sizeof(*config));
}

int p31u_config2_cmd(uint8_t cmd)
{
    return p31u_set(P31U_PORT_CONFIG2_CMD,
                    &cmd,
                    1);
}

int p31u_get_config2(p31u_config2_t* config)
{
    if (!config)
        return P31U_ERR_NULL;

#if P31U_USE_NTOH
    int status;
    status = p31u_get(P31U_PORT_CONFIG2_GET,
                      config,
                      sizeof(*config));
    if (status == P31U_OK) {
        //TODO: ntoh.
    }
    return status;
#else
    return p31u_get(P31U_PORT_CONFIG2_GET,
                    config,
                    sizeof(*config));
#endif
}

int p31u_set_config2(p31u_req_const p31u_config2_t* config)
{
    if (!config)
        return P31U_ERR_NULL;

#if P31U_USE_HTON
    for (int i = 0; i < 8; ++i) {;
        config->batt_maxvoltage = htons(config->batt_maxvoltage);
        config->batt_safevoltage = htons(config->batt_safevoltage);
        config->batt_criticalvoltage = htons(config->batt_criticalvoltage);
        config->batt_normalvoltage = htons(config->batt_normalvoltage);
    }
#endif
    return p31u_set(P31U_PORT_CONFIG2_SET,
                    config,
                    sizeof(*config));
}

int p31u_set_config3(p31u_req_const p31u_config3_t* config)
{
    if (!config)
        return P31U_ERR_NULL;

#if P31U_USE_HTON
    for (int i = 0; i < 8; ++i) {;
        config->cur_lim[i] = htons(config->cur_lim[i]);
    }
#endif
    return p31u_set(P31U_PORT_CONFIG3,
                    config,
                    sizeof(*config));
}
