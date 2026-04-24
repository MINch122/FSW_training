/***********************************************************************
 *  Copyright (c) 2025, Yonsei University as represented by the
 *  Department of Satellite Systems (DSS) & Astrodynamic & Control Lab (ACL)
 *  All rights reserved. This software was created at DSS
 *  
 *  Author : Hyeok-jin Kweon
 * 
 *  \file linux_gpio.c
 *
 ***********************************************************************/
/*
 * NOTE: This relies on the Linux Kernel I/O system
 * Documented here: 
 */

/************************************************************************
 * Includes
 ************************************************************************/
#include <fcntl.h>
#include <sys/select.h>
#include <poll.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <errno.h>
#include <gpiod.h>

#include "cfe_psp.h"
#include "cfe_psp_module.h"
#include "osapi.h"

#include "iodriver_impl.h"
#include "iodriver_discrete_io.h"

/********************************************************************
 * Local Defines
 ********************************************************************/
typedef struct {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    char name[32];
    bool default_val;
    bool isout;
} linux_gpio_internal_config_t;

typedef struct {
    bool usedstate;
    linux_gpio_internal_config_t internal;
} linux_gpio_entry_t;

#define LINUX_GPIO_TABLE_SIZE   10
#define LINUX_GPIO_TBL_MUTEX_NAME   "PSP_GPIO_TBL"
static linux_gpio_entry_t gpiotbl[LINUX_GPIO_TABLE_SIZE];
static osal_id_t gpiotbl_Mutex;

#define LINUX_GPIO_DEFAULT_NAME     "gpio_default_name"
/* Mutex definition */
#define LINUX_GPIO_NO_MUTEX_HASH     ((int32_t)-1)

/* Subsystem Definition */
#define LINUX_GPIO_OPEN_SUBSYS    CFE_PSP_IODriver_OPEN_SUBSYSTEM
#define LINUX_GPIO_WRITE_SUBSYS   CFE_PSP_IODriver_WRITE_SUBSYSTEM
#define LINUX_GPIO_READ_SUBSYS    CFE_PSP_IODriver_READ_SUBSYSTEM
#define LINUX_GPIO_CONFIG_SUBSYS  CFE_PSP_IODriver_CONFIG_SUBSYSTEM
#define LINUX_GPIO_CLOSE_SUBSYS   CFE_PSP_IODriver_CLOSE_SUBSYSTEM

static void linux_gpio_Init(uint32_t local_module_id);
static int32_t linux_gpio_open(void *arg);
static int32_t linux_gpio_write(void *arg);
static int32_t linux_gpio_read(void *arg);
static int32_t linux_gpio_close(uint32_t arg);
static int32_t linux_gpio_config(void *arg);

static int32_t linux_gpio_DevCmd(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg);
static int32_t linux_gpio_DevMutex(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg);

/********************************************************************
 * Global Data
 ********************************************************************/
/* linux_gpio device command that is called by iodriver to start up linux_gpio */
CFE_PSP_IODriver_API_t linux_gpio_DevApi = {.DeviceCommand = linux_gpio_DevCmd,
                                              .DeviceMutex = linux_gpio_DevMutex};

CFE_PSP_MODULE_DECLARE_IODEVICEDRIVER(linux_gpio);

/***********************************************************************
 * Global Functions
 ***********************************************************************/
void linux_gpio_Init(uint32_t local_module_id) {
    memset(gpiotbl, 0, sizeof(gpiotbl));
    OS_MutSemCreate(&gpiotbl_Mutex, LINUX_GPIO_TBL_MUTEX_NAME, 0);
    return;
}
/***********************************************************************
 * Util Functions
 ***********************************************************************/
static int32_t linux_gpio_allocate_handle(void) {
    OS_MutSemTake(gpiotbl_Mutex);
    for (int i = 0; i < LINUX_GPIO_TABLE_SIZE; i++) {
        if (!gpiotbl[i].usedstate) {
            gpiotbl[i].usedstate = true;
            OS_MutSemGive(gpiotbl_Mutex);
            return i;
        }
    }
    OS_MutSemGive(gpiotbl_Mutex);
    return CFE_PSP_IODriver_DISCRETE_IO_TABLE_FULL_ERROR; // revise `TABLE FULL`
}
static void linux_gpio_free_handle(int h) {
    OS_MutSemTake(gpiotbl_Mutex);
    memset(&gpiotbl[h], 0, sizeof(linux_gpio_entry_t));
    OS_MutSemGive(gpiotbl_Mutex);

    return;
}

