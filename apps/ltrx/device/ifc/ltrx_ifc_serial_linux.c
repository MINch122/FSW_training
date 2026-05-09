/**
 * @file ltrx_ifc_serial_linux.c
 * 
 * @author ryu@yonsei.ac.kr
 * @brief Test-purpose Linux serial interface.
 * @version 1.0
 * @date 2026-03-12
 * 
 * ACL Yonsei, 2026
 */
#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include "ltrx_ifc.h"
#include "ltrx_test_utils.h"

#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#if LTRX_SERIAL_SIMULATED
#include <pthread.h>
#include <semaphore.h> /* ensures no more than one dev thread runs */
#endif

typedef enum {
    SERIAL_OK                =  0,
    SERIAL_ERR_INVARG        = -1,
    SERIAL_ERR_OPEN          = -2,
    SERIAL_ERR_CONFIG        = -3,
    SERIAL_ERR_WRITE         = -4,
    SERIAL_ERR_READ          = -5,
} serial_ret_t;

static int serial_fd = -1;

#if LTRX_SERIAL_SIMULATED
    static sim_device_t sim_dev;
    static int slave_fd = -1;
    static sem_t sim_dev_sem;
    static bool running_mock = false;
#endif

static speed_t baud_to_speed(int baud)
{
    switch (baud) {
    case 9600:    return B9600;
    case 19200:   return B19200;
    case 38400:   return B38400;
    case 57600:   return B57600;
    case 115200:  return B115200;
    case 230400:  return B230400;
    case 460800:  return B460800;
    case 921600:  return B921600;
    default:      return B0;
    }
}

static int serial_configure(int fd, int baud)
{
    struct termios tio;

    if (tcgetattr(fd, &tio) != 0) {
        fprintf(stderr, "Error getting termios: %s\n", strerror(errno));
        close(fd);
        return SERIAL_ERR_CONFIG;
    }

    speed_t speed = baud_to_speed(baud);
    if (speed == B0) {
        fprintf(stderr, "Unsupported baud rate: %d\n", baud);
        close(fd);
        return SERIAL_ERR_CONFIG;
    }

    cfsetospeed(&tio, speed);
    cfsetispeed(&tio, speed);

    tio.c_cflag |= (CLOCAL | CREAD);            /* ignore modem control and read */
    tio.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE); /* 8n1 */
    tio.c_cflag |= CS8;
#ifdef CRTSCTS
    tio.c_cflag &= ~CRTSCTS;
#endif
    tio.c_lflag &= ~(ISIG | ICANON | IEXTEN); /* noncanonical 8-bit */
    tio.c_lflag &= ~(ECHO | ECHONL);                /* shut up and listen */
    tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK);     /* ignore brk signals */
    tio.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | ISTRIP); /* don't mess with on/off */
    tio.c_iflag &= ~(INLCR | IGNCR);                /* don't mess with LNCR */
    tio.c_oflag &= ~(OPOST); /* no postprocessing (raw out) */

    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &tio) != 0) {
        fprintf(stderr, "Error setting termios: %s\n", strerror(errno));
        close(fd);
        return SERIAL_ERR_CONFIG;
    }

    return SERIAL_OK;
}

/**
 * @brief Write data to the serial port.
 * 
 * @param fd Port descriptor.
 * @param data Data to write.
 * @param len Number of bytes to write.
 * @return ssize_t Number of bytes written on success.
 *         SERIAL_ERR_INVARG if data is NULL.
 *         SERIAL_ERR_WRITE if write() error occurs.
 */
static ssize_t serial_write(int fd,
                            const void* data,
                            size_t len)
{
    if (!data)
        return SERIAL_ERR_INVARG;

    ssize_t written = write(fd, data, len);
    if (written < 0) {
        fprintf(stderr, "Error writing to serial port: %s\n", strerror(errno));
        return SERIAL_ERR_WRITE;
    }

    return written;
}

/**
 * @brief Read data from the serial port with a timeout.
 * 
 *        The CFE Serial layer does not support a "strict" timeout, i.e.,
 *        tries only a single read() call even if the timeout has not expired,
 *        which can result in an unintended error if a big chunk is expected
 *        on a slow link.
 *        This function is a workaround that patiently waits until @a len bytes
 *        are collected within the timeout.
 * 
 * @param buffer Read data buffer.
 * @param len Number of bytes to read.
 * @param timeout_ms Timeout in milliseconds (max ~65 sec).
 * @return  ssize_t Number of bytes read on success. Can be less than @a len if timeout occurs.
 *          SERIAL_ERR_INVARG if buffer is NULL.
 *          SERIAL_ERR_READ if read() error occurs.
 */
