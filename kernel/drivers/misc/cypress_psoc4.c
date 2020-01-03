/*
 * drivers/misc/cypress_psoc4.c
 *
 * Cypress PSoC4 4000S family driver.
 * This driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/sysctl.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/cypress_psoc4.h>

/**
 * Ioctl commands
 */
#define CYPRESS_IOC_MAGIC    'Y'
#define CYPRESS_DRV_IOC_R_CARD_UID       _IOR(CYPRESS_IOC_MAGIC, 0, int)
#define CYPRESS_DRV_IOC_R_CARD_SECTOR    _IOR(CYPRESS_IOC_MAGIC, 1, int)
#define CYPRESS_DRV_IOC_SEND_ACK         _IOW(CYPRESS_IOC_MAGIC, 2, int)
#define CYPRESS_DRV_IOC_RESET_MCU        _IOW(CYPRESS_IOC_MAGIC, 3, int)
#define CYPRESS_DRV_IOC_ENABLE_IRQ       _IOW(CYPRESS_IOC_MAGIC, 4, int)
#define CYPRESS_DRV_IOC_DISABLE_IRQ      _IOW(CYPRESS_IOC_MAGIC, 5, int)
#define CYPRESS_DRV_IOC_WAIT_R_CARD      _IOR(CYPRESS_IOC_MAGIC, 6, int)

#define INPUT_PASS_TO_DEVICE    2

/**
 * Driver data struct
 */
struct cypress_psoc4_drvdata {
    int irq;

#ifdef CONFIG_CYPRESS_PSOC4_SYSFS
    bool is_suspend_mcu;
#endif
    bool is_disable_irq;
    bool is_new_card;
    bool is_data_ready;

    atomic_t opened;
    wait_queue_head_t wait;
    struct mutex lock;
    spinlock_t   spin_lock;
    struct i2c_client *client;
    struct device *dev;
    struct class *class;
    struct input_dev *input;
#ifdef CONFIG_CYPRESS_PSOC4_FASYNC
    struct fasync_struct *fasync;
#endif
    struct work_struct work;
    struct miscdevice miscdev;
    struct cypress_psoc4_platform_data *pdata;
    union cypress_psoc4_i2c_msg i2cmsg;
};

/**
 * Functions
 */
static int cypress_psoc4_i2c_master_recv(struct i2c_client *client, char *buf, int count)
{
    int retval;
    struct i2c_msg msg;

    msg.addr  = client->addr;
    msg.flags = client->flags & I2C_M_TEN;
    msg.flags |= I2C_M_RD;
    msg.len   = count;
    msg.buf   = buf;

    retval = i2c_transfer(client->adapter, &msg, 1);
    return (retval == 1) ? count : retval;
}

static int cypress_psoc4_i2c_master_send(struct i2c_client *client, char *buf, int count)
{
    int retval;
    struct i2c_msg msg;

    msg.addr  = client->addr;
    msg.flags = client->flags & I2C_M_TEN;
    msg.len   = count;
    msg.buf   = buf;

    retval = i2c_transfer(client->adapter, &msg, 1);
    return (retval == 1) ? count : retval;
}

static inline void cypress_psoc4_enable_irq(struct cypress_psoc4_drvdata *psoc4)
{
    if (psoc4->is_disable_irq) {
        enable_irq(psoc4->irq);
        spin_lock(&psoc4->spin_lock);
        psoc4->is_disable_irq = false;
        spin_unlock(&psoc4->spin_lock);
    }
}

static inline void cypress_psoc4_disable_irq(struct cypress_psoc4_drvdata *psoc4)
{
    if (!psoc4->is_disable_irq) {
        disable_irq_nosync(psoc4->irq);
        spin_lock(&psoc4->spin_lock);
        psoc4->is_disable_irq = true;
        spin_unlock(&psoc4->spin_lock);
    }
}

