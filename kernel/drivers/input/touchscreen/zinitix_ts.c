/*
 * include/linux/zinitix_ts.h
 *
 * ZINITIX capacitive touchscreen driver.
 *
 * Copyright 2018, <qiuwei.wang@ingenic.com>
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
#include <linux/input/zinitix_ts.h>

struct zinitix_ts_coord {
    uint16_t x;
    uint16_t y;
    uint8_t width;
    uint8_t sub_status;
};

struct zinitix_ts_event {
    uint16_t status;
    uint16_t event_flag;
    struct zinitix_ts_coord coord[CONFIG_ZINITIX_SUPPORT_FINGER_NUM];
};

struct zinitix_ts_drvdata {
    int irq;
    unsigned short x_max;
    unsigned short y_max;

    struct i2c_client *client;
    struct device *dev;
    struct input_dev *input;
    struct mutex lock;
    struct work_struct work;
    struct workqueue_struct *workqueue;
    struct zinitix_ts_event ts_event;
    struct zinitix_ts_platform_data *pdata;
};

static int zinitix_ts_i2c_send(struct i2c_client *client, uint8_t *buf, uint16_t count)
{
    struct i2c_msg msg;
    int retval;

    msg.addr = client->addr;
    msg.flags = client->flags & I2C_M_TEN;
    msg.len = count;
    msg.buf = buf;
    retval = i2c_transfer(client->adapter, &msg, 1);
    return (retval == 1) ? count : retval;
}

static int zinitix_ts_i2c_recv(struct i2c_client *client, uint8_t *buf, uint16_t count)
{
    struct i2c_msg msg;
    int retval;

    msg.addr = client->addr;
    msg.flags = client->flags & I2C_M_TEN;
    msg.flags |= I2C_M_RD;
    msg.len = count;
    msg.buf = buf;
    retval = i2c_transfer(client->adapter, &msg, 1);
    return (retval == 1) ? count : retval;
}

static int zinitix_ts_read_data(struct i2c_client *client, uint16_t reg, uint8_t *data, uint16_t count)
{
    int retval;
    uint8_t msg[2];

    msg[0] = reg & 0xFF;
    msg[1] = reg >> 8;
    retval = zinitix_ts_i2c_send(client, msg, 2);
    if (retval < 0)
        return retval;
    else if (retval != 2)
        return -EIO;
    return zinitix_ts_i2c_recv(client, data, count);
}

static inline int zinitix_ts_write_cmd(struct i2c_client *client, uint16_t reg)
{
    uint8_t msg[2];

    msg[0] = reg & 0xFF;
    msg[1] = reg >> 8;
    return zinitix_ts_i2c_send(client, msg, 2);
}

static int zinitix_ts_write_reg(struct i2c_client *client, uint16_t reg, uint16_t val)
{
    uint8_t msg[4];

    msg[0] = reg & 0xFF;
    msg[1] = reg >> 8;
    msg[2] = val & 0xFF;
    msg[3] = val >> 8;
    return zinitix_ts_i2c_send(client, msg, 4);
}

#ifdef CONFIG_ZINITIX_ONESHOT_UPGRADE
static int zinitix_ts_write_data(struct i2c_client *client, uint16_t reg, uint8_t *data, uint16_t count)
{
    int retval;
    uint8_t *msg = kzalloc(count + 2, GFP_KERNEL);
    if (!msg) {
        dev_err(&client->dev, "Failed to allocate msg memory\n");
        return -ENOMEM;
    }
    memcpy(msg + 2, data, count);
    msg[0] = reg & 0xFF;
    msg[1] = reg >> 8;
    retval = zinitix_ts_i2c_send(client, msg, count + 2);
    kfree(msg);
    return retval;
}
#endif

static int zinitix_ts_init_touch(struct zinitix_ts_drvdata *zinitix)
{
    int i, retval;
    uint16_t fw_version;
    uint16_t reg_version;
    struct i2c_client *client = zinitix->client;

    for (i = 0; i < 10; i++) {
        if (zinitix_ts_write_cmd(client, ZINITIX_SWRESET_CMD) > 0)
            break;
        msleep(1);
    }

    retval = zinitix_ts_read_data(client, ZINITIX_FIRMWARE_VERSION, (uint8_t  *)&fw_version, 2);
    if (retval < 0)
        return retval;
    dev_info(zinitix->dev, "firmware version: 0x%04x\n", fw_version);

    retval = zinitix_ts_read_data(client, ZINITIX_DATA_VERSION_REG, (uint8_t *)&reg_version, 2);
    if (retval < 0)
        return retval;
    dev_info(zinitix->dev, "register data version: 0x%04x\n", reg_version);

#ifdef CONFIG_ZINITIX_ONESHOT_UPGRADE

#endif

    retval = zinitix_ts_write_reg(client, ZINITIX_INITIAL_TOUCH_MODE, TOUCH_POINT_MODE);
    if (retval < 0)
        return retval;
    retval = zinitix_ts_write_reg(client, ZINITIX_TOUCH_MODE, TOUCH_POINT_MODE);
    if (retval < 0)
        return retval;
    retval = zinitix_ts_write_reg(client, ZINITIX_SUPPORTED_FINGER_NUM, (uint16_t)CONFIG_ZINITIX_SUPPORT_FINGER_NUM);
    if (retval < 0)
        return retval;
    retval = zinitix_ts_write_reg(client, ZINITIX_X_RESOLUTION, (uint16_t)zinitix->x_max);
    if (retval < 0)
        return retval;
    retval = zinitix_ts_write_reg(client, ZINITIX_Y_RESOLUTION, (uint16_t)zinitix->y_max);
    if (retval < 0)
        return retval;
    retval = zinitix_ts_write_cmd(client, ZINITIX_CALIBRATE_CMD);
    if (retval < 0)
        return retval;

#ifdef CONFIG_ZINITIX_ONESHOT_UPGRADE

#endif

    for (i = 0; i < 10; i++) {
        retval = zinitix_ts_write_cmd(client, ZINITIX_CLEAR_INT_STATUS_CMD);
        if (retval < 0)
            continue;
    }
    return retval;
}

static int zinitix_ts_power_sequence(struct zinitix_ts_drvdata *zinitix)
{
    int retval;
    uint16_t chip_id;
    struct i2c_client *client = zinitix->client;

    retval = zinitix_ts_write_reg(client, 0xc000, 0x0001);
    if (retval < 0)
        return retval;

    retval = zinitix_ts_read_data(client, 0xcc00, (uint8_t *)&chip_id, 2);
    if (retval < 0)
        return retval;
    dev_info(zinitix->dev, "chip id: 0x%04x\n", chip_id);

    retval = zinitix_ts_write_cmd(client, 0xc004);
    if (retval < 0)
        return retval;
    udelay(10);

    retval = zinitix_ts_write_reg(client, 0xc002, 0x0001);
    if (retval < 0)
        return retval;
    msleep(1);

    retval = zinitix_ts_write_reg(client, 0xc001, 0x0001);
    if (retval < 0)
        return retval;
    msleep(FIRMWARE_ON_DELAY);

    return 0;
}

static void zinitix_ts_power_ctrl(struct zinitix_ts_drvdata *zinitix, bool power_en)
{
    struct zinitix_ts_platform_data *pdata = zinitix->pdata;

    if (power_en) {
        if (gpio_is_valid(pdata->ts_pwr_pin)) {
            gpio_direction_output(pdata->ts_pwr_pin, !pdata->pwr_en_level);
            msleep(1);
        }
        gpio_direction_output(pdata->ts_rst_pin, 1);
        msleep(2);
        gpio_direction_output(pdata->ts_rst_pin, 0);
        msleep(2);
        gpio_direction_output(pdata->ts_rst_pin, 1);
        msleep(CHIP_ON_DELAY);
    } else {
        if (gpio_is_valid(pdata->ts_pwr_pin))
            gpio_direction_output(pdata->ts_pwr_pin, !pdata->pwr_en_level);
        gpio_direction_output(pdata->ts_rst_pin, 0);
        msleep(CHIP_OFF_DELAY);
    }
}

static void zinitix_ts_workhandler(struct work_struct *work)
{
    struct zinitix_ts_drvdata *zinitix =
                container_of(work, struct zinitix_ts_drvdata, work);
    struct zinitix_ts_event *ts_event = &zinitix->ts_event;
    struct i2c_client *client = zinitix->client;
    int i, retval;

    mutex_lock(&zinitix->lock);
    /**
     * Read touch-point info
     */
    retval = zinitix_ts_read_data(client, ZINITIX_POINT_STATUS_REG, (uint8_t *)ts_event, 8);
    if (retval < 0 || ts_event->status == 0) {
        if (retval < 0)
            dev_err(zinitix->dev, "Failed to read point reg, L%d\n", __LINE__);
        goto exit_workhandler;
    }
    retval = zinitix_ts_read_data(client, ZINITIX_POINT_STATUS_REG+4, (uint8_t *)ts_event+8, 2);
    if (retval < 0) {
        dev_err(zinitix->dev, "Failed to read point reg, L%d\n", __LINE__);
        goto exit_workhandler;
    }
    for (i = 1; i < CONFIG_ZINITIX_SUPPORT_FINGER_NUM; i++) {
        if (zinitix_bit_test(ts_event->event_flag, i)) {
            retval = zinitix_ts_read_data(client, ZINITIX_POINT_STATUS_REG+(i*4)+2,
                        (uint8_t *)(&ts_event->coord[i]), sizeof(struct zinitix_ts_coord));
            if (retval < 0) {
                dev_err(zinitix->dev, "Failed to read point reg, L%d\n", __LINE__);
                goto exit_workhandler;
            }
        }
    }

    /**
     * Report touch-point info
     */
    for (i = 0; i < CONFIG_ZINITIX_SUPPORT_FINGER_NUM; i++) {
        if (zinitix_bit_test(ts_event->coord[i].sub_status, SUB_BIT_DOWN) ||
            zinitix_bit_test(ts_event->coord[i].sub_status, SUB_BIT_MOVE) ||
            zinitix_bit_test(ts_event->coord[i].sub_status, SUB_BIT_EXIST)) {
#if CONFIG_ZINITIX_SUPPORT_FINGER_NUM > 1
            input_report_abs(zinitix->input, ABS_MT_POSITION_X, ts_event->coord[i].x);
            input_report_abs(zinitix->input, ABS_MT_POSITION_Y, ts_event->coord[i].y);
            input_report_abs(zinitix->input, ABS_MT_TOUCH_MAJOR, ts_event->coord[i].width);
            input_report_abs(zinitix->input, ABS_MT_WIDTH_MAJOR, ts_event->coord[i].width);
            input_mt_sync(zinitix->input);
#else
            input_report_abs(zinitix->input, ABS_X, ts_event->coord[i].x);
            input_report_abs(zinitix->input, ABS_Y, ts_event->coord[i].y);
            input_report_abs(zinitix->input, ABS_PRESSURE, ts_event->coord[i].width);
            input_report_key(zinitix->input, BTN_TOUCH, 1);
#endif
        } else if (zinitix_bit_test(ts_event->coord[i].sub_status, SUB_BIT_UP)) {
#if CONFIG_ZINITIX_SUPPORT_FINGER_NUM > 1
            input_report_abs(zinitix->input, ABS_MT_TOUCH_MAJOR, 0);
            input_report_abs(zinitix->input, ABS_MT_WIDTH_MAJOR, 0);
            input_mt_sync(zinitix->input);
#else
            input_report_abs(zinitix->input, ABS_PRESSURE, 0);
            input_report_key(zinitix->input, BTN_TOUCH, 0);
#endif
        } else {
            memset(&ts_event->coord[i], 0, sizeof(struct zinitix_ts_coord));
        }
    }
    input_sync(zinitix->input);

