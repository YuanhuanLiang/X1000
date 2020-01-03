/*
 *  Copyright (C) 2010 Ingenic Semiconductor Inc.
 *
 *  Author: <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * This file is a part of generic dmaengine, it's
 * used for other device to use dmaengine.
 */
#ifndef __JZ_PWM_H__
#define __JZ_PWM_H__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#define PWM_PERIOD_MIN	200
#define PWM_PERIOD_MAX	100000000

enum pwm_dutyratio {
	PWM_OFF = 0,
	PWM_HALF = 127,
	PWM_FULL = 255,
};

struct jz_pwm_dev {
	const char *name;
	unsigned int pwm_id;
	bool active_low;
	unsigned int dutyratio;
	unsigned int max_dutyratio;
	unsigned int period_ns;
};

struct jz_pwm_platform_data {
	int num_devs;
	struct jz_pwm_dev *devs;
};

struct jz_pwm_classdev {
	const char *name;
	unsigned int dutyratio;
	unsigned int max_dutyratio;
	unsigned int period_ns;
	u8 active_level;
	u8 enable;
	int flags;

#define JZ_PWM_DEV_SUSPENDED        (1 << 0)
#define JZ_PWM_DEV_CORE_SUSPENDRESUME  (1 << 16)

	void (*dutyratio_set)(struct jz_pwm_classdev *cdev, unsigned int dutyradio);
	void (*active_level_set)(struct jz_pwm_classdev *cdev);
	void (*pwm_update)(struct jz_pwm_classdev *cdev);
	void (*pwm_enable)(struct jz_pwm_classdev *cdev);

	struct device *dev;
	struct list_head node;
};

extern int jz_pwm_classdev_register(struct device *parent, struct jz_pwm_classdev *pwm_cdev);
extern void jz_pwm_classdev_unregister(struct jz_pwm_classdev *pwm_cdev);
extern void jz_pwm_classdev_suspend(struct jz_pwm_classdev *pwm_cdev);
extern void jz_pwm_classdev_resume(struct jz_pwm_classdev *pwm_cdev);
#endif /* __JZ_PWM_H__ */