#ifdef CONFIG_CYPRESS_PSOC4_SYSFS
/**
 * cypress_psoc4_show_disable_irq()
 * @dev: pointer to device
 * @attr: pointer to device_attribute
 * @buf: buffer from userspace
 *
 * This function shows the psoc4->is_disable_irq value to userspace.
 * If psoc4->is_disable_irq is 1, indicates MCU interrupt source is disabled.
 */
static ssize_t cypress_psoc4_show_disable_irq(struct device *dev,
                    struct device_attribute *attr,
                    char *buf)
{
    struct cypress_psoc4_drvdata *psoc4 = dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", psoc4->is_disable_irq);
}

/**
 * cypress_psoc4_store_disable_irq()
 * @dev: pointer to device
 * @attr: pointer to device_attribute
 * @buf: buffer from userspace
 * @count: store byte numbers
 *
 * This function parses disable_irq condition from @buf,
 * if a non-zero condition, the MCU interrupt source will be disable.
 */
static ssize_t cypress_psoc4_store_disable_irq(struct device *dev,
                    struct device_attribute *attr,
                    const char *buf,
                    size_t count)
{
    struct cypress_psoc4_drvdata *psoc4 = dev_get_drvdata(dev);
    unsigned long state;
    size_t error;

    error = kstrtoul(buf, 10, &state);
    if (error)
        return error;

    if (psoc4->irq < 0) {
        dev_info(psoc4->dev, "Not have valid interrupt source\n");
        return 0;
    }

    if (state)
        cypress_psoc4_disable_irq(psoc4);
    else
        cypress_psoc4_enable_irq(psoc4);

    return count;
}

/**
 * cypress_psoc4_show_suspend_mcu()
 * @dev: pointer to device
 * @attr: pointer to device_attribute
 * @buf: buffer from userspace
 *
 * This function shows the psoc4->is_suspend_mcu value to userspace.
 * If psoc4->is_suspend_mcu is 1, indicates MCU has been suspended.
 */
static ssize_t cypress_psoc4_show_suspend_mcu(struct device *dev,
                    struct device_attribute *attr,
                    char *buf)
{
    struct cypress_psoc4_drvdata *psoc4 = dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", psoc4->is_suspend_mcu);
}

/**
 * cypress_psoc4_store_suspend_mcu()
 * @dev: pointer to device
 * @attr: pointer to device_attribute
 * @buf: buffer from userspace
 * @count: store byte numbers
 *
 * This function parses is_suspend_mcu condition from @buf.
 * the master will send suspend or resume instruction to MCU.
 */
static ssize_t cypress_psoc4_store_suspend_mcu(struct device *dev,
                    struct device_attribute *attr,
                    const char *buf,
                    size_t count)
{
    struct cypress_psoc4_drvdata *psoc4 = dev_get_drvdata(dev);
    unsigned long state;
    unsigned char txcmd[2];
    size_t error;

    error = kstrtoul(buf, 10, &state);
    if (error)
        return error;

    if (gpio_is_valid(psoc4->pdata->mcu_int_pin)) {
        gpio_direction_output(psoc4->pdata->mcu_int_pin,
                state ? psoc4->pdata->mcu_int_level : !psoc4->pdata->mcu_int_level);
    }

    if (state) {
        if (!psoc4->is_suspend_mcu)
            txcmd[0] = MSG_TYPE_MASTER_S_SUSPEND;
        else
            return count;
    } else {
        txcmd[0] = MSG_TYPE_MASTER_S_SUSPEND;
    }

    txcmd[1] = MSG_PACKET_EOP;

    mutex_lock(&psoc4->lock);
    if (cypress_psoc4_i2c_master_send(psoc4->client, txcmd, 2) != 2) {
        mutex_unlock(&psoc4->lock);
        return -EIO;
    }
    psoc4->is_suspend_mcu = !!state;
    mutex_unlock(&psoc4->lock);

    return count;
}

