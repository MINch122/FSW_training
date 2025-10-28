/* Auto-Generated file. Never change this code! */

/**
 * Required header files
 */
#include "cfe_srl_module_all.h"
#include "cfe_psp.h"
#include "iodriver_base.h"
#include "iodriver_discrete_io.h"
#include "iodriver_serial_io.h"

/**
 * Global data
*/
CFE_SRL_IO_Handle_t *Handles[CFE_SRL_GNRL_DEVICE_NUM];
/**************************************************
 * Index of Each device
 * 0 : I2C0 Handle
 * 1 : I2C1 Handle
 * 2 : I2C2 Handle
 * 3 : UART Handle
 * 4 : RS422 Handle
 **************************************************/

CFE_SRL_GPIO_Handle_t GPIO[CFE_SRL_TOT_GPIO_NUM];


/************************************************************************
 * Early Initialization function executed at cFE ES
 * Append object to `cfe_es_objtab.c`
 * Declaration is located at
 * `cfe/modules/core_private/fsw/inc/cfe_srl_core_internal.h`
 ************************************************************************/
int32 CFE_SRL_EarlyInit(void) {
	int32 Status;

	CFE_PSP_IODriver_Serial_cfg_t Config = {0};	// FD will be inserted in `CFE_SRL_HandleInit()`

	Status = CFE_SRL_PriorInit();
	if(Status != CFE_SUCCESS) return Status;
	CFE_ES_WriteToSysLog("%s: Prior Initialized.", __func__);

	/**************************************************
	 * Serial Comm. Init
 	 * Only `ready == true` interface is initialized
	 **************************************************/
	/* I2C0 Init */
	Config.cfg.i2c = (CFE_PSP_I2C_cfg_t) {.tenbit = false,
            							  .pec_en = false,
            							  .retries = 3};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_I2C0_HANDLE_INDEXER], "I2C0", "/dev/i2c-0", SRL_DEVTYPE_I2C, CFE_SRL_I2C0_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: I2C0 Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_I2C0_INIT_ERR;
	}
	CFE_ES_WriteToSysLog("%s: I2C0 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_I2C0_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_I2C0_HANDLE_INDEXER])->DevName);

	/* I2C1 Init */
	Config.cfg.i2c = (CFE_PSP_I2C_cfg_t) {.tenbit = false,
            							  .pec_en = false,
            							  .retries = 3};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_I2C1_HANDLE_INDEXER], "I2C1", "/dev/i2c-1", SRL_DEVTYPE_I2C, CFE_SRL_I2C1_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: I2C1 Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_I2C1_INIT_ERR;
	}
	CFE_ES_WriteToSysLog("%s: I2C1 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_I2C1_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_I2C1_HANDLE_INDEXER])->DevName);

	/* I2C2 Init */
	Config.cfg.i2c = (CFE_PSP_I2C_cfg_t) {.tenbit = false,
            							  .pec_en = false,
            							  .retries = 3};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_I2C2_HANDLE_INDEXER], "I2C2", "/dev/i2c-2", SRL_DEVTYPE_I2C, CFE_SRL_I2C2_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: I2C2 Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_I2C2_INIT_ERR;
	}
	CFE_ES_WriteToSysLog("%s: I2C2 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_I2C2_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_I2C2_HANDLE_INDEXER])->DevName);

	/* UART Init */
	Config.cfg.uart = (CFE_PSP_UART_cfg_t) {.baud = 115200,
            							    .databits = 8,
            							    .parity = 0,
            							    .stopbits = 1};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_UART_HANDLE_INDEXER], "UART", "/dev/ttyS0", SRL_DEVTYPE_UART, CFE_SRL_UART_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: UART Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_UART_INIT_ERR;
	}
	CFE_ES_WriteToSysLog("%s: UART Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_UART_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_UART_HANDLE_INDEXER])->DevName);

	/* RS422 Init */
	Config.cfg.uart = (CFE_PSP_UART_cfg_t) {.baud = 115200,
            							    .databits = 8,
            							    .parity = 0,
            							    .stopbits = 1};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_RS422_HANDLE_INDEXER], "RS422", "/dev/ttyS1", SRL_DEVTYPE_UART, CFE_SRL_RS422_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: RS422 Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_RS422_INIT_ERR;
	}
	CFE_ES_WriteToSysLog("%s: RS422 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_RS422_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_RS422_HANDLE_INDEXER])->DevName);

	/* GPIO SP_IN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_SP_IN_GPIO_INDEXER], "/dev/gpiochip2", 4, "SP_IN", 0, false);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO SP_IN Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_SP_IN_INIT_ERR;
	}

	/* GPIO SP_OUT1 Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_SP_OUT1_GPIO_INDEXER], "/dev/gpiochip2", 3, "SP_OUT1", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO SP_OUT1 Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_SP_OUT1_INIT_ERR;
	}

	/* GPIO SP_OUT2 Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_SP_OUT2_GPIO_INDEXER], "/dev/gpiochip0", 28, "SP_OUT2", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO SP_OUT2 Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_SP_OUT2_INIT_ERR;
	}

	/* GPIO ADCS_EN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_ADCS_EN_GPIO_INDEXER], "/dev/gpiochip0", 29, "ADCS_EN", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO ADCS_EN Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_ADCS_EN_INIT_ERR;
	}

	/* GPIO THRUSTER Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_THRUSTER_GPIO_INDEXER], "/dev/gpiochip2", 5, "THRUSTER", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO THRUSTER Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_THRUSTER_INIT_ERR;
	}

	Status = CFE_SRL_InitCSP();
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: CSP Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_CSP_INIT_ERR;
	}
	CFE_ES_WriteToSysLog("%s: CSP Successfully Initialized.\n", __func__);

return CFE_SUCCESS;
}
