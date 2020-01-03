/*
 * gc0328 Camera Driver
 *
 * Copyright (C) 2017, Ingenic Semiconductor Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-subdev.h>
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>


#define REG_CHIP_ID                        0xf0
#define PID_GC0328                         0x9d

/* Private v4l2 controls */
#define V4L2_CID_PRIVATE_BALANCE           (V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_PRIVATE_EFFECT            (V4L2_CID_PRIVATE_BASE + 1)

#define REG14                              0x14
#define REG14_HFLIP_IMG                    0x01 /* Horizontal mirror image ON/OFF */
#define REG14_VFLIP_IMG                    0x02 /* Vertical flip image ON/OFF */

 /* whether sensor support high resolution (> vga) preview or not */
#define SUPPORT_HIGH_RESOLUTION_PRE        1

/*
 * Struct
 */
struct regval_list {
    u8 reg_num;
    u8 value;
};

struct mode_list {
    u8 index;
    const struct regval_list *mode_regs;
};

/* Supported resolutions */
enum gc0328_width {
    W_QCIF   = 176,
    W_QVGA   = 320,
    W_CIF    = 352,
    W_VGA    = 640,
};

enum gc0328_height {
    H_QCIF   = 144,
    H_QVGA   = 240,
    H_CIF    = 288,
    H_VGA    = 480,
};

struct gc0328_win_size {
    char *name;
    enum gc0328_width width;
    enum gc0328_height height;
    const struct regval_list *regs;
};


struct gc0328_priv {
    struct v4l2_subdev subdev;
    struct gc0328_camera_info *info;
    enum v4l2_mbus_pixelcode cfmt_code;
    const struct gc0328_win_size *win;
    int    model;
    u8 balance_value;
    u8 effect_value;
    u16 flag_vflip:1;
    u16 flag_hflip:1;
};

static inline int gc0328_write_reg(struct i2c_client * client, unsigned char addr, unsigned char value)
{
    return i2c_smbus_write_byte_data(client, addr, value);
}
static inline char gc0328_read_reg(struct i2c_client *client, unsigned char addr)
{
    char ret;

    ret = i2c_smbus_read_byte_data(client, addr);
    if (ret < 0)
        return ret;

    return ret;
}
/*
 * Registers settings
 */

#define ENDMARKER { 0xff, 0xff }