static ssize_t serial_read(int fd,
                           void* buffer,
                           size_t len,
                           uint16_t timeout_ms)
{
    int ret;
    ssize_t rb;

    size_t read_bytes   = 0;
    size_t requested;
    uint8_t* pos        = buffer;
    uint32_t elapsed_ms = 0;

    fd_set fds;
    struct timeval tv;
    struct timespec start, now;

    bool freewheel = false;
    int freewheel_retries = 3;

    if (!buffer)
        return SERIAL_ERR_INVARG;

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    /**
     * Time latch will never fail - only "never" means "possibly" in space.
     */
    ret = clock_gettime(CLOCK_MONOTONIC, &start);
    if (ret != 0)
        freewheel = true;

    while (!freewheel || freewheel_retries-- > 0) {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        ret = select(fd + 1, &fds, NULL, NULL, &tv);

        if (ret < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "Error in select: %s\n", strerror(errno));
            return SERIAL_ERR_READ;
        }

        if (ret == 0) {
            /* Timeout */
            return (ssize_t) read_bytes;
        }

        requested = len - read_bytes;
        rb = read(fd, pos, requested);
    
        if (rb < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "Error reading from serial port: %s\n", strerror(errno));
            return SERIAL_ERR_READ;
        }

        if (rb == 0) {
            /* Modem hangup due to a poor termios setting. Return. */
            fprintf(stderr, "read() returned 0.\n");
            return SERIAL_ERR_READ;
        }

        /* Am I being too cautious? */
        if (rb > (ssize_t)requested) {
            fprintf(stderr, "We are tricked by space magic: read %zd > expected %zu\n", rb, requested);
            return SERIAL_ERR_READ;
        }

        pos += rb;
        read_bytes += rb;

        if (read_bytes == len) {
            // All data read (len > SSIZE_MAX is just academic).
            return (ssize_t) read_bytes;
        }

        if (!freewheel) {
            ret = clock_gettime(CLOCK_MONOTONIC, &now);
            if (ret != 0)
                freewheel = true;
            else {
                elapsed_ms = (now.tv_sec - start.tv_sec) * 1000
                            + (now.tv_nsec - start.tv_nsec) / 1000000;

                if (elapsed_ms >= timeout_ms) {
                    // Timeout
                    return (ssize_t) read_bytes;
                }

                tv.tv_sec = (timeout_ms - elapsed_ms) / 1000;
                tv.tv_usec = ((timeout_ms - elapsed_ms) % 1000) * 1000;
                if (tv.tv_sec == 0 && tv.tv_usec < 50000)
                    tv.tv_usec = 50000; /* Avoid busy loop (50 ms). */
            }
        }
    
    }
    return (ssize_t) read_bytes; /* Timeout after freewheel retries */
}



#if LTRX_SERIAL_SIMULATED

    static int pty_create(int *master_fd, char *slave_name, size_t name_len)
    {
        int mfd = posix_openpt(O_RDWR | O_NOCTTY);
        if (mfd < 0)
            return -1;

        if (grantpt(mfd) != 0 || unlockpt(mfd) != 0) {
            close(mfd);
            return -1;
        }

        char *name = ptsname(mfd);
        if (!name) {
            close(mfd);
            return -1;
        }

        snprintf(slave_name, name_len, "%s", name);
        *master_fd = mfd;
        return 0;
    }

    static void *sim_device_thread(void *arg)
    {
        sim_device_t *dev = arg;
        uint8_t rx_buf[512];

        sem_wait(&sim_dev_sem);

        LOG_SIM("Device thread started with expect_tx_len=%zu, response_len=%zu, delay_ms=%u",
            dev->expect_tx_len, dev->response_len, dev->delay_ms);

        /* read from the host */
        size_t total = 0;
        while (total < dev->expect_tx_len) {
            ssize_t n = read(slave_fd, rx_buf + total,
                            dev->expect_tx_len - total);
            if (n <= 0) break;
            total += n;
        }

        /*  verify the command matches */
        if (total == dev->expect_tx_len && dev->expect_tx) {
            if (memcmp(rx_buf, dev->expect_tx, dev->expect_tx_len) != 0) {
                LOG_SIM("    [SIM] command mismatch: expected %zu bytes, got %zu bytes", dev->expect_tx_len, total);
                return NULL;
            }
        }

        /* simulate hardware latency. */
        if (dev->delay_ms > 0)
            usleep(dev->delay_ms * 1000);

        /* send the canned response (if any). */
        if (dev->response_len > 0) {
            LOG_SIM("Sending response of length %zu...", dev->response_len);
            log_hexdump(LOG_LEVEL_SIM, dev->response, dev->response_len);

            uint32_t crc = LTRX_CalculateCRC32(((uint8_t *)dev->response) + LTRX_BEACON_HEADER_SIZE, dev->response_len - LTRX_BEACON_HEADER_SIZE);
            ((LTRX_BeaconCmdHeader_t *)dev->response)->CRC = crc;
            write(slave_fd, dev->response, dev->response_len);
        }

        sem_post(&sim_dev_sem);
        return NULL;
    }

    bool sim_device_set_expect(const void *expect_tx, size_t expect_tx_len, size_t offset)
    {
        if (expect_tx_len + offset > 512) {
            fprintf(stderr, "Expect command too long for simulation\n");
            return false;
        }
        if (expect_tx && expect_tx_len > 0)
            memcpy((uint8_t *)sim_dev.expect_tx + offset, expect_tx, expect_tx_len);
        sim_dev.expect_tx_len = expect_tx_len + offset;
        return true;
    }

    bool sim_device_set_response(const void *response, size_t response_len, size_t offset)
    {
        if (response_len + offset > 512) {
            fprintf(stderr, "Response too long for simulation\n");
            return false;
        }
        if (response && response_len > 0)
            memcpy((uint8_t *)sim_dev.response + offset, response, response_len);
        sim_dev.response_len = response_len + offset;
        return true;
    }

    void sim_device_set_delay(uint32_t delay_ms)
    {
        sim_dev.delay_ms = delay_ms;
    }

