
/*
 * include/linux/74hc595.h
 *
 * simple 74hc595 control driver
 * Platform driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef __74HC595_H
#define __74HC595_H

struct sn74hc595_platform_data {
    unsigned int en_level;
    unsigned int out_bits;
    unsigned int init_val;

    unsigned int data_pin;
    unsigned int rclk_pin;
    unsigned int sclk_pin;
    unsigned int sclr_pin;
    unsigned int oe_pin;
};

#endif