static const struct regval_list gc0328_init_regs[] = {
    //config list from net
    {0xfe , 0x80},
    {0xfe , 0x80},
    {0xfc , 0x16},
    {0xfc , 0x16},
    {0xfc , 0x16},
    {0xfc , 0x16},
    {0xf1 , 0x00},
    {0xf2 , 0x00},
    {0xfe , 0x00},
    {0x4f , 0x00},
    {0x03 , 0x00},
    {0x04 , 0xc0},
    {0x42 , 0x00},
    {0x77 , 0x5a},
    {0x78 , 0x40},
    {0x79 , 0x56},

    {0xfe , 0x00},
    {0x0d , 0x01},
    {0x0e , 0xe8},
    {0x0f , 0x02},
    {0x10 , 0x88},
    {0x09 , 0x00},
    {0x0a , 0x00},
    {0x0b , 0x00},
    {0x0c , 0x00},
    {0x16 , 0x00},
    {0x17 , 0x16}, //mirror
    {0x18 , 0x0e},
    {0x19 , 0x06},

    {0x1b , 0x48},
    {0x1f , 0xC8},
    {0x20 , 0x01},
    {0x21 , 0x78},
    {0x22 , 0xb0},
    {0x23 , 0x06},
    {0x24 , 0x11},
    {0x26 , 0x00},
    {0x50 , 0x01}, //crop mode

    //global gain for range
    {0x70 , 0x45},

    /////////////banding/////////////
    {0x05 , 0x00},//hb
    {0x06 , 0x6a},//
    {0x07 , 0x00},//vb
    {0x08 , 0x70},//
    {0xfe , 0x01},//

    {0x29 , 0x00},//anti-flicker step [11:8]
    {0x2a , 0x96},//anti-flicker step [7:0]

    {0x2b , 0x02},//exp level 0
    {0x2c , 0x58},//

    {0x2d , 0x02},//exp level 1
    {0x2e , 0x58},//

    {0x2f , 0x02},//exp level 2
    {0x30 , 0x58},//

    {0x31 , 0x02},//exp level 3
    {0x32 , 0x58},//

    {0xfe , 0x00},//

    ///////////////AWB//////////////
    {0xfe , 0x01},
    {0x50 , 0x00},
    {0x4f , 0x00},
    {0x4c , 0x01},
    {0x4f , 0x00},
    {0x4f , 0x00},
    {0x4f , 0x00},
    {0x4f , 0x00},
    {0x4f , 0x00},
    {0x4d , 0x30},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4d , 0x40},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4d , 0x50},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4d , 0x60},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4d , 0x70},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4f , 0x01},
    {0x50 , 0x88},
    {0xfe , 0x00},

    //////////// BLK//////////////////////
    {0xfe , 0x00},
    {0x27 , 0xb7},
    {0x28 , 0x7F},
    {0x29 , 0x20},
    {0x33 , 0x20},
    {0x34 , 0x20},
    {0x35 , 0x20},
    {0x36 , 0x20},
    {0x32 , 0x08},
    {0x3b , 0x00},
    {0x3c , 0x00},
    {0x3d , 0x00},
    {0x3e , 0x00},
    {0x47 , 0x00},
    {0x48 , 0x00},

    //////////// block enable/////////////
    {0x40 , 0x7f},
    {0x41 , 0x26},
    {0x42 , 0xfb},
    {0x44 , 0x02}, //yuv
    {0x45 , 0x00},
    {0x46 , 0x02},
    {0x4f , 0x01},
    {0x4b , 0x01},
    {0x50 , 0x01},

    /////DN & EEINTP/////
    {0x7e , 0x0a},
    {0x7f , 0x03},
    {0x81 , 0x15},
    {0x82 , 0x90},
    {0x83 , 0x02},
    {0x84 , 0xe5},
    {0x90 , 0x2c},
    {0x92 , 0x02},
    {0x94 , 0x02},
    {0x95 , 0x35},

    ////////////YCP///////////
    {0xd1 , 0x24},// 0x30 for front
    {0xd2 , 0x24},// 0x30 for front
    {0xd3 , 0x40},
    {0xdd , 0xd3},
    {0xde , 0x38},
    {0xe4 , 0x88},
    {0xe5 , 0x40},
    {0xd7 , 0x0e},

    ///////////rgb gamma ////////////
    {0xfe , 0x00},
    {0xbf , 0x0e},
    {0xc0 , 0x1c},
    {0xc1 , 0x34},
    {0xc2 , 0x48},
    {0xc3 , 0x5a},
    {0xc4 , 0x6e},
    {0xc5 , 0x80},
    {0xc6 , 0x9c},
    {0xc7 , 0xb4},
    {0xc8 , 0xc7},
    {0xc9 , 0xd7},
    {0xca , 0xe3},
    {0xcb , 0xed},
    {0xcc , 0xf2},
    {0xcd , 0xf8},
    {0xce , 0xfd},
    {0xcf , 0xff},

    /////////////Y gamma//////////
    {0xfe , 0x00},
    {0x63 , 0x00},
    {0x64 , 0x05},
    {0x65 , 0x0b},
    {0x66 , 0x19},
    {0x67 , 0x2e},
    {0x68 , 0x40},
    {0x69 , 0x54},
    {0x6a , 0x66},
    {0x6b , 0x86},
    {0x6c , 0xa7},
    {0x6d , 0xc6},
    {0x6e , 0xe4},
    {0x6f , 0xff},

    //////////////ASDE/////////////
    {0xfe , 0x01},
    {0x18 , 0x02},
    {0xfe , 0x00},
    {0x97 , 0x30},
    {0x98 , 0x00},
    {0x9b , 0x60},
    {0x9c , 0x60},
    {0xa4 , 0x50},
    {0xa8 , 0x80},
    {0xaa , 0x40},
    {0xa2 , 0x23},
    {0xad , 0x28},

    //////////////abs///////////
    {0xfe , 0x01},
    {0x9c , 0x00},
    {0x9e , 0xc0},
    {0x9f , 0x40},

    ////////////// AEC////////////
    {0xfe , 0x01},
    {0x08 , 0xa0},
    {0x09 , 0xe8},
    {0x10 , 0x08},
    {0x11 , 0x21},
    {0x12 , 0x11},
    {0x13 , 0x45},
    {0x15 , 0xfc},
    {0x18 , 0x02},
    {0x21 , 0xf0},
    {0x22 , 0x60},
    {0x23 , 0x30},
    {0x25 , 0x00},
    {0x24 , 0x14},
    {0x3d , 0x80},
    {0x3e , 0x40},

    ////////////////AWB///////////
    {0xfe , 0x01},
    {0x51 , 0x88},
    {0x52 , 0x12},
    {0x53 , 0x80},
    {0x54 , 0x60},
    {0x55 , 0x01},
    {0x56 , 0x02},
    {0x58 , 0x00},
    {0x5b , 0x02},
    {0x5e , 0xa4},
    {0x5f , 0x8a},
    {0x61 , 0xdc},
    {0x62 , 0xdc},
    {0x70 , 0xfc},
    {0x71 , 0x10},
    {0x72 , 0x30},
    {0x73 , 0x0b},
    {0x74 , 0x0b},
    {0x75 , 0x01},
    {0x76 , 0x00},
    {0x77 , 0x40},
    {0x78 , 0x70},
    {0x79 , 0x00},
    {0x7b , 0x00},
    {0x7c , 0x71},
    {0x7d , 0x00},
    {0x80 , 0x70},
    {0x81 , 0x58},
    {0x82 , 0x98},
    {0x83 , 0x60},
    {0x84 , 0x58},
    {0x85 , 0x50},
    {0xfe , 0x00},

    ////////////////LSC////////////////
    {0xfe , 0x01},
    {0xc0 , 0x10},
    {0xc1 , 0x0c},
    {0xc2 , 0x0a},
    {0xc6 , 0x0e},
    {0xc7 , 0x0b},
    {0xc8 , 0x0a},
    {0xba , 0x26},
    {0xbb , 0x1c},
    {0xbc , 0x1d},
    {0xb4 , 0x23},
    {0xb5 , 0x1c},
    {0xb6 , 0x1a},
    {0xc3 , 0x00},
    {0xc4 , 0x00},
    {0xc5 , 0x00},
    {0xc9 , 0x00},
    {0xca , 0x00},
    {0xcb , 0x00},
    {0xbd , 0x00},
    {0xbe , 0x00},
    {0xbf , 0x00},
    {0xb7 , 0x07},
    {0xb8 , 0x05},
    {0xb9 , 0x05},
    {0xa8 , 0x07},
    {0xa9 , 0x06},
    {0xaa , 0x00},
    {0xab , 0x04},
    {0xac , 0x00},
    {0xad , 0x02},
    {0xae , 0x0d},
    {0xaf , 0x05},
    {0xb0 , 0x00},
    {0xb1 , 0x07},
    {0xb2 , 0x03},
    {0xb3 , 0x00},
    {0xa4 , 0x00},
    {0xa5 , 0x00},
    {0xa6 , 0x00},
    {0xa7 , 0x00},
    {0xa1 , 0x3c},
    {0xa2 , 0x50},
    {0xfe , 0x00},

    ///////////////CCT ///////////
    {0xb1 , 0x12},
    {0xb2 , 0xf5},
    {0xb3 , 0xfe},
    {0xb4 , 0xe0},
    {0xb5 , 0x15},
    {0xb6 , 0xc8},

    /*   /////skin CC for front //////
    {0xb1 , 0x00},
    {0xb2 , 0x00},
    {0xb3 , 0x00},
    {0xb4 , 0xf0},
    {0xb5 , 0x00},
    {0xb6 , 0x00},
    */

    ///////////////AWB////////////////
    {0xfe , 0x01},
    {0x50 , 0x00},
    {0xfe , 0x01},
    {0x4f , 0x00},
    {0x4c , 0x01},
    {0x4f , 0x00},
    {0x4f , 0x00},
    {0x4f , 0x00},
    {0x4d , 0x34},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x02},
    {0x4e , 0x02},
    {0x4d , 0x44},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4d , 0x53},
    {0x4e , 0x00},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4e , 0x04},
    {0x4d , 0x65},
    {0x4e , 0x04},
    {0x4d , 0x73},
    {0x4e , 0x20},
    {0x4d , 0x83},
    {0x4e , 0x20},
    {0x4f , 0x01},
    {0x50 , 0x88},
    /////////output////////
    {0xfe , 0x00},
    {0xf1 , 0x07},
    {0xf2 , 0x01},
    ENDMARKER,
};


