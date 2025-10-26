/**
 * @file oem_io.c
 * @brief OEM7 physical port I/O layer.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _OEM_IO_H_
#define _OEM_IO_H_


#include "oem_config.h"
#include "oem_basetype.h"


/**
 * @brief Custom port write function.
 * 
 * @param portIndex  Port index to which this callback is attached.
 * @param data  Data to send.
 * @param size  Byte size.
 * @return int. Actually written number of bytes or negative on error.
 *         Using an appropriate oem_ret_t code is recommended.
 */
typedef int (*oem_io_write_t) (int portIndex,
                               const void* data,
                               size_t size);


/**
 * @brief Custom port read function.
 * 
 * @details 1) This read function shall not block for more than @a timeout ms.
 *          2) A partial read (fewer than @a size bytes) is not considered
 *             an error.
 *          3) Shall return a negative error code only if either an actual I/O
 *             error occurred or no bytes were read before the timeout expired.
 * 
 * @param portIndex  Port index to which this callback is attached.
 * @param[out] data  Data buffer.
 * @param size  Byte size.
 * @param timeout Millisecond timeout.
 * 
 * @return int. Actually read number of bytes or negative on error.
 *         Using an appropriate oem_ret_t code is recommended.
 */
typedef int (*oem_io_read_t) (int portIndex,
                              void* data,
                              size_t size,
                              uint16_t timeout);


/**
 * @brief Initializes an empty I/O port.
 *        An I/O port is a physical serial port to the OEM receiver and not to
 *        be confused with the OEM message ports defined in oem_ports_t.
 * 
 *        The portIndex determines which physical port to be used for sending 
 *        or receiving CMD/LOG messages. Actual driver layers shall be imple-
 *        mented in the @a writeFunc and @a readFunc callbacks.
 * 
 * @param portIndex Uninitialized port index. 0 to (OEM_PHYSICAL_PORTS - 1).
 * @param writeFunc Custom write callback.
 * @param readFunc  Custom read callback.
 * @return OEM_OK:            Success.
 *         OEM_ERR_IO_INDEX:  Invalid port index.
 *         OEM_ERR_NULL:      Null @a writeFunc or @a readFunc.
 *         OEM_ERR_EXISTS:    The port has already been initialized.
 *         OEM_ERR_NOMEM:     Port read buffer allocation failed.
 */
int OEM_IO_PortInit(int portIndex,
                    oem_io_write_t writeFunc,
                    oem_io_read_t readFunc);


/**
 * @brief Deletes a previously initialized I/O port. Initialized ports cannot
 *        be initalized again before deleting it.
 *        Returns OEM_OK for uninitialized ports.
 * 
 * @param portIndex 0 to (OEM_PHYSICAL_PORTS - 1).
 * @return OEM_OK:            Success.
 *         OEM_ERR_IO_INDEX:  Invalid port index.
 */
int OEM_IO_PortDelete(int portIndex); 


/**
 * @brief Writes @a data to the I/O port designated by @a portIndex. Internally
 *        calls the oem_io_write_t callback attached by OEM_IO_PortInit().
 * 
 *        This method intentionally omits sanity checks on @a data or @a size.
 *        It's the user's responsibility to handle NULL data or zero size in
 *        or out of the callback.
 * 
 * @param portIndex Initialized port index.
 * @param data      Data to send.
 * @param size      Size of @a data.
 * @return OEM_OK:            Success.
 *         OEM_ERR_IO_INDEX:  Invalid port index.
 *         OEM_ERR_IO_UNSET:  Port not initialized.
 *         OEM_ERR_NULL:      Write callback is null.
 *         Otherwise the return value of the attached oem_io_write_t callback.
 */
int OEM_IO_PortWrite(int portIndex,
                     const void* data,
                     size_t size);


/**
 * @brief Reads data from the I/O port designated by @a portIndex. Internally
 *        calls the oem_io_read_t callback attached by OEM_IO_PortInit().
 * 
 * @param portIndex Initialized port index.
 * @param[out] data Data buffer.
 * @param size      Read size.
 * @param timeout   Millisecond timeout.
 * @return OEM_OK:            Success.
 *         OEM_ERR_IO_INDEX:  Invalid port index.
 *         OEM_ERR_IO_UNSET:  Port not initialized.
 *         OEM_ERR_NULL:      Read callback is null.
 *         Otherwise the return value of the attached oem_io_read_t callback.
 */
int OEM_IO_PortRead(int portIndex,
                    void* data,
                    size_t length,
                    uint16_t timeout);

#endif
