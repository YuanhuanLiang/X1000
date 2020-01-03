/*
 * linux/drivers/misc/fxl_lock_body.c
 *
 * fxl lock body driver
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
#include <linux/ioctl.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fxl_lock_body.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/timer.h>
#include <asm/signal.h>
#include <asm-generic/siginfo.h>


#define DRV_NAME                        "fxl_lock_body"
#define FXL_LOCK_BODY_DEV_BASENAME      "fxl_lock_body"

#define FXL_LOCK_BODY_IOC_MAGIC         'X'
#define FXL_LOCK_BODY_IOC_READ_VALUE     _IOR(FXL_LOCK_BODY_IOC_MAGIC, 0, int)   /* set power */
#define FXL_LOCK_BODY_IOC_WAIT_STATUS    _IOR(FXL_LOCK_BODY_IOC_MAGIC, 1, int)   /* get lock status */

struct fxl_lock_body_dev_data {
    int id;
    int bolique_tongue_shrink_pin;
    int square_tongue_stretch_pin;
    int square_tongue_shrink_pin;

    int bolique_tongue_shrink_irq;
    int square_tongue_stretch_irq;
    int square_tongue_shrink_irq;
    unsigned char last_status;

    struct timer_list timer;

    struct device *dev;
    struct miscdevice miscdev;

    wait_queue_head_t       wait;
    struct work_struct      work;
    struct workqueue_struct *workqueue;
    struct fxl_lock_body_platform_data *pdata;
};

static void lock_body_timerhandler(unsigned long data)
{
    struct fxl_lock_body_dev_data *dev_data = (struct fxl_lock_body_dev_data *)data;

    /**
     * Start workqueue
     */
    queue_work(dev_data->workqueue, &dev_data->work);
}


static unsigned char get_gpio_value(struct fxl_lock_body_dev_data *dev_data)
{
    uint8_t gpios_value = 0;
    if(gpio_get_value(dev_data->bolique_tongue_shrink_pin))
        gpios_value |= (1<<0);
    else
        gpios_value &=~(1<<0);

    if(gpio_get_value(dev_data->square_tongue_stretch_pin))
        gpios_value |= (1<<1);
    else
        gpios_value &=~(1<<1);

    if(gpio_get_value(dev_data->square_tongue_shrink_pin))
        gpios_value |= (1<<2);
    else
        gpios_value &=~(1<<2);

    return gpios_value;
}

static void fxl_lock_body_work(struct work_struct *work)
{
    unsigned char lock_status = 0;

    struct fxl_lock_body_dev_data *dev_data = \
                         container_of(work, struct fxl_lock_body_dev_data, work);
    lock_status = get_gpio_value(dev_data);
    if (dev_data->last_status != lock_status) {
        dev_data->last_status = lock_status;
        wake_up_interruptible(&dev_data->wait);
    }
}

static irqreturn_t fxl_lock_body_irq_handler(int irq, void *devid)
{
    struct fxl_lock_body_dev_data *dev_data = devid;

    dev_data->timer.expires = jiffies + HZ / 5;

    mod_timer(&dev_data->timer, dev_data->timer.expires);

    return IRQ_HANDLED;
}

