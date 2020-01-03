/*
 * gc0328.c
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <soc/gpio.h>

#include <linux/sensor_board.h>
#include <tx-isp/tx-isp-common.h>
#include <tx-isp/sensor-common.h>
#include <apical-isp/apical_math.h>


#define GC0328_CHIP_ID                  0x9d
#define SENSOR_VERSION                  "H20190107"


static int reset_gpio = -1;
module_param(reset_gpio, int, S_IRUGO);
MODULE_PARM_DESC(reset_gpio, "Sensor reset GPIO NUM");

static int pwron_gpio = -1;
module_param(pwron_gpio, int, S_IRUGO);
MODULE_PARM_DESC(pwron_gpio, "Power on GPIO NUM");

static int i2c_sel1_gpio = -1;
module_param(i2c_sel1_gpio, int, S_IRUGO);
MODULE_PARM_DESC(i2c_sel1_gpio, "I2C select 1 GPIO NUM");

static int dvp_gpio_func = DVP_PA_LOW_8BIT;
module_param(dvp_gpio_func, int, S_IRUGO);
MODULE_PARM_DESC(dvp_gpio_func, "Sensor DVP GPIO function");



struct tx_isp_sensor_attribute gc0328_attr;
static struct sensor_board_info *gc0328_board_info;


unsigned int gc0328_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	return 0;
}

unsigned int gc0328_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_sensor_attribute gc0328_attr={
	.name = "gc0328",
	.chip_id = GC0328_CHIP_ID,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = V4L2_SBUS_MASK_SAMPLE_8BITS | V4L2_SBUS_MASK_ADDR_8BITS,
	.cbus_device = 0x21,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP,
	.dvp = {
		.mode = SENSOR_DVP_HREF_MODE,
		.blanking = {
			.vblanking = 0,
			.hblanking = 0,
		},
	},
	.max_again = 0xff << (TX_ISP_GAIN_FIXED_POINT - 4),
	.max_dgain = 0,
	.min_integration_time = 1,
	.min_integration_time_native = 1,
	.max_integration_time_native = 592 - 4,
	.integration_time_limit = 592 - 4,
	.total_width = 746,
	.total_height = 592,
	.max_integration_time = 592 - 4,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 1,
	.dgain_apply_delay = 1,
	.sensor_ctrl.alloc_again = gc0328_alloc_again,
	.sensor_ctrl.alloc_dgain = gc0328_alloc_dgain,
	.one_line_expr_in_us = 44,
	//void priv; /* point to struct tx_isp_sensor_board_info */
};

static struct camera_reg_op gc0328_init_regs_640_480_25fps[] = {
    {CAMERA_REG_OP_DATA, 0xfe, 0x80},
    {CAMERA_REG_OP_DATA, 0xfe, 0x80},
    {CAMERA_REG_OP_DATA, 0xfc, 0x16},
    {CAMERA_REG_OP_DATA, 0xfc, 0x16},
    {CAMERA_REG_OP_DATA, 0xfc, 0x16},
    {CAMERA_REG_OP_DATA, 0xfc, 0x16},
    {CAMERA_REG_OP_DATA, 0xf1, 0x00},
    {CAMERA_REG_OP_DATA, 0xf2, 0x00},
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x03, 0x00},
    {CAMERA_REG_OP_DATA, 0x04, 0xc0},
    {CAMERA_REG_OP_DATA, 0x42, 0x00},
    {CAMERA_REG_OP_DATA, 0x77, 0x5a},
    {CAMERA_REG_OP_DATA, 0x78, 0x40},
    {CAMERA_REG_OP_DATA, 0x79, 0x56},

    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0x0d, 0x01},
    {CAMERA_REG_OP_DATA, 0x0e, 0xe8},
    {CAMERA_REG_OP_DATA, 0x0f, 0x02},
    {CAMERA_REG_OP_DATA, 0x10, 0x88},
    {CAMERA_REG_OP_DATA, 0x09, 0x00},
    {CAMERA_REG_OP_DATA, 0x0a, 0x00},
    {CAMERA_REG_OP_DATA, 0x0b, 0x00},
    {CAMERA_REG_OP_DATA, 0x0c, 0x00},
    {CAMERA_REG_OP_DATA, 0x16, 0x00},
    {CAMERA_REG_OP_DATA, 0x17, 0x16}, //mirror
    {CAMERA_REG_OP_DATA, 0x18, 0x0e},
    {CAMERA_REG_OP_DATA, 0x19, 0x06},

    {CAMERA_REG_OP_DATA, 0x1b, 0x48},
    {CAMERA_REG_OP_DATA, 0x1f, 0xC8},
    {CAMERA_REG_OP_DATA, 0x20, 0x01},
    {CAMERA_REG_OP_DATA, 0x21, 0x78},
    {CAMERA_REG_OP_DATA, 0x22, 0xb0},
    {CAMERA_REG_OP_DATA, 0x23, 0x06},
    {CAMERA_REG_OP_DATA, 0x24, 0x11},
    {CAMERA_REG_OP_DATA, 0x26, 0x00},

