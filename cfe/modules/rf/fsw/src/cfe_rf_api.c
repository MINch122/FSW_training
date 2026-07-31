/************************************************************************
 * Author : Hyeokjin Kweon
 * 
 * Last Modified : 2025 - 05 - 27
 * 
 * Brief : RF Comm. Core Module's API function 
 *         Used several App require RF function (CI, TO)
 ************************************************************************/

#include "cfe_rf_typedef.h"
#include "cfe_rf_extern_typedefs.h"
#include "cfe_rf_msgids.h"
#include "cfe_msg.h"
#include "cfe_sb.h"
#include "rpt_msgids.h"
#include "osapi.h"
/**
 * Global data
 */
static csp_socket_t *Socket = NULL;

/* Forward Declaration */
void CFE_RF_CommandIngestTask(void);

/**
 * @deprecated Not used
 */
void CFE_RF_Cleanup(void);


/***********************************************************
 * RF CI Init
 * This function initialize RF configuration for CSP,
 * And Create child task waiting TC.
 **********************************************************/
int32 CFE_RF_CommandIngestInit(CFE_ES_TaskId_t *TaskIdPtr) {
    int Status;

    /* First, cleanup the existing resources, if exist */
    /* These function do NOT care the resource is exist or not */
    if(Socket) csp_close(Socket);
    /* csp_unbind does not exist in libcsp API */

    /* Then, start the RF ingest init */
    Socket = csp_socket(CSP_O_NONE);
    if (Socket == NULL) {
        CFE_ES_WriteToSysLog("%s: csp_socket failed! NO RC\n", __func__);
        return -1; // Revise to `csp_socket failed`
    }

    Status = csp_bind(Socket, CFE_RF_UPORT_PING);
    if (Status != CSP_ERR_NONE && Status != CSP_ERR_USED) {
        CFE_ES_WriteToSysLog("%s: csp_bind failed at Port: %d RC=%d\n", __func__, CFE_RF_UPORT_PING, Status);
        return Status;
    }

    Status = csp_bind(Socket, CFE_RF_UPORT_TC);
    if (Status != CSP_ERR_NONE && Status != CSP_ERR_USED) {
        CFE_ES_WriteToSysLog("%s: csp_bind failed at Port: %d RC=%d\n", __func__, CFE_RF_UPORT_TC, Status);
        return Status;
    }

    Status = csp_listen(Socket, 10);
    if (Status != CSP_ERR_NONE) {
        CFE_ES_WriteToSysLog("%s: csp_listen failed! RC=%d\n", __func__, Status);
        return Status;
    }

    Status = CFE_ES_CreateChildTask(TaskIdPtr, "CI_TASK", CFE_RF_CommandIngestTask, CFE_ES_TASK_STACK_ALLOCATE, CI_TASK_STACK_SIZE(2), CI_TASK_PRIORITY(100), 0);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("%s: Create Ingest task failed. RC = 0x%08X\n", __func__, Status);
        return Status;
    }

    CFE_ES_WriteToSysLog("%s: CFE RF Init Successfully done.\n", __func__);

    return CFE_SUCCESS;
}


/***********************************
 * RF CI child task
 ***********************************/
