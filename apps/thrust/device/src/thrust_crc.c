/**
 * @file thrust_crc.c
 * @brief iG4U CRC-16 계산 구현
 *
 * ICD Section 4.1에 제시된 알고리즘과 동일
 * Polynomial : 0x1021
 * Initial    : 0xFFFF
 */

#include "thrust_crc.h"

uint16_t THRUST_CalcCRC16(const uint8_t *data, uint32_t length)
{
    uint16_t crc = THRUST_CRC16_INIT;

    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)((uint16_t)data[i] << 8);
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (uint16_t)((crc << 1) ^ THRUST_CRC16_POLY);
            }
            else
            {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}
