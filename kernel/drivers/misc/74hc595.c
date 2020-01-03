/*
 * linux/drivers/misc/74hc595.c
 *
 * simple 74hc595 control driver
 * Platform driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com>
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
#include <linux/74hc595.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/rwlock.h>
#include <asm/atomic.h>

/*
 * ioctl commands
 */
#define SN74HC595_IOC_MAGIC         'J'
#define SN74HC595_IOC_WRITE         _IOW(SN74HC595_IOC_MAGIC, 1, int)   /* set output data */
#define SN74HC595_IOC_READ          _IOR(SN74HC595_IOC_MAGIC, 2, int)   /* get last output data */
#define SN74HC595_IOC_CLEAR         _IOW(SN74HC595_IOC_MAGIC, 3, int)   /* clear shift register */
#define SN74HC595_IOC_GET_OUTBITS   _IOR(SN74HC595_IOC_MAGIC, 4, int)   /* get output bits length */

#define SN74HC595_DEV_BASENAME      "sn74hc595_"

struct sn74hc595_dev_data {
    int id;
    unsigned int last_data;

    atomic_t opened;
    rwlock_t rwlock;
    struct device *dev;
    struct miscdevice miscdev;
    struct sn74hc595_platform_data *pdata;
};


static void sn74hc595_dev_senddata(struct sn74hc595_dev_data *dev_data, unsigned int data)
{
    struct sn74hc595_platform_data *pdata = dev_data->pdata;
    int i, bits;

    write_lock(&dev_data->rwlock);

    dev_data->last_data = data;
    bits = sizeof(data) * 8;
    data <<= (bits - pdata->out_bits);

    for (i = 0; i < pdata->out_bits; i++) {
        if(data & (1 << (bits - 1)))
            gpio_direction_output(pdata->data_pin, 1);
        else
            gpio_direction_output(pdata->data_pin, 0);

        //udelay(1);
        gpio_direction_output(pdata->sclk_pin, 1);
        //udelay(1);
        gpio_direction_output(pdata->sclk_pin, 0);
        data <<= 1;
    }

    //udelay(1);
    gpio_direction_output(pdata->data_pin, 0);
    gpio_direction_output(pdata->rclk_pin, 1);
    udelay(1);
    gpio_direction_output(pdata->rclk_pin, 0);

    write_unlock(&dev_data->rwlock);
}

static int sn74hc595_dev_open(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sn74hc595_dev_data *dev_data = container_of(miscdev, struct sn74hc595_dev_data, miscdev);

    if (atomic_read(&dev_data->opened)) {
        dev_err(dev_data->dev, "74HC595 dev%d is busy, cannot open\n", dev_data->id);
        return -EBUSY;
    }

    atomic_inc(&dev_data->opened);
    return 0;
}

static int sn74hc595_dev_release(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sn74hc595_dev_data *dev_data = container_of(miscdev, struct sn74hc595_dev_data, miscdev);

    atomic_dec(&dev_data->opened);
    return 0;
}

static ssize_t sn74hc595_dev_read(struct file *filp, char __user *buf, size_t size, loff_t *offp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sn74hc595_dev_data *dev_data = container_of(miscdev, struct sn74hc595_dev_data, miscdev);
    unsigned int read_bytes = (dev_data->pdata->out_bits + 7) / 8;

    if (size != read_bytes) {
        printk("WARNING: read size isn't equal to %d bytes\n", read_bytes);
    }

    read_lock(&dev_data->rwlock);
    if (copy_to_user(buf, (void *)&(dev_data->last_data), read_bytes)) {
        dev_err(dev_data->dev, "74HC595 dev%d read failed\n", dev_data->id);
        read_unlock(&dev_data->rwlock);
        return -EFAULT;
    }

    read_unlock(&dev_data->rwlock);
    return read_bytes;
}

static ssize_t sn74hc595_dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *offp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sn74hc595_dev_data *dev_data = container_of(miscdev, struct sn74hc595_dev_data, miscdev);
    unsigned int write_bytes = (dev_data->pdata->out_bits + 7) / 8;
    unsigned int data = 0;

    if (size != write_bytes) {
        printk("WARNING: write size isn't equal to %d bytes\n", write_bytes);
    }

    if (copy_from_user((void *)&data, buf, write_bytes)) {
        dev_err(dev_data->dev, "74HC595 dev%d write failed\n", dev_data->id);
        return -EFAULT;
    }

    sn74hc595_dev_senddata(dev_data, data);
    return write_bytes;
}

