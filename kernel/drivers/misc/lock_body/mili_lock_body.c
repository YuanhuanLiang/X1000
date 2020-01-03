/*
 * linux/drivers/misc/mili_lock_body.c
 *
 * mili lock body driver
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
#include <linux/mili_lock_body.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/timer.h>
#include <asm/signal.h>
#include <asm-generic/siginfo.h>


#define DRV_NAME                         "mili_lock_body"
#define MILI_LOCK_BODY_DEV_BASENAME      "mili_lock_body"

#define MILI_LOCK_BODY_IOC_MAGIC         'X'
#define MILI_LOCK_BODY_IOC_READ_VALUE     _IOR(MILI_LOCK_BODY_IOC_MAGIC, 0, int)   /* set power */
#define MILI_LOCK_BODY_IOC_WAIT_STATUS    _IOR(MILI_LOCK_BODY_IOC_MAGIC, 1, int)   /* get lock status */

struct mili_lock_body_dev_data {
    int id;
    int square_tongue_master_lock_pin;
    int square_tongue_anti_lock_pin;
    int lock_cylinder_rotation_pin;

    int square_tongue_master_lock_irq;
    int square_tongue_anti_lock_irq;
    int lock_cylinder_rotation_irq;
    unsigned char last_status;

    struct timer_list timer;

    struct device *dev;
    struct miscdevice miscdev;

    wait_queue_head_t       wait;
    struct work_struct      work;
    struct workqueue_struct *workqueue;
    struct mili_lock_body_platform_data *pdata;
};


static void lock_body_timerhandler(unsigned long data)
{
    struct mili_lock_body_dev_data *dev_data = (struct mili_lock_body_dev_data *)data;

    /**
     * Start workqueue
     */
    queue_work(dev_data->workqueue, &dev_data->work);
}


static unsigned char get_gpio_value(struct mili_lock_body_dev_data *dev_data)
{
    uint8_t gpios_value = 0;
    if(gpio_get_value(dev_data->square_tongue_master_lock_pin))
        gpios_value |= (1<<0);
    else
        gpios_value &=~(1<<0);

    if(gpio_get_value(dev_data->square_tongue_anti_lock_pin))
        gpios_value |= (1<<1);
    else
        gpios_value &=~(1<<1);

    if(gpio_get_value(dev_data->lock_cylinder_rotation_pin))
        gpios_value |= (1<<2);
    else
        gpios_value &=~(1<<2);

    printk(" ------------> 0x%x\n", gpios_value);
    return gpios_value;
}

static void mili_lock_body_work(struct work_struct *work)
{
    unsigned char lock_status = 0;

    struct mili_lock_body_dev_data *dev_data = \
                         container_of(work, struct mili_lock_body_dev_data, work);
    lock_status = get_gpio_value(dev_data);
    if (dev_data->last_status != lock_status) {
        dev_data->last_status = lock_status;
        wake_up_interruptible(&dev_data->wait);
    }
}

static irqreturn_t mili_lock_body_irq_handler(int irq, void *devid)
{
    struct mili_lock_body_dev_data *dev_data = devid;

    dev_data->timer.expires = jiffies + HZ / 5;

    mod_timer(&dev_data->timer, dev_data->timer.expires);

    return IRQ_HANDLED;
}

