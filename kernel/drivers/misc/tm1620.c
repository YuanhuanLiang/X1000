/*
 * linux/drivers/misc/tm1620.c
 *
 *  TM1620 Digital LED control driver
 * Platform driver support for Ingenic X1000 SoC.
 *
 * Copyright 2018, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/tm1620.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/rwlock.h>


/**
 * ioctl commands
 */
#define TM1620_IOC_MAGIC                'M'
#define TM1620_IOC_SET_ALL_DIGITAL      _IOW(TM1620_IOC_MAGIC, 0, int)
#define TM1620_IOC_SET_A_DIGITAL        _IOW(TM1620_IOC_MAGIC, 1, int)
#define TM1620_IOC_CLOSE_ALL_DIGITAL    _IOW(TM1620_IOC_MAGIC, 2, int)
#define TM1620_IOC_OPEN_ALL_DIGITAL     _IOW(TM1620_IOC_MAGIC, 3, int)


struct tm1620_grid {
    uint8_t id;
    uint8_t display_data;
};

struct tm1620_drvdata {
    uint8_t display_data[6];
    struct tm1620_grid grid;
    struct device *dev;
    struct mutex lock;
    struct miscdevice miscdev;
    struct tm1620_platform_data *pdata;
};


static void tm1620_send_data(struct tm1620_drvdata *tm1620, uint8_t data)
{
    struct tm1620_platform_data *pdata = tm1620->pdata;
    int i;

    gpio_set_value(pdata->stb_pin, 0);

    for (i = 0; i < 8; i++) {
        gpio_set_value(pdata->clk_pin, 0);
        if (data & (1 << i))
            gpio_set_value(pdata->dio_pin, 1);
        else
            gpio_set_value(pdata->dio_pin, 0);
        udelay(1);
        gpio_set_value(pdata->clk_pin, 1);
        udelay(1);
    }
}

static void tm1620_all_display(struct tm1620_drvdata *tm1620, uint8_t *display_data)
{
    struct tm1620_platform_data *pdata = tm1620->pdata;
    int i;

    gpio_set_value(pdata->stb_pin, 1);
    gpio_set_value(pdata->clk_pin, 1);
    gpio_set_value(pdata->dio_pin, 1);

    tm1620_send_data(tm1620, W_ADDR_AUTO_ADD);
    gpio_set_value(pdata->stb_pin, 1);

    tm1620_send_data(tm1620, S_ADDR_TO_00H);
    for (i = 0; i < 12; i++)
        tm1620_send_data(tm1620, tm1620->display_data[i/2]);
    gpio_set_value(pdata->stb_pin, 1);

    tm1620_send_data(tm1620, DISPLAY_OPEN | PULSE_WIDTH_10_16);
    gpio_set_value(pdata->stb_pin, 1);
}

static void tm1620_grid_display(struct tm1620_drvdata *tm1620, uint8_t grid, uint8_t display_data)
{
    struct tm1620_platform_data *pdata = tm1620->pdata;
    uint8_t addr;

    addr = 0xC0 | ((grid - 1) << 1);

    gpio_set_value(pdata->stb_pin, 1);
    gpio_set_value(pdata->clk_pin, 1);
    gpio_set_value(pdata->dio_pin, 1);

    tm1620_send_data(tm1620, W_ADDR_FIXED);
    gpio_set_value(pdata->stb_pin, 1);

    tm1620_send_data(tm1620, addr);
    tm1620_send_data(tm1620, display_data);
    gpio_set_value(pdata->stb_pin, 1);

    tm1620_send_data(tm1620, DISPLAY_OPEN | PULSE_WIDTH_10_16);
    gpio_set_value(pdata->stb_pin, 1);
}

static int tm1620_dev_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int tm1620_dev_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long tm1620_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct tm1620_drvdata *tm1620 = container_of(miscdev, struct tm1620_drvdata, miscdev);
    void __user *arg_p = (void __user *)arg;

    switch(cmd) {
    case TM1620_IOC_SET_ALL_DIGITAL:
        mutex_lock(&tm1620->lock);
        if (copy_from_user((void *)tm1620->display_data, arg_p,
                    sizeof(tm1620->display_data))) {
            mutex_unlock(&tm1620->lock);
            return -EIO;
        }
        tm1620_all_display(tm1620, tm1620->display_data);
        mutex_unlock(&tm1620->lock);
        break;
    case TM1620_IOC_SET_A_DIGITAL:
        mutex_lock(&tm1620->lock);
        if (copy_from_user((void *)&tm1620->grid, arg_p,
                    sizeof(struct tm1620_grid))) {
            mutex_unlock(&tm1620->lock);
            return -EIO;
        }
        tm1620_grid_display(tm1620, tm1620->grid.id, tm1620->grid.display_data);
        mutex_unlock(&tm1620->lock);
        break;
    case TM1620_IOC_CLOSE_ALL_DIGITAL:
        mutex_lock(&tm1620->lock);
        tm1620_send_data(tm1620, DISPLAY_CLOSE);
        gpio_set_value(tm1620->pdata->stb_pin, 1);
        mutex_unlock(&tm1620->lock);
        break;
    case TM1620_IOC_OPEN_ALL_DIGITAL:
        mutex_lock(&tm1620->lock);
        tm1620_send_data(tm1620, DISPLAY_OPEN | PULSE_WIDTH_10_16);
        gpio_set_value(tm1620->pdata->stb_pin, 1);
        mutex_unlock(&tm1620->lock);
        break;
    default:
        dev_err(tm1620->dev, "Not supported CMD:0x%x\n", cmd);
        return -EINVAL;
    }

    return 0;
}