/// @brief Init gpio & return handle
/// @param arg [in] Casted to `CFE_PSP_IODriver_Gpio_init_t`
/// @return gpio handle (>= 0) for success, (< 0) for error
int32_t linux_gpio_open(void *arg) {
    ARGCHECK(arg, CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE);

    int32_t StatusCode;
    int32_t handle;
    CFE_PSP_IODriver_Gpio_init_t *cfg = (CFE_PSP_IODriver_Gpio_init_t *)arg;
    struct gpiod_chip *tempchip;
    struct gpiod_line *templine;

    handle = linux_gpio_allocate_handle();
    if (handle < 0) return CFE_PSP_IODriver_DISCRETE_IO_TABLE_FULL_ERROR;

    tempchip = gpiod_chip_open(cfg->path);
    if (!tempchip) {
        linux_gpio_free_handle(handle);
        return CFE_PSP_IODriver_DISCRETE_IO_CHIP_OPEN_ERROR;
    }

    templine = gpiod_chip_get_line(tempchip, cfg->line);
    if (!templine) {
        gpiod_chip_close(tempchip);
        linux_gpio_free_handle(handle);
        return CFE_PSP_IODriver_DISCRETE_IO_GET_LINE_ERROR;
    }

    if (cfg->isout) StatusCode = gpiod_line_request_output(templine, cfg->name, cfg->default_val);
    else StatusCode = gpiod_line_request_input(templine, cfg->name);
    if (StatusCode < 0) {
        gpiod_chip_close(tempchip);
        linux_gpio_free_handle(handle);
        return cfg->isout ? CFE_PSP_IODriver_DISCRETE_IO_REQUEST_OUTPUT_ERROR
                          : CFE_PSP_IODriver_DISCRETE_IO_REQUEST_INPUT_ERROR;
    }

    gpiotbl[handle].internal.chip = tempchip;
    gpiotbl[handle].internal.line = templine;
    gpiotbl[handle].internal.isout = cfg->isout;
    gpiotbl[handle].internal.default_val = cfg->default_val;
    snprintf(gpiotbl[handle].internal.name, sizeof((linux_gpio_internal_config_t *)0)->name,
             "%s", cfg->name ? cfg->name : LINUX_GPIO_DEFAULT_NAME);

    return handle;
}

/// @brief Set gpio level
/// @param arg [in] Casted to `CFE_PSP_IODriver_GpioVal_t`
/// @return `0`for success, negative for error.
int32_t linux_gpio_write(void *arg) {
    ARGCHECK(arg, CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE);

    int32_t StatusCode;
    CFE_PSP_IODriver_GpioVal_t *val = (CFE_PSP_IODriver_GpioVal_t *)arg;
    if (val->handle >= LINUX_GPIO_TABLE_SIZE) return CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE;

    OS_MutSemTake(gpiotbl_Mutex);
    linux_gpio_entry_t entry = gpiotbl[val->handle];
    if (!entry.usedstate) {
        OS_MutSemGive(gpiotbl_Mutex);
        return CFE_PSP_IODriver_DISCRETE_IO_NOT_OPEN_ERROR;
    }
    if (!entry.internal.isout) {
        OS_MutSemGive(gpiotbl_Mutex);
        return CFE_PSP_IODriver_DISCRETE_IO_INVALID_DIRECTION_ERROR;
    }
    OS_MutSemGive(gpiotbl_Mutex);

    StatusCode = gpiod_line_set_value(entry.internal.line, (int)(val->level));
    if (StatusCode < 0) return CFE_PSP_IODriver_DISCRETE_IO_SET_ERROR;

    return CFE_PSP_SUCCESS;
}

