#include <linux/init.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#include <asm/uaccess.h>
#include <asm/ioctl.h>
#include <linux/switch.h>
#include <linux/sgm42609_motor.h>

#define IOC_MOTOR_MAGIC     'K'
#define MOTOR_SET_FUNCTION  _IO(IOC_MOTOR_MAGIC, 20)
#define MOTOR_GET_STATUS        _IO(IOC_MOTOR_MAGIC, 21)
#define MOTOR_SET_SPEED           _IO(IOC_MOTOR_MAGIC, 22)
#define MOTOR_GET_SPEED          _IO(IOC_MOTOR_MAGIC, 23)
#define MOTOR_SET_CYCLE           _IO(IOC_MOTOR_MAGIC, 24)
#define MOTOR_GET_CYCLE          _IO(IOC_MOTOR_MAGIC, 25)


#define DRV_NAME "sgm42609_motor"
#define CLASS_NAME "motor_class"
/* pwm cycle unit : ms
 * mininum : 10
 * maxnum : 1000
 * */
#define DEFAULT_MOTOR_CYCLE 1000
#define MOTOR_DEBUG(format, ...) printk("%s, "format, __FUNCTION__, ##__VA_ARGS__)

enum motor_status {
    MOTOR_COAST = 0x0,
    MOTOR_FORWARD = 0x01,
    MOTOR_REVERSE = 0x02,
    MOTOR_BRAKE = 0x03,
    MOTOR_FAULT = 0xff,
};

struct motor_device {
    int id;
    char name[20];
    int in1_gpio;
    int in2_gpio;
    int fault_gpio;
    int fault_irq;
    int power_gpio;
    int power_level;
    unsigned int speed;     //speed: 0-10
    struct work_struct fault_work;
    struct switch_dev sdev;
    unsigned int cycle;     //10-1000   unit : ms

    struct hrtimer hrt;
    bool active_level;
    ktime_t interval1;
    ktime_t interval2;
    int status;
    int count;

    struct mutex mmutex;
    struct miscdevice mdev;
    int cancel;
};

static enum hrtimer_restart gpio_level_flip(struct hrtimer *timer)
{
    struct motor_device *motor = container_of(timer, struct motor_device, hrt);

    if (motor->cancel) {
            return HRTIMER_NORESTART;
    }
    motor->active_level = !motor->active_level;
    switch (motor->status) {
        case MOTOR_FORWARD:
            gpio_set_value(motor->in1_gpio, motor->active_level ? 1 : 0);
            gpio_set_value(motor->in2_gpio, 0);
            break;
        case MOTOR_REVERSE:
            gpio_set_value(motor->in1_gpio, 0);
            gpio_set_value(motor->in2_gpio, motor->active_level ? 1 : 0);
            break;
        case MOTOR_BRAKE:
            gpio_set_value(motor->in1_gpio, 1);
            gpio_set_value(motor->in2_gpio, 1);
            motor->cancel = 1;
            return HRTIMER_NORESTART;
        case MOTOR_COAST:
            gpio_set_value(motor->in1_gpio, 0);
            gpio_set_value(motor->in2_gpio, 0);
            motor->cancel = 1;
            return HRTIMER_NORESTART;
        default:
            motor->cancel = 1;
            return HRTIMER_NORESTART;
    }

    hrtimer_forward_now(&(motor->hrt), motor->active_level ? motor->interval1 : motor->interval2);
    return HRTIMER_RESTART;
}

/*
 * IOCTL interface
 * sgm42609 return from sleep mode need 2ms
 */
