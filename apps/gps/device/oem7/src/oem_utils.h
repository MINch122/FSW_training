/**
 * @file oem_utils.h
 * @brief Debug output & CRC utilities.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _OEM_UTILS_H_
#define _OEM_UTILS_H_

#include "oem_types.h"


/* ════════════════════════════════════════════════════════════════════════
 *  Linked list utilities (log callback container)
 * ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Opaque list object.
 */
typedef struct oem_list_s oem_list_t;

/**
 * @brief Create an empty list.
 * @return Pointer to the list, or NULL on calloc() failure.
 */
oem_list_t* oem_list_create(void);

/**
 * @brief Free the list and all its nodes.
 * @param list Pointer to the list.
 */
void oem_list_free(oem_list_t* list);

/**
 * @brief Append a node containing @a data pointer to the back of @a list.
 * @param list Pointer to the list.
 * @param data Pointer to the data to append.
 * @return OEM_OK, OEM_ERR_NULL (@a list is null), or OEM_ERR_NOMEM.
 */
int oem_list_add_back(oem_list_t* list, void* data);

/**
 * @brief Return the number of nodes in @a list.
 * @param list Pointer to the list.
 * @return Number of nodes, or 0 if @a list is null.
 */
int oem_list_nodes(const oem_list_t* list);

/**
 * @brief Move the iteration cursor to the head of the list.
 * @param list Pointer to the list.
 * @return true if successful, false if @a list is null or empty.
 */
bool oem_list_tohead(oem_list_t* list);

/**
 * @brief Move the iteration cursor to the next node. The cursor must be
 *        first initialized by oem_list_tohead() before calling this.
 * @param list Pointer to the list.
 * @return true if successful, false if the cursor is at the end, @a list is
 *         null/empty or oem_list_tohead() was never called before.
 */
bool oem_list_tonext(oem_list_t* list);

/**
 * @brief Return the data pointer of the node at the current cursor.
 * @param list Pointer to the list.
 * @return Data pointer, or NULL if @a list is null, empty, or
 *         if oem_list_tohead() was never called before.
 */
void* oem_list_getdata(const oem_list_t* list);


/* ════════════════════════════════════════════════════════════════════════
 *  CRC32
 * ════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Return CRC32 checksum.
 * @param data    Data.
 * @param length  Byte-size.
 * @return 32-bit CRC. 
 */
oem_crc oem_crc32(const void* data, size_t length);



/* ════════════════════════════════════════════════════════════════════════
 *  Debug output (only when OEM_DEBUG is defined)
 * ════════════════════════════════════════════════════════════════════════ */

  #if OEM_DEBUG

    typedef enum {
        LL_NORMAL   = 0,
        LL_ERROR    = 1,
        LL_WARN     = 2,
        LL_INFO     = 3,
        LL_MAX, /* Placeholder. */
    } log_level_t;

    void oem_debug(const char* caller, log_level_t level, const char* str, ...);
    void oem_debug_hexdump(log_level_t level, const void *data, size_t len, bool c);
    #define oem_debug_error(...)    oem_debug(__func__, LL_ERROR, ##__VA_ARGS__)
    #define oem_debug_warning(...)  oem_debug(__func__, LL_WARN, ##__VA_ARGS__)
    #define oem_debug_info(...)     oem_debug(__func__, LL_INFO, ##__VA_ARGS__)
    #define oem_debug_normal(...)         oem_debug(__func__, LL_NORMAL, ##__VA_ARGS__)

  #else /* if OEM_DEBUG */

    #define oem_debug_error(...)    do {} while (0)
    #define oem_debug_warning(...)  do {} while (0)
    #define oem_debug_info(...)     do {} while (0)
    #define oem_debug_normal(...)         do {} while (0)

  #endif /* if OEM_DEBUG */

#endif /* ifndef _OEM_UTILS_H_ */