#if 1
    {CAMERA_REG_OP_DATA, 0x50, 0x01},
#else
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0x50, 0x01},
    {CAMERA_REG_OP_DATA, 0x51, 0x00}, //win_y
    {CAMERA_REG_OP_DATA, 0x52, 0x00},
    {CAMERA_REG_OP_DATA, 0x53, 0x00}, //win_x
    {CAMERA_REG_OP_DATA, 0x54, 0x00},
    {CAMERA_REG_OP_DATA, 0x55, 0x01}, //win_height
    {CAMERA_REG_OP_DATA, 0x56, 0xe0},
    {CAMERA_REG_OP_DATA, 0x57, 0x02}, //win_width
    {CAMERA_REG_OP_DATA, 0x58, 0x80},

    {CAMERA_REG_OP_DATA, 0x5a, 0x0e},
    {CAMERA_REG_OP_DATA, 0x59, 0x11},
    {CAMERA_REG_OP_DATA, 0x5b, 0x02},
    {CAMERA_REG_OP_DATA, 0x5c, 0x04},
    {CAMERA_REG_OP_DATA, 0x5d, 0x00},
    {CAMERA_REG_OP_DATA, 0x5e, 0x00},
    {CAMERA_REG_OP_DATA, 0x5f, 0x02},
    {CAMERA_REG_OP_DATA, 0x60, 0x04},
    {CAMERA_REG_OP_DATA, 0x61, 0x00},
    {CAMERA_REG_OP_DATA, 0x62, 0x00},
