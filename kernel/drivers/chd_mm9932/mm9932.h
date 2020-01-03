/*
 * gpio_ir.h
 *
 *  Created on: 2015-5-10
 *      Author: robinson
 */

#ifndef _CHD_MM9932_DRIVER_H_
#define _CHD_MM9932_DRIVER_H_

#define DEBUG 0

#define MM9932_DEVNAME				"mm9932"

/* IOCTL commands */
#define MM9932_GET_UNID				0xD201
#define MM9932_GET_VOL				0xD202
#define MM9932_GET_VOL_ALL			0xD203
#define MM9932_SET_VOL				0xD204
#define MM9932_READ_REG				0xD205
#define MM9932_WRITE_REG			0xD206

/* MPU register address */
#define MM9932_REG1_DCDC_OUT1		0x20
#define MM9932_REG2_DCDC_OUT2		0x30
#define MM9932_REG3_DCDC_OUT3		0x40
#define MM9932_REG4_LDO_OUT4		0x50
#define MM9932_REG5_LDO_OUT5		0x54
#define MM9932_REG6_LDO_OUT6		0x60
#define MM9932_REG7_LDO_OUT7		0x64

/* Driver data struct */
struct mm9932_drvdata {
	dev_t  mid;
	int major;
    struct i2c_client *client;
	struct cdev chd;
    struct device *dev;
    struct class *cls;
};

#endif /* MM9932_IR_H_ */

