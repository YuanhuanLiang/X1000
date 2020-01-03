/*
 * gc0328db Camera Driver
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
#include <linux/mutex.h>
#include <linux/delay.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-subdev.h>
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>
#include "double_camera.h"


#define DEFAULT_WIDTH                       640
#define DEFAULT_HEIGHT                      480

#define CHIP_ID_GC0328                      0x9d

#define REG_BIT_AUTO_AWB                    (1<<1)

#define REG_HFLIP_IMG                       0x2 /* Horizontal mirror image reg bit */
#define REG_VFLIP_IMG                       0x1 /* Vertical flip image reg bit */

#define REG_CHIP_ID                         0xf0
#define REG_PAGE                            0xfe



#define double_channel_do(ret1, ret2, pch, f, args...)   \
        do {                                             \
            int di;                                      \
            for (di=0; di<2; di++) {                     \
                (!di)?(ret1=f(args)):(ret2=f(args));     \
                switch_channel(pch);                     \
            }                                            \
        } while(0)                                       \

/*
 * Struct
 */
struct regval_list {
    unsigned char reg_num;
    unsigned char value;
};


/* Supported resolutions */
enum gc0328db_width {
    W_QCIF   = 176,
    W_QVGA   = 320,
    W_CIF    = 352,
    W_VGA    = 640,
};

enum gc0328db_height {
    H_QCIF   = 144,
    H_QVGA   = 240,
    H_CIF    = 288,
    H_VGA    = 480,
};

struct gc0328db_win_size {
    char *name;
    enum gc0328db_width width;
    enum gc0328db_height height;
    const struct regval_list *regs;
};

struct gc0328db_priv {
    struct mutex reg_lock;
    struct mutex cfg_lock;

    unsigned char power_en;
    unsigned int  exposure;
    unsigned char exposure_mode;
    unsigned int  w_balance;
    unsigned char w_balance_mode;
    unsigned char flag_vflip:1;
    unsigned char flag_hflip:1;
    enum v4l2_mbus_pixelcode cfmt_code;
    struct v4l2_mbus_framefmt mf;
    const struct gc0328db_win_size *win;

    struct double_camera_op db_camera_op;
    struct v4l2_ctrl_handler ctrl_handler;

    struct workqueue_struct* gc0328db_work_queue;
    struct work_struct switch_channel_work;
    struct work_struct resume_work;
};


static inline int gc0328db_write_reg(struct i2c_client * client, unsigned char addr, unsigned char value)
{
    return i2c_smbus_write_byte_data(client, addr, value);
}
static inline char gc0328db_read_reg(struct i2c_client *client, unsigned char addr)
{
    return i2c_smbus_read_byte_data(client, addr);
}
/*
 * Registers settings
 */

#define ENDMARKER                       {0xff, 0xff}

static const struct regval_list gc0328db_sensor_init_regs[] = {
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
    {0x17 , 0x14}, //mirror
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
    {0xf1 , 0x00},
    {0xf2 , 0x00},
    ENDMARKER,
};


static const struct regval_list gc0328db_qcif_regs[] = {
    {0xfe , 0x00},
    {0x50 , 0x01}, {0x51 , 0x00},
    {0x52 , 0x00}, {0x53 , 0x00},
    {0x54 , 0x00}, {0x55 , 0x00},
    {0x56 , 0x90}, {0x57 , 0x00},
    {0x58 , 0xb0},

    {0x5a , 0x00}, {0x59 , 0xaa},
    {0x5b , 0x02}, {0x5c , 0x04},
    {0x5d , 0x00}, {0x5e , 0x00},
    {0x5f , 0x02}, {0x60 , 0x04},
    {0x61 , 0x00}, {0x62 , 0x00},
    ENDMARKER,
};

static const struct regval_list gc0328db_qvga_regs[] = {
    {0xfe , 0x00},
    {0x50 , 0x01}, {0x51 , 0x00},
    {0x52 , 0x00}, {0x53 , 0x00},
    {0x54 , 0x00}, {0x55 , 0x00},
    {0x56 , 0xf0}, {0x57 , 0x01},
    {0x58 , 0x40},

    {0x5a , 0x00}, {0x59 , 0x22},
    {0x5b , 0x00}, {0x5c , 0x00},
    {0x5d , 0x00}, {0x5e , 0x00},
    {0x5f , 0x00}, {0x60 , 0x00},
    {0x61 , 0x00}, {0x62 , 0x00},
    ENDMARKER,
};

static const struct regval_list gc0328db_cif_regs[] = {
    {0xfe , 0x00},
    {0x50 , 0x01}, {0x51 , 0x00},
    {0x52 , 0x00}, {0x53 , 0x00},
    {0x54 , 0x00}, {0x55 , 0x01},
    {0x56 , 0x20}, {0x57 , 0x01},
    {0x58 , 0x60},

    {0x5a , 0x00}, {0x59 , 0x55},
    {0x5b , 0x02}, {0x5c , 0x04},
    {0x5d , 0x00}, {0x5e , 0x00},
    {0x5f , 0x02}, {0x60 , 0x04},
    {0x61 , 0x00}, {0x62 , 0x00},
    ENDMARKER,
};

