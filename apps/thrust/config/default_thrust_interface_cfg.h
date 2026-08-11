#ifndef THRUST_INTERFACE_CFG_H
#define THRUST_INTERFACE_CFG_H

/**
 * @file
 * THRUST App Public Interface Configuration
 *
 * 외부 앱(SCH, TO 등)이 볼 수 있는 공개 설정값.
 * 내부 전용 설정은 thrust_internal_cfg.h 에 있다.
 */

/* SB 파이프 이름 (TO, SCH 등 외부에서 참조 가능) */
#define THRUST_PIPE_NAME "THRUST_CMD_PIPE"

#endif /* THRUST_INTERFACE_CFG_H */
