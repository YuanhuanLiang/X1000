#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <assert.h>
#include <linux/fs.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "utils/log.h"
#include <utils/assert.h>


extern volatile unsigned char irq_flag_io;

#define LOG_TAG "gpio"
#define GPIO_SYSFS               "/sys/class/gpio/"

typedef enum _gpio_irq_mode {
    GPIO_NONE,
    GPIO_RISING,
    GPIO_FALLING,
    GPIO_BOTH,
} gpio_irq_mode;
#define MAX_GPIO_NUM  256

struct gpio_data {
    int fd;
};

enum {
    unexport,
    export,
    direction,
    value_rw,
    edge,
};

static int is_inited = 0;
static struct gpio_data gpio_datas[MAX_GPIO_NUM];

static void check_init(void)
{
    if (!is_inited) {
        int i;
        for (i = 0; i < MAX_GPIO_NUM; i++) {
            gpio_datas[i].fd = -1;
        }
        is_inited = 1;
    }
}

#define check_gpio_requested(gpio)  \
do { \
    assert (is_inited); \
    assert ((gpio) < MAX_GPIO_NUM); \
    assert (gpio_datas[gpio].fd >= 0); \
} while (0)

static int gpio_open_file(unsigned int gpio, int file)
{
    char filename[64] = {0};
    int mode = O_RDWR;

    switch (file) {
    case unexport:
        snprintf(filename, sizeof(filename),
                GPIO_SYSFS"unexport");
        mode = O_WRONLY;
        break;
    case export:
        snprintf(filename, sizeof(filename),
                GPIO_SYSFS"export");
        mode = O_WRONLY;
        break;
    case direction:
        snprintf(filename, sizeof(filename),
                GPIO_SYSFS"gpio%d/direction", gpio);
        break;
    case value_rw:
        snprintf(filename, sizeof(filename),
                GPIO_SYSFS"gpio%d/value", gpio);
        mode = O_RDWR;
        break;
    case edge:
        snprintf(filename, sizeof(filename),
                GPIO_SYSFS"gpio%d/edge", gpio);
        break;
    default:
        assert(0);
    }

    int fd = open(filename, mode);
    if (fd < 0) {
        LOGE("error to open %s, %s\n", filename, strerror(errno));
        return -1;
    }

    return fd;
}

static int gpio_write_int(int fd, int val)
{
    char value[12] = {0};

    sprintf(value, "%d", val);
    return write(fd, value, sizeof(value));
}

static int gpio_read_int(int fd)
{
    char value_read[12] = {0};

    int ret = read(fd, value_read, sizeof(value_read));
    if (ret <= 0)
        return -1;

    return atoi(value_read);
}

static int sys_request_gpio(unsigned int gpio)
{
    int fd, ret;

    fd = gpio_open_file(gpio, export);
    if (fd < 0)
        return fd;

    ret = gpio_write_int(fd, gpio);
    if (ret <= 0) {
        close(fd);
        return ret;
    }

    close(fd);

    return 0;
}

static int sys_free_gpio(unsigned int gpio)
{
    int fd, ret;

    fd = gpio_open_file(gpio, unexport);
    if (fd < 0)
        return fd;

    ret = gpio_write_int(fd, gpio);
    if (ret <= 0) {
        LOGE("failed to unexport gpio: %d, %s\n", gpio, strerror(errno));
        close(fd);
        return ret;
    }

    close(fd);

    return 0;
}

int gpio_request(unsigned int gpio) {
    int ret, fd;

    check_init();
    assert(gpio < MAX_GPIO_NUM);
    assert(gpio_datas[gpio].fd < 0);

    ret = sys_request_gpio(gpio);
    if (ret < 0) {
        LOGE("failed to export gpio: %d, %s, now force unexport it\n", gpio, strerror(errno));
        ret = sys_free_gpio(gpio);
        if (ret)
            return ret;
        ret = sys_request_gpio(gpio);
        if (ret) {
            LOGE("failed to export gpio again: %d, %s\n", gpio, strerror(errno));
            return ret;
        }
    }

    fd = gpio_open_file(gpio, value_rw);
    if (fd < 0) {
        LOGE("failed to open gpio value: %d, %s\n", gpio, strerror(errno));
        sys_free_gpio(gpio);
        return fd;
    }

    gpio_datas[gpio].fd = fd;

    return 0;
}


int gpio_free(unsigned int gpio) {
    int ret;

    check_gpio_requested(gpio);

    close(gpio_datas[gpio].fd);
    gpio_datas[gpio].fd = -1;
    ret = sys_free_gpio(gpio);

    return ret;
}

int gpio_get_value(unsigned int gpio) {
    int ret, fd;

    check_gpio_requested(gpio);
    fd = gpio_datas[gpio].fd;

    lseek(fd, 0x00, SEEK_SET);
    ret = gpio_read_int(fd);
    if (ret < 0) {
        LOGE("failed to read gpio value: %d, %s\n", gpio, strerror(errno));
    }

    return ret;
}

