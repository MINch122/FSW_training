/**
 * @file oem_task.c
 * @brief OEM7 log receiving tasks.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#include "oem_task.h"
#include "oem_io.h"
#include "msg/oem_msg_common.h"
#include "oem_utils.h"

#include <string.h>
#if OEM_DEBUG
#include <stdio.h> /* Response print */
#endif

static union readTaskMsgBuf {
    uint8_t buf[OEM_TASK_MSG_BUF_SIZE];
    oem_binary_header_t header;
} readTaskMsgBuf;

int _DoLogHandle(void* msg, oem_task_context_t* ctx);

bool OEM_Task_IsSynced(const void* msg)
{
    const uint8_t* byte;
    if ((byte = msg) != NULL      &&
        *byte++ == OEM_SYNC_BYTE1 &&
        *byte++ == OEM_SYNC_BYTE2 &&
        *byte   == OEM_SYNC_BYTE3) {
        return true;
    }
    return false;
}

static int ProcessMessage(void* message,
                          oem_task_context_t* ctx)
{
    const oem_binary_header_t* header = message;
    const oem_binary_response* response;
    oem_crc atSiteCrc;
    int ret;

    if (!ctx)
        return OEM_ERR_NULL;

    if ((header->messageType & OEM_MSGTYPE_RESPONSE)
        != OEM_MSGTYPE_RESPONSE) {
        /**
         * This is a log message; call the handler.
         * 
         * An implementation may launch the handler as a temporary thread if
         * certain callback sequences are time-consuming. In such scenarios
         * the single message buffer in this task should be newly allocated or
         * segmented accordingly. This is not the case for our current mission.
         */
        return _DoLogHandle(message, ctx);
    }

    /**
     * Else, this is a response message.
     */
    ctx->isResponse = true;
    response = message;

    /**
     * Reject if too short.
     */
    if (sizeof(*header) + header->messageLength < sizeof(*response)) {
        ret = OEM_ERR_LEN_MSG;
        goto early_return_mproc;
    }

    /**
     * Print the ASCII response (debugging only).
     */
#if OEM_DEBUG
        printf("OEM >> ");
        const oem_binary_response* r = message;
        for (unsigned i = 0; i < header->messageLength - sizeof(r->responseId); i++)
            putc(r->response[i], stdout);
        printf("\n");
#endif

    /**
     * Store the Response ID first. We might want to know what this is even if
     * the CRC is garbage anyway.
     */
    ctx->respId = response->responseId;

    /**
     * CRC verification.
     */
    atSiteCrc = OEM_CalculateBlockCRC32(message,
                                        sizeof(*header) 
                                          + header->messageLength);

    /**
     * Pass the validation if the CRC was not received at all.
     * (The response is still worth reading)
     */
    if (!ctx->crcReadSkipped && ctx->msgCrc != atSiteCrc) {
        DebugError("response: crc verification failed for MID %d:"
                    "expected %08X, got %08X\n",
                    header->messageID,
                    atSiteCrc,
                    ctx->msgCrc);
        ret = OEM_ERR_CRC;
        goto early_return_mproc;
    }

    ret = OEM_OK;

early_return_mproc:
    ctx->taskLevel = TASK_RESPONSE;
    return ret;
}