#endif

    //global gain for range
    {CAMERA_REG_OP_DATA, 0x70, 0x45},

    /////////////banding/////////////
    {CAMERA_REG_OP_DATA, 0x05, 0x00},//hb
    {CAMERA_REG_OP_DATA, 0x06, 0x6a},//
    {CAMERA_REG_OP_DATA, 0x07, 0x00},//vb
    {CAMERA_REG_OP_DATA, 0x08, 0x70},//
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},//

    {CAMERA_REG_OP_DATA, 0x29, 0x00},//anti-flicker step [11:8]
    {CAMERA_REG_OP_DATA, 0x2a, 0x96},//anti-flicker step [7:0]

    {CAMERA_REG_OP_DATA, 0x2b, 0x02},//exp level 0
    {CAMERA_REG_OP_DATA, 0x2c, 0x58},//

    {CAMERA_REG_OP_DATA, 0x2d, 0x02},//exp level 1
    {CAMERA_REG_OP_DATA, 0x2e, 0x58},//

    {CAMERA_REG_OP_DATA, 0x2f, 0x02},//exp level 2
    {CAMERA_REG_OP_DATA, 0x30, 0x58},//

    {CAMERA_REG_OP_DATA, 0x31, 0x02},//exp level 3
    {CAMERA_REG_OP_DATA, 0x32, 0x58},//

    {CAMERA_REG_OP_DATA, 0xfe, 0x00},//

    ///////////////AWB//////////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},
    {CAMERA_REG_OP_DATA, 0x50, 0x00},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4c, 0x01},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4d, 0x30},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4d, 0x40},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4d, 0x50},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4d, 0x60},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4d, 0x70},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4f, 0x01},
    {CAMERA_REG_OP_DATA, 0x50, 0x88},
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},

    //////////// BLK//////////////////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0x27, 0xb7},
    {CAMERA_REG_OP_DATA, 0x28, 0x7F},
    {CAMERA_REG_OP_DATA, 0x29, 0x20},
    {CAMERA_REG_OP_DATA, 0x33, 0x20},
    {CAMERA_REG_OP_DATA, 0x34, 0x20},
    {CAMERA_REG_OP_DATA, 0x35, 0x20},
    {CAMERA_REG_OP_DATA, 0x36, 0x20},
    {CAMERA_REG_OP_DATA, 0x32, 0x08},
    {CAMERA_REG_OP_DATA, 0x3b, 0x00},
    {CAMERA_REG_OP_DATA, 0x3c, 0x00},
    {CAMERA_REG_OP_DATA, 0x3d, 0x00},
    {CAMERA_REG_OP_DATA, 0x3e, 0x00},
    {CAMERA_REG_OP_DATA, 0x47, 0x00},
    {CAMERA_REG_OP_DATA, 0x48, 0x00},

    //////////// block enable/////////////
    {CAMERA_REG_OP_DATA, 0x40, 0x7f},
    {CAMERA_REG_OP_DATA, 0x41, 0x26},
    {CAMERA_REG_OP_DATA, 0x42, 0xfb},
    {CAMERA_REG_OP_DATA, 0x44, 0x02}, //yuv
    {CAMERA_REG_OP_DATA, 0x45, 0x00},
    {CAMERA_REG_OP_DATA, 0x46, 0x02},
    {CAMERA_REG_OP_DATA, 0x4f, 0x01},
    {CAMERA_REG_OP_DATA, 0x4b, 0x01},
    {CAMERA_REG_OP_DATA, 0x50, 0x01},

    /////DN & EEINTP/////
    {CAMERA_REG_OP_DATA, 0x7e, 0x0a},
    {CAMERA_REG_OP_DATA, 0x7f, 0x03},
    {CAMERA_REG_OP_DATA, 0x81, 0x15},
    {CAMERA_REG_OP_DATA, 0x82, 0x90},
    {CAMERA_REG_OP_DATA, 0x83, 0x02},
    {CAMERA_REG_OP_DATA, 0x84, 0xe5},
    {CAMERA_REG_OP_DATA, 0x90, 0x2c},
    {CAMERA_REG_OP_DATA, 0x92, 0x02},
    {CAMERA_REG_OP_DATA, 0x94, 0x02},
    {CAMERA_REG_OP_DATA, 0x95, 0x35},

    ////////////YCP///////////
    {CAMERA_REG_OP_DATA, 0xd1, 0x24},// 0x30 for front
    {CAMERA_REG_OP_DATA, 0xd2, 0x24},// 0x30 for front
    {CAMERA_REG_OP_DATA, 0xd3, 0x40},
    {CAMERA_REG_OP_DATA, 0xdd, 0xd3},
    {CAMERA_REG_OP_DATA, 0xde, 0x38},
    {CAMERA_REG_OP_DATA, 0xe4, 0x88},
    {CAMERA_REG_OP_DATA, 0xe5, 0x40},
    {CAMERA_REG_OP_DATA, 0xd7, 0x0e},

    ///////////rgb gamma ////////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0xbf, 0x0e},
    {CAMERA_REG_OP_DATA, 0xc0, 0x1c},
    {CAMERA_REG_OP_DATA, 0xc1, 0x34},
    {CAMERA_REG_OP_DATA, 0xc2, 0x48},
    {CAMERA_REG_OP_DATA, 0xc3, 0x5a},
    {CAMERA_REG_OP_DATA, 0xc4, 0x6e},
    {CAMERA_REG_OP_DATA, 0xc5, 0x80},
    {CAMERA_REG_OP_DATA, 0xc6, 0x9c},
    {CAMERA_REG_OP_DATA, 0xc7, 0xb4},
    {CAMERA_REG_OP_DATA, 0xc8, 0xc7},
    {CAMERA_REG_OP_DATA, 0xc9, 0xd7},
    {CAMERA_REG_OP_DATA, 0xca, 0xe3},
    {CAMERA_REG_OP_DATA, 0xcb, 0xed},
    {CAMERA_REG_OP_DATA, 0xcc, 0xf2},
    {CAMERA_REG_OP_DATA, 0xcd, 0xf8},
    {CAMERA_REG_OP_DATA, 0xce, 0xfd},
    {CAMERA_REG_OP_DATA, 0xcf, 0xff},

    /////////////Y gamma//////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0x63, 0x00},
    {CAMERA_REG_OP_DATA, 0x64, 0x05},
    {CAMERA_REG_OP_DATA, 0x65, 0x0b},
    {CAMERA_REG_OP_DATA, 0x66, 0x19},
    {CAMERA_REG_OP_DATA, 0x67, 0x2e},
    {CAMERA_REG_OP_DATA, 0x68, 0x40},
    {CAMERA_REG_OP_DATA, 0x69, 0x54},
    {CAMERA_REG_OP_DATA, 0x6a, 0x66},
    {CAMERA_REG_OP_DATA, 0x6b, 0x86},
    {CAMERA_REG_OP_DATA, 0x6c, 0xa7},
    {CAMERA_REG_OP_DATA, 0x6d, 0xc6},
    {CAMERA_REG_OP_DATA, 0x6e, 0xe4},
    {CAMERA_REG_OP_DATA, 0x6f, 0xff},

    //////////////ASDE/////////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},
    {CAMERA_REG_OP_DATA, 0x18, 0x02},
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0x97, 0x30},
    {CAMERA_REG_OP_DATA, 0x98, 0x00},
    {CAMERA_REG_OP_DATA, 0x9b, 0x60},
    {CAMERA_REG_OP_DATA, 0x9c, 0x60},
    {CAMERA_REG_OP_DATA, 0xa4, 0x50},
    {CAMERA_REG_OP_DATA, 0xa8, 0x80},
    {CAMERA_REG_OP_DATA, 0xaa, 0x40},
    {CAMERA_REG_OP_DATA, 0xa2, 0x23},
    {CAMERA_REG_OP_DATA, 0xad, 0x28},

    //////////////abs///////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},
    {CAMERA_REG_OP_DATA, 0x9c, 0x00},
    {CAMERA_REG_OP_DATA, 0x9e, 0xc0},
    {CAMERA_REG_OP_DATA, 0x9f, 0x40},

    ////////////// AEC////////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},
    {CAMERA_REG_OP_DATA, 0x08, 0xa0},
    {CAMERA_REG_OP_DATA, 0x09, 0xe8},
    {CAMERA_REG_OP_DATA, 0x10, 0x08},
    {CAMERA_REG_OP_DATA, 0x11, 0x21},
    {CAMERA_REG_OP_DATA, 0x12, 0x11},
    {CAMERA_REG_OP_DATA, 0x13, 0x45},
    {CAMERA_REG_OP_DATA, 0x15, 0xfc},
    {CAMERA_REG_OP_DATA, 0x18, 0x02},
    {CAMERA_REG_OP_DATA, 0x21, 0xf0},
    {CAMERA_REG_OP_DATA, 0x22, 0x60},
    {CAMERA_REG_OP_DATA, 0x23, 0x30},
    {CAMERA_REG_OP_DATA, 0x25, 0x00},
    {CAMERA_REG_OP_DATA, 0x24, 0x14},
    {CAMERA_REG_OP_DATA, 0x3d, 0x80},
    {CAMERA_REG_OP_DATA, 0x3e, 0x40},

    ////////////////AWB///////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},
    {CAMERA_REG_OP_DATA, 0x51, 0x88},
    {CAMERA_REG_OP_DATA, 0x52, 0x12},
    {CAMERA_REG_OP_DATA, 0x53, 0x80},
    {CAMERA_REG_OP_DATA, 0x54, 0x60},
    {CAMERA_REG_OP_DATA, 0x55, 0x01},
    {CAMERA_REG_OP_DATA, 0x56, 0x02},
    {CAMERA_REG_OP_DATA, 0x58, 0x00},
    {CAMERA_REG_OP_DATA, 0x5b, 0x02},
    {CAMERA_REG_OP_DATA, 0x5e, 0xa4},
    {CAMERA_REG_OP_DATA, 0x5f, 0x8a},
    {CAMERA_REG_OP_DATA, 0x61, 0xdc},
    {CAMERA_REG_OP_DATA, 0x62, 0xdc},
    {CAMERA_REG_OP_DATA, 0x70, 0xfc},
    {CAMERA_REG_OP_DATA, 0x71, 0x10},
    {CAMERA_REG_OP_DATA, 0x72, 0x30},
    {CAMERA_REG_OP_DATA, 0x73, 0x0b},
    {CAMERA_REG_OP_DATA, 0x74, 0x0b},
    {CAMERA_REG_OP_DATA, 0x75, 0x01},
    {CAMERA_REG_OP_DATA, 0x76, 0x00},
    {CAMERA_REG_OP_DATA, 0x77, 0x40},
    {CAMERA_REG_OP_DATA, 0x78, 0x70},
    {CAMERA_REG_OP_DATA, 0x79, 0x00},
    {CAMERA_REG_OP_DATA, 0x7b, 0x00},
    {CAMERA_REG_OP_DATA, 0x7c, 0x71},
    {CAMERA_REG_OP_DATA, 0x7d, 0x00},
    {CAMERA_REG_OP_DATA, 0x80, 0x70},
    {CAMERA_REG_OP_DATA, 0x81, 0x58},
    {CAMERA_REG_OP_DATA, 0x82, 0x98},
    {CAMERA_REG_OP_DATA, 0x83, 0x60},
    {CAMERA_REG_OP_DATA, 0x84, 0x58},
    {CAMERA_REG_OP_DATA, 0x85, 0x50},
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},

    ////////////////LSC////////////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},
    {CAMERA_REG_OP_DATA, 0xc0, 0x10},
    {CAMERA_REG_OP_DATA, 0xc1, 0x0c},
    {CAMERA_REG_OP_DATA, 0xc2, 0x0a},
    {CAMERA_REG_OP_DATA, 0xc6, 0x0e},
    {CAMERA_REG_OP_DATA, 0xc7, 0x0b},
    {CAMERA_REG_OP_DATA, 0xc8, 0x0a},
    {CAMERA_REG_OP_DATA, 0xba, 0x26},
    {CAMERA_REG_OP_DATA, 0xbb, 0x1c},
    {CAMERA_REG_OP_DATA, 0xbc, 0x1d},
    {CAMERA_REG_OP_DATA, 0xb4, 0x23},
    {CAMERA_REG_OP_DATA, 0xb5, 0x1c},
    {CAMERA_REG_OP_DATA, 0xb6, 0x1a},
    {CAMERA_REG_OP_DATA, 0xc3, 0x00},
    {CAMERA_REG_OP_DATA, 0xc4, 0x00},
    {CAMERA_REG_OP_DATA, 0xc5, 0x00},
    {CAMERA_REG_OP_DATA, 0xc9, 0x00},
    {CAMERA_REG_OP_DATA, 0xca, 0x00},
    {CAMERA_REG_OP_DATA, 0xcb, 0x00},
    {CAMERA_REG_OP_DATA, 0xbd, 0x00},
    {CAMERA_REG_OP_DATA, 0xbe, 0x00},
    {CAMERA_REG_OP_DATA, 0xbf, 0x00},
    {CAMERA_REG_OP_DATA, 0xb7, 0x07},
    {CAMERA_REG_OP_DATA, 0xb8, 0x05},
    {CAMERA_REG_OP_DATA, 0xb9, 0x05},
    {CAMERA_REG_OP_DATA, 0xa8, 0x07},
    {CAMERA_REG_OP_DATA, 0xa9, 0x06},
    {CAMERA_REG_OP_DATA, 0xaa, 0x00},
    {CAMERA_REG_OP_DATA, 0xab, 0x04},
    {CAMERA_REG_OP_DATA, 0xac, 0x00},
    {CAMERA_REG_OP_DATA, 0xad, 0x02},
    {CAMERA_REG_OP_DATA, 0xae, 0x0d},
    {CAMERA_REG_OP_DATA, 0xaf, 0x05},
    {CAMERA_REG_OP_DATA, 0xb0, 0x00},
    {CAMERA_REG_OP_DATA, 0xb1, 0x07},
    {CAMERA_REG_OP_DATA, 0xb2, 0x03},
    {CAMERA_REG_OP_DATA, 0xb3, 0x00},
    {CAMERA_REG_OP_DATA, 0xa4, 0x00},
    {CAMERA_REG_OP_DATA, 0xa5, 0x00},
    {CAMERA_REG_OP_DATA, 0xa6, 0x00},
    {CAMERA_REG_OP_DATA, 0xa7, 0x00},
    {CAMERA_REG_OP_DATA, 0xa1, 0x3c},
    {CAMERA_REG_OP_DATA, 0xa2, 0x50},
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},

    ///////////////CCT ///////////
    {CAMERA_REG_OP_DATA, 0xb1, 0x12},
    {CAMERA_REG_OP_DATA, 0xb2, 0xf5},
    {CAMERA_REG_OP_DATA, 0xb3, 0xfe},
    {CAMERA_REG_OP_DATA, 0xb4, 0xe0},
    {CAMERA_REG_OP_DATA, 0xb5, 0x15},
    {CAMERA_REG_OP_DATA, 0xb6, 0xc8},

    /*/////skin CC for front //////
    {CAMERA_REG_OP_DATA, 0xb1, 0x00},
    {CAMERA_REG_OP_DATA, 0xb2, 0x00},
    {CAMERA_REG_OP_DATA, 0xb3, 0x00},
    {CAMERA_REG_OP_DATA, 0xb4, 0xf0},
    {CAMERA_REG_OP_DATA, 0xb5, 0x00},
    {CAMERA_REG_OP_DATA, 0xb6, 0x00},
    */

    ///////////////AWB////////////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},
    {CAMERA_REG_OP_DATA, 0x50, 0x00},
    {CAMERA_REG_OP_DATA, 0xfe, 0x01},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4c, 0x01},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4f, 0x00},
    {CAMERA_REG_OP_DATA, 0x4d, 0x34},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x02},
    {CAMERA_REG_OP_DATA, 0x4e, 0x02},
    {CAMERA_REG_OP_DATA, 0x4d, 0x44},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4d, 0x53},
    {CAMERA_REG_OP_DATA, 0x4e, 0x00},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4d, 0x65},
    {CAMERA_REG_OP_DATA, 0x4e, 0x04},
    {CAMERA_REG_OP_DATA, 0x4d, 0x73},
    {CAMERA_REG_OP_DATA, 0x4e, 0x20},
    {CAMERA_REG_OP_DATA, 0x4d, 0x83},
    {CAMERA_REG_OP_DATA, 0x4e, 0x20},
    {CAMERA_REG_OP_DATA, 0x4f, 0x01},
    {CAMERA_REG_OP_DATA, 0x50, 0x88},

    /////////output////////
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0xf1, 0x07},
    {CAMERA_REG_OP_DATA, 0xf2, 0x01},

	{CAMERA_REG_OP_END, 0x0, 0x0}, /* END MARKER */
};

