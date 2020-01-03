/*
 * linux/drivers/pwm/jz-pwm/pwm-class.c
 *
 * simple PWM based LED or beeper control
 * Platform driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <mach/jz_pwm.h>


DECLARE_RWSEM(jz_pwm_list_lock);
EXPORT_SYMBOL_GPL(jz_pwm_list_lock);

LIST_HEAD(jz_pwm_list);
EXPORT_SYMBOL_GPL(jz_pwm_list);

static struct class *jz_pwm_class;


static void pwm_data_update(struct jz_pwm_classdev *pwm_cdev)
{
    if(pwm_cdev->pwm_update)
        pwm_cdev->pwm_update(pwm_cdev);
}

static void pwm_dutyratio_set(struct jz_pwm_classdev *pwm_cdev, unsigned int dutyratio)
{
    if(dutyratio > pwm_cdev->max_dutyratio)
        dutyratio = pwm_cdev->max_dutyratio;
    else if(dutyratio < 0)
        dutyratio = 0;

    pwm_cdev->dutyratio = dutyratio;
    pwm_cdev->enable = !!dutyratio;

    if(!(pwm_cdev->flags & JZ_PWM_DEV_SUSPENDED))
        pwm_cdev->dutyratio_set(pwm_cdev, dutyratio);
}

static void pwm_period_set(struct jz_pwm_classdev *pwm_cdev, unsigned int period)
{
    if(period > PWM_PERIOD_MAX)
        period = PWM_PERIOD_MAX;
    else if(period < PWM_PERIOD_MIN)
        period = PWM_PERIOD_MIN;

    pwm_cdev->period_ns = period;
}

static ssize_t jz_pwm_active_level_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", pwm_cdev->active_level);
}

static ssize_t jz_pwm_active_level_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);
    unsigned long state;
    ssize_t ret = -EINVAL;

    ret = kstrtoul(buf, 10, &state);
    if(ret)
        return ret;

    pwm_cdev->active_level = !!state;
    pwm_cdev->active_level_set(pwm_cdev);
    return size;
}

static ssize_t jz_pwm_dutyratio_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", pwm_cdev->dutyratio);
}

static ssize_t jz_pwm_dutyratio_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);
    unsigned long state;
    ssize_t ret = -EINVAL;

    ret = kstrtoul(buf, 10, &state);
    if(ret)
        return ret;

    pr_debug("%s: buf is %s state is %ld\n", __func__, buf, state);
    pwm_dutyratio_set(pwm_cdev, state);
    return size;
}

static ssize_t jz_pwm_max_dutyratio_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", pwm_cdev->max_dutyratio);
}

static ssize_t jz_pwm_period_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", pwm_cdev->period_ns);
}

static ssize_t jz_pwm_period_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);
    unsigned long state;
    ssize_t ret = -EINVAL;

    ret = kstrtoul(buf, 10, &state);
    if(ret)
        return ret;

    pr_debug("%s: buf is %s state is %ld\n", __func__, buf, state);
    pwm_period_set(pwm_cdev, state);
    return size;
}

static ssize_t jz_pwm_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);

    return sprintf(buf, "%u\n", pwm_cdev->enable);
}

static ssize_t jz_pwm_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
    struct jz_pwm_classdev *pwm_cdev = dev_get_drvdata(dev);
    unsigned long state;
    ssize_t ret = -EINVAL;

    ret = kstrtoul(buf, 10, &state);
    if(ret)
        return ret;

    pwm_cdev->enable = !!state;
    pwm_cdev->pwm_enable(pwm_cdev);

    return size;
}

static struct device_attribute jz_pwm_class_attrs[] = {
    __ATTR(active_level, 0644, jz_pwm_active_level_show, jz_pwm_active_level_store),
    __ATTR(dutyratio, 0644, jz_pwm_dutyratio_show, jz_pwm_dutyratio_store),
    __ATTR(max_dutyratio, 0444, jz_pwm_max_dutyratio_show, NULL),
    __ATTR(period, 0644, jz_pwm_period_show, jz_pwm_period_store),
    __ATTR(enable, 0644, jz_pwm_enable_show, jz_pwm_enable_store),
    __ATTR_NULL,
};

/**
 * jz_pwm_classdev_suspend - suspend an jz_pwm_classdev.
 * @pwm_cdev: the jz_pwm_classdev to suspend.
 */
