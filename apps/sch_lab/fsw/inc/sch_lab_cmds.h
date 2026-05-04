#ifndef SCH_LAB_CMDS_H
#define SCH_LAB_CMDS_H

#include "sch_lab_msgstruct.h"

void SCH_LAB_NoopCmd(const SCH_LAB_NoopCmd_t *Cmd);
void SCH_LAB_ResetCountersCmd(const SCH_LAB_ResetCountersCmd_t *Cmd);
void SCH_LAB_SetEntryStateCmd(const SCH_LAB_SetEntryStateCmd_t *Cmd);
void SCH_LAB_AddEntryCmd(const SCH_LAB_AddEntryCmd_t *Cmd);
void SCH_LAB_DeleteEntryCmd(const SCH_LAB_DeleteEntryCmd_t *Cmd);

#endif
