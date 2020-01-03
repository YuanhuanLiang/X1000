/*
 * linux/drivers/misc/em20918.c
 *
 * em20918 control interface driver
 *
 * Copyright 2018, Howrd <kai.shen@ingenic.com>
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/async.h>
#include <linux/ioport.h>
#include <linux/wakelock.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/input/em20918.h>




/* device name */
#define EM20918_NAME                    "ir_em20918"
#define LABEL_EM20918                   "ir_em20918 int"

/* IR ioctl define */
#define IR_IOC_IR_SUSPEND               _IOW('I', 1, int)
#define IR_IOC_IR_RESUME                _IOW('I', 2, int)
#define IR_IOC_OP_REG                   _IOW('I', 3, int)
#define IR_IOC_CAPTURE_SETTING          _IOW('I', 4, int)

/* reg operate */
#define REG_READ                        1
#define REG_WRITE                       2




struct em20918_dev_data {
    struct i2c_client *client;
    struct em20918_platform_data *pdata;
    int irq;

    struct mutex lock;

    struct miscdevice miscdev;
    atomic_t opened;

    struct workqueue_struct *work_queue;
    struct work_struct irq_work;

    int timer_status;
    struct timer_list capture_timer;

    struct input_dev *input;

    struct reg_op_t reg_op;
    struct capture_setting_t capture_setting;
    u32 capture_int_num;
};


static void em20918_capture_setting(struct em20918_dev_data *dev_data);
static int em20918_suspend(struct i2c_client *client, pm_message_t mesg);
static int em20918_resume(struct i2c_client *client);


/* reg table */
static struct em20918_reg_table {
    struct reg_status_t reg_pid;
    struct reg_status_t reg_config;
    struct reg_status_t reg_interrupt;
    struct reg_status_t reg_ps_lt;
    struct reg_status_t reg_ps_ht;
    struct reg_status_t reg_ps_data;
    struct reg_status_t reg_reset;
    struct reg_status_t reg_offset;
}
em20918_reg_table = {
    .reg_pid        = {.reg_addr = EM20918_REG_PID,       .default_val = EM20918_PID,},
    .reg_config     = {.reg_addr = EM20918_REG_CONFIG,    .default_val = PS_EN_EN | PS_SLP_100MS | PS_DR_200MA | 0x3,},
    .reg_interrupt  = {.reg_addr = EM20918_REG_INTERRUPT, .default_val = DEF_ZERO,},
    .reg_ps_lt      = {.reg_addr = EM20918_REG_PS_LT,     .default_val = DEF_ZERO,},
    .reg_ps_ht      = {.reg_addr = EM20918_REG_PS_HT,     .default_val = DEF_0XFF,},
    .reg_ps_data    = {.reg_addr = EM20918_REG_PS_DATA,   .default_val = DEF_ZERO,},
    .reg_reset      = {.reg_addr = EM20918_REG_RESET,     .default_val = DEF_ZERO,},
    .reg_offset     = {.reg_addr = EM20918_REG_OFFSET,    .default_val = DEF_ZERO,},
};



void multi_set_bit (u8 *val, u8 bit_start, u8 bits, u8 mask)
{
    *val &= ~mask;
    *val |= bits << bit_start;
}

#if 0
/*
 * note: em20918 can`t use this interface, will make sda pull down
 *       please use em20918_i2c_read2()
 */
static int em20918_i2c_read(const struct i2c_client *client, u8 reg_addr, u8 *val)
{
#if 0
    *val = i2c_smbus_read_byte_data(client, reg_addr);
#else
    int ret;
    struct i2c_msg msg[2];

    msg[0].addr     = client->addr;  //i2c addr 7bit
    msg[0].flags    = client->flags;
    msg[0].len      = 1;
    msg[0].buf      = &reg_addr;

    msg[1].addr     = client->addr;
    msg[1].flags    = I2C_M_RD;
    msg[1].len      = 1;
    msg[1].buf      = val;

    ret = i2c_transfer(client->adapter, msg, 2);
    if (ret < 0)
        dev_err(&client->dev, "%s:i2c read error\n", __func__);
    return ret;
#endif
}
#endif

/*
 * note: em20918 use this interface to read reg, only frist reg is correct
 *       the others are all wrong
 */