exit_workhandler:
    zinitix_ts_write_cmd(client, ZINITIX_CLEAR_INT_STATUS_CMD);
    mutex_unlock(&zinitix->lock);
    enable_irq(zinitix->irq);
}

static irqreturn_t zinitix_ts_irqhandler(int irq, void *devid)
{
    struct zinitix_ts_drvdata *zinitix = devid;

    disable_irq_nosync(zinitix->irq);
    if (!work_pending(&zinitix->work))
        queue_work(zinitix->workqueue, &zinitix->work);
    else
        enable_irq(zinitix->irq);
    return IRQ_HANDLED;
}

static int zinitix_ts_gpio_init(struct zinitix_ts_drvdata *zinitix)
{
    struct zinitix_ts_platform_data *pdata = zinitix->pdata;
    int error;

    if (gpio_is_valid(pdata->ts_pwr_pin)) {
        error = gpio_request(pdata->ts_pwr_pin, "ts_pwr_pin");
        if (error < 0) {
            dev_err(zinitix->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->ts_pwr_pin, error);
            goto err_gpio_request1;
        }
        gpio_direction_output(pdata->ts_pwr_pin, !pdata->pwr_en_level);
    }

    if (gpio_is_valid(pdata->ts_rst_pin)) {
        error = gpio_request(pdata->ts_rst_pin, "ts_rst_pin");
        if (error < 0) {
            dev_err(zinitix->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->ts_rst_pin, error);
            goto err_gpio_request2;
        }
        gpio_direction_output(pdata->ts_rst_pin, 0);
    } else {
        dev_err(zinitix->dev, "Invalid ts_rst_pin: %d\n", pdata->ts_rst_pin);
        error = -ENODEV;
        goto err_gpio_request2;
    }

    if (gpio_is_valid(pdata->ts_int_pin)) {
        error = gpio_request(pdata->ts_int_pin, "ts_int_pin");
        if (error < 0) {
            dev_err(zinitix->dev, "Failed to request GPIO %d, error %d\n",
                    pdata->ts_int_pin, error);
            goto err_gpio_request3;
        }

        /**
         * Request TP interrupt source
         */
        zinitix->irq = gpio_to_irq(pdata->ts_int_pin);
        if (zinitix->irq < 0) {
            dev_err(zinitix->dev, "Unable to get irq number for GPIO %d, error %d\n",
                    pdata->ts_int_pin, error);
            goto err_request_irq;
        }
        error = request_any_context_irq(zinitix->irq,                             \
                                        zinitix_ts_irqhandler,                    \
                                        IRQF_TRIGGER_FALLING | IRQF_DISABLED,     \
                                        dev_name(zinitix->dev),                   \
                                        zinitix);
        if (error < 0) {
            dev_err(zinitix->dev, "Unable to clain irq %d, error %d\n",
                    zinitix->irq, error);
            goto err_request_irq;
        } else {
            disable_irq(zinitix->irq);
        }
    } else {
        dev_err(zinitix->dev, "Invalid ts_int_pin: %d\n", pdata->ts_int_pin);
        error = -ENODEV;
        goto err_gpio_request3;
    }

    return 0;

err_request_irq:
    gpio_free(pdata->ts_int_pin);
err_gpio_request3:
    gpio_free(pdata->ts_rst_pin);
err_gpio_request2:
    gpio_free(pdata->ts_pwr_pin);
err_gpio_request1:
    return error;
}