void jz_pwm_classdev_suspend(struct jz_pwm_classdev *pwm_cdev)
{
    pwm_cdev->flags |= JZ_PWM_DEV_SUSPENDED;
    pwm_cdev->dutyratio_set(pwm_cdev, 0);
}
EXPORT_SYMBOL_GPL(jz_pwm_classdev_suspend);

/**
 * jz_pwm_classdev_resume - resume an jz_pwm_classdev.
 * @pwm_cdev: the jz_pwm_classdev to resume.
 */
void jz_pwm_classdev_resume(struct jz_pwm_classdev *pwm_cdev)
{
    pwm_cdev->dutyratio_set(pwm_cdev, pwm_cdev->dutyratio);
    pwm_cdev->flags &= ~JZ_PWM_DEV_SUSPENDED;
}
EXPORT_SYMBOL_GPL(jz_pwm_classdev_resume);

static int jz_pwm_class_suspend(struct device *dev, pm_message_t state)
{
    struct jz_pwm_classdev *cdev = dev_get_drvdata(dev);

    if(cdev->flags & JZ_PWM_DEV_CORE_SUSPENDRESUME)
        jz_pwm_classdev_suspend(cdev);

    return 0;
}

static int jz_pwm_class_resume(struct device *dev)
{
    struct jz_pwm_classdev *cdev = dev_get_drvdata(dev);

    if(cdev->flags & JZ_PWM_DEV_CORE_SUSPENDRESUME)
        jz_pwm_classdev_resume(cdev);

    return 0;
}

/**
 * jz_pwm_classdev_register - register a new object of jz_pwm_classdev class.
 * @parent: The device to register.
 * @pwm_cdev: the jz_pwm_classdev structure for this device.
 */
int jz_pwm_classdev_register(struct device *parent, struct jz_pwm_classdev *pwm_cdev)
{
    pwm_cdev->dev = device_create(jz_pwm_class, parent, 0, pwm_cdev,
                                        "%s", pwm_cdev->name);
    if(IS_ERR(pwm_cdev->dev))
        return PTR_ERR(pwm_cdev->dev);

    /* add to the list of jz pwm */
    down_write(&jz_pwm_list_lock);
    list_add_tail(&pwm_cdev->node, &jz_pwm_list);
    up_write(&jz_pwm_list_lock);

    if(!pwm_cdev->max_dutyratio)
        pwm_cdev->max_dutyratio = PWM_FULL;

    pwm_data_update(pwm_cdev);

    dev_dbg(parent, "Registered JZ PWM device: %s\n",
    pwm_cdev->name);

    return 0;
}
EXPORT_SYMBOL_GPL(jz_pwm_classdev_register);

/**
 * jz_pwm_classdev_unregister - unregisters a object of led_properties class.
 * @pwm_cdev: the pwm device to unregister
 *
 * Unregisters a previously registered via jz_pwm_classdev_register object.
 */
void jz_pwm_classdev_unregister(struct jz_pwm_classdev *pwm_cdev)
{
    pwm_dutyratio_set(pwm_cdev, PWM_OFF);
    device_unregister(pwm_cdev->dev);

    down_write(&jz_pwm_list_lock);
    list_del(&pwm_cdev->node);
    up_write(&jz_pwm_list_lock);
}
EXPORT_SYMBOL_GPL(jz_pwm_classdev_unregister);

static int __init jz_pwm_class_init(void)
{
    jz_pwm_class = class_create(THIS_MODULE, "jz-pwm");
    if(IS_ERR(jz_pwm_class)) {
        printk("%s: unable to create jz-pwm class!\n", __func__);
        return PTR_ERR(jz_pwm_class);
    }

    jz_pwm_class->suspend = jz_pwm_class_suspend;
    jz_pwm_class->resume  = jz_pwm_class_resume;
    jz_pwm_class->dev_attrs = jz_pwm_class_attrs;

    return 0;
}

static void __exit jz_pwm_class_exit(void)
{
    class_destroy(jz_pwm_class);
}

subsys_initcall(jz_pwm_class_init);
module_exit(jz_pwm_class_exit);

MODULE_AUTHOR("Qiuwei, Wang");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("JZ PWM Class Interface");