static int em20918_i2c_read_page(const struct i2c_client *client, u8 reg_addr, u8 *val, u32 len)
{
    int ret;
    struct i2c_msg msg[2];

    msg[0].addr     = client->addr;  //i2c addr 7bit
    msg[0].flags    = client->flags;
    msg[0].len      = 1;
    msg[0].buf      = &reg_addr;

    msg[1].addr     = client->addr;
    msg[1].flags    = I2C_M_RD;
    msg[1].len      = len;
    msg[1].buf      = val;

    ret = i2c_transfer(client->adapter, msg, 2);
    if (ret < 0)
        dev_err(&client->dev, "%s:i2c read error\n", __func__);
    return ret;
}

static int em20918_i2c_read2(const struct i2c_client *client, u8 reg_addr, u8 *val)
{
    int ret;
    u8 tmp[2];

    ret  = em20918_i2c_read_page(client, reg_addr, tmp, 2);
    *val = tmp[0];

    return ret;
}


static int em20918_i2c_write(const struct i2c_client *client, u8 reg_addr, u8 val)
{
#if 0
    return i2c_smbus_write_byte_data(client, reg_addr, val);
#else
    int ret;
    struct i2c_msg msg;
    u8 send[2];

    send[0]     = reg_addr;
    send[1]     = val;

    msg.addr    = client->addr;
    msg.flags   = client->flags;
    msg.len     = 2;
    msg.buf     = send;

    ret = i2c_transfer(client->adapter, &msg, 1);
    if (ret < 0)
        dev_err(&client->dev, "%s:i2c write error\n", __func__);
    return ret;
#endif
}

#if 0
//no test
static int em20918_i2c_write_page(const struct i2c_client *client, u8 reg_addr, u8 *val, u32 len)
{
    struct i2c_msg msg;
    u8 send[1+len];

    send[0]     = reg_addr;
    memcpy(&send[1], val, len*sizeof(u8));

    msg.addr    = client->addr;
    msg.flags   = client->flags;
    msg.len     = 1+len;
    msg.buf     = send;

    return i2c_transfer(client->adapter, &msg, 1);
}
#endif

static void em20918_print_reg_table(struct em20918_dev_data *dev_data)
{
    int i;
    struct reg_status_t *reg_current;

    reg_current = (struct reg_status_t*)&em20918_reg_table;
    for (i=0; i<sizeof(em20918_reg_table)/sizeof(struct reg_status_t); i++) {
        em20918_i2c_read2(dev_data->client, reg_current->reg_addr, &reg_current->current_val);
        printk("reg: %x[%x]\n", reg_current->reg_addr, reg_current->current_val);

        reg_current++;
    }
}

static int em20918_dev_open(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct em20918_dev_data *dev_data = container_of(miscdev, struct em20918_dev_data, miscdev);

    if (atomic_read(&dev_data->opened)) {
        printk("EBUSY\n");
        return -EBUSY;
    }

    atomic_inc(&dev_data->opened);

    return 0;
}

static int em20918_dev_release(struct inode *inode, struct file *filp)
{
    struct miscdevice *miscdev = filp->private_data;
    struct em20918_dev_data *dev_data = container_of(miscdev, struct em20918_dev_data, miscdev);

    atomic_dec(&dev_data->opened);

    return 0;
}