int gpio_set_value(unsigned int gpio, int val) {
    int ret, fd;

    check_gpio_requested(gpio);
    fd = gpio_datas[gpio].fd;

    lseek(fd, 0x00, SEEK_SET);
    ret = gpio_write_int(fd, val);
    if (ret <= 0) {
        LOGE("failed to write gpio value: %d, %s\n", gpio, strerror(errno));
        close(fd);
        return ret;
    }

    return 0;
}

static int gpio_set_direction(unsigned int gpio, int direct) {
    int ret, fd;
    char *str = NULL;

    check_gpio_requested(gpio);

    fd = gpio_open_file(gpio, direction);
    if (fd < 0)
        return fd;

    if (direct)
        str = "out";
    else
        str = "in";

    ret = write(fd, str, strlen(str));
    if (ret <= 0) {
        LOGE("failed to set gpio direction: %d, %s\n", gpio, strerror(errno));
        close(fd);
        return ret;
    }

    close(fd);
    return 0;
}

int gpio_direction_input(unsigned int gpio)
{
    return gpio_set_direction(gpio, 0);
}

int gpio_direction_output(unsigned int gpio, int value)
{
    int ret = gpio_set_direction(gpio, 1);
    if (ret < 0)
        return ret;

    return gpio_set_value(gpio, value);
}

int gpio_enable_irq(unsigned int gpio, gpio_irq_mode irq_mode)
{
    int ret, fd;
    char *buf = NULL;

    check_gpio_requested(gpio);
    fd = gpio_datas[gpio].fd;

    fd = gpio_open_file(gpio, edge);
    if (fd < 0)
        return fd;

    switch (irq_mode) {
    case GPIO_NONE:
        buf = "none";
        break;
    case GPIO_RISING:
        buf = "rising";
        break;
    case GPIO_FALLING:
        buf = "falling";
        break;
    case GPIO_BOTH:
        buf = "both";
        break;
    //default:
    //    panic(0, "error gpio irq mode: %d\n", irq_mode);
    }

    ret = write(fd, buf, strlen(buf));
    if (ret != strlen(buf)) {
        LOGE("failed to write gpio irq mode: %d, %s\n", gpio, strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int gpio_irq_wait_timeout(unsigned int gpio, int timeout_ms)
{
    int ret, fd;
    struct pollfd irqdesc = {
        .events = POLLPRI | POLLERR ,
    };

    check_gpio_requested(gpio);

    fd = gpio_datas[gpio].fd;

    irqdesc.fd = fd;
    lseek(fd, 0, SEEK_SET);
    ret = poll(&irqdesc, 1, timeout_ms);
    if (ret < 0) {
        LOGE("failed to wait gpio irq: %d, %s\n", gpio, strerror(errno));
        close(fd);
        return ret;
    }

    /* timeout */
    if (ret == 0)
        return -1;

    ret = gpio_read_int(fd);
    if (ret < 0) {
        LOGE("failed to read gpio value: %d, %s\n", gpio, strerror(errno));
    }

    return ret;
}

int gpio_irq_wait(unsigned int gpio)
{
    return gpio_irq_wait_timeout(gpio, -1);
}


void *gpio_irq_read_thread(void *data)
{
    unsigned int io_irq = (unsigned int)data;
    while (1) {
        int ret = gpio_irq_wait_timeout(io_irq, 50);
        if (ret >= 0) {
            irq_flag_io = 1;
        } else {
            ret = gpio_get_value(io_irq);
            if (!ret) {
                irq_flag_io = 1;
            }
        }
        usleep(1000);
    }
}

void gpio_irq_linuxport_free(unsigned int io_res, unsigned int io_irq)
 {
    gpio_free(io_res);
    gpio_free(io_irq);
}

void create_a_attached_thread(pthread_t *thread, void *(*start_routine)(void *),
        void *arg) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_t t;
    if (thread == NULL)
        thread = &t;

    int error = pthread_create(thread, &attr, start_routine, arg);
    assert(!error);
}

void sl2523_gpio_init(unsigned int io_res, unsigned int io_irq)
{
    int ret;

    printf("SL2523_GPIO_INT: %d\n", io_irq);
    printf("SL2523_GPIO_PD: %d\n", io_res);

    ret = gpio_request(io_irq);
    assert(!ret);

    ret = gpio_request(io_res);
    assert(!ret);

    gpio_direction_output(io_res, 1);
    usleep(30 * 1000);

    gpio_direction_input(io_irq);

    ret = gpio_enable_irq(io_irq, GPIO_FALLING);
    assert(!ret);

    create_a_attached_thread(NULL, gpio_irq_read_thread, (void *)io_irq);
}
