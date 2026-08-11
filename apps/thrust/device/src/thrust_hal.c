/**
 * @file thrust_hal.c
 * @brief iG4U RS-422 송수신 구현
 *
 * [ICD Section 3.2 확정 사항]
 *   Baud: 921,600 bps (TBD), 8N1, LSB First
 *
 * [TBD]
 *   CFE_SRL API 사용 방법은 OBC 플랫폼 확정 후 아래 TBD 부분을 채울 것
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    // 표준 시스템 콜 (read, write, close) [cite: 20]
#include <fcntl.h>     // 파일 제어 (O_RDWR 등)
#include <termios.h>   // 시리얼 통신 설정 핵심 헤더 [cite: 20, 53]
#include <time.h>      // 시간 측정을 위한 헤더
#include "osapi.h"
#include "thrust_hal.h"

static int thrust_fd = THRUST_HAL_ERROR;  // 아직 포트가 열리지 않았음을 명시

/**
 * @brief RS-422 포트 초기화 (921,600 bps, 8-N-1, LSB First)
 * 이 함수는 app_init에서 호출되어야 함.
 * device에는 경로가 들어가야 함 ex  "/dev/ttyS1"
 */


int32_t Thrust_HAL_Init(const char* device) {
    struct termios options;

    // 1. 장치 열기 (리눅스 표준 open)
    thrust_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    // O_RDWR: 양방향 송수신 설정
    if (thrust_fd == THRUST_HAL_ERROR) // 장치를 여는데 실패한 경우
    {
        OS_printf("THRUST HAL: Failed to open %s\n", device);
        return THRUST_HAL_ERROR;
    }

    // 2. 현재 설정 가져오기
    // 해당 포트에서 데이터를 가져온다
    if (tcgetattr(thrust_fd, &options) != 0)
    {
        OS_printf("THRUST HAL: Failed to read termios from %s\n", device);
        close(thrust_fd);
        thrust_fd = THRUST_HAL_ERROR;
        return THRUST_HAL_ERROR;
    }

    // 3. 보드레이트 설정: 921,600 bps [cite: 54]
    // 입출력 속도 수정
    if (cfsetispeed(&options, B921600) != 0 || cfsetospeed(&options, B921600) != 0)
    {
        OS_printf("THRUST HAL: Failed to set baud rate on %s\n", device);
        close(thrust_fd);
        thrust_fd = THRUST_HAL_ERROR;
        return THRUST_HAL_ERROR;
    }

    // 4. 제어 설정 (8-N-1, LSB First 기반) [cite: 55, 56, 57, 58]
    options.c_cflag |= (CLOCAL | CREAD); // 수신 가능 모드
    options.c_cflag &= ~PARENB;          // Parity: No [cite: 56]
    options.c_cflag &= ~CSTOPB;          // Stop Bits: 1 [cite: 57]
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;              // Data Bits: 8 [cite: 55]

    // 5. 로우 모드 설정 (Raw Mode: 데이터 가공 없이 그대로 송수신)
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    // 6. 설정 적용
    if (tcsetattr(thrust_fd, TCSANOW, &options) != 0)
    {
        OS_printf("THRUST HAL: Failed to apply serial settings to %s\n", device);
        close(thrust_fd);
        thrust_fd = THRUST_HAL_ERROR;
        return THRUST_HAL_ERROR;
    }

    OS_printf("THRUST HAL: Serial init success (921600bps)\n");
    return THRUST_HAL_OK;
}

/**
 * @brief 패킷 전송 (리눅스 표준 write 사용)
 */
int32_t Thrust_HAL_Write(const uint8_t* data, uint32_t len) {
    if (thrust_fd < 0) return THRUST_HAL_ERROR;

    // write 함수는 실패 시 -1을 반환
    // data는 메모리의 주소가 담긴 버퍼
    ssize_t sent = write(thrust_fd, data, len);
    // 성공 여부를 판별해서 반환
    return (sent == (ssize_t)len) ? THRUST_HAL_OK : THRUST_HAL_ERROR;
    // device ID 로 나눠서 정의
}

// Thrust_HAL_Write((uint8_t*)&myPacket, sizeof(myPacket));  실제 호출 예시

/**
 * @brief 패킷 수신 (리눅스 표준 read 사용)
 */
int32_t Thrust_HAL_Read(uint8_t* buffer, uint32_t target_len, uint32_t timeout_ms)
{
    struct timespec start, now;
    uint32_t received = 0;

    // 타이머 스타트
    clock_gettime(CLOCK_MONOTONIC, &start);

    // target_len을 다 채울 때까지 계속 반복
    while (received < target_len)
    {
        // 목표로 한 바이트씩 혹은 남은 만큼 읽기 시도
        int32_t res = read(thrust_fd, &buffer[received], target_len - received);

        if (res > 0) {
            received += res;
        }

        // 중간중간 시간 체크
        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec - start.tv_nsec) / 1000000;

        if (elapsed >= (long)timeout_ms) {
            return THRUST_HAL_ERROR; // 시간 초과 시 즉시 실패 반환
        }

        if (res <= 0) usleep(20); // 데이터가 안 오면 잠깐 쉬기
    }
    return (int32_t)received; // target_len만큼 다 읽으면 총 길이 반환
}


void Thrust_HAL_Close(void)
{
    if (thrust_fd >= 0)
    {
        close(thrust_fd);  // 리눅스 표준 시스템 콜: 파일 닫기
        thrust_fd = THRUST_HAL_ERROR;    // 번호표 반납 (다시 초기값으로)
    }
}
