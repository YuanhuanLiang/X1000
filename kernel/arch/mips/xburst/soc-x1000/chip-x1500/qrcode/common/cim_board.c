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
#ifdef CAMERA_SENSOR_RESET
        .gpio_rst = CAMERA_SENSOR_RESET,
#endif
        .gpio_power = CAMERA_FRONT_SENSOR_PWDN,
        .gpio_en = CAMERA_VDD_EN,
    },
};

static int camera_sensor_reset(struct device *dev)
{
    if (CAMERA_VDD_EN != -1)
        gpio_direction_output(CAMERA_VDD_EN, 1);
#ifdef CAMERA_SENSOR_RESET
    if (CAMERA_SENSOR_RESET != -1) {
        gpio_direction_output(CAMERA_SENSOR_RESET, 0);
        mdelay(20);
        gpio_direction_output(CAMERA_SENSOR_RESET, 1);
        mdelay(20);
    }
#endif
    return 0;
}

static int camera_sensor_power(struct device *dev, int on) {
    if (CAMERA_VDD_EN != -1)
        gpio_direction_output(CAMERA_VDD_EN, on);

    /* enable or disable the camera */
    if (CAMERA_FRONT_SENSOR_PWDN != -1) {
        gpio_direction_output(CAMERA_FRONT_SENSOR_PWDN, on ? 0 : 1);
        //gpio_direction_output(CAMERA_FRONT_SENSOR_PWDN, on ? 1 : 0);
        mdelay(20);
    }

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
#ifdef CAMERA_SENSOR_RESET
    if (CAMERA_SENSOR_RESET != -1) {
        if (gpio_request(CAMERA_SENSOR_RESET, "sensor_reset") < 0) {
            pr_err("%s: CAMERA_SENSOR_RESET request failed\n", __func__);
            return -1;
        }
    }
#endif

    if (CAMERA_VDD_EN != -1) {
        if (gpio_request(CAMERA_VDD_EN, "vdd_en") < 0) {
            pr_err("%s: CAMERA_VDD_EN request failed\n", __func__);
            return -1;
        }
    }

    if (CAMERA_FRONT_SENSOR_PWDN != -1) {
        if (gpio_request(CAMERA_FRONT_SENSOR_PWDN, "front_power_pwdn") < 0) {
            pr_err("%s: CAMERA_FRONT_SENSOR_PWDN request failed\n", __func__);
            return -1;
        }
    }

    return 0;
}

struct sensor_platform_data cim_sensor_pdata = {
    .name = "sensor",
    .gpio_en = CAMERA_VDD_EN,
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
