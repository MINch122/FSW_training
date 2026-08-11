#ifndef THRUST_MISSION_CFG_H
#define THRUST_MISSION_CFG_H

#include <stdbool.h>

/**
 * @file
 * THRUST App Mission Configuration
 *
 * 미션 레벨 공개 설정. thrust_interface_cfg.h 를 포함한다.
 * 미션별 오버라이드가 필요한 경우 이 파일을 교체한다.
 */
#include "thrust_interface_cfg.h"

/* true: print THRUST TX/RX HEX and parsed RX data with OS_printf(). */
#ifndef THRUST_DEBUG_OS_PRINT
#define THRUST_DEBUG_OS_PRINT true
#endif

#endif /* THRUST_MISSION_CFG_H */
