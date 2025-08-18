/**
 * @file p31u_device_config.h
 * @brief Compile time device settings for the GomSpace NanoPower P31u Power
 *        System driver.
 * Astrodynamics & Control Lab. 2024. ryu@yonsei.ac.kr
 */
#ifndef _GOMSPACE_P31U_DEVICE_CONFIG_H_
#define _GOMSPACE_P31U_DEVICE_CONFIG_H_


/**
 * Converts the byte-order of all incoming data from the device.
 * P31u operates in the big endian format. The target system can be treated
 * as if it is in little endian if this is set True.
 */
#define P31U_USE_NTOH                   1

/**
 * Converts the byte-order of all outgoing commands and data to the device.
 * P31u operates in the big endian format. The target system can be treated
 * as if it is in little endian if this is set True.
 */
#define P31U_USE_HTON                   1


#endif