/**
 * cypress_psoc4_store_reset_mcu()
 * @dev: pointer to device
 * @attr: pointer to device_attribute
 * @buf: buffer from userspace
 * @count: store byte numbers
 *
 * This function parses reset condition from @buf,
 * if a non-zero condition, the MCU will be reset by set mcu_rst_pin.
 */
static ssize_t cypress_psoc4_store_reset_mcu(struct device *dev,
                    struct device_attribute *attr,
                    const char *buf,
                    size_t count)
{
    struct cypress_psoc4_drvdata *psoc4 = dev_get_drvdata(dev);
    struct cypress_psoc4_platform_data *pdata = psoc4->pdata;
    unsigned long state;
    size_t error;

    error = kstrtoul(buf, 10, &state);
    if (error)
        return error;

    if (!!state) {
        mutex_lock(&psoc4->lock);
        if (gpio_is_valid(pdata->mcu_rst_pin)) {
            gpio_direction_output(pdata->mcu_rst_pin, pdata->mcu_rst_level);
            udelay(5);
            gpio_direction_output(pdata->mcu_rst_pin, !pdata->mcu_rst_level);
        }
        mutex_unlock(&psoc4->lock);
    }

    return count;
}

static DEVICE_ATTR(disable_irq, S_IWUSR | S_IRUGO,
            cypress_psoc4_show_disable_irq,
            cypress_psoc4_store_disable_irq);

static DEVICE_ATTR(suspend_mcu, S_IWUSR | S_IRUGO,
            cypress_psoc4_show_suspend_mcu,
            cypress_psoc4_store_suspend_mcu);

static DEVICE_ATTR(reset_mcu, S_IWUSR | S_IWGRP,
            NULL,
            cypress_psoc4_store_reset_mcu);

static struct attribute *cypress_psoc4_attrs[] = {
    &dev_attr_disable_irq.attr,
    &dev_attr_suspend_mcu.attr,
    &dev_attr_reset_mcu.attr,
    NULL,
};

static struct attribute_group cypress_psoc4_attr_group = {
    .attrs =  cypress_psoc4_attrs,
};
#endif /* CONFIG_CYPRESS_PSOC4_SYSFS */

#ifdef CONFIG_CYPRESS_PSOC4_FASYNC
static int cypress_psoc4_dev_fasync(int fd, struct file *filp, int mode)
{
    struct miscdevice *miscdev = filp->private_data;
    struct cypress_psoc4_drvdata *psoc4 =
                container_of(miscdev, struct cypress_psoc4_drvdata, miscdev);

    return fasync_helper(fd, filp, mode, &psoc4->fasync);
}
#endif /* CONFIG_CYPRESS_PSOC4_FASYNC */

static int cypress_psoc4_dev_open(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct cypress_psoc4_drvdata *psoc4 =
                container_of(miscdev, struct cypress_psoc4_drvdata, miscdev);

    if (atomic_read(&psoc4->opened)) {
        return -EBUSY;
    }
    atomic_inc(&psoc4->opened);

    return 0;
}

static int cypress_psoc4_dev_release(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct cypress_psoc4_drvdata *psoc4 =
                container_of(miscdev, struct cypress_psoc4_drvdata, miscdev);

#ifdef CONFIG_CYPRESS_PSOC4_FASYNC
    cypress_psoc4_dev_fasync(-1, filp, 0);
#endif
    atomic_dec(&psoc4->opened);

    return 0;
}

