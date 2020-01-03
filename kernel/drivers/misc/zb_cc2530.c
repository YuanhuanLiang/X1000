/*
 * linux/drivers/misc/zb_cc2530.c
 *
 * simple uart zigbee control driver
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
#include <linux/init.h>
#include <linux/zb_cc2530.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <asm/atomic.h>
#include <linux/ioctl.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/signal.h>
#include <asm-generic/siginfo.h>
/*
 * ioctl commands
 */
#define ZB_CC2530_IOC_MAGIC             'Z'
#define ZB_CC2530_IOC_G_MSG             _IOR(ZB_CC2530_IOC_MAGIC, 0, int)   /* get massage */
#define ZB_CC2530_IOC_POWER             _IOW(ZB_CC2530_IOC_MAGIC, 1, int)   /* set power */
#define ZB_CC2530_IOC_RST               _IOW(ZB_CC2530_IOC_MAGIC, 2, int)   /* send reset sig to 2530 by io rst_pin */
#define ZB_CC2530_IOC_WAKE              _IOW(ZB_CC2530_IOC_MAGIC, 3, int)   /* send wake up sig to 2530 by io wake_pin */



#define ZB_CC2530_DEV_BASENAME          "zigbee"

#define POWER_ON                        1
#define POWER_OFF                       0


struct zb_cc2530_dev_data {
    int irq;

    bool is_power_en;
    bool is_disable_int;

    atomic_t opened;
    struct device *dev;
    struct miscdevice miscdev;

    wait_queue_head_t       wait;
    struct work_struct      work;
    struct workqueue_struct *workqueue;

};

static inline void zb_cc2530_enable_int(struct zb_cc2530_dev_data *dev_data)
{
    if (dev_data->is_disable_int) {
        dev_data->is_disable_int = false;
        enable_irq(dev_data->irq);
    }
}

static inline void zb_cc2530_disable_int(struct zb_cc2530_dev_data *dev_data)
{
    if (!dev_data->is_disable_int) {
        disable_irq_nosync(dev_data->irq);
        dev_data->is_disable_int = true;
    }
}

static void zb_cc2530_msg_work(struct work_struct *work)
{
    struct zb_cc2530_dev_data *dev_data = \
                         container_of(work, struct zb_cc2530_dev_data, work);
    wake_up_interruptible(&dev_data->wait);
}

static irqreturn_t zb_cc2530_irq_handler(int irq, void *devid)
{
    struct zb_cc2530_dev_data *dev_data = devid;
    zb_cc2530_disable_int(dev_data);
    queue_work(dev_data->workqueue, &dev_data->work);
    zb_cc2530_enable_int(dev_data);
    return IRQ_HANDLED;
}


static void zb_cc2530_power(struct zb_cc2530_dev_data *dev_data, unsigned long state)
{
    struct zb_cc2530_platform_data *pdata = dev_data->dev->platform_data;

    dev_data->is_power_en = !!state;

    if (gpio_is_valid(pdata->en_pin)) {
        if (dev_data->is_power_en) {
            // pull up rst pin, enable interrupt and restore uart pin func when power enable
            if (gpio_is_valid(pdata->rst_pin)) {
                gpio_direction_output(pdata->rst_pin, 1);
            }
            pdata->restore_pin_status();
            gpio_direction_output(pdata->en_pin, pdata->en_level);
            zb_cc2530_enable_int(dev_data);
        } else {
            // drap down rst pin,disable interrupt and set uart pin out low when power diable
            zb_cc2530_disable_int(dev_data);
            if (gpio_is_valid(pdata->rst_pin)) {
                gpio_direction_output(pdata->rst_pin, 0);
            }
            pdata->set_pin_status();
            gpio_direction_output(pdata->en_pin, !pdata->en_level);
        }
    }
}

static void zb_cc2530_reset(struct zb_cc2530_dev_data *dev_data)
{
    struct zb_cc2530_platform_data *pdata = dev_data->dev->platform_data;

    if (gpio_is_valid(pdata->rst_pin)) {
        gpio_direction_output(pdata->rst_pin, pdata->rst_level);
        udelay(100);
        gpio_direction_output(pdata->rst_pin, !pdata->rst_level);
    }
}

static void zb_cc2530_wake(struct zb_cc2530_dev_data *dev_data)
{
    struct zb_cc2530_platform_data *pdata = dev_data->dev->platform_data;

    if (gpio_is_valid(pdata->wake_pin)) {
        gpio_direction_output(pdata->wake_pin, pdata->wake_level);
        udelay(100);
        gpio_direction_output(pdata->wake_pin, !pdata->wake_level);
    }
}