static const struct regval_list gc0328_qcif_regs[] = {
    {0xfe , 0x00},
    {0x50 , 0x01},
    {0x51 , 0x00},
    {0x52 , 0x00},
    {0x53 , 0x00},
    {0x54 , 0x00},
    {0x55 , 0x00},
    {0x56 , 0x90},
    {0x57 , 0x00},
    {0x58 , 0xb0},

    {0x5a , 0x00},
    {0x59 , 0xaa},
    {0x5b , 0x02},
    {0x5c , 0x04},
    {0x5d , 0x00},
    {0x5e , 0x00},
    {0x5f , 0x02},
    {0x60 , 0x04},
    {0x61 , 0x00},
    {0x62 , 0x00},
    ENDMARKER,
};

static const struct regval_list gc0328_qvga_regs[] = {
    {0xfe , 0x00},
    {0x50 , 0x01},
    {0x51 , 0x00},
    {0x52 , 0x00},
    {0x53 , 0x00},
    {0x54 , 0x00},
    {0x55 , 0x00},
    {0x56 , 0xf0},
    {0x57 , 0x01},
    {0x58 , 0x40},

    {0x5a , 0x00},
    {0x59 , 0x22},
    {0x5b , 0x00},
    {0x5c , 0x00},
    {0x5d , 0x00},
    {0x5e , 0x00},
    {0x5f , 0x00},
    {0x60 , 0x00},
    {0x61 , 0x00},
    {0x62 , 0x00},
    ENDMARKER,
};

