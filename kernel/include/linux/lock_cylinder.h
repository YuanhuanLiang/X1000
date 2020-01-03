/*
 * include/linux/lock_cylinder.h
 *
 * Smartdoor lock cylinder driver.
 * This driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com / panddio@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef __LOCK_CYLINDER_H
#define __LOCK_CYLINDER_H

struct lock_cylinder_platform_data {
    bool reverse_endian;
    char inserted_level;
    char pwr_en_level;

    unsigned int detect_pin;
    unsigned int pwr_pin;
    unsigned int id_pin;
};

#endif /* __LOCK_CYLINDER_H */
