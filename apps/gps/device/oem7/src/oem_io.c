/**
 * @file oem_io.c
 * @brief OEM7 physical port I/O layer.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#include "oem_io.h"
#include "oem_utils.h"

#include <string.h>
#include <time.h>
#include <stdlib.h>

#if OEM_DEBUG
#include <stdio.h>
#endif

typedef struct {
    uint8_t* buffer;
    size_t head;
    size_t tail;
    size_t count;
    size_t capacity;
} oem_readbuf_t;

typedef struct {
    int index;
    oem_io_write_t write;
    oem_io_read_t  read;
    oem_readbuf_t  rbuf;
    bool initialized;
} oem_io_interface_t;


static oem_io_interface_t interfaces[OEM_IO_INTERFACES];

static oem_io_interface_t* get_ifc(int iface_idx)
{
    if (iface_idx < 0 || iface_idx >= OEM_IO_INTERFACES)
        return NULL;
    return &interfaces[iface_idx];
}

static int init_rbuf(oem_readbuf_t* rbuf, size_t buf_size)
{
    if (!rbuf)
        return OEM_ERR_NULL;

    rbuf->buffer = malloc(buf_size);
    if (rbuf->buffer == NULL)
        return OEM_ERR_NOMEM;
    
    rbuf->head = 0;
    rbuf->tail = 0;
    rbuf->count = 0;
    rbuf->capacity = buf_size;

    return OEM_OK;
}

static int init_ifc(int iface_idx,
                    size_t buf_size,
                    oem_io_write_t writeFunc,
                    oem_io_read_t readFunc)
{
    oem_io_interface_t* ifc = get_ifc(iface_idx);
    if (!ifc)
        return OEM_ERR_IO_IFACE_INDEX;

    if (!writeFunc || !readFunc)
        return OEM_ERR_NULL;

    if (ifc->initialized)
        return OEM_ERR_EXISTS;

    int ret = init_rbuf(&ifc->rbuf, buf_size);
    if (ret != OEM_OK)
        return ret;
    ifc->index = iface_idx;
    ifc->write = writeFunc;
    ifc->read = readFunc;
    ifc->initialized = true;

    return OEM_OK;
}

static int delete_ifc(int iface_idx)
{
    oem_io_interface_t* ifc = get_ifc(iface_idx);
    if (!ifc)
        return OEM_ERR_IO_IFACE_INDEX;

    if (!ifc->initialized)
        return OEM_OK;

    if (ifc->rbuf.buffer)
        free(ifc->rbuf.buffer);
    
    memset(ifc, 0, sizeof(*ifc));

    return OEM_OK;
}

int oem_io_init_interface(int iface_idx,
                          size_t read_buf_size,
                          oem_io_write_t write_callback,
                          oem_io_read_t read_callback)
{
    return init_ifc(iface_idx,
                    read_buf_size == 0 ? OEM_IO_DEFAULT_READBUF_SIZE : read_buf_size,
                    write_callback,
                    read_callback);
}

int oem_io_delete_interface(int iface_idx)
{
    return delete_ifc(iface_idx);
}

int oem_io_write(int iface_idx,
                 const void* data,
                 size_t size)
{
    oem_io_interface_t* ifc = get_ifc(iface_idx);
    if (!ifc)
        return OEM_ERR_IO_IFACE_INDEX;

    if (!ifc->initialized || !ifc->write)
        return OEM_ERR_IO_IFACE_UNSET;

    if (!data || size == 0)
        return OEM_ERR_NULL;

    int ret = ifc->write(iface_idx, data, size);
    if (ret < 0)
        return ret;
    if (ret != (int)size)
        return OEM_ERR_IO_WRITE_PARTIAL;

    return OEM_OK;
}

size_t oem_io_available(int iface_idx)
{
    oem_io_interface_t* ifc = get_ifc(iface_idx);
    if (!ifc)
        return 0;

    if (!ifc->initialized)
        return 0;

    return ifc->rbuf.count;
}

int oem_io_peek(int iface_idx,
                const void** data,
                size_t* size)
{
    oem_io_interface_t* ifc = get_ifc(iface_idx);
    if (!ifc)
        return OEM_ERR_IO_IFACE_INDEX;
    
    if (!ifc->initialized)
        return OEM_ERR_IO_IFACE_UNSET;

    if (!data)
        return OEM_ERR_NULL;

    *data = ifc->rbuf.buffer + ifc->rbuf.tail;
    if (size)
        *size = ifc->rbuf.count;

    return OEM_OK;
}

int oem_io_consume(int iface_idx,
                   size_t size)
{
    oem_io_interface_t* ifc = get_ifc(iface_idx);
    if (!ifc)
        return OEM_ERR_IO_IFACE_INDEX;

    if (!ifc->initialized)
        return OEM_ERR_IO_IFACE_UNSET;

    if (size > ifc->rbuf.count)
        return OEM_ERR_RANGE;

    ifc->rbuf.count -= size;
    ifc->rbuf.tail += size;

    /**
     * Rewind the buffer if there is nothing left.
     */
    if (ifc->rbuf.count == 0) {
        ifc->rbuf.head = 0;
        ifc->rbuf.tail = 0;
    }

    return OEM_OK;
}

int oem_io_fill(int iface_idx,
                uint16_t timeout)
{
    oem_io_interface_t* ifc = get_ifc(iface_idx);
    if (!ifc)
        return OEM_ERR_IO_IFACE_INDEX;
    if (!ifc->initialized || !ifc->read)
        return OEM_ERR_IO_IFACE_UNSET;

    if (ifc->rbuf.count == ifc->rbuf.capacity)
        return OEM_ERR_FULL;
 
    if (ifc->rbuf.tail > 0) {
        /* move the remaining data to the beginning of the buffer */
        memmove(ifc->rbuf.buffer,
                ifc->rbuf.buffer + ifc->rbuf.tail,
                ifc->rbuf.count);
        ifc->rbuf.head = ifc->rbuf.count;
        ifc->rbuf.tail = 0;
    }

    size_t unfilled = ifc->rbuf.capacity - ifc->rbuf.head;
    int ret = ifc->read(iface_idx, ifc->rbuf.buffer + ifc->rbuf.head, unfilled, timeout);
    if (ret < 0) {
        if (ret != OEM_ERR_IO_TIMEOUT)
            oem_debug_error("I/O read error for port %d: %d\n",
                            iface_idx, ret);
        return ret;
    }

    ifc->rbuf.count += ret;
    ifc->rbuf.head += ret;

    return OEM_OK;
}
