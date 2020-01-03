/*
 * linux/drivers/misc/sgm42507.c
 *
 * sgm42507  driver
 * Platform driver support for Ingenic X1000 SoC.
 *
 * Copyright 2017, Monk <rongjin.su@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/sgm42507.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/io.h>


#define DRV_NAME                        "sgm42507"

#define SGM42507_IOC_MAGIC              'M'
#define SGM42507_IOC_POWER              _IOW(SGM42507_IOC_MAGIC, 1, int)   /* set power */
#define SGM42507_IOC_EN                 _IOW(SGM42507_IOC_MAGIC, 2, int)   /* enable dirver */
#define SGM42507_IOC_DIR                _IOW(SGM42507_IOC_MAGIC, 3, int)   /* set out direction A,B*/

#define SGM42507_DEV_BASENAME           "sgm42507"


#define SGM42507_DRI_ENABLE             1
#define SGM42507_DRI_DISABLE            0

#define SGM42507_POWER_ON               1
#define SGM42507_POWER_OFF              0

#define SGM42507_DIRECTION_A            0
#define SGM42507_DIRECTION_B            1

struct sgm42507_dev_data {
    int id;
    int pwr_en_pin;
    int dri_en_pin;
    int direction_pin;

    struct device *dev;
    struct miscdevice miscdev;

    struct sgm42507_platform_data *pdata;
};


static void sgm42507_power(struct sgm42507_dev_data* dev_data, unsigned char onoff)
{
    if (gpio_is_valid(dev_data->pwr_en_pin))
        gpio_direction_output(dev_data->pwr_en_pin,onoff);
}

static void sgm42507_enable(struct sgm42507_dev_data* dev_data, unsigned char en)
{
    if (gpio_is_valid(dev_data->dri_en_pin))
        gpio_direction_output(dev_data->dri_en_pin,en);
}

static void sgm42507_direction(struct sgm42507_dev_data* dev_data, unsigned char dir)
{
    if (gpio_is_valid(dev_data->direction_pin))
        gpio_direction_output(dev_data->direction_pin,dir);
}


static int sgm42507_gpio_init(struct sgm42507_dev_data* dev_data)
{
    int err;

    struct sgm42507_platform_data* pdata = dev_data->pdata;

    dev_data->pwr_en_pin    = pdata->pwr_en_pin;
    dev_data->dri_en_pin    = pdata->dri_en_pin;
    dev_data->direction_pin = pdata->direction_pin;


    if (gpio_is_valid(dev_data->pwr_en_pin)) {
        err = gpio_request(dev_data->pwr_en_pin, "motor sgm42507 power enable");
        if (err < 0) {
            dev_err(dev_data->dev," sgm42507: %s enable[%d] pwr_en_pin failed.\n",
                    __func__, dev_data->pwr_en_pin);
            goto err_pwr_en;
        }
        sgm42507_power(dev_data, SGM42507_POWER_OFF);
    }

    if (gpio_is_valid(dev_data->dri_en_pin)) {
        err = gpio_request(dev_data->dri_en_pin, "motor sgm42507 dirver enable");
        if (err < 0) {
            dev_err(dev_data->dev," sgm42507: %s enable[%d] dri_en_pin failed.\n",
                    __func__, dev_data->dri_en_pin);
            goto err_dri_en;
        }
        sgm42507_enable(dev_data, SGM42507_DRI_DISABLE);
    }

    if (gpio_is_valid(dev_data->direction_pin)) {
        err = gpio_request(dev_data->direction_pin, "motor sgm42507 direction");
        if (err < 0) {
            dev_err(dev_data->dev," sgm42507: %s enable[%d] direction_pin failed.\n",
                    __func__, dev_data->direction_pin);
            goto err_direction;
        }
    }


    return 0;

err_direction:
    gpio_free(dev_data->dri_en_pin);
err_dri_en:
    gpio_free(dev_data->pwr_en_pin);
err_pwr_en:
    return -1;
}

static void sgm42507_gpio_free(struct sgm42507_dev_data* dev_data)
{
    if (gpio_is_valid(dev_data->pwr_en_pin))
        gpio_free(dev_data->pwr_en_pin);

    if (gpio_is_valid(dev_data->dri_en_pin))
        gpio_free(dev_data->dri_en_pin);

    if (gpio_is_valid(dev_data->direction_pin))
        gpio_free(dev_data->direction_pin);
}


