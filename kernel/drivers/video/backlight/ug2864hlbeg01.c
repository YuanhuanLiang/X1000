/*
 *  LCD control code for  ug2864hlbeg01
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

struct ug2864hlbeg01_data {
    int lcd_power;
    struct lcd_device *lcd;
    struct lcd_platform_data *ctrl;
};

static int ug2864hlbeg01_set_power(struct lcd_device *lcd, int power)
{
    struct ug2864hlbeg01_data *dev = lcd_get_data(lcd);

    if(power != dev->lcd_power) {
        dev->lcd_power = power;
        dev->ctrl->power_on(lcd, power);
    }
    return 0;
}

static int ug2864hlbeg01_get_power(struct lcd_device *lcd)
{
    struct ug2864hlbeg01_data *dev = lcd_get_data(lcd);

    return dev->lcd_power;
}

static int ug2864hlbeg01_set_mode(struct lcd_device *lcd, struct fb_videomode *mode)
{
    return 0;
}

static struct lcd_ops ug2864hlbeg01_ops = {
    .set_power = ug2864hlbeg01_set_power,
    .get_power = ug2864hlbeg01_get_power,
    .set_mode = ug2864hlbeg01_set_mode,
};

static int ug2864hlbeg01_probe(struct platform_device *pdev)
{
    struct ug2864hlbeg01_data *dev;
    dev = kzalloc(sizeof(struct ug2864hlbeg01_data), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->ctrl = pdev->dev.platform_data;
    if (dev->ctrl == NULL) {
        dev_info(&pdev->dev, "no platform data!");
        return -EINVAL;
    }

    dev_set_drvdata(&pdev->dev, dev);

    dev->lcd = lcd_device_register(" ug2864hlbeg01_slcd", &pdev->dev,
            dev, &ug2864hlbeg01_ops);
    if (IS_ERR(dev->lcd)) {
        dev_info(&pdev->dev, "lcd device register error\n");
        dev->lcd = NULL;
        return PTR_ERR(dev->lcd);
    } else {
        dev_info(&pdev->dev, "lcd device( ug2864hlbeg01) register success\n");
    }

    dev->ctrl->power_on(dev->lcd, FB_BLANK_NORMAL);
    dev->lcd_power = FB_BLANK_NORMAL;

    return 0;
}

static int ug2864hlbeg01_remove(struct platform_device *pdev)
{
    struct ug2864hlbeg01_data *dev = dev_get_drvdata(&pdev->dev);

    dev->ctrl->power_on(dev->lcd, FB_BLANK_POWERDOWN);
    dev->lcd_power = FB_BLANK_POWERDOWN;

    lcd_device_unregister(dev->lcd);
    dev_set_drvdata(&pdev->dev, NULL);
    kfree(dev);

    return 0;
}

static struct platform_driver ug2864hlbeg01_driver = {
    .driver = {
        .name = "ug2864hlbeg01_slcd",
        .owner = THIS_MODULE,
    },
    .probe = ug2864hlbeg01_probe,
    .remove = ug2864hlbeg01_remove,
};

static int __init  ug2864hlbeg01_init(void)
{
	return platform_driver_register(& ug2864hlbeg01_driver);
}
module_init(ug2864hlbeg01_init);

static void __exit  ug2864hlbeg01_exit(void)
{
	platform_driver_unregister(& ug2864hlbeg01_driver);
}
module_exit(ug2864hlbeg01_exit);

MODULE_DESCRIPTION(" ug2864hlbeg01hlbeg01 lcd panel driver");
MODULE_LICENSE("GPL");