static const struct regval_list gc0328db_vga_regs[] = {
    ENDMARKER,
};

static struct regval_list gc0328db_enable_auto_wb_regs[] = {
    {0xfe, 0x00},
    {0x42, 0x00},
    ENDMARKER,
};

static const struct regval_list gc0328db_wb_incandescence_regs[] = {
    {0xfe, 0x00},
    {0x77, 0x48}, {0x78, 0x40},
    {0x79, 0x5c},
    ENDMARKER,
};

static const struct regval_list gc0328db_wb_daylight_regs[] = {
    {0xfe, 0x00},
    {0x77, 0x74}, {0x78, 0x52},
    {0x79, 0x40},
    ENDMARKER,
};

static const struct regval_list gc0328db_wb_fluorescent_regs[] = {
    {0xfe, 0x00},
    {0x77, 0x40}, {0x78, 0x42},
    {0x79, 0x50},
    ENDMARKER,
};

static const struct regval_list gc0328db_wb_cloud_regs[] = {
    {0xfe, 0x00},
    {0x77, 0x8c}, {0x78, 0x50},
    {0x79, 0x40},
    ENDMARKER,
};

static struct regval_list gc0328db_set_white_balance_regs[] = {
    {0xfe, 0x00},
    {0x77, 0x00}, {0x78, 0x00},
    {0x79, 0x00},
    ENDMARKER,
};


#define GC0328DB_SIZE(n, w, h, r) \
    {.name = n, .width = w , .height = h, .regs = r }

static const struct gc0328db_win_size gc0328db_supported_win_sizes[] = {
    GC0328DB_SIZE("QCIF", W_QCIF, H_QCIF, gc0328db_qcif_regs),
    GC0328DB_SIZE("QVGA", W_QVGA, H_QVGA, gc0328db_qvga_regs),
    GC0328DB_SIZE("CIF", W_CIF, H_CIF, gc0328db_cif_regs),
    GC0328DB_SIZE("VGA", W_VGA, H_VGA, gc0328db_vga_regs),
};

static const struct regval_list gc0328db_enable_output_regs[] = {
    {0xfe, 0x00},
    {0xf2, 0x01},
    {0xf1, 0x07},
    ENDMARKER,
};

static const struct regval_list gc0328db_disable_output_regs[] = {
    {0xfe, 0x00},
    {0xf1, 0x00},
    {0xf2, 0x00},
    ENDMARKER,
};

static struct regval_list gc0328db_mirror_h_regs[] = {
    {0xfe, 0x00},
    {0x17, 0x00},
    ENDMARKER,
};

static struct regval_list gc0328db_mirror_v_regs[] = {
    {0xfe, 0x00},
    {0x17, 0x00},
    ENDMARKER,
};

static struct regval_list gc0328db_chip_id_regs[] = {
    {0xfe, 0x00},
    {0xf0, 0x00},
    ENDMARKER,
};

static struct regval_list gc0328db_enable_auto_exposure_regs[] = {
    {0xfe, 0x00},
    {0x4f, 0x01},
    ENDMARKER,
};

static const struct regval_list gc0328db_disable_auto_exposure_regs[] = {
    {0xfe, 0x00},
    {0x4f, 0x00},
    ENDMARKER,
};

static struct regval_list gc0328db_set_exposure_time_regs[] = {
    {0xfe, 0x00},
    {0x03, 0x00},
    {0x04, 0x00},
    ENDMARKER,
};

static enum v4l2_mbus_pixelcode gc0328db_codes[] = {
    V4L2_MBUS_FMT_Y8_1X8,
    V4L2_MBUS_FMT_YUYV8_2X8,
    V4L2_MBUS_FMT_YUYV8_1_5X8,
    V4L2_MBUS_FMT_JZYUYV8_1_5X8,
};



/*
 * General functions
 */

static struct gc0328db_priv *to_gc0328db(const struct i2c_client *client)
{
    struct double_camera_op* dbc = container_of(i2c_get_clientdata(client),
                            struct double_camera_op, subdev);

    return container_of(dbc, struct gc0328db_priv, db_camera_op);
}

static int gc0328db_write_array(struct i2c_client *client,
                                const struct regval_list *vals)
{
    int ret;
    struct gc0328db_priv *priv = to_gc0328db(client);

    mutex_lock(&priv->reg_lock);
    while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
        dev_vdbg(&client->dev, "write array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
        ret = gc0328db_write_reg(client, vals->reg_num, vals->value);
        if (ret < 0) {
            mutex_unlock(&priv->reg_lock);
            return ret;
        }
        vals++;
    }
    mutex_unlock(&priv->reg_lock);
    return 0;
}

static int gc0328db_read_array(struct i2c_client  *client,
                               struct regval_list *vals)
{
    int ret;
    struct gc0328db_priv *priv = to_gc0328db(client);