static long cypress_psoc4_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct cypress_psoc4_drvdata *psoc4 =
                container_of(miscdev, struct cypress_psoc4_drvdata, miscdev);
    struct card_t __user *card = (void __user *)arg;
    unsigned int count = 0;

    switch(cmd) {
    case CYPRESS_DRV_IOC_WAIT_R_CARD:
        if (filp->f_flags & O_NONBLOCK) {
            dev_err(psoc4->dev, "Error, dev is opened with O_NONBLOCK\n");
            return -EAGAIN;
        }
        if (wait_event_interruptible(psoc4->wait, psoc4->is_new_card)) {
            return -ERESTARTSYS;
        }
        break;

    case CYPRESS_DRV_IOC_R_CARD_UID:
        mutex_lock(&psoc4->lock);
        /**
         * Copy data to userspace
         */
        count = sizeof(psoc4->i2cmsg.card);
        if (psoc4->is_new_card) {
            if (copy_to_user(card, (void *)&psoc4->i2cmsg.card, count)) {
                mutex_unlock(&psoc4->lock);
                return -EIO;
            }
            psoc4->is_new_card = false;
        }

        mutex_unlock(&psoc4->lock);
        break;

    case CYPRESS_DRV_IOC_R_CARD_SECTOR:
        mutex_lock(&psoc4->lock);

        /**
         * Copy data from userspace
         */
        count = sizeof(psoc4->i2cmsg.card);
        if (copy_from_user((void *)&psoc4->i2cmsg.card, card, count)) {
            mutex_unlock(&psoc4->lock);
            return -EIO;
        }

        psoc4->i2cmsg.card.type = MSG_TYPE_MASTER_G_SECTOR_DATA;
        psoc4->i2cmsg.card.eof = MSG_PACKET_EOP;
        if (cypress_psoc4_i2c_master_send(psoc4->client,
                    psoc4->i2cmsg.buf, count) != count) {
            mutex_unlock(&psoc4->lock);
            return -EIO;
        }

        mutex_unlock(&psoc4->lock);

        if (wait_event_timeout(psoc4->wait, psoc4->is_data_ready, 1 * HZ) == 0) {
            /**
             * Wait timeout
             */
            return -ETIMEDOUT;
        } else {
            if (!psoc4->is_data_ready)
                return -EIO;
        }

        /**
         * Copy data to userspace
         */
        mutex_lock(&psoc4->lock);
        psoc4->is_data_ready = false;
        if (copy_to_user(card, (void *)&psoc4->i2cmsg.card, count)) {
            mutex_unlock(&psoc4->lock);
            return -EIO;
        }
        mutex_unlock(&psoc4->lock);
        break;

    case CYPRESS_DRV_IOC_SEND_ACK:
        mutex_lock(&psoc4->lock);
        psoc4->i2cmsg.ack.type = MSG_TYPE_MASTER_S_ACK;
        psoc4->i2cmsg.ack.eof = MSG_PACKET_EOP;
        cypress_psoc4_i2c_master_send(psoc4->client, psoc4->i2cmsg.buf, 2);
        mutex_unlock(&psoc4->lock);
        break;

    case CYPRESS_DRV_IOC_RESET_MCU:
        if (gpio_is_valid(psoc4->pdata->mcu_rst_pin)) {
            gpio_direction_output(psoc4->pdata->mcu_rst_pin, psoc4->pdata->mcu_rst_level);
            udelay(5);
            gpio_direction_output(psoc4->pdata->mcu_rst_pin, !psoc4->pdata->mcu_rst_level);
        }
        break;

    case CYPRESS_DRV_IOC_ENABLE_IRQ:
        cypress_psoc4_enable_irq(psoc4);
        break;

    case CYPRESS_DRV_IOC_DISABLE_IRQ:
        cypress_psoc4_disable_irq(psoc4);
        break;

    default:
        dev_err(psoc4->dev, "Not supported CMD:0x%x\n", cmd);
        return -EINVAL;
    }

    return count;
}

static struct file_operations cypress_psoc4_dev_fops = {
    .owner   = THIS_MODULE,
#ifdef CONFIG_CYPRESS_PSOC4_FASYNC
    .fasync  = cypress_psoc4_dev_fasync,
#endif
    .open    = cypress_psoc4_dev_open,
    .release = cypress_psoc4_dev_release,
    .unlocked_ioctl = cypress_psoc4_dev_ioctl,
};

static int cypress_psoc4_input_open(struct input_dev *input)
{
    return 0;
}

static void cypress_psoc4_input_close(struct input_dev *input)
{

}

