#ifndef THRUST_INTERNAL_CFG_H
#define THRUST_INTERNAL_CFG_H

/**
 * @file
 * THRUST App Internal Configuration
 *
 * 이 앱 내부에서만 사용하는 설정.
 * 외부 앱 인터페이스에 영향을 주지 않으므로 자유롭게 변경 가능.
 */

/* SB 파이프 깊이 */
#define THRUST_PIPE_DEPTH  16

/* RS-422 UART 설정 (ICD 3.2절) */
#define THRUST_UART_DEVICE "/dev/pts/3"   /* OBC의 RS-422 포트 */
#define THRUST_UART_BAUD_RATE   921600    /* ICD 3.2: 921600 bps */
#define THRUST_UART_TIMEOUT_SEC 1         /* 수신 타임아웃 (초) */

#endif /* THRUST_INTERNAL_CFG_H */
