/*
 * [board]-cim.c - This file defines camera host driver (cim) on the board.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Author: xiaoyangfu <xyfu@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <media/soc_camera.h>
#include <mach/platform.h>
#include <mach/jz_camera.h>
#include <mach/jz_sensor.h>
#include <linux/regulator/machine.h>
#include <gpio.h>
#include "board_base.h"


#if defined(CONFIG_VIDEO_JZ_CIM_HOST_V13) || defined(CONFIG_JZ_CIM_CORE)
struct jz_camera_pdata mensa_camera_pdata = {
    .mclk_10khz = 2400,
    .flags = 0,
    .cam_sensor_pdata[FRONT_CAMERA_INDEX] = {
#ifdef FRONT_CAMERA_SENSOR_RESET
        .gpio_rst   = FRONT_CAMERA_SENSOR_RESET,
#endif
#ifdef FRONT_CAMERA_SENSOR_PWDN
        .gpio_power = FRONT_CAMERA_SENSOR_PWDN,
#endif
#ifdef FRONT_CAMERA_VDD_EN
        .gpio_en    = FRONT_CAMERA_VDD_EN,
#endif
        .gpio_rst_level   = FRONT_CAMERA_SENSOR_RESET_LEVEL,
        .gpio_power_level = FRONT_CAMERA_VDD_EN_LEVEL,
        .gpio_en_level    = FRONT_CAMERA_SENSOR_PWDN_LEVEL,

        .ir_power_en        = FRONT_CAMERA_IR_POWER_EN,
        .ir_power_en_level  = FRONT_CAMERA_IR_POWER_EN__LEVEL,
    },
};

static int camera_sensor_reset(struct device *dev)
{
#ifdef FRONT_CAMERA_VDD_EN
    if (gpio_is_valid(FRONT_CAMERA_VDD_EN)) {
        gpio_direction_output(FRONT_CAMERA_VDD_EN, FRONT_CAMERA_VDD_EN_LEVEL);
    }
#endif
#ifdef FRONT_CAMERA_SENSOR_RESET
    if (gpio_is_valid(FRONT_CAMERA_SENSOR_RESET)) {
        gpio_direction_output(FRONT_CAMERA_SENSOR_RESET, FRONT_CAMERA_SENSOR_RESET_LEVEL);
        mdelay(20);
        gpio_direction_output(FRONT_CAMERA_SENSOR_RESET, !FRONT_CAMERA_SENSOR_RESET_LEVEL);
        mdelay(20);
    }
#endif
    return 0;
}

static int camera_sensor_power(struct device *dev, int on) {
    /* enable or disable the IR Power */
    if (gpio_is_valid(FRONT_CAMERA_IR_POWER_EN)) {
        gpio_direction_output(FRONT_CAMERA_IR_POWER_EN, on ?
                FRONT_CAMERA_IR_POWER_EN__LEVEL : !FRONT_CAMERA_IR_POWER_EN__LEVEL);
        mdelay(150);
    }

#ifdef FRONT_CAMERA_VDD_EN
        gpio_direction_output(FRONT_CAMERA_VDD_EN, on ? \
            FRONT_CAMERA_VDD_EN_LEVEL : !FRONT_CAMERA_VDD_EN_LEVEL);
#endif
    /* enable or disable the camera */
#ifdef FRONT_CAMERA_SENSOR_PWDN
    if (gpio_is_valid(FRONT_CAMERA_SENSOR_PWDN)) {
        gpio_direction_output(FRONT_CAMERA_SENSOR_PWDN, on ? \
            FRONT_CAMERA_SENSOR_PWDN_LEVEL : !FRONT_CAMERA_SENSOR_PWDN_LEVEL);
        mdelay(20);
    }
#endif

    return 0;
}
#endif

#ifdef CONFIG_VIDEO_JZ_CIM_HOST_V13
static struct soc_camera_link iclink_front = {
    .bus_id     = 0,        /* Must match with the camera ID */
    .board_info = &jz_v4l2_camera_devs[FRONT_CAMERA_INDEX],
    .i2c_adapter_id = 0,
    .power = camera_sensor_power,
    .reset = camera_sensor_reset,
};

struct platform_device mensa_front_camera_sensor = {
    .name = "soc-camera-pdrv",
    .id   = -1,
    .dev  = {
        .platform_data = &iclink_front,
    },
};
#endif

#ifdef CONFIG_JZ_CAMERA_SENSOR
int camera_sensor_gpio_init(struct device *dev) {
#ifdef FRONT_CAMERA_SENSOR_RESET
    if (FRONT_CAMERA_SENSOR_RESET != -1) {
        if (gpio_request(FRONT_CAMERA_SENSOR_RESET, "sensor_reset") < 0) {
            pr_err("%s: FRONT_CAMERA_SENSOR_RESET request failed\n", __func__);
            return -1;
        }
    }
#endif

#ifdef FRONT_CAMERA_VDD_EN
        if (gpio_request(FRONT_CAMERA_VDD_EN, "vdd_en") < 0) {
            pr_err("%s: FRONT_CAMERA_VDD_EN request failed\n", __func__);
            return -1;
        }
#endif

    if (FRONT_CAMERA_SENSOR_PWDN != -1) {
        if (gpio_request(FRONT_CAMERA_SENSOR_PWDN, "front_power_pwdn") < 0) {
            pr_err("%s: FRONT_CAMERA_SENSOR_PWDN request failed\n", __func__);
            return -1;
        }
    }

    return 0;
}

struct sensor_platform_data cim_sensor_pdata = {
    .name = "sensor",
#ifdef FRONT_CAMERA_VDD_EN
    .gpio_en = FRONT_CAMERA_VDD_EN,
#endif
    .init = camera_sensor_gpio_init,
    .reset = camera_sensor_reset,
    .power_on = camera_sensor_power,
};
#endif

static int __init mensa_board_cim_init(void) {
    /* camera host */
#if defined(CONFIG_VIDEO_JZ_CIM_HOST_V13) || defined(CONFIG_JZ_CIM_CORE)
    jz_device_register(&jz_cim_device, &mensa_camera_pdata);
#endif
    /* camera sensor */
#ifdef CONFIG_SOC_CAMERA
    platform_device_register(&mensa_front_camera_sensor);
#endif

    return 0;
}

//arch_initcall(mensa_board_cim_init);
core_initcall(mensa_board_cim_init);