static long motor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    int status, speed, cycle;
    void __user *argp = (void __user *)arg;

    struct miscdevice *mdev = file->private_data;
    struct motor_device *motor = container_of(mdev, struct motor_device, mdev);

    mutex_lock(&motor->mmutex);

    if(motor->fault_irq != -1) {
        cancel_work_sync(&motor->fault_work);
        disable_irq(motor->fault_irq);
    }


    switch (cmd) {
        case MOTOR_SET_FUNCTION:
            status = (int)arg;

            if(motor->status == status) {
                break;
            } else if ((status < MOTOR_COAST) || (status > MOTOR_BRAKE)) {
                MOTOR_DEBUG("motor function invalid parameter\n");
                ret = -EINVAL;
                goto motor_ioctl_err;
            }

            motor->cancel = 1;
            hrtimer_cancel(&motor->hrt);
            motor->status = status;

            if (motor->status == MOTOR_BRAKE) {
                gpio_set_value(motor->in1_gpio, 1);
                gpio_set_value(motor->in2_gpio, 1);
                msleep(2);
                break;
            } else if ((motor->status == MOTOR_FORWARD) && (motor->speed != 0)) {
                gpio_set_value(motor->in1_gpio, 1);
                gpio_set_value(motor->in2_gpio, 0);
                msleep(2);
            } else if ((motor->status == MOTOR_REVERSE) && (motor->speed != 0)) {
                gpio_set_value(motor->in1_gpio, 0);
                gpio_set_value(motor->in2_gpio, 1);
                msleep(2);
            } else {
                gpio_set_value(motor->in1_gpio, 0);
                gpio_set_value(motor->in2_gpio, 0);
                break;
            }

            if((motor->speed > 0) && (motor->speed < 10)) {
                motor->cancel = 0;
                motor->active_level = 1;
                motor->interval1 = ktime_set(0, (long) motor->cycle * 1000 * motor->speed * 100);
                motor->interval2 = ktime_set(0, (long) motor->cycle * 1000 * (10 - motor->speed) * 100);
                hrtimer_start(&(motor->hrt), motor->interval1, HRTIMER_MODE_REL);
            }
            break;

        case MOTOR_GET_STATUS:
            copy_to_user(argp, &motor->status, sizeof(int));
            break;

        case MOTOR_SET_SPEED:
            speed = (int)arg;

            if ((speed < 0) || (speed > 10)) {
                MOTOR_DEBUG("motor speed  %d invalid, range : 0-10\n", speed);
                ret =  -EINVAL;
                goto motor_ioctl_err;
            }

            motor->cancel = 1;
            hrtimer_cancel(&motor->hrt);
            motor->speed = speed;

            if((motor->speed == 0) && (motor->status != MOTOR_BRAKE)) {
                gpio_set_value(motor->in1_gpio, 0);
                gpio_set_value(motor->in2_gpio, 0);
                break;
            } else if (motor->status == MOTOR_BRAKE) {
                gpio_set_value(motor->in1_gpio, 1);
                gpio_set_value(motor->in2_gpio, 1);
                msleep(2);
                break;
            } else if (motor->status == MOTOR_FORWARD) {
                gpio_set_value(motor->in1_gpio, 1);
                gpio_set_value(motor->in2_gpio, 0);
                msleep(2);
            } else if (motor->status == MOTOR_REVERSE) {
                gpio_set_value(motor->in1_gpio, 0);
                gpio_set_value(motor->in2_gpio, 1);
                msleep(2);
            } else {
                gpio_set_value(motor->in1_gpio, 0);
                gpio_set_value(motor->in2_gpio, 0);
                break;
            }

            if((motor->speed > 0) && (motor->speed < 10)) {
                motor->cancel = 0;
                motor->active_level = 1;
                motor->interval1 = ktime_set(0, (long) motor->cycle * 1000 * motor->speed * 100);
                motor->interval2 = ktime_set(0, (long) motor->cycle * 1000 * (10 - motor->speed) * 100);
                hrtimer_start(&(motor->hrt), motor->interval1, HRTIMER_MODE_REL);
            }
            break;

        case MOTOR_GET_SPEED:
            copy_to_user(argp, &motor->speed, sizeof(int));
            break;

        case MOTOR_SET_CYCLE:
            cycle = (int)arg;

            if((cycle < 10) || (cycle > 1000)) {
                MOTOR_DEBUG("motor cycle  %d invalid, range : 10-1000\n", cycle);
                ret =  -EINVAL;
                goto motor_ioctl_err;
            }

            motor->cancel = 1;
            hrtimer_cancel(&motor->hrt);
            motor->cycle = cycle;
            motor->interval1 = ktime_set(0, (long) motor->cycle * 1000 * motor->speed * 100);
            motor->interval2 = ktime_set(0, (long) motor->cycle * 1000 * (10 - motor->speed) * 100);

            if ((motor->status == MOTOR_FORWARD) && (motor->speed != 0)) {
                gpio_set_value(motor->in1_gpio, 1);
                gpio_set_value(motor->in2_gpio, 0);
                msleep(2);
            } else if ((motor->status == MOTOR_REVERSE) && (motor->speed != 0)) {
                gpio_set_value(motor->in1_gpio, 0);
                gpio_set_value(motor->in2_gpio, 1);
                msleep(2);
            } else if (motor->status == MOTOR_BRAKE) {
                gpio_set_value(motor->in1_gpio, 1);
                gpio_set_value(motor->in2_gpio, 1);
                msleep(2);
                break;
            } else {
                gpio_set_value(motor->in1_gpio, 0);
                gpio_set_value(motor->in2_gpio, 0);
                break;
            }

            if((motor->speed > 0) && (motor->speed < 10)) {
                motor->cancel = 0;
                motor->active_level = 1;
                hrtimer_start(&(motor->hrt), motor->interval1, HRTIMER_MODE_REL);
            }
            break;
        case MOTOR_GET_CYCLE:
            copy_to_user(argp, &motor->cycle, sizeof(int));
            break;

        default:
            ret =  -EINVAL;
            goto motor_ioctl_err;
    }

    if(motor->fault_irq != -1)
        enable_irq(motor->fault_irq);

    mutex_unlock(&motor->mmutex);
    return 0;

