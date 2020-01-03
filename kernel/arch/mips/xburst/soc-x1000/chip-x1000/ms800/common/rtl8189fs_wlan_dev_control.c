/* Copyright (c) 2008 -2014 Espressif System.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  sdio stub code template
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>

#include "board_base.h"


void sif_platform_reset_target(void)
{
    if (gpio_is_valid(RTL8189FS_CHIP_EN)) {
        gpio_direction_output(RTL8189FS_CHIP_EN, 0);
        msleep(1);
        gpio_direction_output(RTL8189FS_CHIP_EN, 1);
    }
}

void sif_platform_target_poweroff(void)
{
    printk("=== Dislaunching RTL8189FS Wi-Fi driver! ===\n");

    if (gpio_is_valid(RTL8189FS_CHIP_EN))
        gpio_set_value(RTL8189FS_CHIP_EN, 0);
    if (gpio_is_valid(RTL8189FS_IOPWR_EN))
        gpio_set_value(RTL8189FS_IOPWR_EN, !RTL8189FS_IOPWR_EN_LEVEL);
    if (gpio_is_valid(RTL8189FS_PWR_EN))
        gpio_set_value(RTL8189FS_PWR_EN, !RTL8189FS_PWR_EN_LEVEL);
}

void sif_platform_target_poweron(void)
{
    printk("=== Launching RTL8189FS Wi-Fi driver! ===\n");
    if (gpio_is_valid(RTL8189FS_IOPWR_EN))
        gpio_set_value(RTL8189FS_IOPWR_EN, RTL8189FS_IOPWR_EN_LEVEL);
    if (gpio_is_valid(RTL8189FS_PWR_EN))
        gpio_set_value(RTL8189FS_PWR_EN, RTL8189FS_PWR_EN_LEVEL);
    if (gpio_is_valid(RTL8189FS_CHIP_EN))
        gpio_set_value(RTL8189FS_CHIP_EN, 1);
}

int sif_platform_gpio_request(void)
{
   if (gpio_is_valid(RTL8189FS_PWR_EN)) {
        if (gpio_request(RTL8189FS_PWR_EN, "pwr_en")) {
            pr_err("ERROR: no rtl8189fs pwr_en pin available\n");
            goto err_wlan_init;
        }
        gpio_direction_output(RTL8189FS_PWR_EN, RTL8189FS_PWR_EN_LEVEL);
    }
    if (gpio_is_valid(RTL8189FS_IOPWR_EN)) {
        if (gpio_request(RTL8189FS_IOPWR_EN, "iopwr_en")) {
            pr_err("ERROR: no rtl8189fs iopwr_en pin available\n");
        }
        gpio_direction_output(RTL8189FS_IOPWR_EN, RTL8189FS_IOPWR_EN_LEVEL);
    }

    if (gpio_is_valid(RTL8189FS_CHIP_EN)) {
        if (gpio_request(RTL8189FS_CHIP_EN, "chip_en")) {
            pr_err("ERROR: no rtl8189fs chip_en pin available\n");
            goto err_wlan_init;
        }
        gpio_direction_output(RTL8189FS_CHIP_EN, 0);
    }
    if (gpio_is_valid(RTL8189FS_WKUP_CPU)) {
        if (gpio_request(RTL8189FS_WKUP_CPU, "wkup_cpu")) {
            pr_err("ERROR: no rtl8189fs wkup_cpu pin available\n");
            goto err_wlan_init;
        }
        gpio_direction_input(RTL8189FS_WKUP_CPU);
    }

    return 0;

err_wlan_init:
    return -EINVAL;
}
int get_rtl8189fs_gpio_wakeup(void)
{
    return RTL8189FS_WKUP_CPU;
}
EXPORT_SYMBOL(get_rtl8189fs_gpio_wakeup);
EXPORT_SYMBOL(sif_platform_reset_target);
EXPORT_SYMBOL(sif_platform_target_poweroff);
EXPORT_SYMBOL(sif_platform_target_poweron);
EXPORT_SYMBOL(sif_platform_gpio_request);
