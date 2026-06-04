/**
 * @file oem_io_serial_linux.c
 * @brief Linux serial I/O port example.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#include "oem_io_serial_linux.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <sys/select.h>
#include <sys/ioctl.h>


typedef struct {
    int fd;
    const char* dev;
    bool initialized;
} serial_port_t;

static serial_port_t ports[OEM_IO_INTERFACES];

static speed_t get_baud(uint32_t baud)
{
    switch (baud) {
        case 1200: return B1200;
        case 1800: return B1800;
        case 2400: return B2400;
        case 4800: return B4800;
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 921600: return B921600;
        default: break;
    }
    return 0;
}

int oem_io_driver_serial_init(int iface_idx,
                              const char* dev,
                              uint32_t baud)
{
    struct termios tio;
    serial_port_t* p;
    int fd = -1;

    if (iface_idx < 0 || iface_idx >= OEM_IO_INTERFACES)
        return OEM_ERR_IO_IFACE_INDEX;

    p = &ports[iface_idx];

    speed_t speed = get_baud(baud);
    if (speed == 0) {
        fprintf(stderr,
                "no baud candidate %u for %s\n",
                baud, dev);
        return OEM_ERR_IO_CONFIG;
    }

    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr,
                "failed to open %s: %s\n",
                dev, strerror(errno));
        return OEM_ERR_IO_CONFIG;
    }

    if (tcgetattr(fd, &tio) != 0) {
        fprintf(stderr,
                "tcgetattr error for %s: %s\n",
                dev, strerror(errno));
        close(fd);
        return OEM_ERR_IO_CONFIG;
    }

    tio.c_cflag |= (CLOCAL | CREAD);            /* ignore modem control and read */
    tio.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE); /* 8n1 */
    tio.c_cflag |= CS8;
#ifdef CRTSCTS
    tio.c_cflag &= ~CRTSCTS;
#endif
    tio.c_lflag &= ~(ISIG | ICANON | IEXTEN);   /* noncanonical 8-bit */
    tio.c_lflag &= ~(ECHO | ECHONL);            /* shut up and listen */
    tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK); /* ignore brk signals */
    tio.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | ISTRIP); /* don't mess with on/off */
    tio.c_iflag &= ~(INLCR | IGNCR);            /* don't mess with LNCR */
    tio.c_oflag &= ~(OPOST);                    /* no postprocessing (raw out) */

    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 0;
        
    if (cfsetispeed(&tio, speed) != 0) {
        fprintf(stderr,
                "unsupported baud %o for %s\n",
                speed, dev);
        close(fd);
        return OEM_ERR_IO_CONFIG;
    }
    
    cfsetospeed(&tio, speed);

    if (tcsetattr(fd, TCSANOW, &tio) < 0) {
        fprintf(stderr,
                "tcsetattr error for %s: %s\n",
                dev, strerror(errno));
        close(fd);
        return OEM_ERR_IO_CONFIG;
    }

    p->fd = fd;
    p->dev = dev;
    p->initialized = true;
    printf("initialized port %s\n", dev);

    return OEM_OK;
}

int oem_io_driver_serial_write(int iface_idx,
                         const void* buf,
                         size_t size)
{
    ssize_t b;

    if (iface_idx < 0 || iface_idx >= OEM_IO_INTERFACES)
        return OEM_ERR_IO_IFACE_INDEX;

    b = write(ports[iface_idx].fd, buf, size);

    if (b < 0) {
        fprintf(stderr,
                "write error for %s: %s\n",
                ports[iface_idx].dev, strerror(errno));
        return OEM_ERR_IO_WRITE;
    }

    return b;
}

int oem_io_driver_serial_read(int iface_idx,
                              void* buf,
                              size_t size,
                              uint16_t timeout)
{
    int fd;
    fd_set fdset;
    struct timeval tv;
    ssize_t b;

    if (iface_idx < 0 || iface_idx >= OEM_IO_INTERFACES)
        return OEM_ERR_IO_IFACE_INDEX;

    FD_ZERO(&fdset);
    FD_SET(ports[iface_idx].fd, &fdset);

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    fd = ports[iface_idx].fd;

    if (select(fd + 1, &fdset, NULL, NULL, &tv) > 0) {
        b = read(fd, buf, size);
        if (b < 0)
            fprintf(stderr,
                    "read error for %s: %s\n",
                    ports[iface_idx].dev, strerror(errno));
        return b <= 0 ? OEM_ERR_IO_READ : b;
    }

    return OEM_ERR_IO_TIMEOUT;
}

int oem_io_driver_serial_close(int iface_idx)
{
    if (iface_idx < 0 || iface_idx >= OEM_IO_INTERFACES)
        return OEM_ERR_IO_IFACE_INDEX;

    if (ports[iface_idx].initialized) {
        close(ports[iface_idx].fd);
        ports[iface_idx].initialized = false;
        ports[iface_idx].fd = -1;
        printf("closed port %s\n", ports[iface_idx].dev);
        ports[iface_idx].dev = NULL;
    }

    return OEM_OK;
}
