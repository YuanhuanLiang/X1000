#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include "board_base.h"

extern void rtc32k_enable(void);
extern void rtc32k_disable(void);

void rda5807m_power_on(bool on) {
    if (on) {
        rtc32k_enable();
        gpio_set_value(GPIO_FM_PWREN, FM_PWREN_LEVEL);
        msleep(10);
    }
    else {
        gpio_set_value(GPIO_FM_PWREN, !FM_PWREN_LEVEL);
        rtc32k_disable();
    }
}

EXPORT_SYMBOL(rda5807m_power_on);

static struct i2c_client *rda5807m_client;
static struct i2c_board_info rda5807m_info = { I2C_BOARD_INFO("radio-rda5807m", 0x10), };

static __init int rda5807m_dev_init(void)
{
    int ret = 0;
    struct i2c_adapter *i2c_adap;

    if (gpio_request(GPIO_FM_PWREN, "fm_power")) {
        printk("ERROR: no fm_power pin available!\n");
        return -EIO;
    }
    gpio_direction_output(GPIO_FM_PWREN, !FM_PWREN_LEVEL);

    i2c_adap = i2c_get_adapter(FM_I2C_ADAPTER);
    if (i2c_adap == NULL) {
        printk("rda5807m_dev: get i2c adapter %d failed!\n", FM_I2C_ADAPTER);
        ret = -ENODEV;
        goto adapter_no_find;
    }

    rda5807m_client = i2c_new_device(i2c_adap, &rda5807m_info);
    if (rda5807m_client == NULL) {
        printk("rda5807m_dev: add new i2c device failed!\n");
        ret = -ENODEV;
        goto device_no_find;
    }
    i2c_put_adapter(i2c_adap);

    printk("rda5807m_dev: register device successed!\n");
    return 0;

device_no_find:
    i2c_put_adapter(i2c_adap);
adapter_no_find:
    gpio_free(GPIO_FM_PWREN);
    return ret;
}
static __exit void rda5807m_dev_exit(void)
{
    i2c_unregister_device(rda5807m_client);
    gpio_free(GPIO_FM_PWREN);
}


late_initcall(rda5807m_dev_init);
module_exit(rda5807m_dev_exit);
MODULE_LICENSE("GPL");
