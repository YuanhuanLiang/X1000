/*
 *  Copyright (C) 2017 Ingenic Semiconductor Co.,Ltd
 *
 *  PWM battery driver
 *
 *  Zhang YanMing <yanming.zhang@ingenic.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __LINUX_POWER_PWM_BATTERY_H__
#define __LINUX_POWER_PWM_BATTERY_H__

#include <linux/power_supply.h>
#include <linux/types.h>

/*
 * struct battery_info - battery information
 * @discharge_bias_current  current out of discharge loop
 * @sleep_current           current of system in sleep mode
 */
struct battery_info {
    unsigned int battery_max_cpt;
    unsigned int sleep_current;
};

/*
 * struct pwm_battery_platform_data - platform data for pwm battery device
 * @gpio_usb                    USB insert detect gpio
 * @gpio_usb_active_low         low level active detect USB insert
 * @gpio_charger                Charger chip status detect gpio
 * @gpio_charger_active_low     low level active detect charging status
 * @charger_debounce            charger debounce time, if not sure set 0
 * @pwm_id                      PWM id
 * @pwm_active_low              PWM low level active
 * @gpio_op                     OP compare output
 * @gpio_op_active_low          low level active detect OP output
 * @pwm_ref_voltage             PWM reference voltage(power domain)
 * @battery_ref_scale           battery voltage sample point scale
 * @batter_info                 see above
 *
 * Note:
 *  For gpio_usb, if your board doesn't has USB insert detect gpio, you should
 *  set it as -1 indicate ignore it and gpio_usb_active_low could be ignored too.
 *
 *  For gpio_charger, if you don't intend to use charger chip to monitor battery
 *  charge status, you should set gpio_charger as -1 indicate ignore it, but
 *  best not to do so!
 */
struct pwm_battery_platform_data {
    int gpio_power;
    int gpio_power_active_low;

    int gpio_usb;
    int gpio_usb_active_low;

    int gpio_charger;
    int gpio_charger_active_low;
    int charger_debounce;

    int pwm_id;
    int pwm_active_low;

    int gpio_op;
    int gpio_op_active_low;

    int pwm_ref_voltage;

    /* battery_ref_scale = (battery_ref_resistor1 + battery_ref_resistor2) / battery_ref_resistor2 */
    int battery_ref_resistor1;     /* KΩ */
    int battery_ref_resistor2;     /* KΩ */

    struct battery_info battery_info;
};

#endif