static int mili_lock_body_gpio_init(struct mili_lock_body_dev_data* dev_data)
{
    int err;

    struct mili_lock_body_platform_data* pdata = dev_data->pdata;

    dev_data->square_tongue_master_lock_pin = pdata->square_tongue_master_lock_pin;
    dev_data->square_tongue_anti_lock_pin   = pdata->square_tongue_anti_lock_pin;
    dev_data->lock_cylinder_rotation_pin    = pdata->lock_cylinder_rotation_pin;


    if (gpio_is_valid(dev_data->square_tongue_master_lock_pin)) {
        err = gpio_request(dev_data->square_tongue_master_lock_pin,
                        "mili lock body square_tongue_master_lock_pin");
        if (err < 0) {
            dev_err(dev_data->dev,"%s enable[%d] square_tongue_master_lock_pin failed.\n",
                    __func__, dev_data->square_tongue_master_lock_pin);
            goto err_square_tongue_master_lock;
        }
        gpio_direction_input(dev_data->square_tongue_master_lock_pin);

        dev_data->square_tongue_master_lock_irq =
                                    gpio_to_irq(dev_data->square_tongue_master_lock_pin);
        if (dev_data->square_tongue_master_lock_irq < 0) {
            err = dev_data->square_tongue_master_lock_irq;
            dev_err(dev_data->dev,
                    "Unable to get square_tongue_master_lock_irq number for GPIO %d, err %d\n",
                    dev_data->square_tongue_master_lock_pin, err);
            goto err_square_tongue_master_lock_to_irq;
        }

        err = request_irq(dev_data->square_tongue_master_lock_irq,                     \
                            mili_lock_body_irq_handler,                                \
                            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,                  \
                            "square_tongue_master_lock_irq",                           \
                            dev_data);
        if (err < 0) {
            dev_err(dev_data->dev, "Unable to clain irq %d, err %d\n",
                    dev_data->square_tongue_master_lock_irq, err);
            goto err_request_square_tongue_master_lock_irq;
        }
        enable_irq_wake(dev_data->square_tongue_master_lock_irq);
        disable_irq_nosync(dev_data->square_tongue_master_lock_irq);
    }


    if (gpio_is_valid(dev_data->square_tongue_anti_lock_pin)) {
        err = gpio_request(dev_data->square_tongue_anti_lock_pin,
                        "mili lock body square_tongue_anti_lock_pin");
        if (err < 0) {
            dev_err(dev_data->dev,
                    "mili lock body: %s enable[%d] square_tongue_anti_lock_pin failed.\n",
                    __func__, dev_data->square_tongue_anti_lock_pin);
            goto err_square_tongue_anti_lock;
        }
        gpio_direction_input(dev_data->square_tongue_anti_lock_pin);

        dev_data->square_tongue_anti_lock_irq =
                                    gpio_to_irq(dev_data->square_tongue_anti_lock_pin);
        if (dev_data->square_tongue_anti_lock_irq < 0) {
            err = dev_data->square_tongue_anti_lock_irq;
            dev_err(dev_data->dev,
                    "Unable to get square_tongue_anti_lock_irq number for GPIO %d, err %d\n",
                    dev_data->square_tongue_anti_lock_pin, err);
            goto err_square_tongue_anti_lock_to_irq;
        }

        err = request_irq(dev_data->square_tongue_anti_lock_irq,                     \
                            mili_lock_body_irq_handler,                              \
                            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,                \
                            "square_tongue_anti_lock_irq",                           \
                            dev_data);
        if (err < 0) {
            dev_err(dev_data->dev, "Unable to clain irq %d, err %d\n",
                    dev_data->square_tongue_anti_lock_irq, err);
            goto err_request_square_tongue_anti_lock_irq;
        }
        enable_irq_wake(dev_data->square_tongue_anti_lock_irq);
        disable_irq_nosync(dev_data->square_tongue_anti_lock_irq);
    }


    if (gpio_is_valid(dev_data->lock_cylinder_rotation_pin)) {
        err = gpio_request(dev_data->lock_cylinder_rotation_pin,
                                "mili lock body lock_cylinder_rotation_pin");
        if (err < 0) {
            dev_err(dev_data->dev,"%s enable[%d] lock_cylinder_rotation_pin failed.\n",
                    __func__, dev_data->lock_cylinder_rotation_pin);
            goto err_lock_cylinder_rotation;
        }
        gpio_direction_input(dev_data->lock_cylinder_rotation_pin);

        dev_data->lock_cylinder_rotation_irq = gpio_to_irq(dev_data->lock_cylinder_rotation_pin);
        if (dev_data->lock_cylinder_rotation_irq < 0) {
            err = dev_data->lock_cylinder_rotation_irq;
            dev_err(dev_data->dev,
                    "Unable to get lock_cylinder_rotation_irq number for GPIO %d, err %d\n",
                    dev_data->square_tongue_anti_lock_pin, err);
            goto err_lock_cylinder_rotation_to_irq;
        }

        err = request_irq(dev_data->lock_cylinder_rotation_irq,                     \
                            mili_lock_body_irq_handler,                             \
                            IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,               \
                            "lock_cylinder_rotation_irq",                           \
                            dev_data);
        if (err < 0) {
            dev_err(dev_data->dev, "Unable to clain irq %d, err %d\n",
                    dev_data->lock_cylinder_rotation_irq, err);
            goto err_request_lock_cylinder_rotation_irq;
        }
        enable_irq_wake(dev_data->lock_cylinder_rotation_irq);
        disable_irq_nosync(dev_data->lock_cylinder_rotation_irq);
    }

    return 0;

err_request_lock_cylinder_rotation_irq:
err_lock_cylinder_rotation_to_irq:
    gpio_free(dev_data->lock_cylinder_rotation_pin);
err_lock_cylinder_rotation:
    free_irq(dev_data->square_tongue_anti_lock_irq, dev_data);
err_request_square_tongue_anti_lock_irq:
err_square_tongue_anti_lock_to_irq:
    gpio_free(dev_data->square_tongue_anti_lock_pin);
err_square_tongue_anti_lock:
    free_irq(dev_data->square_tongue_master_lock_irq, dev_data);
err_request_square_tongue_master_lock_irq:
err_square_tongue_master_lock_to_irq:
    gpio_free(dev_data->square_tongue_master_lock_pin);
err_square_tongue_master_lock:
    return -1;
}