static struct file_operations tm1620_dev_fops = {
    .owner = THIS_MODULE,
    .open = tm1620_dev_open,
    .release = tm1620_dev_release,
    .unlocked_ioctl = tm1620_dev_ioctl,
};

static int tm1620_dev_gpio_init(struct tm1620_drvdata *tm1620)
{
    struct tm1620_platform_data *pdata = tm1620->pdata;
    int retval;

    if (gpio_is_valid(pdata->stb_pin)) {
        retval = gpio_request(pdata->stb_pin, "stb_pin");
        if (retval < 0) {
            dev_err(tm1620->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->stb_pin, retval);
            goto err_gpio_request1;
        }
        gpio_direction_output(pdata->stb_pin, 1);
    } else {
        dev_err(tm1620->dev, "Invalid stb_pin: %d\n", pdata->stb_pin);
        return -ENODEV;
    }

    if (gpio_is_valid(pdata->clk_pin)) {
        retval = gpio_request(pdata->clk_pin, "clk_pin");
        if (retval < 0) {
            dev_err(tm1620->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->clk_pin, retval);
            goto err_gpio_request2;
        }
        gpio_direction_output(pdata->clk_pin, 1);
    } else {
        dev_err(tm1620->dev, "Invalid clk_pin: %d\n", pdata->clk_pin);
        retval = -ENODEV;
        goto err_gpio_request2;
    }

    if (gpio_is_valid(pdata->dio_pin)) {
        retval = gpio_request(pdata->dio_pin, "dio_pin");
        if (retval < 0) {
            dev_err(tm1620->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->dio_pin, retval);
            goto err_gpio_request3;
        }
        gpio_direction_output(pdata->dio_pin, 1);
    } else {
        dev_err(tm1620->dev, "Invalid dio_pin: %d\n", pdata->dio_pin);
        retval = -ENODEV;
        goto err_gpio_request2;
    }

    return 0;

err_gpio_request3:
    gpio_free(pdata->clk_pin);
err_gpio_request2:
    gpio_free(pdata->stb_pin);
err_gpio_request1:
    return retval;
}

static void tm1620_dev_gpio_free(struct tm1620_drvdata *tm1620)
{
    struct tm1620_platform_data *pdata = tm1620->pdata;

    gpio_free(pdata->stb_pin);
    gpio_free(pdata->clk_pin);
    gpio_free(pdata->dio_pin);
}

static int tm1620_dev_probe(struct platform_device *pdev)
{
    struct tm1620_drvdata *tm1620;
    int retval;

    if (!pdev->dev.platform_data) {
        dev_dbg(&pdev->dev, "dev.platform_data cannot be NULL\n");
        return -ENODEV;
    }

    tm1620 = kzalloc(sizeof(struct tm1620_drvdata), GFP_KERNEL);
    if (!tm1620) {
        dev_err(&pdev->dev, "Failed to allocate drvdata memory\n");
        return -ENOMEM;
    }

    tm1620->dev = &pdev->dev;
    tm1620->pdata = pdev->dev.platform_data;

    retval = tm1620_dev_gpio_init(tm1620);
    if (retval < 0)
        goto err_gpio_init;

    tm1620_send_data(tm1620, DISPLAY_6GRID_8SEG);
    gpio_set_value(tm1620->pdata->stb_pin, 1);

    tm1620_send_data(tm1620, DISPLAY_CLOSE);
    gpio_set_value(tm1620->pdata->stb_pin, 1);

    tm1620->miscdev.minor = MISC_DYNAMIC_MINOR;
    tm1620->miscdev.name = pdev->name;
    tm1620->miscdev.fops = &tm1620_dev_fops;
    retval = misc_register(&tm1620->miscdev);
    if (retval < 0) {
        dev_err(&pdev->dev, "misc_register() failed\n");
        goto err_misc_register;
    }

    mutex_init(&tm1620->lock);
    platform_set_drvdata(pdev, tm1620);

    return 0;

err_misc_register:
    tm1620_dev_gpio_free(tm1620);
err_gpio_init:
    kfree(tm1620);
    return retval;
}

static int tm1620_dev_remove(struct platform_device *pdev)
{
    struct tm1620_drvdata *tm1620 = platform_get_drvdata(pdev);

    misc_deregister(&tm1620->miscdev);
    tm1620_dev_gpio_free(tm1620);
    kfree(tm1620);
    return 0;
}
static int tm1620_dev_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct tm1620_drvdata *tm1620 = platform_get_drvdata(pdev);

    tm1620_send_data(tm1620, DISPLAY_CLOSE);
    gpio_set_value(tm1620->pdata->stb_pin, 1);
    return 0;
}

static int tm1620_dev_resume(struct platform_device *pdev)
{
    struct tm1620_drvdata *tm1620 = platform_get_drvdata(pdev);

    tm1620_send_data(tm1620, DISPLAY_OPEN | PULSE_WIDTH_10_16);
    gpio_set_value(tm1620->pdata->stb_pin, 1);
    return 0;
}

static struct platform_driver tm1620_driver = {
    .driver = {
        .name  = "tm1620",
        .owner = THIS_MODULE,
    },
    .probe   = tm1620_dev_probe,
    .remove  = tm1620_dev_remove,
    .suspend = tm1620_dev_suspend,
    .resume  = tm1620_dev_resume,
};

static int __init tm1620_dev_init(void)
{
    return platform_driver_register(&tm1620_driver);
}

static void __exit tm1620_dev_exit(void)
{
    platform_driver_unregister(&tm1620_driver);
}

module_init(tm1620_dev_init);
module_exit(tm1620_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<qiuwei.wang@ingenic.com>");
MODULE_DESCRIPTION("TM1620 device driver");
MODULE_ALIAS("Platform: tm1620");