motor_ioctl_err:
    if(motor->fault_irq != -1)
        enable_irq(motor->fault_irq);

    mutex_unlock(&motor->mmutex);
    return ret;
}

static void motor_fault_work(struct work_struct *work)
{
    struct motor_device *motor = container_of(work, struct motor_device, fault_work);

    if(gpio_get_value(motor->fault_gpio)) {
        switch_set_state(&motor->sdev, 0);
    } else {
        switch_set_state(&motor->sdev, 1);
        MOTOR_DEBUG("%s: fault !!!\n",motor->name);
    }
    enable_irq(motor->fault_irq);

}

static irqreturn_t motor_fault_handler(int irq, void *dev_id)
{
    struct motor_device *motor = dev_id;
    disable_irq_nosync(motor->fault_irq);
    schedule_work(&motor->fault_work);
    return IRQ_HANDLED;
}

static int motor_open(struct inode *inode, struct file *filp)
{
    struct miscdevice *mdev = filp->private_data;
    struct motor_device *motor = container_of(mdev, struct motor_device, mdev);

    mutex_lock(&motor->mmutex);
    if(motor->count == 0) {
        if(motor->power_gpio != -1)
            gpio_set_value(motor->power_gpio, motor->power_level);
        if(motor->fault_irq != -1)
            enable_irq(motor->fault_irq);
    }

    motor->count++;
    mutex_unlock(&motor->mmutex);

    return 0;
}

static int motor_release(struct inode *inode, struct file *filp)
{
    struct miscdevice *mdev = filp->private_data;
    struct motor_device *motor = container_of(mdev, struct motor_device, mdev);

    mutex_lock(&motor->mmutex);
    motor->count--;
    if(motor->count == 0) {

        if(motor->fault_irq != -1) {
            cancel_work_sync(&motor->fault_work);
            disable_irq(motor->fault_irq);
        }

        motor->cancel = 1;
        hrtimer_cancel(&motor->hrt);
        motor->status = MOTOR_COAST;
        gpio_set_value(motor->in1_gpio, 0);
        gpio_set_value(motor->in2_gpio, 0);
        if(motor->power_gpio != -1)
            gpio_set_value(motor->power_gpio, !motor->power_level);
    }
    mutex_unlock(&motor->mmutex);
    return 0;
}

static struct file_operations motor_fops = {
        .owner = THIS_MODULE,
        .open = motor_open,
        .release = motor_release,
        .unlocked_ioctl = motor_ioctl,
};

static ssize_t switch_motor_print_name(struct switch_dev *sdev, char *buf)
{
    return sprintf(buf,"%s\n",sdev->name);
}

static ssize_t switch_motor_print_state(struct switch_dev *sdev, char *buf)
{
    unsigned int state_val = switch_get_state(sdev);

    if (state_val == 1)
        return sprintf(buf, "%s\n", "fault");
    else
        return sprintf(buf, "%s\n", "normal");
}