static void zinitix_ts_gpio_free(struct zinitix_ts_drvdata *zinitix)
{
    struct zinitix_ts_platform_data *pdata = zinitix->pdata;

    free_irq(zinitix->irq, zinitix);
    gpio_free(pdata->ts_int_pin);
    gpio_free(pdata->ts_rst_pin);
    gpio_free(pdata->ts_pwr_pin);
}

static int zinitix_ts_input_open(struct input_dev *input)
{
    struct zinitix_ts_drvdata *zinitix = input_get_drvdata(input);

    zinitix_ts_power_ctrl(zinitix, 1);
    zinitix_ts_power_sequence(zinitix);
    zinitix_ts_init_touch(zinitix);
    enable_irq(zinitix->irq);
    return 0;
}

static void zinitix_ts_input_close(struct input_dev *input)
{
    struct zinitix_ts_drvdata *zinitix = input_get_drvdata(input);

    disable_irq(zinitix->irq);
    zinitix_ts_power_ctrl(zinitix, 0);
}

static int zinitix_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct zinitix_ts_drvdata *zinitix;
    struct zinitix_ts_platform_data *pdata;
    struct input_dev *input;
    int error;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dev_dbg(&client->dev, "Soc no I2C function\n");
        return -ENODEV;
    }

    pdata = dev_get_platdata(&client->dev);
    if (!pdata) {
        dev_dbg(&client->dev, "dev.platform_data cannot be NULL\n");
        return -ENODEV;
    }

    zinitix = kzalloc(sizeof(struct zinitix_ts_drvdata), GFP_KERNEL);
    if (!zinitix) {
        dev_err(&client->dev, "Failed to allocate drvdata memory\n");
        return -ENOMEM;
    }

    input = input_allocate_device();
    if (!input) {
        dev_err(&client->dev, "Unable to allocate input device\n");
        error = -ENOMEM;
        goto err_input_allocate;
    }

    zinitix->client = client;
    zinitix->dev = &client->dev;
    zinitix->input = input;
    zinitix->pdata = pdata;
    zinitix->x_max = pdata->x_max;
    zinitix->y_max = pdata->y_max;

    input->name = client->name;
    input->id.bustype = BUS_I2C;
    input->id.vendor  = 0xAA00;
    input->id.product = 0x0BB0;
    input->id.version = 0x1000;