    mutex_lock(&priv->reg_lock);
    while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
        if (vals->reg_num == REG_PAGE) {
            dev_vdbg(&client->dev, "write array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
            ret = gc0328db_write_reg(client,vals->reg_num, vals->value);
            if (ret < 0) {
                mutex_unlock(&priv->reg_lock);
                return ret;
            }
        } else {
            dev_vdbg(&client->dev, "read array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
            vals->value = gc0328db_read_reg(client,vals->reg_num);
        }
        vals++;
    }
    mutex_unlock(&priv->reg_lock);
    return 0;
}



/*
 * double camera functions
 */
extern int pca9543_switch_i2c_channel(char channel);

static void switch_channel(unsigned char* ch)
{
    *ch = (*ch+1)%2;
    pca9543_switch_i2c_channel(*ch);
}

static void switch_channel_handle(struct work_struct *data)
{
    struct gc0328db_priv* priv =
                    container_of(data, struct gc0328db_priv, switch_channel_work);
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);
    /*
     1, disable current sensor output
     2, switch another channel
     3, enable sensor output
     */
    mutex_lock(&priv->cfg_lock);
    gc0328db_write_array(client, gc0328db_disable_output_regs);
    switch_channel(&priv->db_camera_op.cur_channel);
    gc0328db_write_array(client, gc0328db_enable_output_regs);
    mutex_unlock(&priv->cfg_lock);
}

static void gc0328db_stream_on(struct gc0328db_priv* priv)
{
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);

    gc0328db_write_array(client, gc0328db_enable_output_regs);
}

static void gc0328db_stream_off(struct gc0328db_priv* priv)
{
    int ret1,ret2;
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);

    double_channel_do(ret1, ret2, &priv->db_camera_op.cur_channel,
                    gc0328db_write_array, client, gc0328db_disable_output_regs);
}


static void gc0328db_switch_work_channel(struct double_camera_op* dbc)
{
    struct gc0328db_priv* priv = container_of(dbc,
                            struct gc0328db_priv, db_camera_op);

    schedule_work(&priv->switch_channel_work);
}


/*
 * soc_camera_ops functions
 */
static int gc0328db_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0328db_priv *priv = to_gc0328db(client);

    if (enable) {
        gc0328db_stream_off(priv);
        gc0328db_stream_on(priv);
    } else {
        gc0328db_stream_off(priv);
    }

    return 0;
}

static int enable_auto_white_balance(struct i2c_client *client, unsigned char en)
{
    int ret;

    ret = gc0328db_read_array(client, gc0328db_enable_auto_wb_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read awb regs\n");
        return -1;
    }

    gc0328db_enable_auto_wb_regs[1].value = en ? gc0328db_enable_auto_wb_regs[1].value | REG_BIT_AUTO_AWB
                                               : gc0328db_enable_auto_wb_regs[1].value &~REG_BIT_AUTO_AWB;
    ret = gc0328db_write_array(client, gc0328db_enable_auto_wb_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write awb regs\n");
        return -1;
    }

    return 0;
}

static int set_white_balance_mode(struct i2c_client *client, const struct regval_list *rgls)
{
    int ret;
    struct gc0328db_priv *priv = to_gc0328db(client);

    if (priv->w_balance_mode == V4L2_WHITE_BALANCE_AUTO) {
        ret = enable_auto_white_balance(client, 0);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to en/diable awb\n");
            return -1;
        }
    }

    ret = gc0328db_write_array(client, rgls);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write awb regs\n");
        return -1;
    }

    return 0;
}

static int s_ctrl_auto_white_balance_handle(struct i2c_client *client, int mode)
{
    int ret;
    struct gc0328db_priv *priv = to_gc0328db(client);

    switch(mode) {
    case V4L2_WHITE_BALANCE_AUTO:
        ret = enable_auto_white_balance(client, 1);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to en/diable awb\n");
            return -1;
        }
    break;

    case V4L2_WHITE_BALANCE_MANUAL:
        if (priv->w_balance_mode == V4L2_WHITE_BALANCE_AUTO) {
            ret = enable_auto_white_balance(client, 0);
            if (ret < 0) {
                dev_err(&client->dev,"Failed to en/diable awb\n");
                return -1;
            }
        }
    break;

    case V4L2_WHITE_BALANCE_INCANDESCENT:
        ret = set_white_balance_mode(client, gc0328db_wb_incandescence_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write awb regs\n");
            return -1;
        }
    break;

    case V4L2_WHITE_BALANCE_DAYLIGHT:
        ret = set_white_balance_mode(client, gc0328db_wb_daylight_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write awb regs\n");
            return -1;
        }
    break;

    case V4L2_WHITE_BALANCE_FLUORESCENT:
        ret = set_white_balance_mode(client, gc0328db_wb_fluorescent_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write awb regs\n");
            return -1;
        }
    break;

    case V4L2_WHITE_BALANCE_CLOUDY:
        ret = set_white_balance_mode(client, gc0328db_wb_cloud_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write awb regs\n");
            return -1;
        }
    break;

    case V4L2_WHITE_BALANCE_FLUORESCENT_H:
    case V4L2_WHITE_BALANCE_HORIZON:
    case V4L2_WHITE_BALANCE_FLASH:
    case V4L2_WHITE_BALANCE_SHADE:
        dev_err(&client->dev, "Unsupport while balance cid : %x\n",mode);
        return -1;
    break;

    default:
        dev_err(&client->dev, "Unknow while balance cid : %x\n",mode);
        return -1;
    break;
    }

    return 0;
}

