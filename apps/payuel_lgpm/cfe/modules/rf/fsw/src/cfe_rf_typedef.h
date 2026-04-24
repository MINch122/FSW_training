#ifndef CFE_RF_TYPEDEF_H
#define CFE_RF_TYPEDEF_H

#include "cfe.h"

#include <csp/csp.h>

#define CSP_TIMEOUT(x)              (x)*1000
#define CI_TASK_STACK_SIZE(x)       (x)*4096
#define CI_TASK_PRIORITY(x)         (((x) < 0) ? 0 : ((x) > 255) ? 255 : (x))

#endif /* CFE_RF_TYPEDEF_H */