static void cypress_psoc4_workhandler(struct work_struct *work)
{
    struct cypress_psoc4_drvdata *psoc4 =
                container_of(work, struct cypress_psoc4_drvdata, work);
    unsigned char txcmd[2];
#ifdef CONFIG_CYPRESS_PSOC4_DEBUG
    unsigned short i;
#endif

    mutex_lock(&psoc4->lock);

#ifdef CONFIG_CYPRESS_PSOC4_SYSFS
    psoc4->is_suspend_mcu = false;
    if (gpio_is_valid(psoc4->pdata->mcu_int_pin)) {
        gpio_direction_output(psoc4->pdata->mcu_int_pin, !psoc4->pdata->mcu_int_level);
    }
#endif

    txcmd[0] = MSG_TYPE_MASTER_G_PACKET;
    txcmd[1] = MSG_PACKET_EOP;
    if (cypress_psoc4_i2c_master_send(psoc4->client, txcmd, 2) != 2) {
        goto exit_workhandler;
    }

    /**
     * Receive and parse packet from MCU
     */
    memset(&psoc4->i2cmsg.buf, 0, sizeof(psoc4->i2cmsg.buf));
    cypress_psoc4_i2c_master_recv(psoc4->client, psoc4->i2cmsg.buf, sizeof(psoc4->i2cmsg.buf));

    switch(psoc4->i2cmsg.buf[0]) {
    case MSG_TYPE_SLAVE_S_KEYCODE:
        if (psoc4->i2cmsg.key.eof == MSG_PACKET_EOP) {
            input_event(psoc4->input, EV_KEY, psoc4->i2cmsg.key.code, psoc4->i2cmsg.key.value);
            input_sync(psoc4->input);

#ifdef CONFIG_CYPRESS_PSOC4_DEBUG
            printk("-------------------\n");
            printk("msg_type: %d\n", psoc4->i2cmsg.key.type);
            printk("key_code: %d\n", psoc4->i2cmsg.key.code);
            printk("key_value: %d\n", psoc4->i2cmsg.key.value);
            printk("key_eof: %d\n", psoc4->i2cmsg.key.eof);
            printk("-------------------\n");
#endif
        }
        break;

    case MSG_TYPE_SLAVE_S_CARD_UID:
        if (psoc4->i2cmsg.card.eof == MSG_PACKET_EOP) {
            psoc4->is_new_card = true;
            psoc4->is_data_ready = false;
#ifdef CONFIG_CYPRESS_PSOC4_FASYNC
            kill_fasync(&psoc4->fasync, SIGIO, POLL_IN);
#endif
            wake_up_interruptible(&psoc4->wait);
#ifdef CONFIG_CYPRESS_PSOC4_DEBUG
            printk("-------------------\n");
            for (i = 0; i < 4; i++) {
                printk("uid[%d]=0x%x\n", i, psoc4->i2cmsg.card.uid[i]);
            }
            printk("-------------------\n");
#endif
        }
        break;

    case MSG_TYPE_SLAVE_S_SECTOR_DATA:
        if (psoc4->i2cmsg.card.eof == MSG_PACKET_EOP) {
            psoc4->is_data_ready = true;
#ifdef CONFIG_CYPRESS_PSOC4_DEBUG
            for (i = 0; i < 64; i++) {
                if (i%16 == 0) {
                    printk("\nblock%d:", i/16);
                }
                printk("0x%02x  ", psoc4->i2cmsg.card.data[i]);
            }
            printk("\n");
#endif
            if (waitqueue_active(&psoc4->wait)) {
                wake_up(&psoc4->wait);
            }
        }
        break;

    case MSG_TYPE_SLAVE_S_PM_STATE:
        if (psoc4->i2cmsg.pm.eof == MSG_PACKET_EOP) {
            dev_info(psoc4->dev, "MCU pm state: %d\n", psoc4->i2cmsg.pm.state);
        }
        break;

    default:
        dev_err(psoc4->dev, "Uncertain I2C recv packet type\n");
    }

exit_workhandler:
    mutex_unlock(&psoc4->lock);
}

