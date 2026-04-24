/**
 * @file oem_arch_io_linux.c
 * @brief Linux serial I/O port example.
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * Astrodynamics & Control Lab. 2025.
 */
#include "oem_arch.h"

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
    speed_t baud; 
} serial_ports_t;

static serial_ports_t ports[OEM_PHYSICAL_PORTS] = {
    {
     .dev = "/dev/ttyS1",
     .baud = B115200,
    }
};

int OEM_IO_SerialInit(void)
{
    serial_ports_t* p;
    struct termios tio;

    for (size_t i = 0; i < sizeof(ports)/sizeof(ports[0]); ++i) {

        p = &ports[i];

        p->fd = open(p->dev, O_RDWR | O_NOCTTY);
        if (p->fd < 0) {
            fprintf(stderr,
                    "failed to open %s: %s\n",
                    p->dev, strerror(errno));
            continue;
        }

        if (tcgetattr(p->fd, &tio) < 0) {
            fprintf(stderr,
                    "tcgetattr error for %s: %s\n",
                    p->dev, strerror(errno));
            close(p->fd);
            continue;
        }

        tio.c_cflag |= (CLOCAL | CREAD);
        tio.c_cflag &= ~(PARENB | PARODD | CSTOPB);
        tio.c_cflag &= ~CSIZE;
        tio.c_cflag |= CS8;

        tio.c_lflag &= ~(ISIG | ICANON);
        tio.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
        tio.c_iflag &= ~(INPCK | ISTRIP);
        tio.c_iflag &= ~(IXON | IXOFF | IXANY);
        tio.c_iflag &= ~(ICRNL | INLCR | IGNCR);

        tio.c_oflag &= ~(OPOST);

        if (cfsetispeed(&tio, p->baud) != 0) {
            fprintf(stderr,
                    "unsupported baud %o for %s\n",
                    p->baud, p->dev);
            close(p->fd);
            continue;
        }
    
        cfsetospeed(&tio, p->baud);

        if (tcsetattr(p->fd, TCSANOW, &tio) < 0) {
            fprintf(stderr,
                    "tcsetattr error for %s: %s\n",
                    p->dev, strerror(errno));
            close(p->fd);
            continue;
        }

        printf("initialized port %s\n", p->dev);
    }

    return OEM_OK;
}

int OEM_IO_WriteCallback(int portIndex,
                         const void* buf,
                         size_t size)
{
    ssize_t b;

    if (portIndex < 0 || portIndex >= OEM_PHYSICAL_PORTS) {
        return OEM_ERR_IO_PORT_INDEX;
    }

    b = write(ports[portIndex].fd, buf, size);

    if (b < 0) {
        fprintf(stderr,
                "write error for %s: %s\n",
                ports[portIndex].dev, strerror(errno));
        return OEM_ERR_WRITE;
    }

    return b;
}

int OEM_IO_ReadCallback(int portIndex,
                        void* buf,
                        size_t size,
                        uint16_t timeout)
{
    int fd;
	fd_set fdset;
	struct timeval tv;
    ssize_t b;

    if (portIndex < 0 || portIndex > OEM_PHYSICAL_PORTS) return OEM_ERR_IO_PORT_INDEX;

	FD_ZERO(&fdset);
	FD_SET(ports[portIndex].fd, &fdset);

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

    fd = ports[portIndex].fd;

	if (select(fd + 1, &fdset, NULL, NULL, &tv) > 0) {
		b = read(fd, buf, size);
        if (b < 0)
            fprintf(stderr,
                    "read error for %s: %s\n",
                    ports[portIndex].dev, strerror(errno));
        return b <= 0 ? OEM_ERR_READ : b;
    }

    return OEM_ERR_TIMEOUT;
}