static int fxl_lock_body_gpio_init(struct fxl_lock_body_dev_data* dev_data)
{
    int err;

    struct fxl_lock_body_platform_data* pdata = dev_data->pdata;

    dev_data->bolique_tongue_shrink_pin = pdata->bolique_tongue_shrink_pin;
    dev_data->square_tongue_stretch_pin = pdata->square_tongue_stretch_pin;
    dev_data->square_tongue_shrink_pin  = pdata->square_tongue_shrink_pin;


    if (gpio_is_valid(dev_data->bolique_tongue_shrink_pin)) {
        err = gpio_request(dev_data->bolique_tongue_shrink_pin,
                        "fxl lock body bolique_tongue_shrink_pin");
        if (err < 0) {
            dev_err(dev_data->dev,"%s enable[%d] bolique_tongue_shrink_pin failed.\n",
                    __func__, dev_data->bolique_tongue_shrink_pin);
            goto err_bolique_tongue_shrink;
        }
        gpio_direction_input(dev_data->bolique_tongue_shrink_pin);

        dev_data->bolique_tongue_shrink_irq =
                                    gpio_to_irq(dev_data->bolique_tongue_shrink_pin);
        if (dev_data->bolique_tongue_shrink_irq < 0) {
            err = dev_data->bolique_tongue_shrink_irq;
            dev_err(dev_data->dev,
                    "Unable to get bolique_tongue_shrink_irq number for GPIO %d, err %d\n",
                    dev_data->bolique_tongue_shrink_pin, err);
            goto err_bolique_tongue_shrink_to_irq;
        }

        err = request_irq(dev_data->bolique_tongue_shrink_irq,                     \
                            fxl_lock_body_irq_handler,                             \
                            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,              \
                            "bolique_tongue_shrink_irq",                           \
                            dev_data);
        if (err < 0) {
            dev_err(dev_data->dev, "Unable to clain irq %d, err %d\n",
                    dev_data->bolique_tongue_shrink_irq, err);
            goto err_request_bolique_tongue_shrink_irq;
        }
        enable_irq_wake(dev_data->bolique_tongue_shrink_irq);
        disable_irq_nosync(dev_data->bolique_tongue_shrink_irq);
    }


    if (gpio_is_valid(dev_data->square_tongue_stretch_pin)) {
        err = gpio_request(dev_data->square_tongue_stretch_pin,
                        "fxl lock body square_tongue_stretch_pin");
        if (err < 0) {
            dev_err(dev_data->dev,
                    "fxl lock body: %s enable[%d] square_tongue_stretch_pin failed.\n",
                    __func__, dev_data->square_tongue_stretch_pin);
            goto err_square_tongue_stretch;
        }
        gpio_direction_input(dev_data->square_tongue_stretch_pin);

        dev_data->square_tongue_stretch_irq =
                                    gpio_to_irq(dev_data->square_tongue_stretch_pin);
        if (dev_data->square_tongue_stretch_irq < 0) {
            err = dev_data->square_tongue_stretch_irq;
            dev_err(dev_data->dev,
                    "Unable to get square_tongue_stretch_irq number for GPIO %d, err %d\n",
                    dev_data->square_tongue_stretch_pin, err);
            goto err_square_tongue_stretch_to_irq;
        }

        err = request_irq(dev_data->square_tongue_stretch_irq,                     \
                            fxl_lock_body_irq_handler,                             \
                            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,              \
                            "square_tongue_stretch_irq",                           \
                            dev_data);
        if (err < 0) {
            dev_err(dev_data->dev, "Unable to clain irq %d, err %d\n",
                    dev_data->square_tongue_stretch_irq, err);
            goto err_request_square_tongue_stretch_irq;
        }
        enable_irq_wake(dev_data->square_tongue_stretch_irq);
        disable_irq_nosync(dev_data->square_tongue_stretch_irq);
    }


    if (gpio_is_valid(dev_data->square_tongue_shrink_pin)) {
        err = gpio_request(dev_data->square_tongue_shrink_pin,
                                "fxl lock body square_tongue_shrink_pin");
        if (err < 0) {
            dev_err(dev_data->dev,"%s enable[%d] square_tongue_shrink_pin failed.\n",
                    __func__, dev_data->square_tongue_shrink_pin);
            goto err_square_tongue_shrink;
        }
        gpio_direction_input(dev_data->square_tongue_shrink_pin);

        dev_data->square_tongue_shrink_irq = gpio_to_irq(dev_data->square_tongue_shrink_pin);
        if (dev_data->square_tongue_shrink_irq < 0) {
            err = dev_data->square_tongue_shrink_irq;
            dev_err(dev_data->dev,
                    "Unable to get square_tongue_shrink_irq number for GPIO %d, err %d\n",
                    dev_data->square_tongue_stretch_pin, err);
            goto err_square_tongue_shrink_to_irq;
        }

        err = request_irq(dev_data->square_tongue_shrink_irq,                     \
                            fxl_lock_body_irq_handler,                            \
                            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,             \
                            "square_tongue_shrink_irq",                           \
                            dev_data);
        if (err < 0) {
            dev_err(dev_data->dev, "Unable to clain irq %d, err %d\n",
                    dev_data->square_tongue_shrink_irq, err);
            goto err_request_square_tongue_shrink_irq;
        }
        enable_irq_wake(dev_data->square_tongue_shrink_irq);
        disable_irq_nosync(dev_data->square_tongue_shrink_irq);
    }

    return 0;

err_request_square_tongue_shrink_irq:
err_square_tongue_shrink_to_irq:
    gpio_free(dev_data->square_tongue_shrink_pin);
err_square_tongue_shrink:
    free_irq(dev_data->square_tongue_stretch_irq, dev_data);
err_request_square_tongue_stretch_irq:
err_square_tongue_stretch_to_irq:
    gpio_free(dev_data->square_tongue_stretch_pin);
err_square_tongue_stretch:
    free_irq(dev_data->bolique_tongue_shrink_irq, dev_data);
err_request_bolique_tongue_shrink_irq:
err_bolique_tongue_shrink_to_irq:
    gpio_free(dev_data->bolique_tongue_shrink_pin);
err_bolique_tongue_shrink:
    return -1;
}

