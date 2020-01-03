#include <signal.h>
#include <stdlib.h>
#include <types.h>
#include <pthread.h>

#include <utils/log.h>
#include <gpio/gpio_manager.h>

#define LOG_TAG "test_gpio"

#define LED_GPIO    GPIO_PA(9)
#define KEY_GPIO    GPIO_PA(10)

struct gpio_manager* gpio;

static void msleep(long long msec) {
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do {
        err = nanosleep(&ts, &ts);
    } while (err < 0 && errno == EINTR);
}

static void irq_work(int gpio_num) {
    static gpio_value tmp_value;

    if(gpio_num == KEY_GPIO) {
        msleep(10);

        if(gpio->get_value(KEY_GPIO, &tmp_value) == 0)
                gpio->set_value(LED_GPIO, tmp_value);

        printf("------key: %d ------\n",tmp_value);
    }
}

int main(void) {
    gpio = get_gpio_manager();

    if(gpio->init() < 0) {
        LOGE("Failed to gpio init\n");
        return -1;
    }

    if(gpio->open(KEY_GPIO) < 0) {
        LOGE("Failed to open key pin\n");
        goto open_key_err;
    }

    if(gpio->open(LED_GPIO) < 0) {
        LOGE("Failed to open led pin\n");
        goto open_key_err;
    }

    if(gpio->set_direction(LED_GPIO, GPIO_OUT) < 0) {
        LOGE("Failed to set led pin direction\n");
        goto gpio_set_err;
    }

    if(gpio->set_direction(KEY_GPIO, GPIO_IN) < 0) {
        LOGE("Failed to set key pin direction\n");
        goto gpio_set_err;
    }

    if(gpio->set_value(LED_GPIO, GPIO_HIGH) < 0) {
        LOGE("Failed to set lcd pin value\n");
        goto gpio_set_err;
    }

    gpio->set_irq_func(irq_work);

    if(gpio->enable_irq(KEY_GPIO, GPIO_BOTH) < 0) {
        LOGE("Failed to enable key irq\n");
        goto gpio_set_err;
    }

    while(getchar() != 'Q')
        printf("Enter 'Q' to exit\n");

    gpio->disable_irq(KEY_GPIO);

gpio_set_err:
    gpio->close(KEY_GPIO);
open_key_err:
    gpio->close(LED_GPIO);
    gpio->deinit();
    return 0;
}
