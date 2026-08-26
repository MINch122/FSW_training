/**
 * @file oem_io_serial_linux.h
 * @brief Linux serial I/O port example.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#ifndef _OEM_IO_SERIAL_LINUX_H_
#define _OEM_IO_SERIAL_LINUX_H_

#include "oem_types.h"
#include "oem_config.h"

int oem_io_driver_serial_init(int iface_idx,
                              const char* dev,
                              uint32_t baud);

int oem_io_driver_serial_write(int iface_idx,
                               const void* buf,
                               size_t size);

int oem_io_driver_serial_read(int iface_idx,
                              void* buf,
                              size_t size,
                              uint16_t timeout);

int oem_io_driver_serial_close(int iface_idx);

#endif
