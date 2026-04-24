#ifndef CFE_SRL_DISPATCH_H
#define CFE_SRL_DISPATCH_H


/*---------------------------------------------------------------------------------------*/
/**
 * Processes a single message buffer that has been received from the command pipe
 *
 * @param SBBufPtr Software bus buffer pointer
 */
void CFE_SRL_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* CFE_SRL_DISPATCH_H */