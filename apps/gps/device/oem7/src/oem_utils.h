/**
 * @file oem_utils.h
 * @brief Debug output & CRC utilities.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _OEM_UTILS_H_
#define _OEM_UTILS_H_

#include "oem_basetype.h"
#include "mlk_list.h"

#define oem_callbacklist            mlk_list_t
#define oem_list_create()           mlk_list()
#define oem_list_free(list)         mlk_list_free(list)
#define oem_list_add_back(list, cb) mlk_list_add_ptr_back(list, cb)
#define oem_list_tohead(list)       mlk_list_to_head(list)
#define oem_list_nodes(list)        mlk_list_nodes(list)
#define oem_list_tonext(list)       mlk_list_advance(list)
#define oem_list_getdata(list)      mlk_list_node_data(list)



/**
 * @brief               Return CRC32 checksum.
 * 
 * @param length        Byte-size.
 * @param buffer        Data buffer.
 * @return              32-bit CRC. 
 */
uint32_t OEM_CalculateBlockCRC32(const void* data,
                                 size_t length);

/**
 * @brief               Validate the CRC32.
 * 
 * @param expected      Expected CRC.
 * @param msg           Data to be validated.
 * @param length        Data size in bytes.
 * @return              true if the data CRC is equal to the expected. Otherwise false.
 */
static inline bool OEM_VerifyChecksum(oem_crc expected, const uint8_t* msg, oem_ushort length) {
    return OEM_CalculateBlockCRC32(msg, length) == expected ? true : false;
} 


#if OEM_DEBUG

  typedef enum {
      LL_NORMAL   = 0,
      LL_ERROR    = 1,
      LL_WARN     = 2,
      LL_INFO     = 3,
  } log_level;
  int OEM_Debug(const char* caller, log_level level, const char* str, ...);
  #define DebugError(...)    OEM_Debug(__func__, LL_ERROR, ##__VA_ARGS__)
  #define DebugWarning(...)  OEM_Debug(__func__, LL_WARN, ##__VA_ARGS__)
  #define DebugInfo(...)     OEM_Debug(__func__, LL_INFO, ##__VA_ARGS__)
  #define Debug(...)         OEM_Debug(__func__, LL_NORMAL, ##__VA_ARGS__)

#else /* if OEM_DEBUG */

  #define DebugError(...)    do {} while (0)
  #define DebugWarning(...)  do {} while (0)
  #define DebugInfo(...)     do {} while (0)
  #define Debug(...)         do {} while (0)

#endif /* if OEM_DEBUG */

#endif /* ifndef _OEM_UTILS_H_ */