static const struct regval_list gc0328_cif_regs[] = {
    {0xfe , 0x00},
    {0x50 , 0x01},
    {0x51 , 0x00},
    {0x52 , 0x00},
    {0x53 , 0x00},
    {0x54 , 0x00},
    {0x55 , 0x01},
    {0x56 , 0x20},
    {0x57 , 0x01},
    {0x58 , 0x60},

    {0x5a , 0x00},
    {0x59 , 0x55},
    {0x5b , 0x02},
    {0x5c , 0x04},
    {0x5d , 0x00},
    {0x5e , 0x00},
    {0x5f , 0x02},
    {0x60 , 0x04},
    {0x61 , 0x00},
    {0x62 , 0x00},

    ENDMARKER,
};

static const struct regval_list gc0328_vga_regs[] = {
    ENDMARKER,
};

static const struct regval_list gc0328_wb_auto_regs[] = {
    {0x42,0xfe},{0x77,0x56},
    {0x78,0x40},{0x79,0x4a},
    ENDMARKER,
};

static const struct regval_list gc0328_wb_incandescence_regs[] = {
    {0x42,0x55}, {0x77,0x48},
    {0x78,0x40}, {0x79,0x5c},
    ENDMARKER,
};

static const struct regval_list gc0328_wb_daylight_regs[] = {
    {0x42,0x55}, {0x77,0x74},
    {0x78,0x52}, {0x79,0x40},
    ENDMARKER,
};

static const struct regval_list gc0328_wb_fluorescent_regs[] = {
    {0x42,0x55}, {0x77,0x40},
    {0x78,0x42}, {0x79,0x50},
    ENDMARKER,
};

static const struct regval_list gc0328_wb_cloud_regs[] = {
    {0x42,0x55}, {0x77,0x8c},
    {0x78,0x50}, {0x79,0x40},
    ENDMARKER,
};

static const struct mode_list gc0328_balance[] = {
    {0, gc0328_wb_auto_regs}, {1, gc0328_wb_incandescence_regs},
    {2, gc0328_wb_daylight_regs}, {3, gc0328_wb_fluorescent_regs},
    {4, gc0328_wb_cloud_regs},
};


static const struct regval_list gc0328_effect_normal_regs[] = {
    {0x43,0x00}, {0x4b,0x0a},
    {0x40,0x7f}, {0x10,0x90},
    {0x91,0x00}, {0x95,0x38},
    {0xd3,0x40}, {0xd4,0x80},
    {0xda,0x00}, {0xdb,0x00},
    ENDMARKER,
};

static const struct regval_list gc0328_effect_grayscale_regs[] = {
    {0x43,0x02}, {0x4b,0x0a},
    {0x40,0xff}, {0x10,0x90},
    {0x91,0x00}, {0xd3,0x40},
    {0xd4,0x80}, {0xda,0x00},
    {0xdb,0x00},
    ENDMARKER,
};

static const struct regval_list gc0328_effect_sepia_regs[] = {
    {0x43,0x02}, {0x4b,0x0a},
    {0x40,0xff}, {0x10,0x90},
    {0x91,0x00}, {0xd3,0x40},
    {0xd4,0x80}, {0xda,0xd0},
    {0xdb,0x28},
    ENDMARKER,
};

static const struct regval_list gc0328_effect_colorinv_regs[] = {
    {0x43,0x01}, {0x4b,0x0a},
    {0x40,0xff}, {0x10,0x90},
    {0x91,0x00}, {0xd3,0x40},
    {0xd4,0x80}, {0xda,0x00},
    {0xdb,0x00},
    ENDMARKER,
};

static const struct regval_list gc0328_effect_sepiabluel_regs[] = {
    {0x43,0x02}, {0x4b,0x0a},
    {0x40,0x7f}, {0x10,0x90},
    {0x91,0x00}, {0xd3,0x40},
    {0xd4,0x80}, {0xda,0x50},
    {0xdb,0xe0},
    ENDMARKER,
};

static const struct mode_list gc0328_effect[] = {
    {0, gc0328_effect_normal_regs}, {1, gc0328_effect_grayscale_regs},
    {2, gc0328_effect_sepia_regs}, {3, gc0328_effect_colorinv_regs},
    {4, gc0328_effect_sepiabluel_regs},
};

#define GC0328_SIZE(n, w, h, r) \
    {.name = n, .width = w , .height = h, .regs = r }

static const struct gc0328_win_size gc0328_supported_win_sizes[] = {
    GC0328_SIZE("QCIF", W_QCIF, H_QCIF, gc0328_qcif_regs),
    GC0328_SIZE("QVGA", W_QVGA, H_QVGA, gc0328_qvga_regs),
    GC0328_SIZE("CIF", W_CIF, H_CIF, gc0328_cif_regs),
    GC0328_SIZE("VGA", W_VGA, H_VGA, gc0328_vga_regs),
};