static int zb_cc2530_dev_gpio_init(struct zb_cc2530_dev_data *dev_data)
{
    struct zb_cc2530_platform_data *pdata = dev_data->dev->platform_data;
    int error;

    if (gpio_is_valid(pdata->en_pin)) {
        if (gpio_request(pdata->en_pin, "en_pin") < 0) {
            pr_err("%s: en_pin request failed\n", __FUNCTION__);
            goto err_gpio_request1;
        }
        gpio_direction_output(pdata->en_pin, 0);
    } else {
        dev_err(dev_data->dev, "Invalid en_pin: %d\n", pdata->en_pin);
        goto err_gpio_request1;
    }

    if (gpio_is_valid(pdata->wake_pin)) {
        if (gpio_request(pdata->wake_pin, "wake_pin") < 0) {
            pr_err("%s: wake_pin request failed\n", __FUNCTION__);
            goto err_gpio_request2;
        }
        gpio_direction_output(pdata->wake_pin, 0);
    } else {
        dev_err(dev_data->dev, "Invalid wake_pin: %d\n", pdata->wake_pin);
        goto err_gpio_request2;
    }

    if (gpio_is_valid(pdata->rst_pin)) {
        if (gpio_request(pdata->rst_pin, "rst_pin") < 0) {
            pr_err("%s: rst_pin request failed\n", __FUNCTION__);
            goto err_gpio_request3;
        }
        gpio_direction_output(pdata->rst_pin, 0);
    } else {
        dev_err(dev_data->dev, "Invalid rst_pin: %d\n", pdata->rst_pin);
        goto err_gpio_request3;
    }

    if (gpio_is_valid(pdata->int_pin)) {
        error = gpio_request(pdata->int_pin, "int_pin");
        if (error < 0) {
            dev_err(dev_data->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->int_pin, error);
            goto err_gpio_request4;
        }

        /*
         * Request MCU IO interrupt source
         */
        dev_data->irq = gpio_to_irq(pdata->int_pin);
        if (dev_data->irq < 0) {
            error = dev_data->irq;
            dev_err(dev_data->dev, "Unable to get irq number for GPIO %d, error %d\n",
                    pdata->int_pin, error);
            goto err_request_irq;
        }

        error = request_any_context_irq(dev_data->irq,                          \
                                        zb_cc2530_irq_handler,                  \
                                        IRQF_TRIGGER_RISING | IRQF_DISABLED,    \
                                        dev_name(dev_data->dev),                \
                                        dev_data);
        if (error < 0) {
            dev_err(dev_data->dev, "Unable to clain irq %d, error %d\n",
                    dev_data->irq, error);
            goto err_request_irq;
        } else {
            enable_irq_wake(dev_data->irq);
            dev_data->is_disable_int = false;
            zb_cc2530_disable_int(dev_data);
        }
    } else {
        dev_err(dev_data->dev, "Invalid int_pin: %d\n", pdata->int_pin);
        error = -ENODEV;
        goto err_gpio_request2;
    }
    pdata->set_pin_status();
    return 0;

err_request_irq:
    gpio_free(pdata->int_pin);
err_gpio_request4:
    gpio_free(pdata->rst_pin);
err_gpio_request3:
    gpio_free(pdata->wake_pin);
err_gpio_request2:
    gpio_free(pdata->en_pin);
err_gpio_request1:
    return -ENODEV;
}

static void zb_cc2530_dev_gpio_free(struct zb_cc2530_dev_data *dev_data)
{
    struct zb_cc2530_platform_data *pdata = dev_data->dev->platform_data;
    if (dev_data->irq > 0)
        free_irq(dev_data->irq, dev_data);

    gpio_free(pdata->en_pin);
    gpio_free(pdata->int_pin);
    gpio_free(pdata->wake_pin);
    gpio_free(pdata->rst_pin);

}


static int zb_cc2530_dev_open(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct zb_cc2530_dev_data *dev_data = container_of(miscdev, struct zb_cc2530_dev_data, miscdev);

    if (atomic_read(&(dev_data->opened))) {
        dev_err(dev_data->dev, "zigbee dev is busy, cannot open\n");
        return -EBUSY;
    }
    zb_cc2530_power(dev_data, POWER_ON);
    atomic_inc(&(dev_data->opened));
    return 0;
}

