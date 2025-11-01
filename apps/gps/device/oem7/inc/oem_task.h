/**
 * @file oem_task.h
 * @brief OEM7 log receiving tasks.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2024.
 */
#ifndef _OEM_TASK_H_
#define _OEM_TASK_H_

#include "oem_config.h"
#include "oem_basetype.h"

typedef enum {
    TASK_MAIN       = 1, /* Returend from OEM_Task_ReadTaskSingleRun. */
    TASK_RESPONSE   = 2, /* Returend from ProcessMessage (response handler). */
    TASK_LOG        = 3, /* Returend from _DoLogHandle (log handler). */
    TASK_CALLBACK   = 4, /* Returend from a log handler callback. */
} oem_task_tasklevel;
 
typedef struct {
    oem_enum respId;     /* Response ID, if the message was a Response. */
    int callbackExecCnt; /* Number of executed log callbacks. */
    oem_short mid;       /* Message ID. */
    oem_crc msgCrc;
    uint8_t taskLevel;   /* Last called handling procedure. See oem_task_tasklevel. */
    bool isResponse;     /* Was it a Response message? */
    bool critFailure;    /* Marked true if the handler went broken. */
    bool crcReadSkipped;
} oem_task_context_t;

/**
 * @brief Process a single receiver message from the serial port.
 *
 * @param physicalPort  Physical port index to read from. Use the designated
 *                      indices from OEM_InitPhysicalPort().
 * @param[out] ctx      Log handling context.
 * @return OEM_OK if successful. An oem_ret error code otherwise. See 
 *         ctx->taskLevel to determine where it returned.
 */
int OEM_Task_ReadTaskSingleRun(int physicalPort,
                               oem_task_context_t* ctx);

/**
 * @brief Returns if @a msg has the correct sync word in the first three bytes.
 * 
 * @param msg An OEM message (minimum three bytes).
 * @return true if sync matches. false otherwise or @a msg is NULL.
 */
bool OEM_Task_IsSynced(const void* msg);

#endif