#define N_WIN_SIZES (ARRAY_SIZE(gc0328_supported_win_sizes))

static const struct regval_list gc0328_yuv422_regs[] = {
    ENDMARKER,
};

static const struct regval_list gc0328_rgb565_regs[] = {
    ENDMARKER,
};

static enum v4l2_mbus_pixelcode gc0328_codes[] = {
    V4L2_MBUS_FMT_YUYV8_2X8,
    V4L2_MBUS_FMT_YUYV8_1_5X8,
    V4L2_MBUS_FMT_JZYUYV8_1_5X8,
};

/*
 * Supported controls
 */
static const struct v4l2_queryctrl gc0328_controls[] = {
    {
        .id          = V4L2_CID_PRIVATE_BALANCE,
        .type        = V4L2_CTRL_TYPE_MENU,
        .name        = "whitebalance",
        .minimum     = 0,
        .maximum     = 4,
        .step        = 1,
        .default_value    = 0,
    }, {
        .id          = V4L2_CID_PRIVATE_EFFECT,
        .type        = V4L2_CTRL_TYPE_MENU,
        .name        = "effect",
        .minimum     = 0,
        .maximum     = 4,
        .step        = 1,
        .default_value    = 0,
    },
    {
        .id          = V4L2_CID_VFLIP,
        .type        = V4L2_CTRL_TYPE_BOOLEAN,
        .name        = "Flip Vertically",
        .minimum     = 0,
        .maximum     = 1,
        .step        = 1,
        .default_value    = 0,
    }, {
        .id          = V4L2_CID_HFLIP,
        .type        = V4L2_CTRL_TYPE_BOOLEAN,
        .name        = "Flip Horizontally",
        .minimum     = 0,
        .maximum     = 1,
        .step        = 1,
        .default_value    = 0,
    },
};

/*
 * Supported balance menus
 */
static const struct v4l2_querymenu gc0328_balance_menus[] = {
    {
        .id          = V4L2_CID_PRIVATE_BALANCE,
        .index       = 0,
        .name        = "auto",
    }, {
        .id          = V4L2_CID_PRIVATE_BALANCE,
        .index       = 1,
        .name        = "incandescent",
    }, {
        .id          = V4L2_CID_PRIVATE_BALANCE,
        .index       = 2,
        .name        = "fluorescent",
    },  {
        .id          = V4L2_CID_PRIVATE_BALANCE,
        .index       = 3,
        .name        = "daylight",
    },  {
        .id          = V4L2_CID_PRIVATE_BALANCE,
        .index       = 4,
        .name        = "cloudy-daylight",
    },

};

/*
 * Supported effect menus
 */
static const struct v4l2_querymenu gc0328_effect_menus[] = {
    {
        .id          = V4L2_CID_PRIVATE_EFFECT,
        .index       = 0,
        .name        = "none",
    }, {
        .id          = V4L2_CID_PRIVATE_EFFECT,
        .index       = 1,
        .name        = "mono",
    }, {
        .id          = V4L2_CID_PRIVATE_EFFECT,
        .index       = 2,
        .name        = "sepia",
    },  {
        .id          = V4L2_CID_PRIVATE_EFFECT,
        .index       = 3,
        .name        = "negative",
    }, {
        .id          = V4L2_CID_PRIVATE_EFFECT,
        .index       = 4,
        .name        = "aqua",
    },
};


/*
 * General functions
 */
static struct gc0328_priv *to_gc0328(const struct i2c_client *client)
{
    return container_of(i2c_get_clientdata(client), struct gc0328_priv,
                subdev);
}

static int gc0328_write_array(struct i2c_client *client,
                  const struct regval_list *vals)
{
    int ret;

    while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
        ret = i2c_smbus_write_byte_data(client,
                        vals->reg_num, vals->value);
        dev_vdbg(&client->dev, "array: 0x%02x, 0x%02x",
             vals->reg_num, vals->value);
        if (ret < 0)
            return ret;

        vals++;
    }

    return 0;
}

static int gc0328_mask_set(struct i2c_client *client,
               u8  reg, u8  mask, u8  set)
{
    s32 val = i2c_smbus_read_byte_data(client, reg);
    if (val < 0)
        return val;

    val &= ~mask;
    val |= set & mask;

    dev_vdbg(&client->dev, "masks: 0x%02x, 0x%02x", reg, val);

    return i2c_smbus_write_byte_data(client, reg, val);
}


/*
 * soc_camera_ops functions
 */
static int gc0328_s_stream(struct v4l2_subdev *sd, int enable)
{
    printk("Stream on!\r\n");
    return 0;
}