static int s_ctrl_white_balance_val_handle(struct i2c_client *client, int value)
{
    int ret;
    struct gc0328db_priv *priv = to_gc0328db(client);
    int w_balance = value & 0x00ffffff;

    if (priv->w_balance_mode != V4L2_WHITE_BALANCE_MANUAL) {
        dev_err(&client->dev,"Not in manual mode.\n");
        return -1;
    }

    gc0328db_set_white_balance_regs[1].value = (unsigned char)(0x000000ff & (w_balance>>16));
    gc0328db_set_white_balance_regs[2].value = (unsigned char)(0x000000ff & (w_balance>>8));
    gc0328db_set_white_balance_regs[3].value = (unsigned char)(0x000000ff &  w_balance);

    ret = gc0328db_write_array(client, gc0328db_set_white_balance_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write white_balance regs\n");
        return -1;
    }

    return 0;
}

static int s_ctrl_exposure_handle(struct i2c_client *client, int value)
{
    int ret;
    struct gc0328db_priv *priv = to_gc0328db(client);
    int exposure = value & 0xfff;

    if (priv->exposure_mode != V4L2_EXPOSURE_MANUAL){
        dev_err(&client->dev,"Not in manual mode.\n");
        return -1;
    }

    gc0328db_set_exposure_time_regs[1].value = (unsigned char)(0x0f   & (exposure>>8));
    gc0328db_set_exposure_time_regs[2].value = (unsigned char)(0x00ff & exposure);

    ret = gc0328db_write_array(client, gc0328db_set_exposure_time_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write set_exposure_time regs\n");
        return -1;
    }

    return 0;
}

static int s_ctrl_auto_exposure_handle(struct i2c_client *client, int mode)
{
    int ret;

    switch(mode) {
    case V4L2_EXPOSURE_AUTO:
        ret = gc0328db_write_array(client, gc0328db_enable_auto_exposure_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write enable_auto_exposure regs\n");
            return -1;
        }
        break;

    case V4L2_EXPOSURE_MANUAL:
        ret = gc0328db_write_array(client, gc0328db_disable_auto_exposure_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write disable_auto_exposure regs\n");
            return -1;
        }
        break;

    case V4L2_EXPOSURE_SHUTTER_PRIORITY:
    case V4L2_EXPOSURE_APERTURE_PRIORITY:
        dev_err(&client->dev,"Unsupport exposure cid: %d\n",mode);
        return -1;
    default:
        dev_err(&client->dev,"Unknow exposure cid: %d\n",mode);
        return -1;
    }
    return 0;
}

static int s_ctrl_mirror_vflip(struct i2c_client *client, int value)
{
    int ret,val = 0;

    val = value ? REG_VFLIP_IMG : 0;
    ret = gc0328db_read_array(client, gc0328db_mirror_v_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read mirror regs\n");
        return -1;
    }
    gc0328db_mirror_v_regs[1].value &= ~REG_VFLIP_IMG;
    gc0328db_mirror_v_regs[1].value |= val;
    ret = gc0328db_write_array(client,gc0328db_mirror_v_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write mirror regs\n");
        return -1;
    }
    return 0;
}

static int s_ctrl_mirror_hflip(struct i2c_client *client, int value)
{
    int ret,val = 0;

    val = value ? REG_HFLIP_IMG : 0;
    ret = gc0328db_read_array(client,gc0328db_mirror_h_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read mirror regs\n");
        return -1;
    }
    gc0328db_mirror_h_regs[1].value &= ~REG_HFLIP_IMG;
    gc0328db_mirror_h_regs[1].value |= val;
    ret = gc0328db_write_array(client,gc0328db_mirror_h_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write mirror regs\n");
        return -1;
    }
    return 0;
}

static void set_ctrl_mode(struct gc0328db_priv *priv, unsigned int cid, unsigned int mode)
{
    switch (cid) {
    case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
        priv->w_balance_mode = mode;
        break;
    case V4L2_CID_DO_WHITE_BALANCE:
        priv->w_balance = mode;
        break;
    case V4L2_CID_EXPOSURE_AUTO:
        priv->exposure_mode = mode;
        break;
    case V4L2_CID_EXPOSURE:
        priv->exposure = mode;
        break;
    case V4L2_CID_VFLIP:
        priv->flag_vflip = mode ? 1 : 0;
        break;
    case V4L2_CID_HFLIP:
        priv->flag_hflip = mode ? 1 : 0;
        break;
    default:
        break;
    }
}

static int gc0328db_s_ctrl_do(struct i2c_client *client, struct v4l2_control *ctrl)
{
    int ret = 0;

    switch (ctrl->id) {
    case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
        ret = s_ctrl_auto_white_balance_handle(client,ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set white balance\n");
        }
        break;

    case V4L2_CID_DO_WHITE_BALANCE:
        ret = s_ctrl_white_balance_val_handle(client,ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set white balance\n");
        }
        break;

    case V4L2_CID_EXPOSURE_AUTO:
        ret = s_ctrl_auto_exposure_handle(client, ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set exposure mode.\n");
        }
        break;

    case V4L2_CID_EXPOSURE:
        ret = s_ctrl_exposure_handle(client,ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set exposure\n");
        }
        break;

    case V4L2_CID_VFLIP:
        ret = s_ctrl_mirror_vflip(client, ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set h mirror.\n");
        }
        break;

    case V4L2_CID_HFLIP:
        ret = s_ctrl_mirror_hflip(client, ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set h mirror.\n");
        }
        break;

    default:
        dev_err(&client->dev, "no V4L2 CID: 0x%x ", ctrl->id);
        return -EINVAL;
    }

    return ret;
}