/// @brief Get gpio level
/// @param arg [in, out] Casted to `CFE_PSP_IODriver_GpioVal_t`
/// @return `0` for success, negative for error.
int32_t linux_gpio_read(void *arg) {
    ARGCHECK(arg, CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE);

    int32_t StatusCode;
    CFE_PSP_IODriver_GpioVal_t *val = (CFE_PSP_IODriver_GpioVal_t *)arg;
    if (val->handle >= LINUX_GPIO_TABLE_SIZE) return CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE;

    OS_MutSemTake(gpiotbl_Mutex);
    linux_gpio_entry_t entry = gpiotbl[val->handle];
    if (!entry.usedstate) {
        OS_MutSemGive(gpiotbl_Mutex);
        return CFE_PSP_IODriver_DISCRETE_IO_NOT_OPEN_ERROR;
    }
    if (entry.internal.isout) {
        OS_MutSemGive(gpiotbl_Mutex);
        return CFE_PSP_IODriver_DISCRETE_IO_INVALID_DIRECTION_ERROR;
    }
    OS_MutSemGive(gpiotbl_Mutex);

    StatusCode = gpiod_line_get_value(entry.internal.line);
    if (StatusCode < 0) return CFE_PSP_IODriver_DISCRETE_IO_GET_ERROR;
    
    val->level = StatusCode;

    return CFE_PSP_SUCCESS;
}

/// @brief Change configuration of corresponded gpio line
/// @param arg [in] Casted to `CFE_PSP_IODriver_Gpio_config_t`
/// @return `0` for success, negative for error
int32_t linux_gpio_config(void *arg) {
    ARGCHECK(arg, CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE);

    int32_t StatusCode;

    CFE_PSP_IODriver_Gpio_config_t *cfg = (CFE_PSP_IODriver_Gpio_config_t *)arg;
    if (cfg->handle >= LINUX_GPIO_TABLE_SIZE) return CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE;

    OS_MutSemTake(gpiotbl_Mutex);
    linux_gpio_entry_t *entry = &gpiotbl[cfg->handle];
    if (!entry->usedstate) {
        OS_MutSemGive(gpiotbl_Mutex);
        return CFE_PSP_IODriver_DISCRETE_IO_NOT_OPEN_ERROR;
    }
    OS_MutSemGive(gpiotbl_Mutex);

    gpiod_line_release(entry->internal.line);

    if (cfg->cfg.isout)
        StatusCode = gpiod_line_request_output(entry->internal.line, cfg->cfg.name, cfg->cfg.default_val);
    else 
        StatusCode = gpiod_line_request_input(entry->internal.line, cfg->cfg.name);

    if (StatusCode < 0) return cfg->cfg.isout ? CFE_PSP_IODriver_DISCRETE_IO_REQUEST_OUTPUT_ERROR
                                              : CFE_PSP_IODriver_DISCRETE_IO_REQUEST_INPUT_ERROR;

    OS_MutSemTake(gpiotbl_Mutex);
    entry->internal.isout = cfg->cfg.isout;
    entry->internal.default_val = cfg->cfg.default_val;
    snprintf(entry->internal.name, sizeof(entry->internal.name), "%s",
            cfg->cfg.name ? cfg->cfg.name : LINUX_GPIO_DEFAULT_NAME);
    OS_MutSemGive(gpiotbl_Mutex);

    return CFE_PSP_SUCCESS;
}