static int zb_cc2530_dev_release(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct zb_cc2530_dev_data *dev_data = container_of(miscdev, struct zb_cc2530_dev_data, miscdev);

    zb_cc2530_power(dev_data, POWER_OFF);
    atomic_dec(&(dev_data->opened));
    return 0;
}


static long zb_cc2530_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct zb_cc2530_dev_data *dev_data = container_of(miscdev, struct zb_cc2530_dev_data, miscdev);


    switch(cmd) {
        case ZB_CC2530_IOC_POWER:
            zb_cc2530_power(dev_data, arg);
            break;

        case ZB_CC2530_IOC_RST:
            zb_cc2530_reset(dev_data);
            break;

        case ZB_CC2530_IOC_WAKE:
            zb_cc2530_wake(dev_data);
            break;

        case ZB_CC2530_IOC_G_MSG:
            interruptible_sleep_on(&dev_data->wait);
            break;

        default:
            dev_err(dev_data->dev, "Not supported CMD:0x%x\n", cmd);
            return -EINVAL;
    }
    return 0;
}



static struct file_operations zb_cc2530_dev_fops = {
    .owner          = THIS_MODULE,
    .open           = zb_cc2530_dev_open,
    .release        = zb_cc2530_dev_release,
    .unlocked_ioctl = zb_cc2530_dev_ioctl,
};


static int zb_cc2530_dev_probe(struct platform_device *pdev)
{
    struct zb_cc2530_dev_data *dev_data;

    if (!pdev->dev.platform_data) {
        pr_err("%s: dev platform_data cannot be NULL\n", __FUNCTION__);
        goto out;
    }

    dev_data = kzalloc(sizeof(*dev_data), GFP_KERNEL);
    if (!dev_data) {
        pr_err("%s: allocate dev mem failed\n", __FUNCTION__);
        goto out;
    }

    dev_data->dev = &pdev->dev;

    dev_data->miscdev.minor = MISC_DYNAMIC_MINOR;
    dev_data->miscdev.name  = ZB_CC2530_DEV_BASENAME;
    dev_data->miscdev.fops  = &zb_cc2530_dev_fops;

    if (misc_register(&dev_data->miscdev) < 0) {
        pr_err("%s: misc_register failed\n", __FUNCTION__);
        goto err_misc_register;
    }

    atomic_set(&(dev_data->opened), 0);
    platform_set_drvdata(pdev, dev_data);

    init_waitqueue_head(&dev_data->wait);
    INIT_WORK(&dev_data->work, zb_cc2530_msg_work);
    dev_data->workqueue = create_singlethread_workqueue("zb_cc2530_msg_workqueue");
    if (!dev_data->workqueue) {
        pr_err("%s create_single_workqueue error!\n",__FUNCTION__);
        goto err_create_workqueue;
    }

    if (zb_cc2530_dev_gpio_init(dev_data) < 0) {
        goto err_gpio_init;
    }
    return 0;

err_gpio_init:
    destroy_workqueue(dev_data->workqueue);
err_create_workqueue:
    misc_deregister(&dev_data->miscdev);
err_misc_register:
    kfree(dev_data);
out:
    return -ENODEV;
}

static int zb_cc2530_dev_remove(struct platform_device *pdev)
{
    struct zb_cc2530_dev_data *dev_data = platform_get_drvdata(pdev);
    struct zb_cc2530_platform_data *pdata = dev_data->dev->platform_data;

    destroy_workqueue(dev_data->workqueue);
    misc_deregister(&dev_data->miscdev);
    zb_cc2530_dev_gpio_free(dev_data);
    pdata->restore_pin_status();
    kfree(dev_data);
    return 0;
}


static struct platform_driver zb_cc2530_dev_driver = {
    .driver = {
        .name  = "zb_cc2530",
        .owner = THIS_MODULE,
    },
    .probe   = zb_cc2530_dev_probe,
    .remove  = zb_cc2530_dev_remove,
};

static int __init zb_cc2530_dev_init(void)
{
    return platform_driver_register(&zb_cc2530_dev_driver);
}

static void __exit zb_cc2530_dev_exit(void)
{
    platform_driver_unregister(&zb_cc2530_dev_driver);
}

module_init(zb_cc2530_dev_init);
module_exit(zb_cc2530_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<rongjin.su@ingenic.com>");
MODULE_DESCRIPTION("zigbee CC2530 device driver");
MODULE_ALIAS("Platform: CC2530");