static int gc0328db_handler_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret1, ret2;
    struct v4l2_control control;
    struct gc0328db_priv *priv   = container_of(ctrl->handler, struct gc0328db_priv, ctrl_handler);
    struct v4l2_subdev *sd       = &priv->db_camera_op.subdev;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct double_camera_op* dbc = container_of(sd, struct double_camera_op, subdev);

    dev_info(&client->dev, "Set ctrl\n");

    mutex_lock(&priv->cfg_lock);
    control.id    = ctrl->id;
    control.value = ctrl->val;
    double_channel_do(ret1, ret2, &dbc->cur_channel, gc0328db_s_ctrl_do, client, &control);
    if (ret1 != 0 || ret2 != 0) {
        mutex_unlock(&priv->cfg_lock);
        return -1;
    }
    set_ctrl_mode(priv, control.id, control.value);
    mutex_unlock(&priv->cfg_lock);
    return 0;
}

static int gc0328db_g_volatile_ctrl_do(struct i2c_client *client, struct v4l2_control *ctrl)
{
    int ret = 0;

    switch (ctrl->id) {
    default:
        dev_err(&client->dev, "Has no V4L2 CID: 0x%x ", ctrl->id);
        return -EINVAL;
    }

    return ret;
}

static int gc0328db_handler_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret;
    struct gc0328db_priv *priv   = container_of(ctrl->handler, struct gc0328db_priv, ctrl_handler);
    struct v4l2_subdev *sd       = &priv->db_camera_op.subdev;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct v4l2_control control;

    control.id = ctrl->id;

    ret = gc0328db_g_volatile_ctrl_do(client, &control);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to g ctrl\n");
        return -1;
    }
    ctrl->val = control.value;
    return 0;
}

static int gc0328db_g_chip_ident(struct v4l2_subdev *sd,
                         struct v4l2_dbg_chip_ident *id)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);

    dev_info(&client->dev, "Get chip ident 0x%x\n",CHIP_ID_GC0328);

    id->ident    = CHIP_ID_GC0328;
    id->revision = 0;

    return 0;
}



/* Select the nearest higher resolution for capture */
static const struct gc0328db_win_size *gc0328db_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(gc0328db_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(gc0328db_supported_win_sizes); i++) {
        if (gc0328db_supported_win_sizes[i].width  >= *width &&
            gc0328db_supported_win_sizes[i].height >= *height) {
            *width  = gc0328db_supported_win_sizes[i].width;
            *height = gc0328db_supported_win_sizes[i].height;
            return &gc0328db_supported_win_sizes[i];
        }
    }

    *width  = gc0328db_supported_win_sizes[default_size].width;
    *height = gc0328db_supported_win_sizes[default_size].height;
    return &gc0328db_supported_win_sizes[default_size];
}



static int gc0328db_g_fmt(struct v4l2_subdev *sd,
            struct v4l2_mbus_framefmt *mf)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0328db_priv *priv = to_gc0328db(client);

    dev_info(&client->dev, "Get win of fmt: %dx%d\n", priv->win->width,priv->win->height);

    mf->width      = priv->win->width;
    mf->height     = priv->win->height;
    mf->code       = priv->cfmt_code;

    mf->colorspace = V4L2_COLORSPACE_JPEG;
    mf->field      = V4L2_FIELD_NONE;

    return 0;
}


static int gc0328db_sensor_init_do(struct i2c_client* client)
{
    int ret;

    /* initialize the sensor with default data */
    ret = gc0328db_write_array(client, gc0328db_sensor_init_regs);
    if (ret < 0)
        return -1;

    return 0;
}

static int gc0328db_sensor_init(struct v4l2_subdev *sd)
{
    int ret1,ret2;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct gc0328db_priv *priv   = to_gc0328db(client);
    struct double_camera_op* dbc = container_of(sd, struct double_camera_op, subdev);

    dev_info(&client->dev, "Sensor init\n");

    mutex_lock(&priv->cfg_lock);
    double_channel_do(ret1, ret2, &dbc->cur_channel, gc0328db_sensor_init_do, client);
    if (ret1 != 0 || ret2 != 0) {
        mutex_unlock(&priv->cfg_lock);
        return -1;
    }
    mutex_unlock(&priv->cfg_lock);

    return 0;
}


static int gc0328db_s_fmt_do(struct v4l2_subdev *sd,
                      struct v4l2_mbus_framefmt *mf)
{
    /* current do not support set format, use unify format yuv422i */
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0328db_priv *priv = to_gc0328db(client);
    int ret;

    dev_info(&client->dev, "Set win of fmt : %dx%d\n",mf->width,mf->height);
    priv->win = gc0328db_select_win(&mf->width, &mf->height);
    /* set size win */
    ret = gc0328db_write_array(client, priv->win->regs);
    if (ret < 0) {
        dev_err(&client->dev, "%s: Error\n", __func__);
        return ret;
    }

