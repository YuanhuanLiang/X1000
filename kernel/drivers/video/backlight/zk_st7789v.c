/*
 *  LCD control code for truly
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/lcd.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>

struct zk_st7789v_data {
    int lcd_power;
    struct lcd_device *lcd;
    struct lcd_platform_data *ctrl;
};

static int zk_st7789v_set_power(struct lcd_device *lcd, int power)
{
    struct zk_st7789v_data *dev= lcd_get_data(lcd);

    if(power != dev->lcd_power) {
        dev->lcd_power = power;
        dev->ctrl->power_on(lcd, power);
    }

    return 0;
}

static int zk_st7789v_get_power(struct lcd_device *lcd)
{
    struct zk_st7789v_data *dev= lcd_get_data(lcd);

    return dev->lcd_power;
}

static int zk_st7789v_set_mode(struct lcd_device *lcd, struct fb_videomode *mode)
{
    return 0;
}

static struct lcd_ops zk_st7789v_ops = {
    .set_power = zk_st7789v_set_power,
    .get_power = zk_st7789v_get_power,
    .set_mode = zk_st7789v_set_mode,
};

static int zk_st7789v_probe(struct platform_device *pdev)
{
    struct zk_st7789v_data *dev;
    dev = kzalloc(sizeof(struct zk_st7789v_data), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->ctrl = pdev->dev.platform_data;
    if (dev->ctrl == NULL) {
        dev_info(&pdev->dev, "no platform data!");
        return -EINVAL;
    }

    dev_set_drvdata(&pdev->dev, dev);

    dev->lcd = lcd_device_register("zk_st7789v_slcd", &pdev->dev,
                       dev, &zk_st7789v_ops);
    if (IS_ERR(dev->lcd)) {
        dev_info(&pdev->dev, "lcd device register error\n");
        dev->lcd = NULL;
        return PTR_ERR(dev->lcd);
    } else {
        dev_info(&pdev->dev, "lcd device(zk_st7789v) register success\n");
    }

    dev->ctrl->power_on(dev->lcd, FB_BLANK_NORMAL);
    dev->lcd_power = FB_BLANK_NORMAL;

    return 0;
}

static int zk_st7789v_remove(struct platform_device *pdev)
{
    struct zk_st7789v_data *dev = dev_get_drvdata(&pdev->dev);

    dev->ctrl->power_on(dev->lcd, FB_BLANK_POWERDOWN);
    dev->lcd_power = FB_BLANK_POWERDOWN;

    lcd_device_unregister(dev->lcd);
    dev_set_drvdata(&pdev->dev, NULL);
    kfree(dev);

    return 0;
}

#ifdef CONFIG_PM
static int zk_st7789v_suspend(struct platform_device *pdev,
        pm_message_t state)
{
    return 0;
}

static int zk_st7789v_resume(struct platform_device *pdev)
{
    return 0;
}
#else
#define zk_st7789v_suspend NULL
#define zk_st7789v_resume  NULL
#endif

static struct platform_driver zk_st7789v_driver = {
    .driver     = {
        .name   = "zk_st7789v_slcd",
        .owner  = THIS_MODULE,
    },
    .probe      = zk_st7789v_probe,
    .remove     = zk_st7789v_remove,
    .suspend    = zk_st7789v_suspend,
    .resume     = zk_st7789v_resume,
};

static int __init zk_st7789v_init(void)
{
    return platform_driver_register(&zk_st7789v_driver);
}
module_init(zk_st7789v_init);

static void __exit zk_st7789v_exit(void)
{
    platform_driver_unregister(&zk_st7789v_driver);
}
module_exit(zk_st7789v_exit);

MODULE_DESCRIPTION("zk_st7789v lcd panel driver");
MODULE_LICENSE("GPL");
