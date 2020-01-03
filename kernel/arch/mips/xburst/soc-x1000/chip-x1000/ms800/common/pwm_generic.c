/*
 * [board]-pwm_generic.c
 * Platform device support for Ingenic X1000 SoC.*
 *
 * Copyright (C) 2016 Ingenic Semiconductor Co., Ltd.
 * Author: Wang Qiuwei <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/leds.h>
#include <mach/platform.h>
#include <mach/jz_pwm.h>
#include "board_base.h"


#ifdef CONFIG_JZ_PWM_GENERIC
static struct jz_pwm_dev pwm_devs[] = {
#ifdef CONFIG_JZ_PWM_BIT0
    {
        .name       = "pwm0",
        .pwm_id     = 0,
        .active_low = false,
        .dutyratio  = 0,
        .max_dutyratio = 100,
        .period_ns     = 300000,
    },
#endif

#ifdef CONFIG_JZ_PWM_BIT1
    {
        .name       = "pwm1",
        .pwm_id     = 1,
        .active_low = false,
        .dutyratio  = 0,
        .max_dutyratio = 100,
        .period_ns     = 300000,
    },
#endif

#ifdef CONFIG_JZ_PWM_BIT2
    {
        .name       = "pwm2",
        .pwm_id     = 2,
        .active_low = false,
        .dutyratio  = 0,
        .max_dutyratio = 100,
        .period_ns     = 300000,
    },
#endif
#ifdef CONFIG_JZ_PWM_BIT3
    {
        .name       = "pwm3",
        .pwm_id     = 3,
        .active_low = false,
        .dutyratio  = 0,
        .max_dutyratio = 100,
        .period_ns     = 300000,
    },
#endif
#ifdef CONFIG_JZ_PWM_BIT4
    {
        .name       = "pwm4",
        .pwm_id     = 4,
        .active_low = false,
        .dutyratio  = 0,
        .max_dutyratio = 100,
        .period_ns     = 300000,
    },
#endif
};

static struct jz_pwm_platform_data pwm_devs_info = {
    .num_devs = ARRAY_SIZE(pwm_devs),
    .devs     = pwm_devs,
};

struct platform_device jz_pwm_devs = {
    .name = "jz-pwm-dev",
    .dev  = {
        .platform_data = &pwm_devs_info,
    },
};
#endif
