#ifndef LTRX_MISSION_CFG_H
#define LTRX_MISSION_CFG_H

#include "ltrx_interface_cfg.h"

#ifndef LTRX_DEBUG
#define LTRX_DEBUG false
#endif

#ifndef DEBUG_LTRX
#define DEBUG_LTRX LTRX_DEBUG
#endif

#if DEBUG_LTRX
#define LTRX_APP_printf(...) OS_printf(__VA_ARGS__)
#else
#define LTRX_APP_printf(...) do { } while (0)
#endif

#endif /* LTRX_MISSION_CFG_H */