    return 0;
}

static int gc0328db_s_fmt(struct v4l2_subdev *sd,
                   struct v4l2_mbus_framefmt *mf)
{
    int ret1,ret2;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct gc0328db_priv *priv   = to_gc0328db(client);
    struct double_camera_op* dbc = container_of(sd, struct double_camera_op, subdev);

    dev_info(&client->dev, "Set fmt\n");

    mutex_lock(&priv->cfg_lock);
    double_channel_do(ret1, ret2, &dbc->cur_channel, gc0328db_s_fmt_do, sd, mf);
    if (ret1 != 0 || ret2 != 0) {
        mutex_unlock(&priv->cfg_lock);
        return -1;
    }
    memcpy(&priv->mf, mf, sizeof(struct v4l2_mbus_framefmt));
    mutex_unlock(&priv->cfg_lock);

    return 0;
}


static int gc0328db_try_fmt(struct v4l2_subdev *sd,
                     struct v4l2_mbus_framefmt *mf)
{
    const struct gc0328db_win_size *win;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /*
     * select suitable win
     */
    win = gc0328db_select_win(&mf->width, &mf->height);

    if (mf->field == V4L2_FIELD_ANY) {
        mf->field = V4L2_FIELD_NONE;
    } else if (mf->field != V4L2_FIELD_NONE) {
        dev_err(&client->dev, "Field type invalid.\n");
        return -ENODEV;
    }

    switch (mf->code) {
    case V4L2_MBUS_FMT_Y8_1X8:
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

static int gc0328db_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
                                         enum v4l2_mbus_pixelcode *code)
{
    if (index >= ARRAY_SIZE(gc0328db_codes))
        return -EINVAL;

    *code = gc0328db_codes[index];
    return 0;
}


static char gc0328db_g_chip_id_do(struct i2c_client *client)
{
    int ret;

    ret = gc0328db_read_array(client, gc0328db_chip_id_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read chip id\n");
        return ret;
    }
    return gc0328db_chip_id_regs[1].value;
}

static void gc0328db_g_chip_id(char* id1, char* id2, struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    struct double_camera_op* dbc = container_of(sd, struct double_camera_op, subdev);

    dev_info(&client->dev, "Get chip id\n");

    double_channel_do(*id1, *id2, &dbc->cur_channel, gc0328db_g_chip_id_do, client);
}


static int gc0328db_s_power(struct v4l2_subdev *sd, int on)
{
    int ret;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct gc0328db_priv *priv = to_gc0328db(client);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
    struct v4l2_ctrl *ctrl;

    dev_info(&client->dev,"s_power %s\n",on?"on":"off");

    ret = soc_camera_set_power(&client->dev, ssdd, on);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to set power.\n");
        return -1;
    }

    if (on) {
        /* active i2c switch */
        priv->db_camera_op.cur_channel  = 1;
        switch_channel(&priv->db_camera_op.cur_channel);
        ret = gc0328db_sensor_init(sd);
        if (ret < 0) {
            dev_err(&client->dev, "Failed to init sensor.\n");
            soc_camera_set_power(&client->dev, ssdd, !on);
            return -1;
        }
    }

    if (!on) {
        ctrl = v4l2_ctrl_find(priv->db_camera_op.subdev.ctrl_handler,
                              V4L2_CID_EXPOSURE_AUTO);
        if (ctrl) {
            priv->exposure_mode = V4L2_EXPOSURE_AUTO;
            ctrl->cur.val       = V4L2_EXPOSURE_AUTO;
        }

        ctrl = v4l2_ctrl_find(priv->db_camera_op.subdev.ctrl_handler,
                              V4L2_CID_EXPOSURE);
        if (ctrl) {
            priv->exposure      = 0;
            ctrl->cur.val       = 0;
        }

        ctrl = v4l2_ctrl_find(priv->db_camera_op.subdev.ctrl_handler,
                              V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE);
        if (ctrl) {
            priv->w_balance_mode = V4L2_WHITE_BALANCE_AUTO;
            ctrl->cur.val        = V4L2_WHITE_BALANCE_AUTO;
        }

        ctrl = v4l2_ctrl_find(priv->db_camera_op.subdev.ctrl_handler,
                              V4L2_CID_DO_WHITE_BALANCE);
        if (ctrl) {
            priv->w_balance      = 0;
            ctrl->cur.val        = 0;
        }

        ctrl = v4l2_ctrl_find(priv->db_camera_op.subdev.ctrl_handler,
                              V4L2_CID_VFLIP);
        if (ctrl) {
            priv->flag_vflip     = 0;
            ctrl->cur.val        = 0;
        }

        ctrl = v4l2_ctrl_find(priv->db_camera_op.subdev.ctrl_handler,
                              V4L2_CID_HFLIP);
        if (ctrl) {
            priv->flag_hflip     = 0;
            ctrl->cur.val        = 0;
        }
    }
    priv->power_en = on;
    return 0;
}