static long sn74hc595_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct sn74hc595_dev_data *dev_data = container_of(miscdev, struct sn74hc595_dev_data, miscdev);
    struct sn74hc595_platform_data *pdata = dev_data->pdata;
    void __user *argp = (void __user *)arg;
    unsigned int data;
    unsigned int bytes = (pdata->out_bits + 7) / 8;

    switch(cmd) {
    case SN74HC595_IOC_WRITE:
        if (copy_from_user((void *)&data, argp, bytes)) {
            dev_err(dev_data->dev, "IOCTL: 74HC595 dev%d write failed\n", dev_data->id);
            return -EFAULT;
        }
        sn74hc595_dev_senddata(dev_data, data);
        break;
    case SN74HC595_IOC_READ:
        read_lock(&dev_data->rwlock);
        if (copy_to_user(argp, (void *)&(dev_data->last_data), bytes)) {
            dev_err(dev_data->dev, "IOCTL: 74HC595 dev%d read failed\n", dev_data->id);
            read_unlock(&dev_data->rwlock);
            return -EFAULT;
        }
        read_unlock(&dev_data->rwlock);
        break;
    case SN74HC595_IOC_CLEAR:
        if (gpio_is_valid(pdata->sclr_pin)) {
            gpio_direction_output(pdata->sclr_pin, 0);
            write_lock(&dev_data->rwlock);
            dev_data->last_data = 0;
            write_unlock(&dev_data->rwlock);
        } else {
            sn74hc595_dev_senddata(dev_data, 0);
        }
        break;
    case SN74HC595_IOC_GET_OUTBITS:
        if (copy_to_user(argp, (void *)&(pdata->out_bits), sizeof(pdata->out_bits))) {
            dev_err(dev_data->dev, "IOCTL: 74HC595 dev%d get outbits failed\n", dev_data->id);
            return -EFAULT;
        }
        break;
    default:
        dev_err(dev_data->dev, "Not supported CMD:0x%x\n", cmd);
        return -EINVAL;
    }

    return pdata->out_bits;
}

static struct file_operations sn74hc595_dev_fops = {
    .owner   = THIS_MODULE,
    .open    = sn74hc595_dev_open,
    .release = sn74hc595_dev_release,
    .read    = sn74hc595_dev_read,
    .write   = sn74hc595_dev_write,
    .unlocked_ioctl = sn74hc595_dev_ioctl,
};

static void sn74hc595_dev_gpio_free(struct sn74hc595_dev_data *dev_data)
{
    struct sn74hc595_platform_data *pdata = dev_data->pdata;

    gpio_free(pdata->data_pin);
    gpio_free(pdata->rclk_pin);
    gpio_free(pdata->sclk_pin);
    gpio_free(pdata->sclr_pin);
    gpio_free(pdata->oe_pin);
}

static int sn74hc595_dev_gpio_init(struct sn74hc595_dev_data *dev_data)
{
    struct sn74hc595_platform_data *pdata = dev_data->pdata;

    if (gpio_is_valid(pdata->data_pin)) {
        if (gpio_request(pdata->data_pin, "data_pin") < 0) {
            pr_err("%s: data_pin request failed\n", __FUNCTION__);
            goto err_gpio_request1;
        }
        gpio_direction_output(pdata->data_pin, 0);
    } else {
        dev_err(dev_data->dev, "Invalid data_pin: %d\n", pdata->data_pin);
        goto err_gpio_request1;
    }

    if (gpio_is_valid(pdata->rclk_pin)) {
        if (gpio_request(pdata->rclk_pin, "rclk_pin") < 0) {
            pr_err("%s: rclk_pin request failed\n", __FUNCTION__);
            goto err_gpio_request2;
        }
        gpio_direction_output(pdata->rclk_pin, 0);
    } else {
        dev_err(dev_data->dev, "Invalid rclk_pin: %d\n", pdata->rclk_pin);
        goto err_gpio_request2;
    }

    if (gpio_is_valid(pdata->sclk_pin)) {
        if (gpio_request(pdata->sclk_pin, "sclk_pin") < 0) {
            pr_err("%s: sclk_pin request failed\n", __FUNCTION__);
            goto err_gpio_request3;
        }
        gpio_direction_output(pdata->sclk_pin, 0);
    } else {
        dev_err(dev_data->dev, "Invalid sclk_pin: %d\n", pdata->sclk_pin);
        goto err_gpio_request3;
    }

    if (gpio_is_valid(pdata->sclr_pin)) {
        if (gpio_request(pdata->sclr_pin, "sclr_pin") < 0) {
            pr_err("%s: sclr_pin request failed\n", __FUNCTION__);
            goto err_gpio_request4;
        }
        gpio_direction_output(pdata->sclr_pin, 0);
    }

    if (gpio_is_valid(pdata->oe_pin)) {
        if (gpio_request(pdata->oe_pin, "oe_pin") < 0) {
            pr_err("%s: oe_pin request failed\n", __FUNCTION__);
            goto err_gpio_request5;
        }
        gpio_direction_output(pdata->oe_pin, pdata->en_level);
    }

    return 0;

err_gpio_request5:
    gpio_free(pdata->sclr_pin);
err_gpio_request4:
    gpio_free(pdata->sclk_pin);
err_gpio_request3:
    gpio_free(pdata->rclk_pin);
err_gpio_request2:
    gpio_free(pdata->data_pin);
err_gpio_request1:
    return -ENODEV;
}

