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
        gpio_set_value(GPIO_FM_POWER, 1);
        msleep(10);
    }
    else {
        gpio_set_value(GPIO_FM_POWER, 0);
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

    if(gpio_request(GPIO_FM_POWER, "fm_power")){
        printk("ERROR: no fm_power pin available !!\n");
        return -EIO;
    }
    gpio_direction_output(GPIO_FM_POWER, 0);

    i2c_adap = i2c_get_adapter(2);
    if(i2c_adap == NULL){
        printk("rda5807m_dev : i2c adapter no find !!!\n");
        ret =  ENODEV;
        goto adapter_no_find;
    }

    rda5807m_client = i2c_new_device(i2c_adap, &rda5807m_info);
    if(rda5807m_client == NULL){
        printk("rda5807m_dev : i2c no probed device  !!!\n");
        ret =  ENODEV;
        goto device_no_find;
    }
    i2c_put_adapter(i2c_adap);

    printk("rda5807m_dev : new rda5807m iic_device  success\n");
    return 0;

device_no_find:
    i2c_put_adapter(i2c_adap);
adapter_no_find:
    gpio_free(GPIO_FM_POWER);
    return ret;
}
static __exit void rda5807m_dev_exit(void)
{
    i2c_unregister_device(rda5807m_client);
    gpio_free(GPIO_FM_POWER);
}


late_initcall(rda5807m_dev_init);
module_exit(rda5807m_dev_exit);
MODULE_LICENSE("GPL");