static int sgm42609_motor_probe(struct platform_device *pdev)
{
    int err = 0;
    struct motor_device *motor;
    struct device *dev = &pdev->dev;
    struct sgm42609_motor_platform_data *pdata = dev_get_platdata(dev);

    if(!(gpio_is_valid(pdata->in1_gpio) && gpio_is_valid(pdata->in2_gpio))) {
        MOTOR_DEBUG("ERROR: motor gpio is invalid\n");
        err = -EINVAL;
        goto motor_error0;
    }

    motor = kzalloc(sizeof(struct motor_device), GFP_KERNEL);
    if(motor == NULL) {
        MOTOR_DEBUG("Failed to malloc motor_device\n");
        err = -ENOMEM;
        goto motor_error0;
    }
    platform_set_drvdata(pdev, motor);
    motor->id = pdev->id;
    sprintf(motor->name, "motor%d", motor->id);
    MOTOR_DEBUG("Create motor device : %d\n",motor->id);

    err = gpio_request_one(pdata->in1_gpio, GPIOF_OUT_INIT_LOW, motor->name);
    if (err < 0) {
        MOTOR_DEBUG("Failed to request GPIO %d, error %d\n", pdata->in1_gpio, err);
        goto gpio_request_err1;
    }
    motor->in1_gpio = pdata->in1_gpio;

    err = gpio_request_one(pdata->in2_gpio, GPIOF_OUT_INIT_LOW, motor->name);
    if (err < 0) {
        MOTOR_DEBUG("Failed to request GPIO %d, error %d\n", pdata->in2_gpio, err);
        goto gpio_request_err2;
    }
    motor->in2_gpio = pdata->in2_gpio;

    //default speed 10
    motor->speed = 10;
    mutex_init(&motor->mmutex);
    INIT_WORK(&motor->fault_work, motor_fault_work);

    if (gpio_is_valid(pdata->power_gpio)) {
        err = gpio_request_one(pdata->power_gpio, pdata->power_active_level ? GPIOF_OUT_INIT_LOW : GPIOF_OUT_INIT_HIGH, motor->name);
        if (err < 0) {
            MOTOR_DEBUG("Failed to request GPIO %d, error %d\n", pdata->power_gpio, err);
            goto gpio_request_err3;
        }
        motor->power_gpio = pdata->power_gpio;
        motor->power_level = pdata->power_active_level;
    } else {
        motor->power_gpio = -1;
        motor->power_level = -1;
    }

    if (gpio_is_valid(pdata->fault_gpio)) {
        err = gpio_request_one(pdata->fault_gpio, GPIOF_IN, motor->name);
        if (err < 0) {
            MOTOR_DEBUG("Failed to request GPIO %d, error %d\n", pdata->fault_gpio, err);
            goto gpio_request_err4;
        }
        motor->fault_gpio = pdata->fault_gpio;
        motor->fault_irq = gpio_to_irq(pdata->fault_gpio);

        err = request_irq(motor->fault_irq, motor_fault_handler, IRQ_TYPE_EDGE_BOTH, motor->name, (void *)motor);
        if (err < 0) {
            MOTOR_DEBUG("request  fault_irq %d; error %d\n",motor->fault_irq, err);
            gpio_free(pdata->fault_gpio);
            goto gpio_request_err4;
        }
        disable_irq(motor->fault_irq);
    } else {
        motor->fault_gpio = -1;
        motor->fault_irq = -1;
    }

    motor->sdev.name = motor->name;
    motor->sdev.print_state = switch_motor_print_state;
    motor->sdev.print_name  = switch_motor_print_name;

    err = switch_dev_register(&motor->sdev);
    if (err < 0) {
        MOTOR_DEBUG("motor switch dev register fail.\n");
        goto switch_dev_register_err5;
    }

    motor->cycle = DEFAULT_MOTOR_CYCLE;
    hrtimer_init(&(motor->hrt), CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    motor->hrt.function = &gpio_level_flip;

    motor->mdev.minor = MISC_DYNAMIC_MINOR;
    motor->mdev.name =  motor->name;
    motor->mdev.fops = &motor_fops;

    err = misc_register(&motor->mdev);
    if (err) {
        MOTOR_DEBUG("misc_deregister failed\n");
        goto misc_deregister_err6;
    }

    return 0;

misc_deregister_err6:
    switch_dev_unregister(&motor->sdev);
switch_dev_register_err5:
    if(motor->fault_gpio != -1) {
        free_irq(motor->fault_irq, (void *)motor);
        gpio_free(motor->fault_gpio);
    }
gpio_request_err4:
    if(motor->power_gpio != -1)
        gpio_free(motor->power_gpio);
gpio_request_err3:
    gpio_free(motor->in2_gpio);
gpio_request_err2:
    gpio_free(motor->in1_gpio);
gpio_request_err1:
    kfree(motor);
motor_error0:
    return err;
}
static int sgm42609_motor_remove(struct platform_device *pdev)
{
    struct motor_device *motor = platform_get_drvdata(pdev);
    MOTOR_DEBUG("motor remove device %d\n",motor->id);

    motor->cancel = 1;
    hrtimer_cancel(&motor->hrt);
    gpio_set_value(motor->in1_gpio, 0);
    gpio_set_value(motor->in2_gpio, 0);

    misc_deregister(&motor->mdev);
    switch_dev_unregister(&motor->sdev);
    if(motor->fault_irq != -1) {
        cancel_work_sync(&motor->fault_work);
        disable_irq(motor->fault_irq);
        free_irq(motor->fault_irq, (void *)motor);
        gpio_free(motor->fault_gpio);
    }
    if(motor->power_gpio != -1) {
        gpio_set_value(motor->power_gpio, !motor->power_level);
        gpio_free(motor->power_gpio);
    }
    gpio_free(motor->in2_gpio);
    gpio_free(motor->in1_gpio);
    kfree(motor);

    return 0;
}

static int sgm42609_motor_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct motor_device *motor = platform_get_drvdata(pdev);

    mutex_lock(&motor->mmutex);
    if(motor->fault_irq != -1) {
        cancel_work_sync(&motor->fault_work);
        disable_irq(motor->fault_irq);
    }

    if(motor->count > 0) {
        motor->cancel = 1;
        hrtimer_cancel(&motor->hrt);
        gpio_set_value(motor->in1_gpio, 0);
        gpio_set_value(motor->in2_gpio, 0);

        if(motor->power_gpio != -1)
            gpio_set_value(motor->power_gpio, !motor->power_level);
    }
    mutex_unlock(&motor->mmutex);
    return 0;
}
static int sgm42609_motor_resume(struct platform_device *pdev)
{
    struct motor_device *motor = platform_get_drvdata(pdev);

    mutex_lock(&motor->mmutex);
    if(motor->count > 0) {
        if(motor->power_gpio != -1)
            gpio_set_value(motor->power_gpio, motor->power_level);

        if (motor->status == MOTOR_BRAKE) {
            gpio_set_value(motor->in1_gpio, 1);
            gpio_set_value(motor->in2_gpio, 1);
            mutex_unlock(&motor->mmutex);
            return 0;
        } else if ((motor->status == MOTOR_FORWARD) && (motor->speed != 0)) {
            gpio_set_value(motor->in1_gpio, 1);
            gpio_set_value(motor->in2_gpio, 0);
        } else if ((motor->status == MOTOR_REVERSE) && (motor->speed != 0)) {
            gpio_set_value(motor->in1_gpio, 0);
            gpio_set_value(motor->in2_gpio, 1);
        } else {
            gpio_set_value(motor->in1_gpio, 0);
            gpio_set_value(motor->in2_gpio, 0);
            mutex_unlock(&motor->mmutex);
            return 0;
        }

        if((motor->speed > 0) && (motor->speed < 10)) {
            motor->cancel = 0;
            motor->active_level = 1;
            hrtimer_start(&(motor->hrt), motor->interval1, HRTIMER_MODE_REL);
        }
    }

    if(motor->fault_irq != -1)
        enable_irq(motor->fault_irq);
    mutex_unlock(&motor->mmutex);
    return 0;
}

static struct platform_driver sgm42609_motor_driver = {
        .driver = {
                .name = DRV_NAME,
                .owner = THIS_MODULE,
        },
        .probe = sgm42609_motor_probe,
        .remove = sgm42609_motor_remove,
        .suspend = sgm42609_motor_suspend,
        .resume = sgm42609_motor_resume,
};

static int __init sgm42609_motor_init(void)
{
    return platform_driver_register(&sgm42609_motor_driver);
}

static void __exit sgm42609_motor_exit(void)
{
    platform_driver_unregister(&sgm42609_motor_driver);
}

module_init(sgm42609_motor_init);
module_exit(sgm42609_motor_exit);

MODULE_AUTHOR("xinshuan <shuan.xin@ingenic.com>");
MODULE_DESCRIPTION("motor driver with pwm emulating over gpio");
MODULE_LICENSE("GPL");
