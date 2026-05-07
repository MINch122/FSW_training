/************************************************************************
 * @file meow.h
 *
 *         .-.   .-.         MIMAN flies higher, upon its Linux machine,
 *        /   \ /   \        Minor bugs are quiet, no trouble foreseen,
 *    .-. | E  |  O | .-.
 *   /   \ \  / \  / /   \   Then came the day no SD would mount,
 *   | M |  '`.-.`'  | W |   No disk, no drive, no hope left on ground,
 *    \_.' .-`   `-. '._/
 *      .-' MIMAN   '-.      Yet we would not despair, nor would we drown,
 *     /    Emergency  \     We fought and fought, till the bug was shut down,
 *     |    Operations |
 *      \   Wrapper   /      There be a prize for all the days we tried,
 *       '.___...___.'       We rest with pride; a hug to a cat will suffice.
 *
 *
 * 2023 Astrodynamics & Control Lab. Yonsei Univ. ryu@yonsei.ac.kr.
 *
 * Migrated from the original "meow_app.h" with a *fancy* new core API design.
 *
 ************************************************************************/
#ifndef MEOW_H
#define MEOW_H

#include "cfe.h"
#include "cfe_config.h"

#include "meow_mission_cfg.h"
#include "meow_platform_cfg.h"
#include "meow_perfids.h"
#include "meow_msgids.h"
#include "meow_msg.h"

typedef struct {
    uint16 CmdCounter;
    uint16 ErrCounter;

    MEOW_HkTlm_t   HkTlm;
    MEOW_Report_t  Report;

    uint32 RunStatus;

    CFE_SB_PipeId_t CommandPipe;
    char            PipeName[CFE_MISSION_MAX_API_LEN];
    uint16          PipeDepth;
} MEOW_AppData_t;

extern MEOW_AppData_t MEOW_AppData;

void         MEOW_Main(void);
CFE_Status_t MEOW_Init(void);

#endif /* MEOW_H */