static int gc0328db_g_mbus_config(struct v4l2_subdev *sd,
                            struct v4l2_mbus_config *cfg)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    cfg->flags = V4L2_MBUS_PCLK_SAMPLE_FALLING | V4L2_MBUS_MASTER |
            V4L2_MBUS_VSYNC_ACTIVE_LOW | V4L2_MBUS_HSYNC_ACTIVE_HIGH |
                V4L2_MBUS_DATA_ACTIVE_HIGH;
    cfg->type = V4L2_MBUS_PARALLEL;

    cfg->flags = soc_camera_apply_board_flags(ssdd, cfg);

    return 0;
}


static struct v4l2_subdev_core_ops gc0328db_subdev_core_ops = {
    .s_power      = gc0328db_s_power,
    .g_chip_ident = gc0328db_g_chip_ident,
};

static struct v4l2_subdev_video_ops gc0328db_subdev_video_ops = {
    .s_stream      = gc0328db_s_stream,
    .g_mbus_fmt    = gc0328db_g_fmt,
    .s_mbus_fmt    = gc0328db_s_fmt,
    .try_mbus_fmt  = gc0328db_try_fmt,
    .enum_mbus_fmt = gc0328db_enum_fmt,
    .g_mbus_config = gc0328db_g_mbus_config,
};

static struct v4l2_subdev_ops gc0328db_subdev_ops = {
    .core     = &gc0328db_subdev_core_ops,
    .video    = &gc0328db_subdev_video_ops,
};

static struct v4l2_ctrl_ops gc0328db_v4l2_ctrl_ops = {
    .g_volatile_ctrl = gc0328db_handler_g_volatile_ctrl,
    .s_ctrl          = gc0328db_handler_s_ctrl,
};

static int gc0328db_ctrl_handler_init(struct gc0328db_priv *priv)
{
    struct v4l2_ctrl* ctrl;
    struct i2c_client *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);

    v4l2_ctrl_handler_init(priv->db_camera_op.subdev.ctrl_handler, 16);

    ctrl = v4l2_ctrl_new_std(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0328db_v4l2_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_HFLIP\n");
    }

    ctrl = v4l2_ctrl_new_std(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0328db_v4l2_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_VFLIP\n");
    }

    ctrl = v4l2_ctrl_new_std_menu(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0328db_v4l2_ctrl_ops, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE,
                      V4L2_WHITE_BALANCE_SHADE, 0, V4L2_WHITE_BALANCE_AUTO);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE\n");
    }

    ctrl = v4l2_ctrl_new_std(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0328db_v4l2_ctrl_ops, V4L2_CID_DO_WHITE_BALANCE, 0, 0x00ffffff, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_EXPOSURE\n");
    }

    ctrl = v4l2_ctrl_new_std_menu(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0328db_v4l2_ctrl_ops, V4L2_CID_EXPOSURE_AUTO,
                      V4L2_EXPOSURE_APERTURE_PRIORITY, 0, V4L2_EXPOSURE_AUTO);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_EXPOSURE_AUTO\n");
    }

    ctrl = v4l2_ctrl_new_std(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0328db_v4l2_ctrl_ops, V4L2_CID_EXPOSURE, 0, 4096, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_EXPOSURE\n");
    }

    return 0;
}

static int gc0328db_video_probe(struct i2c_client *client)
{
    int ret = 0;
    unsigned char chip_id1 = 0, chip_id2 = 0;
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    ret = soc_camera_power_on(&client->dev, ssdd);
    if (ret < 0){
        return -1;
    }

    /* check and show product ID and manufacturer ID */

    gc0328db_g_chip_id(&chip_id1, &chip_id2, client);

    if (chip_id1 != CHIP_ID_GC0328 || chip_id2 != CHIP_ID_GC0328) {
        dev_err(&client->dev, "Read sensor %s chip_id %x %x is error\n",
                client->name, chip_id1, chip_id2);
        return -1;
    }

    dev_info(&client->dev, "%s Get sensor chip id-[0x%x]-[0x%x] successed!\n",
            client->name, chip_id1, chip_id2);

    ret = soc_camera_power_off(&client->dev, ssdd);
    if (ret < 0){
        return -1;
    }

    return 0;
}


/*
 * i2c_driver functions
 */
static void resume_handle(struct work_struct *data)
{
    int ret;
    struct v4l2_ctrl ctrl;
    struct gc0328db_priv* priv =
                    container_of(data, struct gc0328db_priv, resume_work);
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);

    /* active i2c switch */
    switch_channel(&priv->db_camera_op.cur_channel);
    ret = gc0328db_sensor_init(&priv->db_camera_op.subdev);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to init sensor.\n");
    }

    ret = gc0328db_s_fmt(&priv->db_camera_op.subdev, &priv->mf);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to s sensor fmt.\n");
    }

    ctrl.handler = &priv->ctrl_handler;

    if (priv->exposure_mode != V4L2_EXPOSURE_AUTO) {
        ctrl.id  = V4L2_CID_EXPOSURE_AUTO;
        ctrl.val = priv->exposure_mode;
        gc0328db_handler_s_ctrl(&ctrl);
        if (priv->exposure_mode == V4L2_EXPOSURE_MANUAL) {
            ctrl.id  = V4L2_CID_EXPOSURE;
            ctrl.val = priv->exposure;
            gc0328db_handler_s_ctrl(&ctrl);
        }
    }

    if (priv->w_balance_mode != V4L2_WHITE_BALANCE_AUTO) {
        ctrl.id  = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;
        ctrl.val = priv->w_balance_mode;
        gc0328db_handler_s_ctrl(&ctrl);
        if (priv->w_balance_mode == V4L2_WHITE_BALANCE_MANUAL) {
            ctrl.id  = V4L2_CID_DO_WHITE_BALANCE;
            ctrl.val = priv->w_balance;
            gc0328db_handler_s_ctrl(&ctrl);
        }
    }

    if (priv->flag_vflip) {
        ctrl.id  = V4L2_CID_VFLIP;
        ctrl.val = priv->flag_vflip;
        gc0328db_handler_s_ctrl(&ctrl);
    }

    if (priv->flag_hflip) {
        ctrl.id  = V4L2_CID_HFLIP;
        ctrl.val = priv->flag_hflip;
        gc0328db_handler_s_ctrl(&ctrl);
    }

    gc0328db_stream_off(priv);
    gc0328db_stream_on(priv);
}



