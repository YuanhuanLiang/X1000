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

struct dk_st7789h2_data {
	int lcd_power;
	struct lcd_device *lcd;
	struct lcd_platform_data *ctrl;
};

static int dk_st7789h2_set_power(struct lcd_device *lcd, int power)
{
	struct dk_st7789h2_data *dev= lcd_get_data(lcd);

	if(power != dev->lcd_power) {
		dev->lcd_power = power;
		dev->ctrl->power_on(lcd, power);
	}

	return 0;
}

static int dk_st7789h2_get_power(struct lcd_device *lcd)
{
	struct dk_st7789h2_data *dev= lcd_get_data(lcd);

	return dev->lcd_power;
}

static int dk_st7789h2_set_mode(struct lcd_device *lcd, struct fb_videomode *mode)
{
	return 0;
}

static struct lcd_ops dk_st7789h2_ops = {
	.set_power = dk_st7789h2_set_power,
	.get_power = dk_st7789h2_get_power,
	.set_mode = dk_st7789h2_set_mode,
};

static int dk_st7789h2_probe(struct platform_device *pdev)
{
	int ret;
	struct dk_st7789h2_data *dev;
	dev = kzalloc(sizeof(struct dk_st7789h2_data), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->ctrl = pdev->dev.platform_data;
	if (dev->ctrl == NULL) {
		dev_info(&pdev->dev, "no platform data!");
		return -EINVAL;
	}

	dev_set_drvdata(&pdev->dev, dev);

	dev->lcd = lcd_device_register("dk_st7789h2_slcd", &pdev->dev,
				       dev, &dk_st7789h2_ops);
	if (IS_ERR(dev->lcd)) {
		dev_info(&pdev->dev, "lcd device register error: %d\n", ret);
		dev->lcd = NULL;
		return PTR_ERR(dev->lcd);
	} else {
		dev_info(&pdev->dev, "lcd device(dk_st7789h2) register success\n");
	}

	dev->ctrl->power_on(dev->lcd, FB_BLANK_NORMAL);
	dev->lcd_power = FB_BLANK_NORMAL;

	return 0;
}

static int dk_st7789h2_remove(struct platform_device *pdev)
{
	struct dk_st7789h2_data *dev = dev_get_drvdata(&pdev->dev);

	dev->ctrl->power_on(dev->lcd, FB_BLANK_POWERDOWN);
	dev->lcd_power = FB_BLANK_POWERDOWN;

	lcd_device_unregister(dev->lcd);
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(dev);

	return 0;
}

#ifdef CONFIG_PM
static int dk_st7789h2_suspend(struct platform_device *pdev,
		pm_message_t state)
{
	return 0;
}

static int dk_st7789h2_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define dk_st7789h2_suspend	NULL
#define dk_st7789h2_resume	NULL
#endif

static struct platform_driver dk_st7789h2_driver = {
	.driver		= {
		.name	= "dk_st7789h2_slcd",
		.owner	= THIS_MODULE,
	},
	.probe		= dk_st7789h2_probe,
	.remove		= dk_st7789h2_remove,
	.suspend	= dk_st7789h2_suspend,
	.resume		= dk_st7789h2_resume,
};

static int __init dk_st7789h2_init(void)
{
	return platform_driver_register(&dk_st7789h2_driver);
}
module_init(dk_st7789h2_init);

static void __exit dk_st7789h2_exit(void)
{
	platform_driver_unregister(&dk_st7789h2_driver);
}
module_exit(dk_st7789h2_exit);

MODULE_DESCRIPTION("dk_st7789h2 lcd panel driver");
MODULE_LICENSE("GPL");
