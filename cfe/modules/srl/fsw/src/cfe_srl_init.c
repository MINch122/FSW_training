/* Auto-Generated file. Never change this code! */
/**
 * Required header files
*/
#include "cfe_srl_module_all.h"

/**
 * Global data
*/
CFE_SRL_IO_Handle_t *Handles[CFE_SRL_GNRL_DEVICE_NUM];
/**************************************************
 * Index of Each device
 * 0 : SOCAT Handle
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

	Status = CFE_SRL_PriorInit();
	if(Status != CFE_SUCCESS) return Status;
	CFE_ES_WriteToSysLog("%s: Prior Initialized.", __func__);
	/**************************************************
	 * Serial Comm. Init
 	 * Only `ready == true` interface is initialized
	 **************************************************/
	/* socat Init */
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_SOCAT_HANDLE_INDEXER], "socat", "/dev/pts/2", SRL_DEVTYPE_UART, CFE_SRL_SOCAT_HANDLE_INDEXER, 115200, 0);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: socat Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_SOCAT_INIT_ERR;
	}
	CFE_ES_WriteToSysLog("%s: socat Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_SOCAT_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_SOCAT_HANDLE_INDEXER])->DevName);

	/* GPIO SP_IN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_SP_IN_GPIO_INDEXER], "/dev/gpiochip2", 4, "SP_IN", 0, false);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO SP_IN Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_SP_IN_INIT_ERR;
	}

	/* GPIO SP_OUT Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_SP_OUT_GPIO_INDEXER], "/dev/gpiochip2", 3, "SP_OUT", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO SP_OUT Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_SP_OUT_INIT_ERR;
	}

	/* GPIO ADCS_EN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_ADCS_EN_GPIO_INDEXER], "/dev/gpiochip0", 29, "ADCS_EN", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO ADCS_EN Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_ADCS_EN_INIT_ERR;
	}

	/* GPIO ADCS_BOOT Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_ADCS_BOOT_GPIO_INDEXER], "/dev/gpiochip0", 28, "ADCS_BOOT", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO ADCS_BOOT Initialization failed! RC=%d\n", __func__, Status);
		// return CFE_SRL_ADCS_BOOT_INIT_ERR;
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