static irqreturn_t cypress_psoc4_irqhandler(int irq, void *devid)
{
    struct cypress_psoc4_drvdata *psoc4 = devid;

    /**
     * Start workqueue to receive packet from MCU
     */
    schedule_work(&psoc4->work);

    return IRQ_HANDLED;
}

static int cypress_psoc4_gpio_init(struct cypress_psoc4_drvdata *psoc4)
{
    struct cypress_psoc4_platform_data *pdata = psoc4->pdata;
    int error;

    if (gpio_is_valid(pdata->mcu_rst_pin)) {
        error = gpio_request(pdata->mcu_rst_pin, "mcu_rst_pin");
        if (error < 0) {
            dev_err(psoc4->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->mcu_rst_pin, error);
            goto err_gpio_request1;
        }
        gpio_direction_output(pdata->mcu_rst_pin, pdata->mcu_rst_level);
        udelay(5);
        gpio_direction_output(pdata->mcu_rst_pin, !pdata->mcu_rst_level);
    } else {
        dev_err(psoc4->dev, "Invalid mcu_rst_pin: %d\n", pdata->mcu_rst_pin);
        error = -ENODEV;
        goto err_gpio_request1;
    }

    if (gpio_is_valid(pdata->mcu_int_pin)) {
        error = gpio_request(pdata->mcu_int_pin, "mcu_int_pin");
        if (error < 0) {
            dev_err(psoc4->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->mcu_int_pin, error);
            goto err_gpio_request2;
        }
        gpio_direction_output(pdata->mcu_int_pin, !pdata->mcu_int_level);
    }

    if (gpio_is_valid(pdata->cpu_int_pin)) {
        error = gpio_request(pdata->cpu_int_pin, "cpu_int_pin");
        if (error < 0) {
            dev_err(psoc4->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->cpu_int_pin, error);
            goto err_gpio_request3;
        }

        /**
         * Request MCU IO interrupt source
         */
        psoc4->irq = gpio_to_irq(pdata->cpu_int_pin);
        if (psoc4->irq < 0) {
            error = psoc4->irq;
            dev_err(psoc4->dev, "Unable to get irq number for GPIO %d, error %d\n",
                    pdata->cpu_int_pin, error);
            goto err_request_irq;
        }

        error = request_any_context_irq(psoc4->irq,                             \
                                        cypress_psoc4_irqhandler,               \
                                        IRQF_TRIGGER_RISING | IRQF_DISABLED,    \
                                        dev_name(psoc4->dev),                   \
                                        psoc4);
        if (error < 0) {
            dev_err(psoc4->dev, "Unable to clain irq %d, error %d\n",
                    psoc4->irq, error);
            goto err_request_irq;
        } else {
            enable_irq_wake(psoc4->irq);
            psoc4->is_disable_irq = false;
        }
    } else {
        dev_err(psoc4->dev, "Invalid cpu_int_pin: %d\n", pdata->cpu_int_pin);
        error = -ENODEV;
        goto err_gpio_request2;
    }

    return 0;

err_request_irq:
    gpio_free(pdata->cpu_int_pin);
err_gpio_request3:
    gpio_free(pdata->mcu_int_pin);
err_gpio_request2:
    gpio_free(pdata->mcu_rst_pin);
err_gpio_request1:
    return error;
}

static void cypress_psoc4_gpio_free(struct cypress_psoc4_drvdata *psoc4)
{
    struct cypress_psoc4_platform_data *pdata = psoc4->pdata;

    if (psoc4->irq > 0)
        free_irq(psoc4->irq, psoc4);

    gpio_free(pdata->cpu_int_pin);
    gpio_free(pdata->mcu_int_pin);
    gpio_free(pdata->mcu_rst_pin);
}