#if CONFIG_ZINITIX_SUPPORT_FINGER_NUM > 1
    set_bit(ABS_MT_POSITION_X, input->absbit);
    set_bit(ABS_MT_POSITION_Y, input->absbit);
    set_bit(ABS_MT_TOUCH_MAJOR, input->absbit);
    set_bit(ABS_MT_WIDTH_MAJOR, input->absbit);

    input_set_abs_params(input, ABS_MT_POSITION_X, 0, zinitix->x_max, 0, 0);
    input_set_abs_params(input, ABS_MT_POSITION_Y, 0, zinitix->y_max, 0, 0);
    input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, 0xFF, 0, 0);
    input_set_abs_params(input, ABS_MT_WIDTH_MAJOR, 0, 0xFF, 0, 0);
#else
    set_bit(ABS_X, input->absbit);
    set_bit(ABS_Y, input->absbit);
    set_bit(ABS_PRESSURE, input->absbit);

    input_set_abs_params(input, ABS_X, 0, zinitix->x_max, 0, 0);
    input_set_abs_params(input, ABS_Y, 0, zinitix->y_max, 0, 0);
    input_set_abs_params(input, ABS_PRESSURE, 0, 0xFF, 0, 0);

    set_bit(EV_KEY, input->evbit);
    set_bit(BTN_TOUCH, input->keybit);
