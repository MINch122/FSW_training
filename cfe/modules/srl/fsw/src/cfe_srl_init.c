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
 * 0 : RS485 Handle
 * 1 : CAN0 Handle
 * 2 : RS422 Handle
 * 3 : UART Handle
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
	/* RS485 Init */
	Config.cfg.uart = (CFE_PSP_UART_cfg_t) {.baud = 250000,
            							    .databits = 8,
            							    .parity = 0,
            							    .stopbits = 1};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_RS485_HANDLE_INDEXER], "RS485", "/dev/ttyS1", SRL_DEVTYPE_UART, CFE_SRL_RS485_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: RS485 Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: RS485 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_RS485_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_RS485_HANDLE_INDEXER])->DevName);

	/* CAN0 Init */
	CFE_PSP_CAN_filter_t filters[0];
	Config.cfg.can = (CFE_PSP_CAN_cfg_t) {.filter = filters,
            							  .filter_num = 0,
            							  .loopback_en = false,
            							  .recv_own_msgs = false,
            							  .recv_err_frame = false};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_CAN0_HANDLE_INDEXER], "CAN0", "can0", SRL_DEVTYPE_CAN, CFE_SRL_CAN0_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: CAN0 Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: CAN0 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_CAN0_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_CAN0_HANDLE_INDEXER])->DevName);

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

	/* UART Init */
	Config.cfg.uart = (CFE_PSP_UART_cfg_t) {.baud = 921600,
            							    .databits = 8,
            							    .parity = 0,
            							    .stopbits = 1};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_UART_HANDLE_INDEXER], "UART", "/dev/ttyS3", SRL_DEVTYPE_UART, CFE_SRL_UART_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: UART Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: UART Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_UART_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_UART_HANDLE_INDEXER])->DevName);

	/* GPIO SP_IN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_SP_IN_GPIO_INDEXER], "/dev/gpiochip2", 4, "SP_IN", 0, false);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO SP_IN Initialization failed! RC=%d\n", __func__, Status);
		return CFE_SRL_SP_IN_INIT_ERR;
	}

	/* GPIO LTRX_EN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_LTRX_EN_GPIO_INDEXER], "/dev/gpiochip2", 5, "LTRX_EN", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO LTRX_EN Initialization failed! RC=%d\n", __func__, Status);
		return CFE_SRL_LTRX_EN_INIT_ERR;
	}

	/* GPIO DEP1_EN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_DEP1_EN_GPIO_INDEXER], "/dev/gpiochip0", 29, "DEP1_EN", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO DEP1_EN Initialization failed! RC=%d\n", __func__, Status);
		return CFE_SRL_DEP1_EN_INIT_ERR;
	}

	/* GPIO DEP2_EN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_DEP2_EN_GPIO_INDEXER], "/dev/gpiochip0", 28, "DEP2_EN", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO DEP2_EN Initialization failed! RC=%d\n", __func__, Status);
		return CFE_SRL_DEP2_EN_INIT_ERR;
	}

	/* GPIO STX_EN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_STX_EN_GPIO_INDEXER], "/dev/gpiochip0", 23, "STX_EN", 1, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO STX_EN Initialization failed! RC=%d\n", __func__, Status);
		return CFE_SRL_STX_EN_INIT_ERR;
	}

	/* GPIO ADCS_EN Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_ADCS_EN_GPIO_INDEXER], "/dev/gpiochip0", 24, "ADCS_EN", 1, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO ADCS_EN Initialization failed! RC=%d\n", __func__, Status);
		return CFE_SRL_ADCS_EN_INIT_ERR;
	}

	/* GPIO ADCS_BOOT Init */
	Status = CFE_SRL_GpioInit(&GPIO[CFE_SRL_ADCS_BOOT_GPIO_INDEXER], "/dev/gpiochip0", 26, "ADCS_BOOT", 0, true);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: GPIO ADCS_BOOT Initialization failed! RC=%d\n", __func__, Status);
		return CFE_SRL_ADCS_BOOT_INIT_ERR;
	}

	Status = CFE_SRL_InitCSP();
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: CSP Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: CSP Successfully Initialized.\n", __func__);

return CFE_SUCCESS;
}