static int cypress_psoc4_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct cypress_psoc4_platform_data *pdata;
    struct cypress_psoc4_drvdata *psoc4;
    struct input_dev *input;
    int i, error;

    pdata = dev_get_platdata(&client->dev);
    if (!pdata) {
        dev_err(&client->dev, "dev.platform_data cannot be NULL\n");
        error = -ENODEV;
        goto err_probe;
    }

    psoc4 = kzalloc(sizeof(struct cypress_psoc4_drvdata), GFP_KERNEL);
    if (!psoc4) {
        dev_err(&client->dev, "Failed to allocate drvdata memory\n");
        error = -ENOMEM;
        goto err_probe;
    }

    input = input_allocate_device();
    if (!input) {
        dev_err(&client->dev, "Unable to allocate input device\n");
        error = -ENOMEM;
        goto err_input_allocate;
    }

    psoc4->client = client;
    psoc4->pdata = pdata;
    psoc4->input = input;
    psoc4->dev = &client->dev;
    input_set_drvdata(input, psoc4);

    input->id.bustype = BUS_HOST;
    input->id.vendor  = 0x1100;
    input->id.product = 0x0110;
    input->id.version = 0x0011;

    input->name = client->name;
    input->phys = "touch-keys/input";
    input->dev.parent = &client->dev;
    input->open = cypress_psoc4_input_open;
    input->close = cypress_psoc4_input_close;

    __set_bit(EV_KEY, input->evbit);
    __set_bit(EV_SYN, input->evbit);
    for (i = 0; i < pdata->keyscode_num; i++)
        input_set_capability(input, EV_KEY, pdata->keyscode[i]);

    error = input_register_device(input);
    if (error) {
        dev_err(&client->dev, "Unable to register input device, error %d\n", error);
        goto err_input_register;
    }

    mutex_init(&psoc4->lock);
    spin_lock_init(&psoc4->spin_lock);
    atomic_set(&psoc4->opened, 0);
    init_waitqueue_head(&psoc4->wait);
    INIT_WORK(&psoc4->work, cypress_psoc4_workhandler);

    psoc4->irq = -1;
    psoc4->is_disable_irq = true;
    psoc4->is_new_card = false;
    psoc4->is_data_ready = false;

    error = cypress_psoc4_gpio_init(psoc4);
    if (error < 0) {
        goto err_gpio_init;
    }

    psoc4->miscdev.minor = MISC_DYNAMIC_MINOR;
    psoc4->miscdev.name  = client->name;
    psoc4->miscdev.fops  = &cypress_psoc4_dev_fops;
    error = misc_register(&psoc4->miscdev);
    if (error < 0) {
        dev_err(&client->dev, "Unable to register miscdevice, error %d\n", error);
        goto err_misc_register;
    }

#ifdef CONFIG_CYPRESS_PSOC4_SYSFS
    psoc4->class = class_create(THIS_MODULE, "cypress");
    if (IS_ERR(psoc4->class)) {
        error = PTR_ERR(psoc4->class);
        dev_err(&client->dev, "Unable to create class, error %d\n", error);
        goto err_class_create;
    }

    psoc4->dev = device_create(psoc4->class, &client->dev, 0, psoc4, "%s", pdata->name);
    if (IS_ERR(psoc4->dev)) {
        error = PTR_ERR(psoc4->dev);
        dev_err(&client->dev, "Unable to create device, error %d\n", error);
        goto err_device_create;
    }

    error = sysfs_create_group(&psoc4->dev->kobj, &cypress_psoc4_attr_group);
    if (error) {
        dev_err(&client->dev, "Unable to create sysfs group, error %d\n", error);
        goto err_sysfs_create;
    }

    psoc4->is_suspend_mcu = false;
    dev_set_drvdata(psoc4->dev, psoc4);
#endif /* CONFIG_CYPRESS_PSOC4_SYSFS */

    i2c_set_clientdata(client, psoc4);
    return 0;

#ifdef CONFIG_CYPRESS_PSOC4_SYSFS
err_sysfs_create:
    device_destroy(psoc4->class, 0);