static int gc0328_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0328_priv *priv = to_gc0328(client);

    switch (ctrl->id) {
    case V4L2_CID_VFLIP:
        ctrl->value = priv->flag_vflip;
        break;
    case V4L2_CID_HFLIP:
        ctrl->value = priv->flag_hflip;
        break;
    case V4L2_CID_PRIVATE_BALANCE:
        ctrl->value = priv->balance_value;
        break;
    case V4L2_CID_PRIVATE_EFFECT:
        ctrl->value = priv->effect_value;
        break;
    default:
        break;
    }
    return 0;
}

static int gc0328_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0328_priv *priv = to_gc0328(client);
    int ret = 0;
    int i = 0;
    u8 value;

    int balance_count = ARRAY_SIZE(gc0328_balance);
    int effect_count = ARRAY_SIZE(gc0328_effect);
    printk("Monk %s %s",__FILE__, __FUNCTION__);
    switch (ctrl->id) {
    case V4L2_CID_PRIVATE_BALANCE:
        if (ctrl->value > balance_count)
            return -EINVAL;

        for(i = 0; i < balance_count; i++) {
            if (ctrl->value == gc0328_balance[i].index) {
                ret = gc0328_write_array(client,
                        gc0328_balance[ctrl->value].mode_regs);
                priv->balance_value = ctrl->value;
                break;
            }
        }
        break;

    case V4L2_CID_PRIVATE_EFFECT:
        if (ctrl->value > effect_count)
            return -EINVAL;

        for(i = 0; i < effect_count; i++) {
            if (ctrl->value == gc0328_effect[i].index) {
                ret = gc0328_write_array(client,
                        gc0328_effect[ctrl->value].mode_regs);
                priv->effect_value = ctrl->value;
                break;
            }
        }
        break;

    case V4L2_CID_VFLIP:
        value = ctrl->value ? REG14_VFLIP_IMG : 0x00;
        priv->flag_vflip = ctrl->value ? 1 : 0;
        ret = gc0328_mask_set(client, REG14, REG14_VFLIP_IMG, value);
        break;

    case V4L2_CID_HFLIP:
        value = ctrl->value ? REG14_HFLIP_IMG : 0x00;
        priv->flag_hflip = ctrl->value ? 1 : 0;
        ret = gc0328_mask_set(client, REG14, REG14_HFLIP_IMG, value);
        break;

    default:
        dev_err(&client->dev, "no V4L2 CID: 0x%x ", ctrl->id);
        return -EINVAL;
    }

    return ret;
}

static int gc0328_g_chip_ident(struct v4l2_subdev *sd,
                   struct v4l2_dbg_chip_ident *id)
{
    id->ident    = SUPPORT_HIGH_RESOLUTION_PRE;
    id->revision = 0;

    return 0;
}

static int gc0328_querymenu(struct v4l2_subdev *sd,
                    struct v4l2_querymenu *qm)
{
    switch (qm->id) {
    case V4L2_CID_PRIVATE_BALANCE:
        memcpy(qm->name, gc0328_balance_menus[qm->index].name,
                sizeof(qm->name));
        break;

    case V4L2_CID_PRIVATE_EFFECT:
        memcpy(qm->name, gc0328_effect_menus[qm->index].name,
                sizeof(qm->name));
        break;
    }

    return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int gc0328_g_register(struct v4l2_subdev *sd,
                 struct v4l2_dbg_register *reg)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret;

    reg->size = 1;
    if (reg->reg > 0xff)
        return -EINVAL;

    ret = i2c_smbus_read_byte_data(client, reg->reg);
    if (ret < 0)
        return ret;

    reg->val = ret;

    return 0;
}

static int gc0328_s_register(struct v4l2_subdev *sd,
                 struct v4l2_dbg_register *reg)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    if (reg->reg > 0xff ||
        reg->val > 0xff)
        return -EINVAL;

    return i2c_smbus_write_byte_data(client, reg->reg, reg->val);
}
#endif

/* Select the nearest higher resolution for capture */
static const struct gc0328_win_size *gc0328_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(gc0328_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(gc0328_supported_win_sizes); i++) {
        if (gc0328_supported_win_sizes[i].width  >= *width &&
            gc0328_supported_win_sizes[i].height >= *height) {
            *width  = gc0328_supported_win_sizes[i].width;
            *height = gc0328_supported_win_sizes[i].height;
            return &gc0328_supported_win_sizes[i];
        }
    }

    *width  = gc0328_supported_win_sizes[default_size].width;
    *height = gc0328_supported_win_sizes[default_size].height;
    return &gc0328_supported_win_sizes[default_size];
}