/*
 * the order of the gc0328_win_sizes is [full_resolution, preview_resolution].
 */
static struct tx_isp_sensor_win_setting gc0328_win_sizes[] = {
	{
		.width		= 640,
		.height		= 480,
		.fps		= 25 << 16 | 1, /* 12.5 fps */
		.mbus_code	= V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace	= V4L2_COLORSPACE_SRGB,
		.regs 		= gc0328_init_regs_640_480_25fps,
	}
};

/*
 * the part of driver was fixed.
 */

static struct camera_reg_op gc0328_stream_on[] = {
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0xf1, 0x07},
    {CAMERA_REG_OP_DATA, 0xf2, 0x01},
	{CAMERA_REG_OP_END, 0x0, 0x0},	/* END MARKER */
};

static struct camera_reg_op gc0328_stream_off[] = {
    {CAMERA_REG_OP_DATA, 0xfe, 0x00},
    {CAMERA_REG_OP_DATA, 0xf1, 0x00},
    {CAMERA_REG_OP_DATA, 0xf2, 0x00},
	{CAMERA_REG_OP_END, 0x0, 0x0},	/* END MARKER */
};

int gc0328_read(struct tx_isp_subdev *sd, unsigned char reg, unsigned char *value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	struct i2c_msg msg[2] = {
		[0] = {
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &reg,
		},
		[1] = {
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= value,
		}
	};
	int ret;
	ret = private_i2c_transfer(client->adapter, msg, 2);
	if (ret > 0)
		ret = 0;

	return ret;
}