err_device_create:
    class_destroy(psoc4->class);
err_class_create:
    misc_deregister(&psoc4->miscdev);
#endif /* CONFIG_CYPRESS_PSOC4_SYSFS */
err_misc_register:
    cypress_psoc4_gpio_free(psoc4);
err_gpio_init:
    input_unregister_device(input);
err_input_register:
    input_free_device(input);
err_input_allocate:
    kfree(psoc4);
err_probe:
    return error;
}

static int cypress_psoc4_remove(struct i2c_client *client)
{
    struct cypress_psoc4_drvdata *psoc4 = i2c_get_clientdata(client);

    cancel_work_sync(&psoc4->work);
#ifdef CONFIG_CYPRESS_PSOC4_SYSFS
    device_destroy(psoc4->class, 0);
    class_destroy(psoc4->class);
    sysfs_remove_group(&psoc4->dev->kobj, &cypress_psoc4_attr_group);
#endif /* CONFIG_CYPRESS_PSOC4_SYSFS */
    misc_deregister(&psoc4->miscdev);
    cypress_psoc4_gpio_free(psoc4);
    input_unregister_device(psoc4->input);
    input_free_device(psoc4->input);
    kfree(psoc4);

    return 0;
}

#ifdef CONFIG_CYPRESS_PSOC4_PM
static int cypress_psoc4_suspend(struct i2c_client *client, pm_message_t mesg)
{
    struct cypress_psoc4_drvdata *psoc4 = i2c_get_clientdata(client);
    unsigned char txcmd[2];

    mutex_lock(&psoc4->lock);
    txcmd[0] = MSG_TYPE_MASTER_S_SUSPEND;
    txcmd[1] = MSG_PACKET_EOP;
    cypress_psoc4_i2c_master_send(psoc4->client, txcmd, 2);
    mutex_unlock(&psoc4->lock);

    if (gpio_is_valid(psoc4->pdata->mcu_int_pin)) {
        gpio_direction_output(psoc4->pdata->mcu_int_pin, psoc4->pdata->mcu_int_level);
    }

    return 0;
}

static int cypress_psoc4_resume(struct i2c_client *client)
{
    struct cypress_psoc4_drvdata *psoc4 = i2c_get_clientdata(client);
#if 0
    unsigned char txcmd[2];

    mutex_lock(&psoc4->lock);
    txcmd[0] = MSG_TYPE_MASTER_S_RESUME;
    txcmd[1] = MSG_PACKET_EOP;
    cypress_psoc4_i2c_master_send(psoc4->client, txcmd, 2);
    mutex_unlock(&psoc4->lock);
#else
    if (gpio_is_valid(psoc4->pdata->mcu_int_pin)) {
        gpio_direction_output(psoc4->pdata->mcu_int_pin, !psoc4->pdata->mcu_int_level);
    }
#endif

    return 0;
}
#endif

static const struct i2c_device_id cypress_psoc4_id_table[] = {
    { "cypress_psoc4", 0 },
    { }
};

static struct i2c_driver cypress_psoc4_i2c_driver = {
    .driver = {
        .name = "cypress_psoc4",
        .owner = THIS_MODULE,
    },
    .probe    = cypress_psoc4_probe,
    .remove   = cypress_psoc4_remove,
#ifdef CONFIG_CYPRESS_PSOC4_PM
    .suspend  = cypress_psoc4_suspend,
    .resume   = cypress_psoc4_resume,
#endif
    .id_table = cypress_psoc4_id_table,
};

static int __init cypress_psoc4_init(void)
{
    return i2c_add_driver(&cypress_psoc4_i2c_driver);
}

static void __exit cypress_psoc4_exit(void)
{
    i2c_del_driver(&cypress_psoc4_i2c_driver);
}

module_init(cypress_psoc4_init);
module_exit(cypress_psoc4_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<qiuwei.wang@ingenic.com>");
MODULE_DESCRIPTION("Cypress PSoC4 driver");