static int gc0328db_probe(struct i2c_client *client,
                    const struct i2c_device_id *did)
{
    int ret = 0;
    struct gc0328db_priv *priv;
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
    struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
    int default_wight  = DEFAULT_WIDTH;
    int default_height = DEFAULT_HEIGHT;

    dev_info(&client->dev, "Probe ...\n");

    if (!ssdd) {
        dev_err(&client->dev, "missing platform data!\n");
        return -EINVAL;
    }

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE
            | I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&client->dev, "Can't support I2C functionality\n");
        return -ENODEV;
    }

    priv = kzalloc(sizeof(struct gc0328db_priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&client->dev,
            "Failed to allocate memory for private data!\n");
        return -ENOMEM;
    }

    mutex_init(&priv->reg_lock);
    mutex_init(&priv->cfg_lock);

    priv->power_en                         = 0;
    priv->db_camera_op.switch_work_channel = gc0328db_switch_work_channel;
    priv->db_camera_op.cur_channel         = 1;
    priv->cfmt_code                        = V4L2_MBUS_FMT_YUYV8_2X8;

    priv->w_balance_mode                   = V4L2_WHITE_BALANCE_AUTO;
    priv->exposure_mode                    = V4L2_EXPOSURE_AUTO;
    priv->flag_vflip                       = 0;
    priv->flag_hflip                       = 0;

    priv->win = gc0328db_select_win(&default_wight, &default_height);

    priv->db_camera_op.subdev.ctrl_handler = &priv->ctrl_handler;

    v4l2_i2c_subdev_init(&priv->db_camera_op.subdev, client, &gc0328db_subdev_ops);

    ret = gc0328db_video_probe(client);
    if (ret < 0) {
        kfree(priv);
        return -1;
    }

    priv->gc0328db_work_queue = create_singlethread_workqueue("work_queue");
    if (priv->gc0328db_work_queue == NULL) {
        kfree(priv);
        dev_err(&client->dev,"Failed to create workquque.\n");
        return -1;
    }
    gc0328db_ctrl_handler_init(priv);

    INIT_WORK(&priv->switch_channel_work, switch_channel_handle);
    INIT_WORK(&priv->resume_work, resume_handle);

    dev_info(&client->dev, "Probe successed\n");
    return 0;
}

static int gc0328db_remove(struct i2c_client *client)
{
    struct gc0328db_priv *priv = to_gc0328db(client);

    v4l2_ctrl_handler_free(&priv->ctrl_handler);
    destroy_workqueue(priv->gc0328db_work_queue);
    kfree(priv);

    return 0;
}

static int gc0328db_suspend(struct i2c_client *client, pm_message_t mesg)
{
    return 0;
}


static int gc0328db_resume(struct i2c_client *client)
{
    int ret;
    struct gc0328db_priv *priv = to_gc0328db(client);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    if (!priv->power_en) {
        return 0;
    }

    ret = soc_camera_set_power(&client->dev, ssdd, priv->power_en);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to set power.\n");
        return -1;
    }

    queue_work(priv->gc0328db_work_queue, &priv->resume_work);

    return 0;
}

static const struct i2c_device_id gc0328db_id[] = {
    { "gc0328_double",  0 },
    { }
};


MODULE_DEVICE_TABLE(i2c, gc0328db_id);

static struct i2c_driver gc0328db_i2c_driver = {
    .driver = {
        .name = "gc0328_double",
    },
    .suspend  = gc0328db_suspend,
    .resume   = gc0328db_resume,
    .probe    = gc0328db_probe,
    .remove   = gc0328db_remove,
    .id_table = gc0328db_id,
};

/*
 * Module functions
 */
static int __init gc0328db_module_init(void)
{
    return i2c_add_driver(&gc0328db_i2c_driver);
}

static void __exit gc0328db_module_exit(void)
{
    i2c_del_driver(&gc0328db_i2c_driver);
}

module_init(gc0328db_module_init);
module_exit(gc0328db_module_exit);

MODULE_DESCRIPTION("double gc0328 camera sensor driver");
MODULE_AUTHOR("Monk <rongjin.su@ingenic.com>");
MODULE_LICENSE("GPL");