int gc0328_write(struct tx_isp_subdev *sd, unsigned char reg, unsigned char value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned char buf[2] = {reg, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= buf,
	};
	int ret;
	ret = private_i2c_transfer(client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;

	return ret;
}

static int gc0328_read_array(struct tx_isp_subdev *sd, struct camera_reg_op *vals)
{
	int ret;
	unsigned char val;
	while (vals->flag != CAMERA_REG_OP_END) {
        if (vals->flag == CAMERA_REG_OP_DATA) {
            if (vals->reg == 0xfe) {
                ret = gc0328_write(sd, vals->reg, vals->val);
                if (ret < 0)
                    return ret;
                printk("%x[%x]\n", vals->reg, vals->val);
            } else {
                ret = gc0328_read(sd, vals->reg, &val);
                if (ret < 0)
                    return ret;
                printk("%x[%x]\n", vals->reg, val);
            }
        } else if (vals->flag == CAMERA_REG_OP_DELAY) {
            private_msleep(vals->val);
        } else {
            pr_debug("%s(%d), error flag: %d\n", __func__, __LINE__, vals->flag);
            return -1;
        }
		vals++;
	}

	return 0;
}

static int gc0328_write_array(struct tx_isp_subdev *sd, struct camera_reg_op *vals)
{
	int ret;
	while (vals->flag != CAMERA_REG_OP_END) {
        if (vals->flag == CAMERA_REG_OP_DATA) {
            ret = gc0328_write(sd, vals->reg, vals->val);
            if (ret < 0)
                return ret;
        } else if (vals->flag == CAMERA_REG_OP_DELAY) {
            private_msleep(vals->val);
        } else {
            pr_debug("%s(%d), error flag: %d\n", __func__, __LINE__, vals->flag);
            return -1;
        }
		vals++;
	}

	return 0;
}

static int gc0328_reset(struct tx_isp_subdev *sd, int val)
{
	return 0;
}

static int gc0328_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	unsigned char v;
	int ret;

	ret = gc0328_read(sd, 0xf0, &v);
	pr_debug("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != GC0328_CHIP_ID)
		return -ENODEV;
	*ident = v;

	return 0;
}

static int gc0328_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	return 0;
}
static int gc0328_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}
static int gc0328_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int gc0328_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int gc0328_init(struct tx_isp_subdev *sd, int enable)
{
	struct tx_isp_sensor *sensor = (container_of(sd, struct tx_isp_sensor, sd));
	struct tx_isp_sensor_win_setting *wsize = &gc0328_win_sizes[0];
	int ret = 0;

	if(!enable)
		return ISP_SUCCESS;

	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = V4L2_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
    ret = gc0328_write_array(sd, wsize->regs);
    if (ret)
        return ret;
    //gc0328_read_array(sd, wsize->regs);
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	sensor->priv = wsize;

	return ret;
}

