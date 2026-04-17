/**
 * @file
 *
 * Purpose:
 *      This header file contains all definitions for the cFE Serial Comm. service
 *      Application Programmer's Interface.
 *
 * Author:   Kweon Hyeok-jin
 *
 */

#ifndef CFE_SRL_API_TYPEDEFS_H
#define CFE_SRL_API_TYPEDEFS_H

#include "common_types.h"
#include "cfe_srl_extern_typedefs.h"

/*****************************************************************************/
/*
** Type Definitions
*/


/* Foward declaration of struct. Refer the `CFE_SRL_IO_Handle_s` */
typedef struct CFE_SRL_IO_Handle_s            CFE_SRL_IO_Handle_t;

/**
 * TRx Function Prototypes
 */
typedef int32 (*CFE_SRL_Write_Function_t)(  /* \brief Tx function of specific handle */
                CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Param);
typedef int32 (*CFE_SRL_Read_Function_t)(   /* \brief (T)Rx function of specific handle */
                CFE_SRL_IO_Handle_t *Handle, CFE_SRL_IO_Param_t *Param);

/**
 * Combined structure which include the I/O TRx function
 */
typedef struct {
    CFE_SRL_Write_Function_t TxFunc;
    CFE_SRL_Read_Function_t RxFunc;
} CFE_SRL_Function_t;

/**
 * \brief SRL I/O Handle.
 * Most of SRL Api function use this struct for serial I/O transaction
 * Specific handle do transaction via it's member function pointer
 * \note Application can manually change the function if other specific I/O
 * protocol required
 */
struct CFE_SRL_IO_Handle_s {
    int FD;
    /**
     * I/O counters & recent errno
     */
    CFE_PSP_IODriver_Serial_cnt_Payload_t Counters;
    /**
     * I/O function for each handle
     */
    CFE_SRL_Function_t Func;
};

/**
 * \brief SRL GPIO Handle.
 * Just contain the `int` value, which is similar to file descriptor
 * \note Member `Handle` internally point the PSP iodrver
 * GPIO table's particular index
 */
struct gpiod_chip;
struct gpiod_line;

typedef struct {
    struct gpiod_chip *Chip;
    struct gpiod_line *Line;
    bool IsOut;
} CFE_SRL_GPIO_Handle_t;

#endif