void CFE_RF_CommandIngestTask(void) {
    int32 Status;
    csp_conn_t *Connection = NULL;
    csp_packet_t *Packet = NULL;

    CFE_RF_ContactTimeTlm_t Tlm = {0,};

    CFE_MSG_Init(CFE_MSG_PTR(Tlm.TelemetryHeader), CFE_SB_ValueToMsgId(CFE_RF_TLM_MID), sizeof(Tlm));

    for (;;) {
        Connection = csp_accept(Socket, CSP_MAX_TIMEOUT);
        if (Connection == NULL) {
            // CFE_ES_WriteToSysLog("%s: No incoming connection! NO RC\n", __func__);
            continue;
        }
        while ((Packet = csp_read(Connection, CSP_TIMEOUT(1))) != NULL) {
            int Port = csp_conn_dport(Connection);
            switch (Port) {
                case CFE_RF_UPORT_PING: {
                    csp_service_handler(Connection, Packet);
                    Packet = NULL;
                    break;
                }
                case CFE_RF_UPORT_TC: {
                    /**
                     * If TC received, Transmit the SB Message
                     * This Time Stamp will be used as Last contact time
                     */
                    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Tlm.TelemetryHeader));
                    CFE_SB_TransmitMsg(CFE_MSG_PTR(Tlm.TelemetryHeader), true);

                    CFE_SB_Buffer_t *BufPtr = NULL;

                    /**
                     * Allocate & Transmit Message buffer
                     */
                    BufPtr = CFE_SB_AllocateMessageBuffer(Packet->length);
                    if (BufPtr != NULL) {
                        memcpy(BufPtr, Packet->data, Packet->length);

                        /**
                         * Zero-copy transmit function
                         */
                        Status = CFE_SB_TransmitBuffer(BufPtr, false);

                        /**
                         * If failed, Release buffer and Transmit again via other function
                         */
                        if (Status != CFE_SUCCESS) {
                            CFE_SB_ReleaseMessageBuffer(BufPtr);
                        }
                    }
                    else {
                        CFE_ES_WriteToSysLog("%s: Allocate message buffer failed, len=%u\n",
                                             __func__, (unsigned int)Packet->length);
                        Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                    }

                    if (Status != CFE_SUCCESS) {
                        
                        /**
                         * Alternative Transmit function
                         */
                        Status = CFE_SB_TransmitMsg((CFE_MSG_Message_t *)Packet->data, false);
                        if (Status != CFE_SUCCESS) {
                            CFE_ES_WriteToSysLog("%s: Transmit message failed! RC=%d\n", __func__, Status);
                        }
                    }
                    /* Free buffer & Remove dangled pointer */
                    csp_buffer_free(Packet);
                    Packet = NULL;
                    break;
                }

                default:
                    CFE_ES_WriteToSysLog("%s: Unknown RF uplink port: %d\n", __func__, Port);
                    csp_buffer_free(Packet);
                    Packet = NULL;
                    break;
            }
        }
        csp_close(Connection);
    } /* End of loop */

    /* This area should never be reached */
    CFE_EVS_SendCrit(626, "CRITICAL: CI Task Finished. Should be restarted.");
    
}

void CFE_RF_Cleanup(void) {
    int32 status = csp_close(Socket);
    /* csp_unbind does not exist in libcsp API */
    OS_printf("%s: close status: %d\n", __func__, status);
}

/*********************************************
 * RF TO function
 * Maybe just `csp_transaction` with GS
 * Refer `cfe_rf_typedef.h` to find the Port
 *********************************************/
int32 CFE_RF_TelemetryEmit(void *BufPtr, size_t Size, uint8_t Port) {
    int32 Status;
    uint16_t TotSendByte = 0;
    uint16_t SendByte = 0;
    uint16_t MaxMtu = RF_MAX_MTU;

    if (BufPtr != NULL) {
        CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
        if (CFE_MSG_GetMsgId((CFE_MSG_Message_t *)BufPtr, &MsgId) == CFE_SUCCESS) {
            CFE_SB_MsgId_Atom_t MsgIdValue = CFE_SB_MsgIdToValue(MsgId);
            if (MsgIdValue == (CFE_SB_MsgId_Atom_t)RPT_REPORT_TLM_MID || MsgIdValue == (CFE_SB_MsgId_Atom_t)RPT_CRITICAL_TLM_MID) {
                MaxMtu = RF_RPT_MAX_MTU;
            }
        }
    }

    while (TotSendByte < Size) {
        SendByte = (Size - TotSendByte > MaxMtu) ? MaxMtu : (Size - TotSendByte);
        Status = CFE_SRL_ApiTransactionCSP(CSP_NODE_GS_KISS, Port, (void *)(((uint8_t *)BufPtr) + TotSendByte), SendByte, NULL, 0);
        if (Status <= 0) {
            return Status;
        }

        TotSendByte += SendByte;
    }
    
    return CFE_SUCCESS;
}