/// @brief Close handle and free resources
/// @param arg [in] handle number
/// @return `0` for success. No error case
int32_t linux_gpio_close(uint32_t arg) {
    if (arg >= LINUX_GPIO_TABLE_SIZE) return CFE_PSP_IODriver_DISCRETE_IO_INVALID_HANDLE;
    struct gpiod_chip *tempchip = NULL;
    struct gpiod_line *templine = NULL;

    OS_MutSemTake(gpiotbl_Mutex);
    linux_gpio_entry_t *entry = &gpiotbl[arg];
    if (!entry->usedstate) {
        OS_MutSemGive(gpiotbl_Mutex);
        return CFE_PSP_SUCCESS;
    }
    tempchip = entry->internal.chip;
    templine = entry->internal.line;
    entry->usedstate = false;
    OS_MutSemGive(gpiotbl_Mutex);

    if (templine) gpiod_line_release(templine);
    if (tempchip) gpiod_chip_close(tempchip);

    linux_gpio_free_handle((int)arg);

    return CFE_PSP_SUCCESS;
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_gpio_DevCmd()                               */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/**
 * \brief Main entry point for API.
 *
 * This function is called through iodriver to invoke the linux_gpio module.
 *
 * \par Assumptions, External Events, and Notes:
 *          None
 *
 * \param[in] CommandCode  The CFE_PSP_IODriver_xxx command.
 * \param[in] SubsystemId  The monitor subsystem identifier
 * \param[in] SubchannelId The monitor subchannel identifier
 * \param[in] Arg          The arguments for the corresponding command.
 *
 * \returns Status code
 * \retval Positive value if successful, negative value for error.
 */
int32_t linux_gpio_DevCmd(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg) {
    
    int32_t StatusCode;

    StatusCode = CFE_PSP_ERROR_NOT_IMPLEMENTED;
    switch (SubsystemId) {
        case LINUX_GPIO_OPEN_SUBSYS:
            /* invoke `open()` */
            StatusCode = linux_gpio_open(Arg.Vptr);
            break;
        case LINUX_GPIO_WRITE_SUBSYS:
            /* invoke `write()` */
            StatusCode = linux_gpio_write(Arg.Vptr);
            break;
        case LINUX_GPIO_READ_SUBSYS:
            /* invoke `read()` */
            StatusCode = linux_gpio_read(Arg.Vptr);
            break;
        case LINUX_GPIO_CONFIG_SUBSYS:
            /* invoke config func */
            StatusCode = linux_gpio_config(Arg.Vptr);
            break;
        case LINUX_GPIO_CLOSE_SUBSYS:
            /* invoke `close()` */
            StatusCode = linux_gpio_close(Arg.U32);
            break;

        default:
            /* do nothing */
            StatusCode = CFE_PSP_IODriver_DISCRETE_IO_INVALID_TYPE_ERROR;
            break;
    }

    return StatusCode;
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/*    linux_gpio_DevMutex()                             */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/**
 * \brief Hash function for lock table indexing.
 *
 * This function is called through iodriver to invoke the linux_gpio hash calculation.
 *
 * \par Assumptions, External Events, and Notes:
 *          None
 *
 * \param[in] CommandCode  The CFE_PSP_IODriver_xxx command.
 * \param[in] SubsystemId  The monitor subsystem identifier
 * \param[in] SubchannelId The monitor subchannel identifier
 * \param[in] Arg          The arguments for the corresponding command.
 *
 * \returns Status code
 * \retval Positive value if use lock/unlock, negative value for no lock.
 */
int32_t linux_gpio_DevMutex(uint32_t CommandCode, uint16_t SubsystemId, uint16_t SubchannelId,
                                    CFE_PSP_IODriver_Arg_t Arg) {

    int32_t hash = 0;
    /* Open Subsystem does not need the lock */
    if (SubsystemId == LINUX_GPIO_OPEN_SUBSYS)
        hash = LINUX_GPIO_NO_MUTEX_HASH;

    
    /* Other Subsystem need lock, hash is calculated by chip, line val (TBD)*/
    else if (Arg.U32 && SubsystemId == LINUX_GPIO_CLOSE_SUBSYS) {
        hash = CFE_PSP_IODriver_HashMutex(hash, (int)Arg.U32);
    }
    else if (Arg.Vptr) {
        if (SubsystemId == LINUX_GPIO_CONFIG_SUBSYS) {
            const CFE_PSP_IODriver_Gpio_config_t *cfg = (CFE_PSP_IODriver_Gpio_config_t *)Arg.Vptr;
            hash = CFE_PSP_IODriver_HashMutex(hash, cfg->handle);
        }
        else {
            const CFE_PSP_IODriver_GpioVal_t *x = (CFE_PSP_IODriver_GpioVal_t *)Arg.Vptr;
            hash = CFE_PSP_IODriver_HashMutex(hash, x->handle);
        }
    }
    
    return hash;
}