#ifndef MISSION_MISSION_CFG_H
#define MISSION_MISSION_CFG_H

#include "mission_interface_cfg.h"
#include "mission_internal_cfg.h"

#ifndef MISSION_ENABLE_LEOP_SEQUENCE
#define MISSION_ENABLE_LEOP_SEQUENCE false
#endif

/* Send the ADCS detumbling sequence command immediately after loading LEOP state. */
#ifndef MISSION_ENABLE_ADCS_DETUMBLE
#define MISSION_ENABLE_ADCS_DETUMBLE false
#endif

#ifndef DEBUG_MISSION
#define DEBUG_MISSION true
#endif

#if DEBUG_MISSION
#define MISSION_APP_printf(...) OS_printf(__VA_ARGS__)
#else
#define MISSION_APP_printf(...) do { } while (0)
#endif

#endif
