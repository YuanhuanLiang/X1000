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
#include <linux/platform_device.h>

struct frd397bp4511_data {
    int lcd_power;
    struct lcd_device *lcd;
    struct lcd_platform_data *ctrl;
};

static int frd397bp4511_set_power(struct lcd_device *lcd, int power)
{
    struct frd397bp4511_data *dev= lcd_get_data(lcd);

    if(power != dev->lcd_power) {
        dev->lcd_power = power;
        dev->ctrl->power_on(lcd, power);
    }

    return 0;
}

static int frd397bp4511_get_power(struct lcd_device *lcd)
{
    struct frd397bp4511_data *dev= lcd_get_data(lcd);

    return dev->lcd_power;
}

static int frd397bp4511_set_mode(struct lcd_device *lcd, struct fb_videomode *mode)
{
    return 0;
}

static struct lcd_ops frd397bp4511_ops = {
    .set_power = frd397bp4511_set_power,
    .get_power = frd397bp4511_get_power,
    .set_mode = frd397bp4511_set_mode,
};

static int frd397bp4511_probe(struct platform_device *pdev)
{
    struct frd397bp4511_data *dev;
    dev = kzalloc(sizeof(struct frd397bp4511_data), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->ctrl = pdev->dev.platform_data;
    if (dev->ctrl == NULL) {
        dev_info(&pdev->dev, "no platform data!");
        return -EINVAL;
    }

    dev_set_drvdata(&pdev->dev, dev);

    dev->lcd = lcd_device_register("frd397bp4511_slcd", &pdev->dev,
                       dev, &frd397bp4511_ops);
    if (IS_ERR(dev->lcd)) {
        dev_info(&pdev->dev, "lcd device register error\n");
        dev->lcd = NULL;
        return PTR_ERR(dev->lcd);
    } else {
        dev_info(&pdev->dev, "lcd device(frd397bp4511) register success\n");
    }

    dev->ctrl->power_on(dev->lcd, FB_BLANK_NORMAL);
    dev->lcd_power = FB_BLANK_NORMAL;

    return 0;
}

static int frd397bp4511_remove(struct platform_device *pdev)
{
    struct frd397bp4511_data *dev = dev_get_drvdata(&pdev->dev);

    dev->ctrl->power_on(dev->lcd, FB_BLANK_POWERDOWN);
    dev->lcd_power = FB_BLANK_POWERDOWN;

    lcd_device_unregister(dev->lcd);
    dev_set_drvdata(&pdev->dev, NULL);
    kfree(dev);

    return 0;
}

#ifdef CONFIG_PM
static int frd397bp4511_suspend(struct platform_device *pdev,
        pm_message_t state)
{
    return 0;
}

static int frd397bp4511_resume(struct platform_device *pdev)
{
    return 0;
}
#else
#define frd397bp4511_suspend NULL
#define frd397bp4511_resume  NULL
#endif

static struct platform_driver frd397bp4511_driver = {
    .driver     = {
        .name   = "frd397bp4511_slcd",
        .owner  = THIS_MODULE,
    },
    .probe      = frd397bp4511_probe,
    .remove     = frd397bp4511_remove,
    .suspend    = frd397bp4511_suspend,
    .resume     = frd397bp4511_resume,
};

static int __init frd397bp4511_init(void)
{
    return platform_driver_register(&frd397bp4511_driver);
}
module_init(frd397bp4511_init);

static void __exit frd397bp4511_exit(void)
{
    platform_driver_unregister(&frd397bp4511_driver);
}
module_exit(frd397bp4511_exit);

MODULE_DESCRIPTION("frd397bp4511 lcd panel driver");
MODULE_LICENSE("GPL");