int OEM_Task_ReadTaskSingleRun(int portIndex,
                               oem_task_context_t* ctx)
{
    const oem_binary_header_t* header = &readTaskMsgBuf.header;
    static oem_task_context_t ctxLocal;
    static uint8_t lastReadToken = 0;
    size_t bodySize;
    int ret;

    if (portIndex < 0 || portIndex > OEM_PHYSICAL_PORTS)
        return OEM_ERR_IO_PORT_INDEX;

    if (!ctx)
        ctx = &ctxLocal;

    memset(ctx, 0, sizeof(*ctx));

    while (1) {
        /**
         * Search for the sync, byte-by-byte.
         * Start from the last fetched token so that we don't end up ignoring
         * sequences such as AA AA 44 12.
         */
        if (lastReadToken != OEM_SYNC_BYTE1) {
            // if (lastReadToken != OEM_SYNC_BYTE3)
            OEM_IO_PortRead(portIndex, &lastReadToken, 1, OEM_SERIAL_READ_TIMEOUT);
        }
        else if (
            OEM_IO_PortRead(portIndex, &lastReadToken, 1, 100) == 0 &&
            lastReadToken == OEM_SYNC_BYTE2                 &&
            OEM_IO_PortRead(portIndex, &lastReadToken, 1, 100) == 0 &&
            lastReadToken == OEM_SYNC_BYTE3)
                break;
        // else
        //     DebugError("\t\t!!!Sync search failed: lt 0x%02X\n", lastReadToken);
    }

    /**
     * Sync found! Now read the header first. We are three bytes 
     * ahead since the sync word is a part of the header as well.
     */
    if (OEM_IO_PortRead(portIndex,
                        readTaskMsgBuf.buf + 3,
                        sizeof(*header) - 3,
                        100)
        != 0) {
        DebugError("A valid sync was found but could not read the header\n");
        ret = OEM_ERR_READ_HEADER;
        goto early_return_task;
    }
    /**
     * Check the header length.
     */
    DebugInfo("New message header retrieved: rx status %08X\n", header->receiverStatus);
    if (header->headerLength != sizeof(*header)) {
        DebugError("header size screwed\n");
        ret = OEM_ERR_LEN_HDR;
        goto early_return_task;
    }

    /**
     * We have a full header. Mark the Message ID in the handling context.
     */
    ctx->mid = header->messageID;

    /**
     * Check if the whole message length exceeds the buffer size.
     * If so, truncate and mark that the CRC verification cannot be done.
     */
    if (sizeof(*header) + header->messageLength > OEM_TASK_MSG_BUF_SIZE) {
        bodySize = OEM_TASK_MSG_BUF_SIZE - sizeof(*header);
        ctx->crcReadSkipped = true;
    }
    else
        bodySize = header->messageLength;

    /**
     * Read the message body. Some log bodies do not follow the header
     * immediately (e.g., LOGLIST). 1 second is a safe measure.
     */
    ret = OEM_IO_PortRead(portIndex,
                          readTaskMsgBuf.buf + sizeof(*header),
                          bodySize,
                          1000);
    if (ret != OEM_OK) {
        DebugError("message body read error: MID %d (mlen %d). Returned %d\n",
                    header->messageID,
                    header->messageLength,
                    ret);
        ret = OEM_ERR_READ_BODY;
        goto early_return_task;
    }

    /**
     * Read the trailing CRC.
     */
    if (ctx->crcReadSkipped == false) {
        ret = OEM_IO_PortRead(portIndex,
                              &ctx->msgCrc,
                              sizeof(oem_crc),
                              100);
        if (ret != OEM_OK) {
            /**
             * OEM7s occasionally fail to report the CRC trailer. We don't handle
             * this as an error, but instead mark this on the context to allow
             * certain handlers to process the message at the risk of bypassing
             * the validation. This behavior can be enabled by calling
             * OEM_Log_DisableCsVerification().
             */
            DebugError("CRC read error: MID %d. Returned %d\n",
                        header->messageID, ret);
            ctx->msgCrc = 0;
            ctx->crcReadSkipped = true;
        }
    }

    /**
     * Prepend the sync bytes and call for the handler.
     */
    readTaskMsgBuf.buf[0] = OEM_SYNC_BYTE1;
    readTaskMsgBuf.buf[1] = OEM_SYNC_BYTE2;
    readTaskMsgBuf.buf[2] = OEM_SYNC_BYTE3;
    return ProcessMessage(readTaskMsgBuf.buf, ctx);

early_return_task:
    ctx->taskLevel = TASK_MAIN;
    return ret;
}