#endif /* LTRX_SERIAL_SIMULATED */


int ltrx_serial_init(const char* dev, int baud)
{
#if LTRX_SERIAL_SIMULATED
    /* Mock device serial port init. */

    if (!dev) {
        char slave_name[64];
        running_mock = true;
        if (pty_create(&slave_fd, slave_name, sizeof(slave_name)) != 0 ||
            serial_configure(slave_fd, 115200) != SERIAL_OK) {
            fprintf(stderr, "Failed to create simulated serial device\n");
            return LTRX_ERROR;
        }

        printf("Simulated serial device created: slave_fd=%d, slave_name=%s\n", slave_fd, slave_name);
        dev = slave_name; // Override dev with the slave side of the pty

        sim_dev.expect_tx = malloc(512);
        sim_dev.expect_tx_len = 0;
        sim_dev.response = malloc(512);
        sim_dev.response_len = 0;
        sim_dev.delay_ms = 100;

        sem_init(&sim_dev_sem, 0, 1);
    }

#endif /* LTRX_SERIAL_SIMULATED */

    /* Real, host serial port. */
    serial_fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0) {
        fprintf(stderr, "Error opening serial port %s: %s\n", dev, strerror(errno));
        return SERIAL_ERR_OPEN;
    }
    return serial_configure(serial_fd, baud) == SERIAL_OK ? LTRX_SUCCESS : LTRX_ERROR;
}
    
int32_t ltrx_serial_transaction(const void* tx_buf,
                                uint16_t tx_len,
                                void* rx_buf,
                                int32_t rx_size,
                                uint16_t timeout_ms)
{
    ssize_t ret;

    /* Bad argument. */
    if (rx_size < 0 && rx_size != -1)
        return LTRX_ERROR;

    if (tx_buf && tx_len > 0) {
        ret = serial_write(serial_fd, tx_buf, tx_len);
        if (ret == SERIAL_ERR_INVARG)
            return LTRX_ERROR_NULL_PTR;
        if (ret != (ssize_t) tx_len)
            return LTRX_ERROR;

        LOG_SERIAL("Written %zd bytes", ret);
        log_hexdump(LOG_LEVEL_SERIAL, tx_buf, (size_t) ret);
    }

#if LTRX_SERIAL_SIMULATED
    if (running_mock) {
        pthread_t thread;
        pthread_create(&thread, NULL, sim_device_thread, &sim_dev);
        pthread_detach(thread);
    }
#endif

    if (rx_buf && (rx_size >= 0 || rx_size == -1)) {
        ret = serial_read(serial_fd,
                          rx_buf,
                          rx_size == -1 ? LTRX_SERIAL_READ_MAX : rx_size,
                          timeout_ms);
        switch (ret) {
        case SERIAL_ERR_INVARG:
            return LTRX_ERROR_NULL_PTR;
        case SERIAL_ERR_READ:
            return LTRX_ERROR;
        default:
            if (ret < 0)
                return LTRX_ERROR;
            break;
        }

        LOG_SERIAL("Read %zd bytes", ret);
        log_hexdump(LOG_LEVEL_SERIAL, rx_buf, (size_t) ret);

        /* rx_size == -1 accepts any length.*/
        if (rx_size == -1)
            return ret == 0 ? LTRX_ERROR_TIMEOUT : (int32_t) ret;

        if (ret < (ssize_t) rx_size)
            return LTRX_ERROR_TIMEOUT;
    }

    return LTRX_SUCCESS;
}
