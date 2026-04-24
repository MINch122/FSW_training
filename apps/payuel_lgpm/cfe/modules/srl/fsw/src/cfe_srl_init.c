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
 * 0 : RS422 Handle
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
	/* RS422 Init */
	Config.cfg.uart = (CFE_PSP_UART_cfg_t) {.baud = 115200,
            							    .databits = 8,
            							    .parity = 0,
            							    .stopbits = 1};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_RS422_HANDLE_INDEXER], "RS422", "/dev/ttyS2", SRL_DEVTYPE_UART, CFE_SRL_RS422_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: RS422 Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: RS422 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_RS422_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_RS422_HANDLE_INDEXER])->DevName);

	Status = CFE_SRL_InitCSP();
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: CSP Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: CSP Successfully Initialized.\n", __func__);

return CFE_SUCCESS;
}
