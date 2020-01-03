#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <types.h>
#include <utils/log.h>
#include <utils/assert.h>

#include <leds_gpio/leds_gpio_manager.h>

#define LOG_TAG                     "test_leds_gpio"


struct leds_gpio_manager* leds_gpio;

int main()
{
    int i,ret;

    leds_gpio = get_leds_gpio_manager();
    LOGE("Keyboard led show !!!\n");
    ret = leds_gpio->keyboard_init();
    if (ret == 0) {
        for (i = 0; i < KB_LED_MAX; ++i) {
            leds_gpio->keyboard_onoff(i,LED_ON);
            sleep(1);
        }

        for (i = 0; i < KB_LED_MAX; ++i) {
            leds_gpio->keyboard_onoff(i,LED_OFF);
            sleep(1);
        }
        leds_gpio->keyboard_deinit();
    }

    LOGE("Backlight led show !!!\n");
    ret = leds_gpio->backlight_init();
    if (ret == 0) {
        for (i = 0; i < BL_LED_MAX; ++i) {
            leds_gpio->backlight_onoff(i,LED_ON);
            sleep(1);
        }

        for (i = 0; i < BL_LED_MAX; ++i) {
            leds_gpio->backlight_onoff(i,LED_OFF);
            sleep(1);
        }
        leds_gpio->backlight_deinit();
    }

    LOGE("Matrix led show !!!\n");
    ret = leds_gpio->matrix_init();
    if (ret == 0) {
        for (i = 0; i < MATRIX_LED_MAX; ++i) {
            leds_gpio->matrix_onoff(i,LED_ON);
            sleep(1);
        }

        leds_gpio->matrix_onoff(MATRIX_LED_MAX-1,LED_OFF);
        sleep(1);

        leds_gpio->matrix_deinit();
    }

    LOGE("Indication led show !!!\n");
    ret = leds_gpio->indication_init();
    if (ret == 0) {
        leds_gpio->indication_onoff(FP_LED_W,LED_ON);
        sleep(1);
        leds_gpio->indication_onoff(FP_LED_B,LED_ON);
        sleep(1);
        leds_gpio->indication_onoff(FP_LED_G,LED_ON);
        sleep(1);
        leds_gpio->indication_onoff(FP_LED_R,LED_ON);
        sleep(1);

        leds_gpio->indication_onoff(FP_LED_W,LED_OFF);
        sleep(1);
        leds_gpio->indication_onoff(FP_LED_B,LED_OFF);
        sleep(1);
        leds_gpio->indication_onoff(FP_LED_G,LED_OFF);
        sleep(1);
        leds_gpio->indication_onoff(FP_LED_R,LED_OFF);
        sleep(1);

        leds_gpio->indication_blink(FP_LED_W, 1000, 3000);
        leds_gpio->indication_blink(FP_LED_B, 1000, 3000);
        leds_gpio->indication_blink(FP_LED_G, 1000, 3000);
        leds_gpio->indication_blink(FP_LED_R, 1000, 3000);
    }

    while(getchar() != 'Q')
        printf("Enter 'Q' to exit\n");

    leds_gpio->indication_deinit();

    return 0;
}