/**
 * @file oem_io.h
 * @brief OEM7 I/O interface layer.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _OEM_IO_H_
#define _OEM_IO_H_


#include "oem_config.h"
#include "oem_types.h"


/**
 * @brief Interface write callback function.
 * 
 * @details
 *        - Called by the I/O layer when commands to OEM receiver are sent.
 *        - Shall return the actual number of bytes written, or a negative error code.
 *        - Shall return a negative error code *only if an actual I/O error occurred.*
 *          Partial writes (written < size) shall not be considered an error. 
 * 
 * @param iface_idx  Interface index to which this callback is registered via
 *                   oem_io_init_interface().
 * @param data  Data to send.
 * @param size  Size of @a data.
 * @return Actually written number of bytes or negative on error. Using
 *         an appropriate oem_ret_t code is recommended.
 */
typedef int (*oem_io_write_t) (int iface_idx,
                               const void* data,
                               size_t size);

/**
 * @brief Interface read callback function.
 * 
 * @details
 *        - Called by the I/O layer when reading data from OEM receiver.
 *        - This read function shall not block for more than @a timeout ms.
 *        - A partial read (fewer than @a size bytes) is not considered
 *          an error.
 *        - Shall return a negative error code only if either
 *            1) an actual I/O error occurred or
 *            2) no bytes were read before the timeout expired, in which case
 *               OEM_ERR_IO_TIMEOUT shall be returned.
 * 
 * @param iface_idx  Interface index to which this callback is registered via
 *                   oem_io_init_interface().
 * @param[out] data  Data buffer.
 * @param size  Size of data to read.
 * @param timeout Millisecond timeout.
 * 
 * @return Actually read number of bytes or negative on error. Using
 *         an appropriate oem_ret_t code is recommended.
 */
typedef int (*oem_io_read_t) (int iface_idx,
                              void* data,
                              size_t size,
                              uint16_t timeout);

/**
 * @brief Attach read/write callbacks to an I/O interface and initialize the
 *        internal buffer.
 * 
 * @details
 *      - This does not initialize the physical I/O port (e.g., serial, socket)
 *           itself. The user must open and configure the interface separately.
 *      - The internal buffer is used for chunk read of incoming data from the
 *           receiver, populated by oem_io_fill() and accessed by oem_io_peek().
 * 
 * @param iface_idx Interface index to which the callbacks are attached.
 * @param read_buf_size Size of the internal read buffer. If zero, the default 
 *                      size of OEM_IO_DEFAULT_READBUF_SIZE is used.
 * @param write_callback Write callback function.
 * @param read_callback  Read callback function.
 * @return OEM_OK: Successful.
 *         OEM_ERR_IO_IFACE_INDEX: Invalid @a iface_idx.
 *         OEM_ERR_NULL: @a write_callback or @a read_callback is null.
 *         OEM_ERR_EXISTS: The interface is already initialized.
 *         OEM_ERR_NOMEM: Failed to allocate the internal buffer.
 */
int oem_io_init_interface(int iface_idx,
                          size_t read_buf_size,
                          oem_io_write_t write_callback,
                          oem_io_read_t read_callback);

/**
 * @brief Free an I/O interface and its internal buffer. The freed interface
 *        index can be reused for another interface initialization.
 * 
 * @param iface_idx  Interface index to free.
 * @return OEM_OK: Successful, or the interface is already uninitialized.
 *         OEM_ERR_IO_IFACE_INDEX: Invalid @a iface_idx.
 */
int oem_io_delete_interface(int iface_idx);

/**
 * @brief Writes data to the I/O interface by calling the write callback 
 *        attached to the interface.
 * 
 * @param iface_idx  Interface index to which the write callback is attached.
 * @param data  Data to write.
 * @param size  Size of data to write.
 * @return OEM_OK: Successful.
 *         OEM_ERR_IO_IFACE_INDEX: Invalid @a iface_idx.
 *         OEM_ERR_NULL: @a data is null or @a size is zero.
 *         OEM_ERR_IO_IFACE_UNSET: The interface is not initialized.
 *         OEM_ERR_IO_WRITE_PARTIAL: Short write occurred (written < size).
 *         else, the negative error code returned by the write callback.
 */
int oem_io_write(int iface_idx,
                 const void* data,
                 size_t size);

/**
 * @brief Return the number of bytes currently available in the I/O interface buffer.
 * 
 * @param iface_idx Interface index to query.
 * @return size_t Number of bytes available in the buffer.
 *          0 if the interface is not initialized or an invalid index is given.
 */
size_t oem_io_available(int iface_idx);

/**
 * @brief Return the pointer and size of the currently available data in the I/O interface buffer.
 * 
 * @details
 *       - The returned data pointer is valid until
 *             1) the next call to oem_io_consume(),
 *             2) the next call to oem_io_fill(), or
 *             3) oem_io_delete_interface() for the same interface.
 *       - The data is _NOT_ consumed. This provides simply a zero-copy access to the
 *             internal buffer.
 *       - The caller shall not modify the data in the buffer unless intended.
 *       - @a size is null-allowed. If null, the available size must be obtained by
 *             oem_io_available() instead.
 * 
 * @param iface_idx Interface index to query.
 * @param data Pointer to store the address of the available data.
 * @param size Pointer to store the size of the available data.
 * @return OEM_OK: Successful.
 *         OEM_ERR_IO_IFACE_INDEX: Invalid @a iface_idx.
 *         OEM_ERR_NULL: @a data is null.
 *         OEM_ERR_IO_IFACE_UNSET: The interface is not initialized.
 */
int oem_io_peek(int iface_idx,
                const void** data,
                size_t* size);

/**
 * @brief Advance the read cursor for the interface by @a size bytes.
 * 
 * @details
 *        - Commits a previously peeked data as consumed.
 *        - Pairs with oem_io_peek() to provide a zero-copy access to the internal buffer.
 *        - Does not block. @a size == 0 is considered a no-op and returns OEM_OK.
 *        - Size must be no greater than the currently available data size, returned from
 *          oem_io_peek() or oem_io_available().
 * 
 * @param iface_idx Interface index to consume data from.
 * @param size Number of bytes to consume.
 * @return OEM_OK: Successful.
 *         OEM_ERR_IO_IFACE_INDEX: Invalid @a iface_idx.
 *         OEM_ERR_IO_IFACE_UNSET: The interface is not initialized.
 *         OEM_ERR_RANGE: @a size is greater than the available data.
 */
int oem_io_consume(int iface_idx,
                   size_t size);

/**
 * @brief Fill the I/O interface buffer by calling the read callback attached to the interface.
 * 
 * @details
 *       - Calls the read callback to fill the internal buffer with new data from the OEM receiver.
 *       - The call synopsis is "read_callback(iface_idx, buffer + have, unfilled, timeout)", i.e.,
 *             tries to fill the buffer, but may return with fewer bytes read.
 *       - Invalidates any pointer previously returned by oem_io_peek() for the same interface.
 * 
 * @param iface_idx Interface index to fill.
 * @param timeout Timeout for the read operation in milliseconds.
 * @return OEM_OK: Successful.
 *         OEM_ERR_IO_IFACE_INDEX: Invalid @a iface_idx.
 *         OEM_ERR_IO_IFACE_UNSET: The interface is not initialized.
 *         OEM_ERR_FULL: The buffer is already full.
 *         else, the negative error code returned by the read callback.
 */
int oem_io_fill(int iface_idx,
                uint16_t timeout);

#endif