static long em20918_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct miscdevice *miscdev = filp->private_data;
    struct em20918_dev_data *dev_data = container_of(miscdev, struct em20918_dev_data, miscdev);
    int ret;
    pm_message_t mesg; /* no effect */

    switch(cmd) {
    case IR_IOC_IR_SUSPEND:
        printk("ir suspend\n");
        em20918_suspend(dev_data->client, mesg);

        break;

    case IR_IOC_IR_RESUME:
        printk("ir resume\n");
        em20918_resume(dev_data->client);
        //em20918_print_reg_table(dev_data);

        break;

    case IR_IOC_OP_REG:
        if (copy_from_user(&dev_data->reg_op, (void __user *)arg, sizeof(struct reg_op_t))) {
            printk("IR_IOC_OP_REG: copy_from_user error\n");
            return  -EFAULT;
        }

        if (REG_READ == dev_data->reg_op.op) {
            ret = em20918_i2c_read2(dev_data->client, dev_data->reg_op.reg_addr, &dev_data->reg_op.val);
            if (ret < 0) {
                printk("IR_IOC_OP_REG: em20918_i2c_read2 err: %d\n", ret);
                return -1;
            }
            if (dev_data->reg_op.val < 0) {
                printk("IR_IOC_OP_REG: em20918_i2c_read err: %d\n", dev_data->reg_op.val);
                return dev_data->reg_op.val;
            }

            if (copy_to_user((void __user *)arg, &dev_data->reg_op, sizeof(struct reg_op_t))) {
                printk("IR_IOC_OP_REG: copy_to_user error\n");
                return -EFAULT;
            }
        } else if (REG_WRITE == dev_data->reg_op.op) {
            ret = em20918_i2c_write(dev_data->client, dev_data->reg_op.reg_addr, dev_data->reg_op.val);
            if (ret < 0) {
                printk("IR_IOC_OP_REG: em20918_i2c_write err: %d\n", ret);
                return -1;
            }
        } else {
            printk("IR_IOC_OP_REG: param err\n");
            return -1;
        }

        break;

    case IR_IOC_CAPTURE_SETTING:
        if (copy_from_user(&dev_data->capture_setting, (void __user *)arg, sizeof(struct capture_setting_t))) {
            printk("IR_IOC_CAPTURE_SETTING: copy_from_user error\n");
            return -EFAULT;
        }

        em20918_capture_setting(dev_data);

        break;

    default:
        dev_err(&dev_data->client->dev, "Not supported CMD:0x%x\n", cmd);
        return -EINVAL;
    }

    return 0;
}

static struct file_operations em20918_dev_fops = {
    .owner                             = THIS_MODULE,
    .open                              = em20918_dev_open,
    .release                           = em20918_dev_release,
    .unlocked_ioctl                    = em20918_dev_ioctl,
};


static irqreturn_t em20918_interrupt(int irq, void *data)
{
    struct em20918_dev_data *dev_data = (struct em20918_dev_data *)data;

    queue_work(dev_data->work_queue, &dev_data->irq_work);

    return IRQ_HANDLED;
}

static void em20918_irq_handler(struct work_struct *irq_work)
{
    struct em20918_dev_data *dev_data = container_of(irq_work, struct em20918_dev_data, irq_work);
    u8 val;

#if 0
    em20918_i2c_read2(dev_data->client, em20918_reg_table.reg_interrupt.reg_addr, &val);
    printk("int flag: %x\n", val);
    multi_set_bit (&val, PS_FLAG_BIT_OFFSET, PS_FLAG_INT_CLEAR, PS_FLAG_MASK);
    multi_set_bit (&val, ALS_FLAG_BIT_OFFSET, ALS_FLAG_INT_CLEAR, ALS_FLAG_MASK);
    em20918_i2c_write(dev_data->client, em20918_reg_table.reg_interrupt.reg_addr, val);
#else
    em20918_i2c_write(dev_data->client, em20918_reg_table.reg_interrupt.reg_addr, 0);
#endif

    em20918_i2c_read2(dev_data->client, em20918_reg_table.reg_ps_data.reg_addr, &val);
    //printk("+++++++++++++++ %d\n", val);

    /* detected interrupt will report dev_data->pdata->interrupt_key_code and ps data */
    input_event(dev_data->input, EV_MSC, dev_data->pdata->interrupt_key_code, val);
    input_sync(dev_data->input);

    dev_data->capture_int_num++;
}

static void em20918_capture_setting(struct em20918_dev_data *dev_data)
{
    //printk("capture: %d -- %d\n", dev_data->capture_setting.capture_time_ms, dev_data->capture_setting.capture_num);
    if (dev_data->capture_setting.capture_time_ms <= 0 ||
        dev_data->capture_setting.capture_num <= 0) {
        del_timer(&dev_data->capture_timer);
    } else {
        mod_timer(&dev_data->capture_timer, jiffies);
    }
}

static void em20918_timer_handler(unsigned long data)
{
    struct em20918_dev_data *dev_data = (struct em20918_dev_data *)data;

    if (dev_data->capture_int_num >= dev_data->capture_setting.capture_num) {
        //printk("--------------- %d\n", dev_data->capture_int_num);

        /* when detected capture >= capture setting value, will report dev_data->pdata->capture_key_code and capture setting value */
        input_event(dev_data->input, EV_MSC, dev_data->pdata->capture_key_code, dev_data->capture_int_num);
        input_sync(dev_data->input);
    }

    mod_timer(&dev_data->capture_timer, jiffies + msecs_to_jiffies(dev_data->capture_setting.capture_time_ms));

    dev_data->capture_int_num = 0;
}