static int sgm42507_dev_open(struct inode *inode, struct file *filp)
{
    /*
    struct miscdevice *miscdev = filp->private_data;
    struct sgm42507_dev_data *dev_data = container_of(miscdev, struct sgm42507_dev_data, miscdev);
    */
    return 0;
}

static int sgm42507_dev_release(struct inode *inode, struct file *filp)
{
    /*
    struct miscdevice *miscdev = filp->private_data;
    struct sgm42507_dev_data *dev_data = container_of(miscdev, struct sgm42507_dev_data, miscdev);
    */
    return 0;
}


static long sgm42507_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sgm42507_dev_data *dev_data = container_of(miscdev, struct sgm42507_dev_data, miscdev);

    unsigned char var = (unsigned char)arg;
    switch(cmd) {
        case SGM42507_IOC_POWER:
            sgm42507_power(dev_data, var);
            break;

        case SGM42507_IOC_EN:
            sgm42507_enable(dev_data, var);
            break;

        case SGM42507_IOC_DIR:
            sgm42507_direction(dev_data, var);
            break;

        default:
            dev_err(dev_data->dev, "Not supported CMD:0x%x\n", cmd);
            return -EINVAL;
    }
    return 0;
}


static struct file_operations sgm42507_dev_fops = {
    .owner          = THIS_MODULE,
    .open           = sgm42507_dev_open,
    .release        = sgm42507_dev_release,
    .unlocked_ioctl = sgm42507_dev_ioctl,
};

static int sgm42507_probe(struct platform_device *pdev)
{
    int err;
    char dev_name[32];
    struct sgm42507_dev_data *dev_data;

    dev_data = kzalloc(sizeof(struct sgm42507_dev_data), GFP_KERNEL);
    if(dev_data == NULL) {
        dev_err(dev_data->dev, "Failed to malloc motor_device\n");
        err = -ENOMEM;
        goto err_dev_data_kzalloc;
    }

    dev_data->dev   = &pdev->dev;
    dev_data->pdata = dev_get_platdata(&pdev->dev);
    dev_data->id    = dev_data->pdata->id;

    err = sprintf(dev_name,"%s-%d",SGM42507_DEV_BASENAME,dev_data->id);
    if (err < 0) {
        dev_err(dev_data->dev, "sprintf dev name failed\n");
        goto err_sprintf_dev_name;
    }

    dev_data->miscdev.minor = MISC_DYNAMIC_MINOR;
    dev_data->miscdev.name  = dev_name;
    dev_data->miscdev.fops  = &sgm42507_dev_fops;
    if (misc_register(&dev_data->miscdev) < 0) {
        dev_err(dev_data->dev, "misc_register failed\n");
        goto err_misc_register;
    }

    platform_set_drvdata(pdev, dev_data);

    err = sgm42507_gpio_init(dev_data);
    if (err < 0) {
        dev_err(dev_data->dev, "Failed to init gpio.\n");
        goto err_gpio_init;
    }

    return 0;

err_gpio_init:
    misc_deregister(&dev_data->miscdev);
err_misc_register:
err_sprintf_dev_name:
    kfree(dev_data);
err_dev_data_kzalloc:
    return err;

}
static int sgm42507_remove(struct platform_device *pdev)
{
    struct sgm42507_dev_data *dev_data = platform_get_drvdata(pdev);

    sgm42507_gpio_free(dev_data);
    misc_deregister(&dev_data->miscdev);
    kfree(dev_data);
    return 0;
}

static int sgm42507_suspend(struct platform_device *pdev, pm_message_t state)
{

    return 0;
}
static int sgm42507_resume(struct platform_device *pdev)
{

    return 0;
}

static struct platform_driver sgm42507_driver = {
        .driver = {
                .name = DRV_NAME,
                .owner = THIS_MODULE,
        },
        .probe = sgm42507_probe,
        .remove = sgm42507_remove,
        .suspend = sgm42507_suspend,
        .resume = sgm42507_resume,
};

static int __init sgm42507_init(void)
{
    return platform_driver_register(&sgm42507_driver);
}

static void __exit sgm42507_exit(void)
{
    platform_driver_unregister(&sgm42507_driver);
}

module_init(sgm42507_init);
module_exit(sgm42507_exit);

MODULE_AUTHOR("Monk Su <rongjin.su@ingenic.com>");
MODULE_DESCRIPTION("sgm42507 driver");
MODULE_LICENSE("GPL");