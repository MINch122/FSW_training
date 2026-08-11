
#ifndef THRUST_HAL_H
#define THRUST_HAL_H

#include <stdint.h>

#define THRUST_HAL_OK      0
#define THRUST_HAL_ERROR  -1

/**
 * @brief 추력기 통신 포트 초기화 (Baud: 921600, 8-N-1)
 * * @param device 장치 경로 (예: "/dev/ttyS1")
 * @return int32_t 성공 시 0, 실패 시 -1
 * @note 담당자 가이드: 이 함수는 app_init에서 호출되어야 함.
 */
int32_t Thrust_HAL_Init(const char* device);

/**
 * @brief 추력기로 데이터 패킷 전송
 * * @param data 전송할 데이터 구조체의 주소 (uint8_t* 로 캐스팅하여 전달)
 * @param len 전송할 총 바이트 수
 * @return int32_t 성공 시 0, 실패 시 -1
 */
int32_t Thrust_HAL_Write(const uint8_t* data, uint32_t len);

/**
 * @brief 추력기로부터 응답 데이터 수신
 * * @param buffer 데이터를 담을 버퍼의 주소
 * @param max_len 버퍼의 최대 크기 (Buffer Overflow 방지용)
 * @return int32_t 실제로 읽은 바이트 수 (실패 시 -1, 데이터 없을 시 0)
 */
int32_t Thrust_HAL_Read(uint8_t* buffer, uint32_t target_len, uint32_t timeout_ms);

/**
 * @brief 통신 포트 닫기 (앱 종료 시 호출)
 */
void Thrust_HAL_Close(void);

#endif /* THRUST_HAL_H */