static int em20918_probe(struct i2c_client *client, const struct i2c_device_id *did)
{
    struct em20918_dev_data *dev_data;
    struct input_dev *input_device;
    int ret = 0;
    u8 val;

    pr_info("%s enter!\n", __func__);

    /* i2c init */
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dev_err(&client->dev, "I2C functionality not supported\n");
        return -ENODEV;
    }
    dev_data = kzalloc(sizeof(struct em20918_dev_data), GFP_KERNEL);
    if (dev_data == NULL) {
        dev_err(&client->dev, "Failed to malloc em20918_dev_data\n");
        return -ENOMEM;
    }

    i2c_set_clientdata(client, dev_data);

    dev_data->client = client;
    dev_data->pdata = dev_get_platdata(&client->dev);


    /* IR device init */
    dev_data->miscdev.minor = MISC_DYNAMIC_MINOR;
    dev_data->miscdev.name  = client->name;
    dev_data->miscdev.fops  = &em20918_dev_fops;
    ret = misc_register(&dev_data->miscdev);
    if (ret < 0) {
        dev_err(&client->dev, "misc_register failed\n");
        goto err1;
    }


    /*  work queue & interupt int */
    dev_data->work_queue = create_singlethread_workqueue("em20918 work queue");
    INIT_WORK(&dev_data->irq_work, em20918_irq_handler);

    if (gpio_is_valid(dev_data->pdata->int_pin)) {
        ret = gpio_request(dev_data->pdata->int_pin, LABEL_EM20918);
        if (ret < 0) {
            dev_err(&client->dev, "gpio_request failed: %s [%d], ret=%d\n", LABEL_EM20918, dev_data->pdata->int_pin, ret);
            goto err2;
        }
        gpio_direction_input(dev_data->pdata->int_pin);

        dev_data->irq = gpio_to_irq(dev_data->pdata->int_pin);
        ret = request_irq(dev_data->irq, em20918_interrupt,
                IRQF_TRIGGER_FALLING, client->dev.driver->name,
                dev_data);
        if (ret < 0) {
            dev_err(&client->dev, "ir: request irq fail\n");
            goto err3;
        }

        disable_irq(dev_data->irq);
    } else
        goto err2;


    /* timer init */
    memset(&dev_data->capture_setting, 0 , sizeof(struct capture_setting_t));
    setup_timer(&dev_data->capture_timer, em20918_timer_handler, (unsigned long)dev_data);


    /* input init */
    input_device = input_allocate_device();
    if (!input_device) {
        ret = -ENOMEM;
        dev_err(&client->dev, "fail to allocate input device\n");
        goto err4;
    }
    input_device->name = EM20918_NAME;
    input_device->dev.parent = &client->dev;

    input_device->id.bustype = BUS_I2C;
    input_device->id.vendor = 0xDEAD;
    input_device->id.product = 0xBEEF;
    input_device->id.version = 10427;
    dev_data->input = input_device;

    set_bit(EV_MSC, input_device->evbit);
    set_bit(dev_data->pdata->interrupt_key_code, input_device->mscbit);
    set_bit(dev_data->pdata->capture_key_code, input_device->mscbit);
    input_set_abs_params(input_device, dev_data->pdata->interrupt_key_code, 0, 0xFF, 0, 0);

    ret = input_register_device(input_device);
    if (ret) {
        dev_err(&client->dev, "fail to register input device\n");
        goto err5;
    }

#if 0
//for test
    em20918_print_reg_table(dev_data);

    em20918_i2c_write(dev_data->client, em20918_reg_table.reg_ps_lt.reg_addr, 2);
    em20918_i2c_write(dev_data->client, em20918_reg_table.reg_ps_ht.reg_addr, 5);

    em20918_i2c_read2(dev_data->client, em20918_reg_table.reg_config.reg_addr, &val);
    multi_set_bit (&val, PS_DR_BIT_OFFSET, PS_DR_200MA, PS_DR_MASK);
    multi_set_bit (&val, PS_SLP_BIT_OFFSET, PS_SLP_100MS, PS_SLP_MASK);
    em20918_i2c_write(dev_data->client, em20918_reg_table.reg_config.reg_addr, val);
    em20918_print_reg_table(dev_data);

    em20918_resume(dev_data->client);

    return 0;