static int gc0328_init(struct v4l2_subdev *sd, u32 val)
{
    int ret;
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0328_priv *priv   = to_gc0328(client);

    int bala_index = priv->balance_value;
    int effe_index = priv->effect_value;

    /* initialize the sensor with default data */
    ret = gc0328_write_array(client, gc0328_init_regs);
    ret = gc0328_write_array(client, gc0328_balance[bala_index].mode_regs);
    ret = gc0328_write_array(client, gc0328_effect[effe_index].mode_regs);
    if (ret < 0)
        goto err;

    printk("bala index %d  effe index %d",bala_index, effe_index);

    dev_info(&client->dev, "%s: Init default", __func__);
    return 0;

err:
    dev_err(&client->dev, "%s: Error %d", __func__, ret);
    return ret;
}

static int gc0328_g_fmt(struct v4l2_subdev *sd,
            struct v4l2_mbus_framefmt *mf)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0328_priv *priv = to_gc0328(client);
    printk("Monk get fmt get win: %dx%d\r\n", priv->win->width,priv->win->height);

    mf->width      = priv->win->width;
    mf->height     = priv->win->height;
    mf->code       = priv->cfmt_code;

    mf->colorspace = V4L2_COLORSPACE_JPEG;
    mf->field      = V4L2_FIELD_NONE;

    return 0;
}

static int gc0328_s_fmt(struct v4l2_subdev *sd,
            struct v4l2_mbus_framefmt *mf)
{
    /* current do not support set format, use unify format yuv422i */
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0328_priv *priv = to_gc0328(client);
    int ret;
    gc0328_init(sd, 1);

    printk("Monk s fmt set win: %dx%d\r\n",mf->width,mf->height);
    priv->win = gc0328_select_win(&mf->width, &mf->height);
    /* set size win */
    ret = gc0328_write_array(client, priv->win->regs);
    if (ret < 0) {
        dev_err(&client->dev, "%s: Error\n", __func__);
        return ret;
    }

    return 0;
}

static int gc0328_try_fmt(struct v4l2_subdev *sd,
              struct v4l2_mbus_framefmt *mf)
{
    const struct gc0328_win_size *win;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /*
     * select suitable win
     */
    printk("Monk set win: %dx%d\r\n",mf->width,mf->height);
    win = gc0328_select_win(&mf->width, &mf->height);

    if (mf->field == V4L2_FIELD_ANY) {
        mf->field = V4L2_FIELD_NONE;
    } else if (mf->field != V4L2_FIELD_NONE) {
        dev_err(&client->dev, "Field type invalid.\n");
        return -ENODEV;
    }

    switch (mf->code) {
    case V4L2_MBUS_FMT_YUYV8_2X8:
    case V4L2_MBUS_FMT_YUYV8_1_5X8:
    case V4L2_MBUS_FMT_JZYUYV8_1_5X8:
        mf->colorspace = V4L2_COLORSPACE_JPEG;
        break;

    default:
        mf->code = V4L2_MBUS_FMT_YUYV8_2X8;
        break;
    }

    return 0;
}

static int gc0328_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
               enum v4l2_mbus_pixelcode *code)
{
    if (index >= ARRAY_SIZE(gc0328_codes))
        return -EINVAL;

    *code = gc0328_codes[index];
    return 0;
}

static int gc0328_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
    return 0;
}

static int gc0328_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
    return 0;
}


/*
 * Frame intervals.  Since frame rates are controlled with the clock
 * divider, we can only do 30/n for integer n values.  So no continuous
 * or stepwise options.  Here we just pick a handful of logical values.
 */

static int gc0328_frame_rates[] = { 30, 15, 10, 5, 1 };

static int gc0328_enum_frameintervals(struct v4l2_subdev *sd,
        struct v4l2_frmivalenum *interval)
{
    if (interval->index >= ARRAY_SIZE(gc0328_frame_rates))
        return -EINVAL;
    interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;
    interval->discrete.numerator = 1;
    interval->discrete.denominator = gc0328_frame_rates[interval->index];
    return 0;
}


/*
 * Frame size enumeration
 */
static int gc0328_enum_framesizes(struct v4l2_subdev *sd,
        struct v4l2_frmsizeenum *fsize)
{
    int i;
    int num_valid = -1;
    __u32 index = fsize->index;

    for (i = 0; i < N_WIN_SIZES; i++) {
        const struct gc0328_win_size *win = &gc0328_supported_win_sizes[index];
        if (index == ++num_valid) {
            fsize->type            = V4L2_FRMSIZE_TYPE_DISCRETE;
            fsize->discrete.width  = win->width;
            fsize->discrete.height = win->height;
            return 0;
        }
    }

    return -EINVAL;
}

static int gc0328_video_probe(struct i2c_client *client)
{
    unsigned char retval = 0, retval_high = 0, retval_low = 0;
    struct v4l2_subdev *subdev = i2c_get_clientdata(client);
    int ret = 0;

    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);


    ret = soc_camera_power_on(&client->dev, ssdd);
    if (ret < 0)
        return ret;

    /*
     * check and show product ID and manufacturer ID
     */

    retval_high = gc0328_read_reg(client, REG_CHIP_ID);
    if (retval_high != PID_GC0328) {
        dev_err(&client->dev, "read sensor %s chip_id high %x is error\n",
                client->name, retval_high);
        return -1;
    }

    dev_info(&client->dev, "read sensor %s id high:0x%x,low:%x successed!\n",
            client->name, retval_high, retval_low);

    ret = soc_camera_power_off(&client->dev, ssdd);

    return 0;
}

