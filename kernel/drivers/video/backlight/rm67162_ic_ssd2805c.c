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

struct rm67162_ic_ssd2805c_data {
	int lcd_power;
	struct lcd_device *lcd;
	struct lcd_platform_data *ctrl;
};

static int rm67162_ic_ssd2805c_set_power(struct lcd_device *lcd, int power)
{
	struct rm67162_ic_ssd2805c_data *dev= lcd_get_data(lcd);

	if(power != dev->lcd_power) {
		dev->lcd_power = power;
		dev->ctrl->power_on(lcd, power);
	}

	return 0;
}

static int rm67162_ic_ssd2805c_get_power(struct lcd_device *lcd)
{
	struct rm67162_ic_ssd2805c_data *dev= lcd_get_data(lcd);

	return dev->lcd_power;
}

static int rm67162_ic_ssd2805c_set_mode(struct lcd_device *lcd, struct fb_videomode *mode)
{
	return 0;
}

static struct lcd_ops rm67162_ic_ssd2805c_ops = {
	.set_power = rm67162_ic_ssd2805c_set_power,
	.get_power = rm67162_ic_ssd2805c_get_power,
	.set_mode = rm67162_ic_ssd2805c_set_mode,
};

static int rm67162_ic_ssd2805c_probe(struct platform_device *pdev)
{
	int ret;
	struct rm67162_ic_ssd2805c_data *dev;
	dev = kzalloc(sizeof(struct rm67162_ic_ssd2805c_data), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->ctrl = pdev->dev.platform_data;
	if (dev->ctrl == NULL) {
		dev_info(&pdev->dev, "no platform data!");
		return -EINVAL;
	}

	dev_set_drvdata(&pdev->dev, dev);

	dev->lcd = lcd_device_register("rm67162_ic_ssd2805c", &pdev->dev,
				       dev, &rm67162_ic_ssd2805c_ops);
	if (IS_ERR(dev->lcd)) {
		ret = PTR_ERR(dev->lcd);
		dev->lcd = NULL;
		dev_info(&pdev->dev, "lcd device register error: %d\n", ret);
	} else {
		dev_info(&pdev->dev, "lcd device register success\n");
	}

	dev->ctrl->power_on(dev->lcd, FB_BLANK_NORMAL);
	dev->lcd_power = FB_BLANK_NORMAL;

	return 0;
}

static int rm67162_ic_ssd2805c_remove(struct platform_device *pdev)
{
	struct rm67162_ic_ssd2805c_data *dev = dev_get_drvdata(&pdev->dev);

	dev->ctrl->power_on(dev->lcd, FB_BLANK_POWERDOWN);
	dev->lcd_power = FB_BLANK_POWERDOWN;

	lcd_device_unregister(dev->lcd);
	dev_set_drvdata(&pdev->dev, NULL);
	kfree(dev);

	return 0;
}


static struct platform_driver rm67162_ic_ssd2805c_driver = {
	.driver		= {
		.name	= "rm67162_ic_ssd2805c",
		.owner	= THIS_MODULE,
	},
	.probe		= rm67162_ic_ssd2805c_probe,
	.remove		= rm67162_ic_ssd2805c_remove,

};

static int __init rm67162_ic_ssd2805c_init(void)
{
	return platform_driver_register(&rm67162_ic_ssd2805c_driver);
}

static void __exit rm67162_ic_ssd2805c_exit(void)
{
	platform_driver_unregister(&rm67162_ic_ssd2805c_driver);
}

module_init(rm67162_ic_ssd2805c_init);
module_exit(rm67162_ic_ssd2805c_exit);
MODULE_LICENSE("GPL");