#endif
    /* check pid */
    em20918_i2c_read2(dev_data->client, em20918_reg_table.reg_pid.reg_addr, &val);
    if (val != em20918_reg_table.reg_pid.default_val) {
        dev_err(&client->dev, "this IR is not em20918: %x, probe fail\n", val);
        goto err6;
    }


    /* em20918 ps disable */
    em20918_i2c_read2(dev_data->client, em20918_reg_table.reg_config.reg_addr, &val);
    multi_set_bit (&val, PS_EN_BIT_OFFSET, PS_EN_DIS, PS_EN_MASK);
    em20918_i2c_write(dev_data->client, em20918_reg_table.reg_config.reg_addr, val);

    return 0;

err6:
    input_unregister_device(dev_data->input);

err5:
    input_free_device(input_device);

err4:
    if (gpio_is_valid(dev_data->pdata->int_pin))
        free_irq(dev_data->irq, dev_data);
    del_timer(&dev_data->capture_timer);

err3:
    if (gpio_is_valid(dev_data->pdata->int_pin))
        gpio_free(dev_data->pdata->int_pin);

err2:
    misc_deregister(&dev_data->miscdev);

    cancel_work_sync(&dev_data->irq_work);
    destroy_workqueue(dev_data->work_queue);

err1:
    kfree(dev_data);

    return ret;
}

static int em20918_remove(struct i2c_client *client)
{
    struct em20918_dev_data *dev_data = i2c_get_clientdata(client);

    cancel_work_sync(&dev_data->irq_work);
    destroy_workqueue(dev_data->work_queue);

    del_timer(&dev_data->capture_timer);

    input_unregister_device(dev_data->input);
    input_free_device(dev_data->input);

    misc_register(&dev_data->miscdev);

    if (gpio_is_valid(dev_data->pdata->int_pin)) {
        free_irq(dev_data->irq, dev_data);
        gpio_free(dev_data->pdata->int_pin);
    }

    kfree(dev_data);

    return 0;
}

static int em20918_suspend(struct i2c_client *client, pm_message_t mesg)
{
    struct em20918_dev_data *dev_data = i2c_get_clientdata(client);
    u8 val;

    disable_irq(dev_data->irq);

    em20918_i2c_read2(dev_data->client, em20918_reg_table.reg_config.reg_addr, &val);
    multi_set_bit (&val, PS_EN_BIT_OFFSET, PS_EN_DIS, PS_EN_MASK);
    em20918_i2c_write(dev_data->client, em20918_reg_table.reg_config.reg_addr, val);

    del_timer(&dev_data->capture_timer);

    return 0;
}

static int em20918_resume(struct i2c_client *client)
{
    struct em20918_dev_data *dev_data = i2c_get_clientdata(client);
    u8 val;

    em20918_capture_setting(dev_data);

    em20918_i2c_read2(dev_data->client, em20918_reg_table.reg_config.reg_addr, &val);
    multi_set_bit (&val, PS_EN_BIT_OFFSET, PS_EN_EN, PS_EN_MASK);
    em20918_i2c_write(dev_data->client, em20918_reg_table.reg_config.reg_addr, val);

    enable_irq(dev_data->irq);

    return 0;
}

static const struct i2c_device_id em20918_ids[] = {
    { EM20918_NAME, 0 },
    { /*end of list*/ }
};

static struct i2c_driver em20918_driver = {
    .driver = {
        .name  = EM20918_NAME,
        .owner = THIS_MODULE,
    },
    .probe      = em20918_probe,
    .remove     = em20918_remove,
    .suspend    = em20918_suspend,
    .resume     = em20918_resume,
    .id_table   = em20918_ids,
};

static int __init em20918_init(void)
{
    return i2c_add_driver(&em20918_driver);
}

static void __exit em20918_exit(void)
{
    i2c_del_driver(&em20918_driver);
}

late_initcall(em20918_init);
module_exit(em20918_exit);

MODULE_AUTHOR("Howrd <kai.shen@ingenic.com>");
MODULE_DESCRIPTION("em20918 control driver");
MODULE_LICENSE("GPL");
