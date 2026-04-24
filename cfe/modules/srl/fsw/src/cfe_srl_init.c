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
 * 0 : I2C1 Handle
 * 1 : RS485 Handle
 * 2 : SPIO Handle
 * 3 : CAN0 Handle
 * 4 : I2C2 Handle
 * 5 : RS422 Handle
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
	/* I2C1 Init */
	Config.cfg.i2c = (CFE_PSP_I2C_cfg_t) {.tenbit = false,
            							  .pec_en = false,
            							  .retries = 3};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_I2C1_HANDLE_INDEXER], "I2C1", "/dev/i2c-1", SRL_DEVTYPE_I2C, CFE_SRL_I2C1_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: I2C1 Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: I2C1 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_I2C1_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_I2C1_HANDLE_INDEXER])->DevName);

	/* RS485 Init */
	Config.cfg.uart = (CFE_PSP_UART_cfg_t) {.baud = 115200,
            							    .databits = 8,
            							    .parity = 0,
            							    .stopbits = 1};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_RS485_HANDLE_INDEXER], "RS485", "/dev/ttyS1", SRL_DEVTYPE_UART, CFE_SRL_RS485_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: RS485 Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: RS485 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_RS485_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_RS485_HANDLE_INDEXER])->DevName);

	/* SPIO Init */
	Config.cfg.spi = (CFE_PSP_SPI_cfg_t) {.mode = 0,
            							  .bpw = 8,
            							  .speed = 2000000};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_SPIO_HANDLE_INDEXER], "SPIO", "/dev/spidev0.0", SRL_DEVTYPE_SPI, CFE_SRL_SPIO_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: SPIO Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: SPIO Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_SPIO_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_SPIO_HANDLE_INDEXER])->DevName);

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

	/* I2C2 Init */
	Config.cfg.i2c = (CFE_PSP_I2C_cfg_t) {.tenbit = false,
            							  .pec_en = false,
            							  .retries = 3};
	Status = CFE_SRL_HandleInit(&Handles[CFE_SRL_I2C2_HANDLE_INDEXER], "I2C2", "/dev/i2c-2", SRL_DEVTYPE_I2C, CFE_SRL_I2C2_HANDLE_INDEXER, &Config);
	if (Status != CFE_SUCCESS) {
		CFE_ES_WriteToSysLog("%s: I2C2 Initialization failed! RC=%d\n", __func__, Status);
	}
	else CFE_ES_WriteToSysLog("%s: I2C2 Initialized. FD=%d || DevName=%s\n", __func__, Handles[CFE_SRL_I2C2_HANDLE_INDEXER]->FD, ((CFE_SRL_Global_Handle_t *)Handles[CFE_SRL_I2C2_HANDLE_INDEXER])->DevName);

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
