/*
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JZ_SENSOR_H__
#define __JZ_SENSOR_H__

struct sensor_platform_data {
	const char *name;
	unsigned int gpio_en;

	int (*init)(struct device *dev);
	int (*reset)(struct device *dev);
	int (*power_on)(struct device *dev, int on);
};

#endif