static int gc0328_s_power(struct v4l2_subdev *sd, int on)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
    struct gc0328_priv *priv = to_gc0328(client);

    return soc_camera_set_power(&client->dev, ssdd, on);
}

static int gc0328_g_mbus_config(struct v4l2_subdev *sd,
        struct v4l2_mbus_config *cfg)
{

    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    cfg->flags = V4L2_MBUS_PCLK_SAMPLE_FALLING | V4L2_MBUS_MASTER |
            V4L2_MBUS_VSYNC_ACTIVE_LOW | V4L2_MBUS_HSYNC_ACTIVE_HIGH |
                V4L2_MBUS_DATA_ACTIVE_HIGH;
    cfg->type  = V4L2_MBUS_PARALLEL;

    cfg->flags = soc_camera_apply_board_flags(ssdd, cfg);

    return 0;
}


static struct v4l2_subdev_core_ops gc0328_subdev_core_ops = {
//    .init        = gc0328_init,
    .s_power      = gc0328_s_power,
    .g_ctrl       = gc0328_g_ctrl,
    .s_ctrl       = gc0328_s_ctrl,
    .g_chip_ident = gc0328_g_chip_ident,
    .querymenu    = gc0328_querymenu,
#ifdef CONFIG_VIDEO_ADV_DEBUG
    .g_register   = gc0328_g_register,
    .s_register   = gc0328_s_register,
#endif
};

static struct v4l2_subdev_video_ops gc0328_subdev_video_ops = {
    .s_stream      = gc0328_s_stream,
    .g_mbus_fmt    = gc0328_g_fmt,
    .s_mbus_fmt    = gc0328_s_fmt,
    .try_mbus_fmt  = gc0328_try_fmt,
    .cropcap       = gc0328_cropcap,
    .g_crop        = gc0328_g_crop,
    .enum_mbus_fmt = gc0328_enum_fmt,
    .g_mbus_config = gc0328_g_mbus_config,
#if 0
    .enum_framesizes = gc0328_enum_framesizes,
    .enum_frameintervals = gc0328_enum_frameintervals,
#endif
};

static struct v4l2_subdev_ops gc0328_subdev_ops = {
    .core     = &gc0328_subdev_core_ops,
    .video    = &gc0328_subdev_video_ops,
};

/*
 * i2c_driver functions
 */

static int gc0328_probe(struct i2c_client *client,
            const struct i2c_device_id *did)
{
    struct gc0328_priv *priv;
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
    struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
    int ret = 0,default_wight = 640,default_height = 480;

    printk("GC0328: %s %s\r\n",__FILE__,__FUNCTION__);

    if (!ssdd) {
        dev_err(&client->dev, "gc0328: missing platform data!\n");
        return -EINVAL;
    }

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE
            | I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&client->dev, "    \n");
        return -ENODEV;
    }

    priv = kzalloc(sizeof(struct gc0328_priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&adapter->dev,
            "Failed to allocate memory for private data!\n");
        return -ENOMEM;
    }


    v4l2_i2c_subdev_init(&priv->subdev, client, &gc0328_subdev_ops);

    priv->win = gc0328_select_win(&default_wight, &default_height);

    priv->cfmt_code  =  V4L2_MBUS_FMT_YUYV8_2X8;

    ret = gc0328_video_probe(client);
    if (ret) {
        kfree(priv);
    }
    return ret;
}

static int gc0328_remove(struct i2c_client *client)
{
    struct gc0328_priv *priv = to_gc0328(client);

    kfree(priv);
    return 0;
}

static const struct i2c_device_id gc0328_id[] = {
    { "gc0328",  0 },
    { }
};


MODULE_DEVICE_TABLE(i2c, gc0328_id);

static struct i2c_driver gc0328_i2c_driver = {
    .driver = {
        .name = "gc0328",
    },
    .probe    = gc0328_probe,
    .remove   = gc0328_remove,
    .id_table = gc0328_id,
};

/*
 * Module functions
 */
static int __init gc0328_module_init(void)
{
    return i2c_add_driver(&gc0328_i2c_driver);
}

static void __exit gc0328_module_exit(void)
{
    i2c_del_driver(&gc0328_i2c_driver);
}

module_init(gc0328_module_init);
module_exit(gc0328_module_exit);

MODULE_DESCRIPTION("camera sensor gc0328 driver");
MODULE_AUTHOR("qipengzhen <aric.pzqi@ingenic.com>");
MODULE_LICENSE("GPL");