static int sn74hc595_dev_probe(struct platform_device *pdev)
{
    struct sn74hc595_dev_data *dev_data;
    char devname[32] = {0};

    if (!pdev->dev.platform_data) {
        dev_err(&pdev->dev, "platform_data cannot be NULL\n");
        goto exit;
    }

    dev_data = kzalloc(sizeof(*dev_data), GFP_KERNEL);
    if (!dev_data) {
        dev_err(&pdev->dev, "allocate memory failed\n");
        goto exit;
    }

    dev_data->id = pdev->id;
    dev_data->dev = &pdev->dev;
    dev_data->pdata = pdev->dev.platform_data;

    if (sn74hc595_dev_gpio_init(dev_data) < 0) {
        goto err_gpio_init;
    }

    snprintf(devname, sizeof(devname), "%s%d", SN74HC595_DEV_BASENAME, pdev->id);
    dev_data->miscdev.minor = MISC_DYNAMIC_MINOR;
    dev_data->miscdev.name  = devname;
    dev_data->miscdev.fops  = &sn74hc595_dev_fops;
    if (misc_register(&dev_data->miscdev) < 0) {
        dev_err(&pdev->dev, "misc_register failed\n");
        goto err_misc_register;
    }

    atomic_set(&dev_data->opened, 0);
    rwlock_init(&dev_data->rwlock);

    sn74hc595_dev_senddata(dev_data, dev_data->pdata->init_val);
    platform_set_drvdata(pdev, dev_data);

    dev_info(&pdev->dev, "driver probe success\n");
    return 0;

err_misc_register:
    sn74hc595_dev_gpio_free(dev_data);
err_gpio_init:
    kfree(dev_data);
exit:
    return -ENODEV;
}

static int sn74hc595_dev_remove(struct platform_device *pdev)
{
    struct sn74hc595_dev_data *dev_data = platform_get_drvdata(pdev);

    if (gpio_is_valid(dev_data->pdata->oe_pin))
        gpio_direction_output(dev_data->pdata->oe_pin, !(dev_data->pdata->en_level));

    misc_deregister(&dev_data->miscdev);
    sn74hc595_dev_gpio_free(dev_data);
    kfree(dev_data);
    return 0;
}

static int sn74hc595_dev_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct sn74hc595_dev_data *dev_data = platform_get_drvdata(pdev);

    sn74hc595_dev_senddata(dev_data, dev_data->pdata->init_val);
    if (gpio_is_valid(dev_data->pdata->oe_pin))
        gpio_direction_output(dev_data->pdata->oe_pin, !(dev_data->pdata->en_level));

    return 0;
}

static int sn74hc595_dev_resume(struct platform_device *pdev)
{
    struct sn74hc595_dev_data *dev_data = platform_get_drvdata(pdev);

    if (gpio_is_valid(dev_data->pdata->oe_pin))
        gpio_direction_output(dev_data->pdata->oe_pin, dev_data->pdata->en_level);

    sn74hc595_dev_senddata(dev_data, !dev_data->pdata->init_val);
    udelay(5000);
    sn74hc595_dev_senddata(dev_data, dev_data->pdata->init_val);
    return 0;
}

static struct platform_driver sn74hc595_dev_driver = {
    .driver = {
        .name  = "sn74hc595",
        .owner = THIS_MODULE,
    },
    .probe   = sn74hc595_dev_probe,
    .remove  = sn74hc595_dev_remove,
    .suspend = sn74hc595_dev_suspend,
    .resume  = sn74hc595_dev_resume,
};

static int __init sn74hc595_dev_init(void)
{
    return platform_driver_register(&sn74hc595_dev_driver);
}

static void __exit sn74hc595_dev_exit(void)
{
    platform_driver_unregister(&sn74hc595_dev_driver);
}

module_init(sn74hc595_dev_init);
module_exit(sn74hc595_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<qiuwei.wang@ingenic.com>");
MODULE_DESCRIPTION("74HC595 device driver");
MODULE_ALIAS("Platform: 74hc595");
