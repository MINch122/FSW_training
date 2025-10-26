#ifndef _OEM_ARCH_H_
#define _OEM_ARCH_H_

#include "oem_basetype.h"
#include "oem_config.h"

int OEM_IO_SerialInit(void);

int OEM_IO_WriteCallback(int portIndex,
                         const void* buf,
                         size_t size);

int OEM_IO_ReadCallback(int portIndex,
                        void* buf,
                        size_t size,
                        uint16_t timeout);

#endif