static int gc0328_s_stream(struct tx_isp_subdev *sd, int enable)
{
	int ret = 0;

	if (enable) {
		ret = gc0328_write_array(sd, gc0328_stream_on);
		pr_debug("gc0328 stream on\n");
	}
	else {
		ret = gc0328_write_array(sd, gc0328_stream_off);
		pr_debug("gc0328 stream off\n");
	}
	return ret;
}

static int gc0328_set_fps(struct tx_isp_subdev *sd, int fps)
{
	return 0;
}

static int gc0328_set_mode(struct tx_isp_subdev *sd, int value)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	struct tx_isp_sensor_win_setting *wsize = NULL;
	int ret = ISP_SUCCESS;
	if(value == TX_ISP_SENSOR_FULL_RES_MAX_FPS){
		wsize = &gc0328_win_sizes[0];
	}else if(value == TX_ISP_SENSOR_PREVIEW_RES_MAX_FPS){
		wsize = &gc0328_win_sizes[0];
	}
	if(wsize){
		sensor->video.mbus.width = wsize->width;
		sensor->video.mbus.height = wsize->height;
		sensor->video.mbus.code = wsize->mbus_code;
		sensor->video.mbus.field = V4L2_FIELD_NONE;
		sensor->video.mbus.colorspace = wsize->colorspace;
		sensor->video.fps = wsize->fps;
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	}
	return ret;
}