static void mili_lock_body_gpio_free(struct mili_lock_body_dev_data* dev_data)
{
    if (dev_data->square_tongue_master_lock_irq > 0)
        free_irq(dev_data->square_tongue_master_lock_irq, dev_data);

    if (dev_data->square_tongue_anti_lock_irq > 0)
        free_irq(dev_data->square_tongue_anti_lock_irq, dev_data);

    if (dev_data->lock_cylinder_rotation_irq > 0)
        free_irq(dev_data->lock_cylinder_rotation_irq, dev_data);

    if (gpio_is_valid(dev_data->square_tongue_master_lock_pin))
        gpio_free(dev_data->square_tongue_master_lock_pin);

    if (gpio_is_valid(dev_data->square_tongue_anti_lock_pin))
        gpio_free(dev_data->square_tongue_anti_lock_pin);

    if (gpio_is_valid(dev_data->lock_cylinder_rotation_pin))
        gpio_free(dev_data->lock_cylinder_rotation_pin);
}


static int mili_lock_body_dev_open(struct inode *inode, struct file *filp)
{

    struct miscdevice *miscdev = filp->private_data;
    struct mili_lock_body_dev_data *dev_data =
                container_of(miscdev, struct mili_lock_body_dev_data, miscdev);

    enable_irq(dev_data->square_tongue_master_lock_irq);
    enable_irq(dev_data->square_tongue_anti_lock_irq);
    enable_irq(dev_data->lock_cylinder_rotation_irq);

    return 0;
}

static int mili_lock_body_dev_release(struct inode *inode, struct file *filp)
{

    struct miscdevice *miscdev = filp->private_data;
    struct mili_lock_body_dev_data *dev_data =
                container_of(miscdev, struct mili_lock_body_dev_data, miscdev);

    disable_irq_nosync(dev_data->square_tongue_master_lock_irq);
    disable_irq_nosync(dev_data->square_tongue_anti_lock_irq);
    disable_irq_nosync(dev_data->lock_cylinder_rotation_irq);

    return 0;
}