static void fxl_lock_body_gpio_free(struct fxl_lock_body_dev_data* dev_data)
{
    if (dev_data->bolique_tongue_shrink_irq > 0)
        free_irq(dev_data->bolique_tongue_shrink_irq, dev_data);

    if (dev_data->square_tongue_stretch_irq > 0)
        free_irq(dev_data->square_tongue_stretch_irq, dev_data);

    if (dev_data->square_tongue_shrink_irq > 0)
        free_irq(dev_data->square_tongue_shrink_irq, dev_data);

    if (gpio_is_valid(dev_data->bolique_tongue_shrink_pin))
        gpio_free(dev_data->bolique_tongue_shrink_pin);

    if (gpio_is_valid(dev_data->square_tongue_stretch_pin))
        gpio_free(dev_data->square_tongue_stretch_pin);

    if (gpio_is_valid(dev_data->square_tongue_shrink_pin))
        gpio_free(dev_data->square_tongue_shrink_pin);
}


static int fxl_lock_body_dev_open(struct inode *inode, struct file *filp)
{

    struct miscdevice *miscdev = filp->private_data;
    struct fxl_lock_body_dev_data *dev_data =
                container_of(miscdev, struct fxl_lock_body_dev_data, miscdev);

    enable_irq(dev_data->bolique_tongue_shrink_irq);
    enable_irq(dev_data->square_tongue_stretch_irq);
    enable_irq(dev_data->square_tongue_shrink_irq);

    return 0;
}

static int fxl_lock_body_dev_release(struct inode *inode, struct file *filp)
{

    struct miscdevice *miscdev = filp->private_data;
    struct fxl_lock_body_dev_data *dev_data =
                container_of(miscdev, struct fxl_lock_body_dev_data, miscdev);

    disable_irq_nosync(dev_data->bolique_tongue_shrink_irq);
    disable_irq_nosync(dev_data->square_tongue_stretch_irq);
    disable_irq_nosync(dev_data->square_tongue_shrink_irq);

    return 0;
}

static long fxl_lock_body_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct fxl_lock_body_dev_data *dev_data =
                    container_of(miscdev, struct fxl_lock_body_dev_data, miscdev);
    unsigned char gpios_value = 0;
    void __user *argp = (void __user *)arg;

    switch(cmd) {
        case FXL_LOCK_BODY_IOC_READ_VALUE:
            gpios_value = get_gpio_value(dev_data);
            if (copy_to_user(argp, (void *)&gpios_value, sizeof(gpios_value))) {
                dev_err(dev_data->dev, "IOCTL: read io value failed\n");
                return -EFAULT;
            }
            break;
        case FXL_LOCK_BODY_IOC_WAIT_STATUS:
            interruptible_sleep_on(&dev_data->wait);
            break;
        default:
            dev_err(dev_data->dev, "Not supported CMD:0x%x\n", cmd);
            return -EINVAL;
    }
    return 0;
}


