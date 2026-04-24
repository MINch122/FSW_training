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
} oem_readbuf_t;

typedef struct {
    int index;
    oem_io_write_t write;
    oem_io_read_t read;
    oem_readbuf_t rbuf;
    bool initialized;
} oem_physical_port_t;

static oem_physical_port_t ports[OEM_PHYSICAL_PORTS];

static int InitReadBuf(oem_readbuf_t* rbuf)
{
    if (!rbuf)
        return OEM_ERR_NULL;

    rbuf->buffer = malloc(OEM_UTILS_READBUF_SIZE);
    if (rbuf->buffer == NULL)
        return OEM_ERR_NOMEM;
    
    rbuf->head = 0;
    rbuf->tail = 0;
    rbuf->count = 0;

    return OEM_OK;
}
static int BufferFeed(oem_physical_port_t* port,
                      uint16_t timeout)
{
    int ret;
    size_t unfilled;

    if (!port)
        return OEM_ERR_NULL;
    
    unfilled = OEM_UTILS_READBUF_SIZE - port->rbuf.head;
    if (unfilled == 0) 
        return 0;

    ret = port->read(port->index,
                     port->rbuf.buffer + port->rbuf.head,
                     unfilled,
                     timeout);

    if (ret >= 0) {
        port->rbuf.count += ret;
        port->rbuf.head += ret;
    }
    else {
        DebugError("Buffer feed error: %d\n", ret);
        return -1;
    }

    return ret;
}

/**
 * Try to dump @a size bytes from the rbuf into @a data.
 * Returns actually dumped bytes. Rewinds the buffer if no data is left
 * after the call.
 */
static size_t BufferConsume(oem_readbuf_t* rbuf,
                            void* data,
                            size_t size)   
{
    size_t consumed;

    if (!data || !rbuf)
        return 0;

    if (rbuf->count == 0)
        return 0;

    consumed = rbuf->count > size ? size : rbuf->count;
    memcpy(data, rbuf->buffer + rbuf->tail, consumed);
    rbuf->count -= consumed;

    /**
     * Rewind the buffer if there is nothing left.
     */
    if (rbuf->count == 0) {
        rbuf->head = 0;
        rbuf->tail = 0;
    }
    else {
        rbuf->tail += consumed;
    }

    return consumed;
}

int OEM_IO_PortInit(int portIndex,
                    oem_io_write_t writeFunc,
                    oem_io_read_t readFunc)
{
    int ret;

    if (portIndex < 0 || portIndex > OEM_PHYSICAL_PORTS)
        return OEM_ERR_IO_PORT_INDEX; 
    if (!writeFunc || !readFunc)
        return OEM_ERR_NULL;

    if (ports[portIndex].initialized)
        return OEM_ERR_EXISTS;

    ret = InitReadBuf(&ports[portIndex].rbuf);
    if (ret != OEM_OK)
        return ret;

    ports[portIndex].index = portIndex;
    ports[portIndex].write = writeFunc;
    ports[portIndex].read = readFunc;
    ports[portIndex].initialized = true;

    return OEM_OK;
}

int OEM_IO_PortDelete(int portIndex)
{
    if (portIndex < 0 || portIndex > OEM_PHYSICAL_PORTS)
        return OEM_ERR_IO_PORT_INDEX;
    
    if (!ports[portIndex].initialized)
        return OEM_OK;

    if (ports[portIndex].rbuf.buffer)
        free(ports[portIndex].rbuf.buffer);
    
    memset(&ports[portIndex], 0, sizeof(ports[portIndex]));

    return OEM_OK;
}

int OEM_IO_PortWrite(int portIndex,
                     const void* data,
                     size_t size)
{
    int ret;

    if (portIndex < 0 || portIndex > OEM_PHYSICAL_PORTS)
        return OEM_ERR_IO_PORT_INDEX;

    if (!ports[portIndex].initialized)
        return OEM_ERR_IO_PORT_UNSET;

    if (ports[portIndex].write == NULL)
        return OEM_ERR_NULL;
    
    ret = ports[portIndex].write(portIndex, data, size);

    if (ret < 0 || (size_t) ret != size)
        return OEM_ERR_WRITE;

    return OEM_OK;
}

#define TP_TIMEDIFF(tp1, tp2)   (((tp2).tv_sec - (tp1).tv_sec) * 1000 \
                                  + ((tp2).tv_nsec - (tp1).tv_nsec) / 1000000)

int OEM_IO_PortRead(int portIndex,
                    void* data,
                    size_t size,
                    uint16_t timeout)
{
    oem_physical_port_t* port;
    struct timespec tp1, tp2;
    size_t dumped;
    size_t remaining;
    uint16_t waitms;
    uint32_t elapsed = 0;
    long longdiff;
    int ret;

    if (!data)
        return OEM_ERR_NULL;

    if (portIndex < 0 || portIndex > OEM_PHYSICAL_PORTS)
        return OEM_ERR_IO_PORT_INDEX;

    if (size > OEM_UTILS_READBUF_SIZE) {
        DebugError("Request for %d bytes exceeds the buffer size %d\n",
                   size,
                   OEM_UTILS_READBUF_SIZE);
        return OEM_ERR_RANGE;
    }
    port = &ports[portIndex];
    if (!port->initialized)
        return OEM_ERR_IO_PORT_UNSET;

    dumped = BufferConsume(&port->rbuf, data, size);

    /**
     * The buffer had enough bytes.
     */
    if (dumped == size)
        return OEM_OK;

    /**
     * Else, we must fetch new data from the port. Repeat until size bytes are
     * retreived.
     */
    remaining = size - dumped;

    clock_gettime(CLOCK_MONOTONIC, &tp1);
    waitms = timeout;

    while (port->rbuf.count < remaining) {

        /**
         * Populate the buffer.
         */
        ret = BufferFeed(port, waitms);
    
        /**
         * Timeout reached or read error occurred. Break.
         */
        if (ret < 0)
            break;

        clock_gettime(CLOCK_MONOTONIC, &tp2);

        longdiff = TP_TIMEDIFF(tp1, tp2);

        /**
         * Clock broken.
         */
        if (longdiff < 0)
            return OEM_ERR_IO_CLOCK;

        /**
         * Prevent possible (pseudo) infinite loop due to
         * a poor design of the read function.
         */
        if (longdiff < 10)
            longdiff = 10;
        
        waitms = longdiff;
        elapsed += waitms;

        if (elapsed >= timeout)
            break;
    }

    if (port->rbuf.count < remaining) {
        return ret < 0 ? ret : OEM_ERR_TIMEOUT;
    }

    BufferConsume(&port->rbuf, (uint8_t*) data + dumped, remaining);
    return OEM_OK;
}
