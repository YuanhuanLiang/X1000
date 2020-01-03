
/*
 * include/linux/zb_cc2530.h
 *
 * simple zigbee cc2530 control driver
 * Platform driver support for Ingenic X1000 SoC.
 *
 * Copyright 2017, Monk <rongjin.su@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef __ZB_CC2530_H_
#define __ZB_CC2530_H_

#define TRIGGER_EDGE                    1

struct zb_cc2530_platform_data {
    unsigned int en_level;
    unsigned int rst_level;
    unsigned int wake_level;

    unsigned int en_pin;
    unsigned int int_pin;
    unsigned int wake_pin;
    unsigned int rst_pin;

    void(*restore_pin_status)(void);
    void(*set_pin_status)(void);
};

#endif