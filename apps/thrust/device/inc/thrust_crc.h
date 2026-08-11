#ifndef THRUST_CRC_H
#define THRUST_CRC_H

/**
 * @file thrust_crc.h
 * @brief iG4U CRC-16 계산 함수 선언
 *
 * ICD Section 4.1 CRC-16 예시 기반
 * Polynomial : 0x1021 (x^16 + x^12 + x^5 + 1)
 * Initial    : 0xFFFF
 */

#include "common_types.h"

#define THRUST_CRC16_POLY     0x1021
#define THRUST_CRC16_INIT     0xFFFF

/**
 * @brief CRC-16 계산
 * @param data   계산 대상 버퍼
 * @param length 버퍼 길이 (bytes)
 * @return 계산된 CRC-16 값
 */
uint16_t THRUST_CalcCRC16(const uint8_t *data, uint32_t length);

#endif /* THRUST_CRC_H */
