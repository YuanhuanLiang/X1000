/*
 * include/linux/tm1620.h
 *
 * TM1620 Digital LED control driver
 * Platform driver support for Ingenic X1000 SoC.
 *
 * Copyright 2018, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _TM1620_H
#define _TM1620_H

/* TM1620 Commands */
#define DISPLAY_6GRID_8SEG    0x02

#define W_ADDR_AUTO_ADD       0x40
#define W_ADDR_FIXED          0x44

#define S_ADDR_TO_00H         0xC0
#define S_ADDR_TO_01H         0xC1
#define S_ADDR_TO_02H         0xC2
#define S_ADDR_TO_03H         0xC3
#define S_ADDR_TO_04H         0xC4
#define S_ADDR_TO_05H         0xC5
#define S_ADDR_TO_06H         0xC6
#define S_ADDR_TO_07H         0xC7
#define S_ADDR_TO_08H         0xC8
#define S_ADDR_TO_09H         0xC9
#define S_ADDR_TO_0AH         0xCA
#define S_ADDR_TO_0BH         0xCB

#define DISPLAY_OPEN          0x88
#define DISPLAY_CLOSE         0x80

#define PULSE_WIDTH_1_16      0x00
#define PULSE_WIDTH_2_16      0x01
#define PULSE_WIDTH_4_16      0x02
#define PULSE_WIDTH_10_16     0x03
#define PULSE_WIDTH_11_16     0x04
#define PULSE_WIDTH_12_16     0x05
#define PULSE_WIDTH_13_16     0x06
#define PULSE_WIDTH_14_16     0x07


struct tm1620_platform_data {
    unsigned int stb_pin;
    unsigned int clk_pin;
    unsigned int dio_pin;
};

#endif
