/************************************************************************
 * Mission override for cFE ES interface configuration.
 ************************************************************************/

#ifndef BEE1012_CFE_ES_INTERFACE_CFG_H
#define BEE1012_CFE_ES_INTERFACE_CFG_H

#include "../cfe/modules/es/config/default_cfe_es_interface_cfg.h"

#undef CFE_MISSION_ES_MAX_APPLICATIONS
#define CFE_MISSION_ES_MAX_APPLICATIONS 32

#endif /* BEE1012_CFE_ES_INTERFACE_CFG_H */