static struct file_operations fxl_lock_body_dev_fops = {
    .owner          = THIS_MODULE,
    .open           = fxl_lock_body_dev_open,
    .release        = fxl_lock_body_dev_release,
    .unlocked_ioctl = fxl_lock_body_dev_ioctl,
};

static int fxl_lock_body_probe(struct platform_device *pdev)
{
    int err;
    char dev_name[32];
    struct fxl_lock_body_dev_data *dev_data;

    dev_data = kzalloc(sizeof(struct fxl_lock_body_dev_data), GFP_KERNEL);
    if(dev_data == NULL) {
        dev_err(dev_data->dev, "Failed to malloc device data\n");
        err = -ENOMEM;
        goto err_dev_data_kzalloc;
    }
    dev_data->dev   = &pdev->dev;
    dev_data->pdata = dev_get_platdata(&pdev->dev);

    dev_data->id            = dev_data->pdata->id;
    err = sprintf(dev_name,"%s-%d",FXL_LOCK_BODY_DEV_BASENAME,dev_data->id);
    if (err < 0) {
        dev_err(dev_data->dev, "spirntf dev name failed\n");
        goto err_sprintf_dev_name;
    }

    dev_data->miscdev.minor = MISC_DYNAMIC_MINOR;
    dev_data->miscdev.name  = dev_name;
    dev_data->miscdev.fops  = &fxl_lock_body_dev_fops;
    if (misc_register(&dev_data->miscdev) < 0) {
        dev_err(dev_data->dev, " misc_register failed\n");
        goto err_misc_register;
    }
    init_waitqueue_head(&dev_data->wait);
    INIT_WORK(&dev_data->work, fxl_lock_body_work);
    dev_data->workqueue = create_singlethread_workqueue("fxl_lock_body_workqueue");
    if (!dev_data->workqueue) {
        dev_err(dev_data->dev, "create_single_workqueue error!\n");
        err = -1;
        goto err_create_workqueue;
    }
    platform_set_drvdata(pdev, dev_data);

    err = fxl_lock_body_gpio_init(dev_data);
    if (err < 0) {
        dev_err(dev_data->dev, "Failed to init gpio.\n");
        goto err_gpio_init;
    }

    init_timer(&dev_data->timer);
    dev_data->timer.data = (unsigned long)dev_data;
    dev_data->timer.function = lock_body_timerhandler;

    return 0;

err_gpio_init:
    destroy_workqueue(dev_data->workqueue);
err_create_workqueue:
    misc_deregister(&dev_data->miscdev);
err_misc_register:
err_sprintf_dev_name:
    kfree(dev_data);
err_dev_data_kzalloc:
    return err;

}
static int fxl_lock_body_remove(struct platform_device *pdev)
{
    struct fxl_lock_body_dev_data *dev_data = platform_get_drvdata(pdev);

    fxl_lock_body_gpio_free(dev_data);
    misc_deregister(&dev_data->miscdev);
    destroy_workqueue(dev_data->workqueue);
    kfree(dev_data);
    return 0;
}

static int fxl_lock_body_suspend(struct platform_device *pdev, pm_message_t state)
{

    return 0;
}
static int fxl_lock_body_resume(struct platform_device *pdev)
{

    return 0;
}

static struct platform_driver fxl_lock_body_driver = {
        .driver = {
                .name = DRV_NAME,
                .owner = THIS_MODULE,
        },
        .probe = fxl_lock_body_probe,
        .remove = fxl_lock_body_remove,
        .suspend = fxl_lock_body_suspend,
        .resume = fxl_lock_body_resume,
};

static int __init fxl_lock_body_init(void)
{
    return platform_driver_register(&fxl_lock_body_driver);
}

static void __exit fxl_lock_body_exit(void)
{
    platform_driver_unregister(&fxl_lock_body_driver);
}

module_init(fxl_lock_body_init);
module_exit(fxl_lock_body_exit);

MODULE_AUTHOR("Monk Su <rongjin.su@ingenic.com>");
MODULE_DESCRIPTION("fxl lock body driver");
MODULE_LICENSE("GPL");