static long mili_lock_body_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct mili_lock_body_dev_data *dev_data =
                    container_of(miscdev, struct mili_lock_body_dev_data, miscdev);
    unsigned char gpios_value = 0;
    void __user *argp = (void __user *)arg;

    switch(cmd) {
        case MILI_LOCK_BODY_IOC_READ_VALUE:
            gpios_value = get_gpio_value(dev_data);
            if (copy_to_user(argp, (void *)&gpios_value, sizeof(gpios_value))) {
                dev_err(dev_data->dev, "IOCTL: read io value failed\n");
                return -EFAULT;
            }
            break;
        case MILI_LOCK_BODY_IOC_WAIT_STATUS:
            interruptible_sleep_on(&dev_data->wait);
            break;
        default:
            dev_err(dev_data->dev, "Not supported CMD:0x%x\n", cmd);
            return -EINVAL;
    }
    return 0;
}


static struct file_operations mili_lock_body_dev_fops = {
    .owner          = THIS_MODULE,
    .open           = mili_lock_body_dev_open,
    .release        = mili_lock_body_dev_release,
    .unlocked_ioctl = mili_lock_body_dev_ioctl,
};

static int mili_lock_body_probe(struct platform_device *pdev)
{
    int err;
    char dev_name[32];
    struct mili_lock_body_dev_data *dev_data;

    dev_data = kzalloc(sizeof(struct mili_lock_body_dev_data), GFP_KERNEL);
    if(dev_data == NULL) {
        dev_err(dev_data->dev, "Failed to malloc device data\n");
        err = -ENOMEM;
        goto err_dev_data_kzalloc;
    }
    dev_data->dev   = &pdev->dev;
    dev_data->pdata = dev_get_platdata(&pdev->dev);

    dev_data->id            = dev_data->pdata->id;
    err = sprintf(dev_name,"%s-%d",MILI_LOCK_BODY_DEV_BASENAME,dev_data->id);
    if (err < 0) {
        dev_err(dev_data->dev, "spirntf dev name failed\n");
        goto err_sprintf_dev_name;
    }

    dev_data->miscdev.minor = MISC_DYNAMIC_MINOR;
    dev_data->miscdev.name  = dev_name;
    dev_data->miscdev.fops  = &mili_lock_body_dev_fops;
    if (misc_register(&dev_data->miscdev) < 0) {
        dev_err(dev_data->dev, " misc_register failed\n");
        goto err_misc_register;
    }
    init_waitqueue_head(&dev_data->wait);
    INIT_WORK(&dev_data->work, mili_lock_body_work);
    dev_data->workqueue = create_singlethread_workqueue("mili_lock_body_workqueue");
    if (!dev_data->workqueue) {
        dev_err(dev_data->dev, "create_single_workqueue error!\n");
        err = -1;
        goto err_create_workqueue;
    }
    platform_set_drvdata(pdev, dev_data);

    err = mili_lock_body_gpio_init(dev_data);
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
static int mili_lock_body_remove(struct platform_device *pdev)
{
    struct mili_lock_body_dev_data *dev_data = platform_get_drvdata(pdev);

    mili_lock_body_gpio_free(dev_data);
    misc_deregister(&dev_data->miscdev);
    destroy_workqueue(dev_data->workqueue);
    kfree(dev_data);
    return 0;
}

static int mili_lock_body_suspend(struct platform_device *pdev, pm_message_t state)
{

    return 0;
}
static int mili_lock_body_resume(struct platform_device *pdev)
{

    return 0;
}

static struct platform_driver mili_lock_body_driver = {
        .driver = {
                .name = DRV_NAME,
                .owner = THIS_MODULE,
        },
        .probe = mili_lock_body_probe,
        .remove = mili_lock_body_remove,
        .suspend = mili_lock_body_suspend,
        .resume = mili_lock_body_resume,
};

static int __init mili_lock_body_init(void)
{
    return platform_driver_register(&mili_lock_body_driver);
}

static void __exit mili_lock_body_exit(void)
{
    platform_driver_unregister(&mili_lock_body_driver);
}

module_init(mili_lock_body_init);
module_exit(mili_lock_body_exit);

MODULE_AUTHOR("Monk Su <rongjin.su@ingenic.com>");
MODULE_DESCRIPTION("mili lock body driver");
MODULE_LICENSE("GPL");