static int gc0328_set_vflip(struct tx_isp_subdev *sd, int enable)
{
	return 0;
}

static int gc0328_g_chip_ident(struct tx_isp_subdev *sd,
		struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

    if (pwron_gpio != -1) {
		ret = gpio_request(pwron_gpio, "gc0328_pwron");
		if (!ret) {
			private_gpio_direction_output(pwron_gpio, 1);
            private_msleep(50);
        }
		else
			printk("gpio requrest fail %d\n", pwron_gpio);
	}
	if (reset_gpio != -1) {
		ret = gpio_request(reset_gpio, "gc0328_reset");
		if (!ret) {
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(50);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(10);
		} else
			printk("gpio requrest fail %d\n", reset_gpio);
	}
    if (i2c_sel1_gpio != -1) {
        ret = gpio_request(i2c_sel1_gpio, "gc0328_i2c_sel1");
        if (!ret)
			private_gpio_direction_output(i2c_sel1_gpio, 1);
		else
			printk("gpio requrest fail %d\n", i2c_sel1_gpio);
    }

	ret = gc0328_detect(sd, &ident);
	if (ret) {
		printk("chip found @ 0x%x (%s) is not an gc0328 chip.\n",
		       client->addr, client->adapter->name);
		return ret;
	}
	printk("gc0328 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	if(chip){
		memcpy(chip->name, "gc0328", sizeof("gc0328"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}
	return 0;
}

static int gc0328_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
{
	long ret = 0;
	if(IS_ERR_OR_NULL(sd)){
		printk("[%d]The pointer is invalid!\n", __LINE__);
		return -EINVAL;
	}
	switch(cmd){
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		if(arg)
			ret = gc0328_set_integration_time(sd, *(int*)arg);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			ret = gc0328_set_analog_gain(sd, *(int*)arg);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = gc0328_set_digital_gain(sd, *(int*)arg);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = gc0328_get_black_pedestal(sd, *(int*)arg);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = gc0328_set_mode(sd, *(int*)arg);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		if(arg)
			ret = gc0328_write_array(sd, gc0328_stream_off);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		if(arg)
			ret = gc0328_write_array(sd, gc0328_stream_on);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = gc0328_set_fps(sd, *(int*)arg);
		break;
	case TX_ISP_EVENT_SENSOR_VFLIP:
		if(arg)
			ret = gc0328_set_vflip(sd, *(int*)arg);
		break;
	default:
		break;;
	}
	return 0;
}

static int gc0328_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
{
	unsigned char val = 0;
	int len = 0;
	int ret = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	ret = gc0328_read(sd, reg->reg & 0xff, &val);
	reg->val = val;
	reg->size = 1;

	return ret;
}

static int gc0328_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	gc0328_write(sd, reg->reg & 0xff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops gc0328_core_ops = {
	.g_chip_ident = gc0328_g_chip_ident,
	.reset = gc0328_reset,
	.init = gc0328_init,
	.g_register = gc0328_g_register,
	.s_register = gc0328_s_register,
};

static struct tx_isp_subdev_video_ops gc0328_video_ops = {
	.s_stream = gc0328_s_stream,
};

static struct tx_isp_subdev_sensor_ops	gc0328_sensor_ops = {
	.ioctl	= gc0328_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops gc0328_ops = {
	.core = &gc0328_core_ops,
	.video = &gc0328_video_ops,
	.sensor = &gc0328_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "gc0328",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};

static int gc0328_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct tx_isp_subdev *sd;
	struct tx_isp_video_in *video;
	struct tx_isp_sensor *sensor;
	struct tx_isp_sensor_win_setting *wsize = &gc0328_win_sizes[0];
	int ret = -1;

    pr_debug("probe ok ----start--->gc0328\n");
	sensor = (struct tx_isp_sensor *)kzalloc(sizeof(*sensor), GFP_KERNEL);
	if(!sensor){
		printk("Failed to allocate sensor subdev.\n");
		return -ENOMEM;
	}
	memset(sensor, 0 ,sizeof(*sensor));

#ifndef MODULE
    gc0328_board_info = get_sensor_board_info(sensor_platform_device.name);
    if (gc0328_board_info) {
        reset_gpio    = gc0328_board_info->gpios.gpio_sensor_rst;
        pwron_gpio    = gc0328_board_info->gpios.gpio_power_on;
        i2c_sel1_gpio = gc0328_board_info->gpios.gpio_i2c_sel1;
        dvp_gpio_func = gc0328_board_info->dvp_gpio_func;
    }
#endif

	/* request mclk of sensor */
	sensor->mclk = clk_get(NULL, "cgu_cim");
	if (IS_ERR(sensor->mclk)) {
		printk("Cannot get sensor input clock cgu_cim\n");
		goto err_get_mclk;
	}
	private_clk_set_rate(sensor->mclk, 24000000);
	private_clk_enable(sensor->mclk);

	ret = set_sensor_gpio_function(dvp_gpio_func);
	if (ret < 0)
		goto err_set_sensor_gpio;
	gc0328_attr.dvp.gpio = dvp_gpio_func;

	 /*
		convert sensor-gain into isp-gain,
	 */
	gc0328_attr.max_again = log2_fixed_to_fixed(gc0328_attr.max_again, TX_ISP_GAIN_FIXED_POINT, LOG2_GAIN_SHIFT);
	gc0328_attr.max_dgain = gc0328_attr.max_dgain;
	sd = &sensor->sd;
	video = &sensor->video;
	sensor->video.attr = &gc0328_attr;
	sensor->video.mbus_change = 0;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = V4L2_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &gc0328_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->gc0328\n");

	return 0;
err_set_sensor_gpio:
	clk_disable(sensor->mclk);
	clk_put(sensor->mclk);
err_get_mclk:
	kfree(sensor);

	return -1;
}

static int gc0328_remove(struct i2c_client *client)
{
	struct tx_isp_subdev *sd = private_i2c_get_clientdata(client);
	struct tx_isp_sensor *sensor = tx_isp_get_subdev_hostdata(sd);

    if(pwron_gpio != -1) {
        private_gpio_direction_output(pwron_gpio, 0);
        private_gpio_free(pwron_gpio);
    }
	if(reset_gpio != -1)
		private_gpio_free(reset_gpio);
    if(i2c_sel1_gpio != -1)
        private_gpio_free(i2c_sel1_gpio);

	private_clk_disable(sensor->mclk);
	private_clk_put(sensor->mclk);
	tx_isp_subdev_deinit(sd);
	kfree(sensor);

	return 0;
}

static const struct i2c_device_id gc0328_id[] = {
	{ "gc0328", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, gc0328_id);

static struct i2c_driver gc0328_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "gc0328",
	},
	.probe		= gc0328_probe,
	.remove		= gc0328_remove,
	.id_table	= gc0328_id,
};

static __init int init_gc0328(void)
{
	int ret = 0;
	ret = private_driver_get_interface();
	if(ret){
		printk("Failed to init gc0328 dirver.\n");
		return -1;
	}

	return private_i2c_add_driver(&gc0328_driver);
}

static __exit void exit_gc0328(void)
{
	i2c_del_driver(&gc0328_driver);
}

module_init(init_gc0328);
module_exit(exit_gc0328);

MODULE_DESCRIPTION("A low-level driver for gc0328 sensors");
MODULE_LICENSE("GPL");