#endif

    set_bit(EV_ABS, input->evbit);
    set_bit(EV_SYN, input->evbit);

    error = input_register_device(input);
    if (error) {
        dev_err(zinitix->dev, "Unable to register input device, error %d\n", error);
        goto err_input_register;
    }

    input->open = zinitix_ts_input_open;
    input->close = zinitix_ts_input_close;
    input_set_drvdata(input, zinitix);

    error = zinitix_ts_gpio_init(zinitix);
    if (error < 0) {
        goto err_gpio_init;
    }

    zinitix->workqueue = create_workqueue("zinitix");
    if (!zinitix->workqueue) {
        dev_err(zinitix->dev, "Failed to creat workqueue\n");
        error = -ENOMEM;
        goto err_creat_workqueue;
    }

    INIT_WORK(&zinitix->work, zinitix_ts_workhandler);
    mutex_init(&zinitix->lock);
    i2c_set_clientdata(client, zinitix);
    return 0;

err_creat_workqueue:
    zinitix_ts_gpio_free(zinitix);
err_gpio_init:
    input_unregister_device(input);
err_input_register:
    input_free_device(input);
err_input_allocate:
    kfree(zinitix);
    return error;
}

static int zinitix_ts_remove(struct i2c_client *client)
{
    struct zinitix_ts_drvdata *zinitix = i2c_get_clientdata(client);

    cancel_work_sync(&zinitix->work);
    destroy_workqueue(zinitix->workqueue);
    mutex_destroy(&zinitix->lock);
    zinitix_ts_gpio_free(zinitix);
    input_unregister_device(zinitix->input);
    input_free_device(zinitix->input);
    kfree(zinitix);

    return 0;
}

static int zinitix_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    struct zinitix_ts_drvdata *zinitix = i2c_get_clientdata(client);

    cancel_work_sync(&zinitix->work);
    zinitix_ts_power_ctrl(zinitix, 0);
    return 0;
}

static int zinitix_ts_resume(struct i2c_client *client)
{
    struct zinitix_ts_drvdata *zinitix = i2c_get_clientdata(client);

    zinitix_ts_power_ctrl(zinitix, 1);
    zinitix_ts_power_sequence(zinitix);
    zinitix_ts_init_touch(zinitix);
    return 0;
}

static const struct i2c_device_id zinitix_ts_id_table[] = {
    { "zinitix", 0 },
    { }
};

static struct i2c_driver zinitix_ts_i2c_driver = {
    .driver = {
        .name = "zinitix",
        .owner = THIS_MODULE,
    },
    .probe   = zinitix_ts_probe,
    .remove   = zinitix_ts_remove,
    .suspend  = zinitix_ts_suspend,
    .resume   = zinitix_ts_resume,
    .id_table = zinitix_ts_id_table,
};

static int __init zinitix_ts_init(void)
{
    return i2c_add_driver(&zinitix_ts_i2c_driver);
}

static void __exit zinitix_ts_exit(void)
{
    i2c_del_driver(&zinitix_ts_i2c_driver);
}

module_init(zinitix_ts_init);
module_exit(zinitix_ts_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<qiuwei.wang@ingenic.com>");
MODULE_DESCRIPTION("ZINITIX TP driver");
