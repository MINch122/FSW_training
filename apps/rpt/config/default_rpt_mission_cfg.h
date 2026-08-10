#ifndef RPT_MISSION_CFG_H
#define RPT_MISSION_CFG_H

#include "rpt_interface_cfg.h"
#include "rpt_internal_cfg.h"

#ifndef DEBUG_RPT
#define DEBUG_RPT false
#endif

#if DEBUG_RPT
#define RPT_APP_printf(...) OS_printf(__VA_ARGS__)
#else
#define RPT_APP_printf(...) do { } while (0)
#endif

#endif
