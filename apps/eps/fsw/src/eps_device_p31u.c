#include "eps_device_p31u.h"

#include "p31u.h"

void EPS_P31U_GetDeviceHkData(EPS_HkTlm_Payload_t* Payload)
{
    p31u_hk_t hk;

    memset(Payload, 0, sizeof(*Payload));

    if (p31u_gethk_all(&hk) == P31U_OK) {
        Payload->vbatt = hk.vbatt;
        for (int i = 0; i < 8; ++i)
            Payload->output[i] = hk.output[i];
        for (int i = 0; i < 6; ++i) {
            Payload->curout[i] = hk.curout[i];
            Payload->latchup[i] = hk.latchup[i]; /* u16 -> u8 */
        }
        Payload->curin[0] = hk.curin[0];
        Payload->curin[1] = hk.curin[1];
        Payload->curin[2] = hk.curin[2];
        Payload->cursun = hk.cursun;
        Payload->cursys = hk.cursys;
        Payload->counter_boot = hk.counter_boot; /* u32 -> u16 */
        Payload->wdt_gnd_time_left = hk.wdt_gnd_time_left;
        Payload->temp[0] = hk.temp[0]; /* TEMP1 */
        Payload->temp[1] = hk.temp[4]; /* BP4a  */
        Payload->temp[2] = hk.temp[5]; /* BP4b  */
        Payload->bootcause = hk.bootcause;
        Payload->battmode = hk.battmode;
    }
    else
        EPS_AppData.Counters.GetHkErrCounter++;
}

void EPS_P31U_GetDeviceBcnData(EPS_BcnTlm_Payload_t* Payload)
{
    p31u_hk_t hk;
    p31u_config_t conf;
    int ret;
    bool err = false;

    memset(Payload, 0, sizeof(*Payload));

    ret = p31u_gethk_all(&hk);
    if (ret == P31U_OK) {
        Payload->vbatt = hk.vbatt;
        for (int i = 0; i < 8; ++i)
            Payload->output[i] = hk.output[i];
        for (int i = 0; i < 6; ++i) {
            Payload->curout[i] = hk.curout[i];
        }
        Payload->curin[0] = hk.curin[0];
        Payload->curin[1] = hk.curin[1];
        Payload->curin[2] = hk.curin[2];
        Payload->cursys = hk.cursys;
        Payload->counter_boot = hk.counter_boot; /* u32 -> u16 */
        Payload->wdt_gnd_time_left = hk.wdt_gnd_time_left;
        Payload->bootcause = hk.bootcause;
        Payload->battmode = hk.battmode;
        Payload->bp4_temp[0] = hk.temp[4]; // add batt temp
        Payload->bp4_temp[1] = hk.temp[5]; // add batt temp
        OS_printf("%s Temp1 : %d || Temp2 %d\n", __func__, Payload->bp4_temp[0], Payload->bp4_temp[1]);
        OS_printf("output:       %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d\n",
            Payload->output[0], Payload->output[1], Payload->output[2], Payload->output[3],
            Payload->output[4], Payload->output[5], Payload->output[6], Payload->output[7]);
    }
    else
        err = true;

    ret = p31u_get_config(&conf);
    
    if (ret == P31U_OK) {
        Payload->battheater_mode = conf.battheater_mode; // add. Indicate Auto or Manual
        OS_printf("Battheater mode : %u\n", Payload->battheater_mode);
    }
    else
        err = true;

    if (err) {OS_printf("%s Fail.\n", __func__); EPS_AppData.Counters.GetBcnErrCounter++;}
}


