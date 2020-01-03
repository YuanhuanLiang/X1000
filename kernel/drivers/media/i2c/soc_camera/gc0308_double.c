/*
 * gc0308db Camera Driver
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
#include <linux/hrtimer.h>
#include "double_camera.h"

/*
 * Timer :initialization the color/white-black(gc0308) one by one(80ms)
 * the sensor timing is exactly same,(24fps) ok
 * suspend_resume failed 16fps
 */
#define DEFAULT_WIDTH                       640
#define DEFAULT_HEIGHT                      480

#define CHIP_ID_GC0308                      0x9b

#define REG_BIT_AUTO_AWB                    (1<<1)

#define REG_HFLIP_IMG                       0x2 /* Horizontal mirror image reg bit */
#define REG_VFLIP_IMG                       0x1 /* Vertical flip image reg bit */

#define REG_CHIP_ID                         0xf0
#define REG_PAGE                            0xfe

#define GC0308_FRAME_TIME_MS                (120)


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
enum gc0308db_width {
    W_QCIF   = 176,
    W_QVGA   = 320,
    W_CIF    = 352,
    W_VGA    = 640,
};

enum gc0308db_height {
    H_QCIF   = 144,
    H_QVGA   = 240,
    H_CIF    = 288,
    H_VGA    = 480,
};

struct gc0308db_win_size {
    char *name;
    enum gc0308db_width width;
    enum gc0308db_height height;
    const struct regval_list *regs;
};

struct gc0308db_priv {
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
    const struct gc0308db_win_size *win;

    struct double_camera_op db_camera_op;
    struct v4l2_ctrl_handler ctrl_handler;

    struct workqueue_struct* gc0308db_work_queue;
    struct work_struct switch_channel_work;
    struct work_struct resume_work;

    struct hrtimer sensor_init_hrtimer;
    ktime_t sensor_init_delay;
    struct completion sensor_init_completion;
};


static inline int gc0308db_write_reg(struct i2c_client * client, unsigned char addr, unsigned char value)
{
    return i2c_smbus_write_byte_data(client, addr, value);
}

static inline char gc0308db_read_reg(struct i2c_client *client, unsigned char addr)
{
    return i2c_smbus_read_byte_data(client, addr);
}
/*
 * Registers settings
 */

#define ENDMARKER                       {0xff, 0xff}

static const struct regval_list gc0308db_sensor_init_regs[] = {
    //config list from net
    {0xfe, 0x80},
    {0xfe, 0x00},   // set page0
    {0xfe, 0x00},   // set page0
    {0xfe, 0x00},   // set page0

    {0xd2, 0x10},   // close AEC
    {0x22, 0x55},   // close AWB

    {0x03, 0x01},
    {0x03, 0x01},
    {0x03, 0x01},

    {0x04, 0x2c},
    {0x04, 0x2c},
    {0x04, 0x2c},

    {0x5a, 0x56},
    {0x5b, 0x40},
    {0x5c, 0x4a},

    {0x22, 0x57},   // Open AWB

    {0xfe, 0x00},
    {0x01, 0x6a},
    {0x02, 0x0c},//70
    {0x02, 0x0c},//70
    {0x02, 0x0c},//70

    {0x0f, 0x00},

    // {0xe2,0x00},
    {0xe3, 0x96},

    {0xe4, 0x01},
    {0xe5, 0x2c},
    {0xe6, 0x01},
    {0xe7, 0x2c},
    {0xe8, 0x01},
    {0xe9, 0x2c},
    {0xea, 0x01},
    {0xeb, 0x2c},

    {0x05, 0x00},
    {0x06, 0x00},
    {0x07, 0x00},
    {0x08, 0x00},
    {0x09, 0x01},
    {0x0a, 0xe8},
    {0x0b, 0x02},
    {0x0c, 0x88},
    {0x0d, 0x02},
    {0x0e, 0x02},
    {0x0e, 0x02},
    {0x0e, 0x02},

    {0x10, 0x22},
    {0x11, 0xfd},
    {0x11, 0xfd},
    {0x11, 0xfd},

    {0x12, 0x2a},
    {0x13, 0x00},

    {0x15, 0x0a},
    {0x16, 0x05},
    {0x17, 0x01},
    {0x17, 0x01},
    {0x17, 0x01},

    {0x18, 0x44},
    {0x19, 0x44},
    {0x1a, 0x1e},
    {0x1b, 0x00},
    {0x1c, 0xc1},
    {0x1d, 0x08},
    {0x1e, 0x60},
    {0x1f, 0x03},   //16


    {0x20, 0xff},
    {0x21, 0xf8},
    {0x22, 0x57},
#if CONFIG_FMT_Y8_1X8
    {0x24, 0xb1}, // only Y
#elif CONFIG_FMT_YUYV8_2X8
    {0x24, 0xa2}, // yuv422
#endif
    {0x28, 0x00},   //add

    {0x26, 0x03},// 03
    {0x2f, 0x01},
    {0x30, 0xf7},
    {0x31, 0x50},
    {0x32, 0x00},
    {0x39, 0x04},
    {0x3a, 0x18},
    {0x3b, 0x20},
    {0x3c, 0x00},
    {0x3d, 0x00},
    {0x3e, 0x00},
    {0x3f, 0x00},
    {0x50, 0x10},
    {0x53, 0x82},
    {0x54, 0x80},
    {0x55, 0x80},
    {0x56, 0x82},
    {0x8b, 0x40},
    {0x8c, 0x40},
    {0x8d, 0x40},
    {0x8e, 0x2e},
    {0x8f, 0x2e},
    {0x90, 0x2e},
    {0x91, 0x3c},
    {0x92, 0x50},
    {0x5d, 0x12},
    {0x5e, 0x1a},
    {0x5f, 0x24},
    {0x60, 0x07},
    {0x61, 0x15},
    {0x62, 0x08},
    {0x64, 0x03},
    {0x66, 0xe8},
    {0x66, 0xe8},
    {0x66, 0xe8},

    {0x67, 0x86},
    {0x68, 0xa2},
    {0x68, 0xa2},
    {0x68, 0xa2},

    {0x69, 0x18},
    {0x6a, 0x0f},
    {0x6b, 0x00},
    {0x6c, 0x5f},
    {0x6d, 0x8f},
    {0x6e, 0x55},
    {0x6f, 0x38},
    {0x70, 0x15},
    {0x71, 0x33},
    {0x72, 0xdc},
    {0x73, 0x80},
    {0x74, 0x02},
    {0x75, 0x3f},
    {0x76, 0x02},
    {0x77, 0x20},
    {0x78, 0x88},
    {0x79, 0x81},
    {0x7a, 0x81},
    {0x7b, 0x22},
    {0x7c, 0xff},
    {0x93, 0x48},
    {0x94, 0x00},
    {0x95, 0x05},
    {0x96, 0xe8},
    {0x97, 0x40},
    {0x98, 0xf0},
    {0xb1, 0x38},
    {0xb2, 0x38},
    {0xbd, 0x38},
    {0xbe, 0x36},
    {0xd0, 0xc9},
    {0xd1, 0x10},

    {0xd3, 0x80},
    {0xd5, 0xf2},
    {0xd6, 0x16},
    {0xdb, 0x92},
    {0xdc, 0xa5},
    {0xdf, 0x23},
    {0xd9, 0x00},
    {0xda, 0x00},
    {0xe0, 0x09},

    {0xed, 0x04},
    {0xee, 0xa0},
    {0xef, 0x40},
    {0x80, 0x03},
    {0x80, 0x03},
    {0x9F, 0x10},
    {0xA0, 0x20},
    {0xA1, 0x38},
    {0xA2, 0x4E},
    {0xA3, 0x63},
    {0xA4, 0x76},
    {0xA5, 0x87},
    {0xA6, 0xA2},
    {0xA7, 0xB8},
    {0xA8, 0xCA},
    {0xA9, 0xD8},
    {0xAA, 0xE3},
    {0xAB, 0xEB},
    {0xAC, 0xF0},
    {0xAD, 0xF8},
    {0xAE, 0xFD},
    {0xAF, 0xFF},
    {0xc0, 0x00},
    {0xc1, 0x10},
    {0xc2, 0x1C},
    {0xc3, 0x30},
    {0xc4, 0x43},
    {0xc5, 0x54},
    {0xc6, 0x65},
    {0xc7, 0x75},
    {0xc8, 0x93},
    {0xc9, 0xB0},
    {0xca, 0xCB},
    {0xcb, 0xE6},
    {0xcc, 0xFF},
    {0xf0, 0x02},
    {0xf1, 0x01},
    {0xf2, 0x01},
    {0xf3, 0x30},
    {0xf9, 0x9f},
    {0xfa, 0x78},

    //---------------------------------------------------------------
    {0xfe, 0x01},// set page1

    {0x00, 0xf5},
    {0x02, 0x1a},
    {0x0a, 0xa0},
    {0x0b, 0x60},
    {0x0c, 0x08},
    {0x0e, 0x4c},
    {0x0f, 0x39},
    {0x11, 0x3f},
    {0x12, 0x72},
    {0x13, 0x13},
    {0x14, 0x42},
    {0x15, 0x43},
    {0x16, 0xc2},
    {0x17, 0xa8},
    {0x18, 0x18},
    {0x19, 0x40},
    {0x1a, 0xd0},
    {0x1b, 0xf5},
    {0x70, 0x40},
    {0x71, 0x58},
    {0x72, 0x30},
    {0x73, 0x48},
    {0x74, 0x20},
    {0x75, 0x60},
    {0x77, 0x20},
    {0x78, 0x32},
    {0x30, 0x03},
    {0x31, 0x40},
    {0x32, 0xe0},
    {0x33, 0xe0},
    {0x34, 0xe0},
    {0x35, 0xb0},
    {0x36, 0xc0},
    {0x37, 0xc0},
    {0x38, 0x04},
    {0x39, 0x09},
    {0x3a, 0x12},
    {0x3b, 0x1C},
    {0x3c, 0x28},
    {0x3d, 0x31},
    {0x3e, 0x44},
    {0x3f, 0x57},
    {0x40, 0x6C},
    {0x41, 0x81},
    {0x42, 0x94},
    {0x43, 0xA7},
    {0x44, 0xB8},
    {0x45, 0xD6},
    {0x46, 0xEE},
    {0x47, 0x0d},

    //Registers of Page0
    {0xfe, 0x00}, // set page0
    {0x10, 0x26},
    {0x10, 0x26},
    {0x10, 0x26},

    {0x11, 0x0d},  // fd,modified by mormo 2010/07/06
    {0x11, 0x0d},  // fd,modified by mormo 2010/07/06
    {0x11, 0x0d},  // fd,modified by mormo 2010/07/06

    {0x1a, 0x2a},  // 1e,modified by mormo 2010/07/06

    {0x1c, 0x49}, // c1,modified by mormo 2010/07/06
    {0x1d, 0x9a}, // 08,modified by mormo 2010/07/06
    {0x1e, 0x61}, // 60,modified by mormo 2010/07/06

    {0x3a, 0x20},

    {0x50, 0x14},  // 10,modified by mormo 2010/07/06
    {0x53, 0x80},
    {0x56, 0x80},

    {0x8b, 0x20}, //LSC
    {0x8c, 0x20},
    {0x8d, 0x20},
    {0x8e, 0x14},
    {0x8f, 0x10},
    {0x90, 0x14},

    {0x94, 0x02},
    {0x95, 0x07},
    {0x96, 0xe0},

    {0xb1, 0x40}, // YCPT
    {0xb2, 0x40},
    {0xb3, 0x48},
    {0xb6, 0xe0},

    {0xd0, 0xc9}, // AECT  c9,modifed by mormo 2010/07/06
    {0xd3, 0x68}, // 80,modified by mormor 2010/07/06
    {0xf2, 0x02},
    {0xf7, 0x12},
    {0xf8, 0x0a},

    //Registers of Page1
    {0xfe, 0x01},// set page1
    {0x02, 0x20},
    {0x04, 0x10},
    {0x05, 0x08},
    {0x06, 0x20},
    {0x08, 0x0a},

    {0x0e, 0x44},
    {0x0f, 0x32},
    {0x10, 0x41},
    {0x11, 0x37},
    {0x12, 0x22},
    {0x13, 0x19},
    {0x14, 0x44},
    {0x15, 0x44},

    {0x19, 0x50},
    {0x1a, 0xd8},

    {0x32, 0x10},

    {0x35, 0x00},
    {0x36, 0x80},
    {0x37, 0x00},
    //-----------Update the registers end---------//

    {0xfe, 0x00}, // set page0
    {0xd2, 0x90},

    //-----------GAMMA Select(3)---------------//
    {0x9F, 0x10},
    {0xA0, 0x20},
    {0xA1, 0x38},
    {0xA2, 0x4E},
    {0xA3, 0x63},
    {0xA4, 0x76},
    {0xA5, 0x87},
    {0xA6, 0xA2},
    {0xA7, 0xB8},
    {0xA8, 0xCA},
    {0xA9, 0xD8},
    {0xAA, 0xE3},
    {0xAB, 0xEB},
    {0xAC, 0xF0},
    {0xAD, 0xF8},
    {0xAE, 0xFD},
    {0xAF, 0xFF},

    {0xfe, 0x00},
    {0xfe, 0x00},
    {0xfe, 0x00},

    {0x14, 0x12}, //0x10
    {0x14, 0x12}, //0x10
    {0x14, 0x12}, //0x10
    {0x25, 0x0f}, // enable output
    ENDMARKER,
};


static const struct regval_list gc0308db_qcif_regs[] = {
    {0xfe, 0x00},
    {0x46, 0x80}, {0x47, 0x00},
    {0x48, 0x00}, {0x49, 0x00},
    {0x4a, 0x90}, {0x4b, 0x00},
    {0x4c, 0xb0},

    {0xfe, 0x01},
    {0x54, 0x33}, {0x55, 0x01},
    {0x56, 0x00}, {0x57, 0x00},
    {0x58, 0x00}, {0x59, 0x00},
    ENDMARKER,
};

static const struct regval_list gc0308db_qvga_regs[] = {
    {0xfe, 0x00},
    {0x46, 0x80}, {0x47, 0x00},
    {0x48, 0x00}, {0x49, 0x00},
    {0x4a, 0xf0}, {0x4b, 0x01},
    {0x4c, 0x40},

    {0xfe, 0x01},
    {0x54, 0x22}, {0x55, 0x01},
    {0x56, 0x00}, {0x57, 0x00},
    {0x58, 0x00}, {0x59, 0x00},
    ENDMARKER,
};

static const struct regval_list gc0308db_cif_regs[] = {
    {0xfe, 0x00},
    {0x46, 0x80}, {0x47, 0x00},
    {0x48, 0x00}, {0x49, 0x01},
    {0x4a, 0x20}, {0x4b, 0x01},
    {0x4c, 0x60},

    {0xfe, 0x01},
    {0x54, 0x55}, {0x55, 0x01},
    {0x56, 0x02}, {0x57, 0x04},
    {0x58, 0x02}, {0x59, 0x04},
    ENDMARKER,
};

static const struct regval_list gc0308db_vga_regs[] = {
    {0xfe, 0x00},
    {0x46, 0x80}, {0x47, 0x00},
    {0x48, 0x00}, {0x49, 0x01},
    {0x4a, 0xE0}, {0x4b, 0x02},
    {0x4c, 0x80},

    {0xfe, 0x01},
    {0x54, 0x11}, {0x55, 0x00},
    {0x56, 0x00}, {0x57, 0x00},
    {0x58, 0x00}, {0x59, 0x00},
    ENDMARKER,
};

static struct regval_list gc0308db_enable_auto_wb_regs[] = {
    {0xfe, 0x00},
    {0x22, 0x00},
    ENDMARKER,
};

static const struct regval_list gc0308db_wb_incandescence_regs[] = {
    {0xfe, 0x00},
#if 0
    /* this is GC0328, need get the config for GC0308 */
    {0x77, 0x48}, {0x78, 0x40},
    {0x79, 0x5c},
#endif
    ENDMARKER,
};

static const struct regval_list gc0308db_wb_daylight_regs[] = {
    {0xfe, 0x00},
#if 0
    /* this is GC0328, need get the config for GC0308 */
    {0x77, 0x74}, {0x78, 0x52},
    {0x79, 0x40},
#endif
    ENDMARKER,
};

static const struct regval_list gc0308db_wb_fluorescent_regs[] = {
    {0xfe, 0x00},
#if 0
    /* this is GC0328, need get the config for GC0308 */
    {0x77, 0x40}, {0x78, 0x42},
    {0x79, 0x50},
#endif
    ENDMARKER,
};

static const struct regval_list gc0308db_wb_cloud_regs[] = {
    {0xfe, 0x00},
#if 0
    /* this is GC0328, need get the config for GC0308 */
    {0x77, 0x8c}, {0x78, 0x50},
    {0x79, 0x40},
#endif
    ENDMARKER,
};

static struct regval_list gc0308db_set_white_balance_regs[] = {
    {0xfe, 0x00},
#if 0
    /* this is GC0328, need get the config for GC0308 */
    {0x77, 0x00}, {0x78, 0x00},
    {0x79, 0x00},
#endif
    ENDMARKER,
};


#define GC0308DB_SIZE(n, w, h, r) \
    {.name = n, .width = w , .height = h, .regs = r }

static const struct gc0308db_win_size gc0308db_supported_win_sizes[] = {
    GC0308DB_SIZE("QCIF", W_QCIF, H_QCIF, gc0308db_qcif_regs),
    GC0308DB_SIZE("QVGA", W_QVGA, H_QVGA, gc0308db_qvga_regs),
    GC0308DB_SIZE("CIF", W_CIF, H_CIF, gc0308db_cif_regs),
    GC0308DB_SIZE("VGA", W_VGA, H_VGA, gc0308db_vga_regs),
};

static const struct regval_list gc0308db_enable_output_regs[] = {
    {0xfe, 0x00},
    {0x25, 0x0f},
    ENDMARKER,
};

static const struct regval_list gc0308db_disable_output_regs[] = {
    {0xfe, 0x00},
    {0x25, 0x00},
    ENDMARKER,
};

static struct regval_list gc0308db_mirror_h_regs[] = {
    {0xfe, 0x00},
    {0x14, 0x10}, /* ingenic for gc0328 {0x17, 0x00} */
    ENDMARKER,
};

static struct regval_list gc0308db_mirror_v_regs[] = {
    {0xfe, 0x00},
    {0x14, 0x10},
    ENDMARKER,
};

static struct regval_list gc0308db_chip_id_regs[] = {
    {0xfe, 0x00},
    {0x00, 0x00},
    ENDMARKER,
};

static struct regval_list gc0308db_enable_auto_exposure_regs[] = {
    {0xfe, 0x00},
    {0xd2, 0x90},
    ENDMARKER,
};

static const struct regval_list gc0308db_disable_auto_exposure_regs[] = {
    {0xfe, 0x00},
    {0xd2, 0x10},
    ENDMARKER,
};

static struct regval_list gc0308db_set_exposure_time_regs[] = {
    {0xfe, 0x00},
    {0x03, 0x00},
    {0x04, 0x00},
    ENDMARKER,
};

static enum v4l2_mbus_pixelcode gc0308db_codes[] = {
    V4L2_MBUS_FMT_Y8_1X8,
    V4L2_MBUS_FMT_YUYV8_2X8,
    V4L2_MBUS_FMT_YUYV8_1_5X8,
    V4L2_MBUS_FMT_JZYUYV8_1_5X8,
};



/*
 * General functions
 */

static struct gc0308db_priv *to_gc0308db(const struct i2c_client *client)
{
    struct double_camera_op* dbc = container_of(i2c_get_clientdata(client),
                            struct double_camera_op, subdev);

    return container_of(dbc, struct gc0308db_priv, db_camera_op);
}

static int gc0308db_write_array(struct i2c_client *client,
                                const struct regval_list *vals)
{
    int ret;
    struct gc0308db_priv *priv = to_gc0308db(client);
    unsigned char read_val = 0;
    mutex_lock(&priv->reg_lock);
    while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
        dev_vdbg(&client->dev, "write array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
        ret = gc0308db_write_reg(client, vals->reg_num, vals->value);
        if (ret < 0) {
            printk("======gc0308db_write_array failed===\n");
            dev_err(&client->dev, "write array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
            mutex_unlock(&priv->reg_lock);
            return ret;
        }

        if (vals->reg_num != REG_PAGE) {
            read_val = gc0308db_read_reg(client,vals->reg_num);
            if (vals->value != read_val) {
                dev_err(&client->dev, "==Read1== read array: 0x%02x,  Except:0x%02x Read:0x%02x",vals->reg_num, vals->value, read_val);
            }
        }

        vals++;
    }
    mutex_unlock(&priv->reg_lock);
    return 0;
}

static int gc0308db_read_array(struct i2c_client  *client,
                               struct regval_list *vals)
{
    int ret;
    struct gc0308db_priv *priv = to_gc0308db(client);

    mutex_lock(&priv->reg_lock);
    while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
        if (vals->reg_num == REG_PAGE) {
            dev_vdbg(&client->dev, "write array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
            ret = gc0308db_write_reg(client,vals->reg_num, vals->value);
            if (ret < 0) {
                mutex_unlock(&priv->reg_lock);
                return ret;
            }
        } else {
            dev_vdbg(&client->dev, "read array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
            vals->value = gc0308db_read_reg(client,vals->reg_num);
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
    struct gc0308db_priv* priv =
                    container_of(data, struct gc0308db_priv, switch_channel_work);
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);
    /*
     1, disable current sensor output
     2, switch another channel
     3, enable sensor output
     */
    mutex_lock(&priv->cfg_lock);
    gc0308db_write_array(client, gc0308db_disable_output_regs);
    switch_channel(&priv->db_camera_op.cur_channel);
    gc0308db_write_array(client, gc0308db_enable_output_regs);
    mutex_unlock(&priv->cfg_lock);
}

static void gc0308db_stream_on(struct gc0308db_priv* priv)
{
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);

    gc0308db_write_array(client, gc0308db_enable_output_regs);
}

static void gc0308db_stream_off(struct gc0308db_priv* priv)
{
    int ret1,ret2;
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);

    double_channel_do(ret1, ret2, &priv->db_camera_op.cur_channel,
                    gc0308db_write_array, client, gc0308db_disable_output_regs);
}


static void gc0308db_switch_work_channel(struct double_camera_op* dbc)
{
    struct gc0308db_priv* priv = container_of(dbc,
                            struct gc0308db_priv, db_camera_op);

    schedule_work(&priv->switch_channel_work);
}


/*
 * soc_camera_ops functions
 */
static int gc0308db_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0308db_priv *priv = to_gc0308db(client);

    if (enable) {
        gc0308db_stream_off(priv);
        gc0308db_stream_on(priv);
    } else {
        gc0308db_stream_off(priv);
    }

    return 0;
}

static int enable_auto_white_balance(struct i2c_client *client, unsigned char en)
{
    int ret;

    ret = gc0308db_read_array(client, gc0308db_enable_auto_wb_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read awb regs\n");
        return -1;
    }

    gc0308db_enable_auto_wb_regs[1].value = en ? gc0308db_enable_auto_wb_regs[1].value | REG_BIT_AUTO_AWB
                                               : gc0308db_enable_auto_wb_regs[1].value &~REG_BIT_AUTO_AWB;
    ret = gc0308db_write_array(client, gc0308db_enable_auto_wb_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write awb regs\n");
        return -1;
    }

    return 0;
}

static int set_white_balance_mode(struct i2c_client *client, const struct regval_list *rgls)
{
    int ret;
    struct gc0308db_priv *priv = to_gc0308db(client);

    if (priv->w_balance_mode == V4L2_WHITE_BALANCE_AUTO) {
        ret = enable_auto_white_balance(client, 0);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to en/diable awb\n");
            return -1;
        }
    }

    ret = gc0308db_write_array(client, rgls);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write awb regs\n");
        return -1;
    }

    return 0;
}

static int s_ctrl_auto_white_balance_handle(struct i2c_client *client, int mode)
{
    int ret;
    struct gc0308db_priv *priv = to_gc0308db(client);

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
        ret = set_white_balance_mode(client, gc0308db_wb_incandescence_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write awb regs\n");
            return -1;
        }
    break;

    case V4L2_WHITE_BALANCE_DAYLIGHT:
        ret = set_white_balance_mode(client, gc0308db_wb_daylight_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write awb regs\n");
            return -1;
        }
    break;

    case V4L2_WHITE_BALANCE_FLUORESCENT:
        ret = set_white_balance_mode(client, gc0308db_wb_fluorescent_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write awb regs\n");
            return -1;
        }
    break;

    case V4L2_WHITE_BALANCE_CLOUDY:
        ret = set_white_balance_mode(client, gc0308db_wb_cloud_regs);
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
    struct gc0308db_priv *priv = to_gc0308db(client);
    int w_balance = value & 0x00ffffff;

    if (priv->w_balance_mode != V4L2_WHITE_BALANCE_MANUAL) {
        dev_err(&client->dev,"Not in manual mode.\n");
        return -1;
    }

    gc0308db_set_white_balance_regs[1].value = (unsigned char)(0x000000ff & (w_balance>>16));
    gc0308db_set_white_balance_regs[2].value = (unsigned char)(0x000000ff & (w_balance>>8));
    gc0308db_set_white_balance_regs[3].value = (unsigned char)(0x000000ff &  w_balance);

    ret = gc0308db_write_array(client, gc0308db_set_white_balance_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write white_balance regs\n");
        return -1;
    }

    return 0;
}

static int s_ctrl_exposure_handle(struct i2c_client *client, int value)
{
    int ret;
    struct gc0308db_priv *priv = to_gc0308db(client);
    int exposure = value & 0xfff;

    if (priv->exposure_mode != V4L2_EXPOSURE_MANUAL){
        dev_err(&client->dev,"Not in manual mode.\n");
        return -1;
    }

    gc0308db_set_exposure_time_regs[1].value = (unsigned char)(0x0f   & (exposure>>8));
    gc0308db_set_exposure_time_regs[2].value = (unsigned char)(0x00ff & exposure);

    ret = gc0308db_write_array(client, gc0308db_set_exposure_time_regs);
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
        ret = gc0308db_write_array(client, gc0308db_enable_auto_exposure_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write enable_auto_exposure regs\n");
            return -1;
        }
        break;

    case V4L2_EXPOSURE_MANUAL:
        ret = gc0308db_write_array(client, gc0308db_disable_auto_exposure_regs);
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

    printk("===========s_ctrl_mirror_vflip=value=%d   ===%d=\n",value,  __LINE__);
#if 1
    val = value ? REG_VFLIP_IMG : 0;
    ret = gc0308db_read_array(client, gc0308db_mirror_v_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read mirror regs\n");
        return -1;
    }
    printk("gc0308db_mirror_v_regs====1=== = 0x%x\n", gc0308db_mirror_v_regs[1].value);
    gc0308db_mirror_v_regs[1].value &= ~REG_VFLIP_IMG;
    gc0308db_mirror_v_regs[1].value |= val;
    ret = gc0308db_write_array(client,gc0308db_mirror_v_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write mirror regs\n");
        return -1;
    }


    ret = gc0308db_read_array(client, gc0308db_mirror_v_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read mirror regs\n");
        return -1;
    }
    printk("gc0308db_mirror_v_regs = 0x%x\n", gc0308db_mirror_v_regs[1].value);
#endif
    return 0;
}

static int s_ctrl_mirror_hflip(struct i2c_client *client, int value)
{
    int ret,val = 0;

    val = value ? REG_HFLIP_IMG : 0;

    printk("===========s_ctrl_mirror_hflip=====%d=\n", __LINE__);
    ret = gc0308db_read_array(client,gc0308db_mirror_h_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read mirror regs\n");
        return -1;
    }
    gc0308db_mirror_h_regs[1].value &= ~REG_HFLIP_IMG;
    gc0308db_mirror_h_regs[1].value |= val;
    ret = gc0308db_write_array(client,gc0308db_mirror_h_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write mirror regs\n");
        return -1;
    }
    return 0;
}

static void set_ctrl_mode(struct gc0308db_priv *priv, unsigned int cid, unsigned int mode)
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

static int gc0308db_s_ctrl_do(struct i2c_client *client, struct v4l2_control *ctrl)
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




static int gc0308db_handler_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret1, ret2;
    struct v4l2_control control;
    struct gc0308db_priv *priv   = container_of(ctrl->handler, struct gc0308db_priv, ctrl_handler);
    struct v4l2_subdev *sd       = &priv->db_camera_op.subdev;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct double_camera_op* dbc = container_of(sd, struct double_camera_op, subdev);

    dev_info(&client->dev, "Set ctrl\n");

    mutex_lock(&priv->cfg_lock);
    control.id    = ctrl->id;
    control.value = ctrl->val;
    double_channel_do(ret1, ret2, &dbc->cur_channel, gc0308db_s_ctrl_do, client, &control);
    if (ret1 != 0 || ret2 != 0) {
        mutex_unlock(&priv->cfg_lock);
        return -1;
    }
    set_ctrl_mode(priv, control.id, control.value);
    mutex_unlock(&priv->cfg_lock);
    return 0;
}

static int gc0308db_g_volatile_ctrl_do(struct i2c_client *client, struct v4l2_control *ctrl)
{
    int ret = 0;

    switch (ctrl->id) {
    default:
        dev_err(&client->dev, "Has no V4L2 CID: 0x%x ", ctrl->id);
        return -EINVAL;
    }

    return ret;
}

static int gc0308db_handler_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret;
    struct gc0308db_priv *priv   = container_of(ctrl->handler, struct gc0308db_priv, ctrl_handler);
    struct v4l2_subdev *sd       = &priv->db_camera_op.subdev;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct v4l2_control control;

    control.id = ctrl->id;

    ret = gc0308db_g_volatile_ctrl_do(client, &control);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to g ctrl\n");
        return -1;
    }
    ctrl->val = control.value;
    return 0;
}

static int gc0308db_g_chip_ident(struct v4l2_subdev *sd,
                         struct v4l2_dbg_chip_ident *id)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);

    dev_info(&client->dev, "Get chip ident 0x%x\n",CHIP_ID_GC0308);

    id->ident    = CHIP_ID_GC0308;
    id->revision = 0;

    return 0;
}



/* Select the nearest higher resolution for capture */
static const struct gc0308db_win_size *gc0308db_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(gc0308db_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(gc0308db_supported_win_sizes); i++) {
        if (gc0308db_supported_win_sizes[i].width  >= *width &&
            gc0308db_supported_win_sizes[i].height >= *height) {
            *width  = gc0308db_supported_win_sizes[i].width;
            *height = gc0308db_supported_win_sizes[i].height;
            return &gc0308db_supported_win_sizes[i];
        }
    }

    *width  = gc0308db_supported_win_sizes[default_size].width;
    *height = gc0308db_supported_win_sizes[default_size].height;
    return &gc0308db_supported_win_sizes[default_size];
}



static int gc0308db_g_fmt(struct v4l2_subdev *sd,
            struct v4l2_mbus_framefmt *mf)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0308db_priv *priv = to_gc0308db(client);

    dev_info(&client->dev, "Get win of fmt: %dx%d\n", priv->win->width,priv->win->height);

    mf->width      = priv->win->width;
    mf->height     = priv->win->height;
    mf->code       = priv->cfmt_code;

    mf->colorspace = V4L2_COLORSPACE_JPEG;
    mf->field      = V4L2_FIELD_NONE;

    return 0;
}


static int gc0308db_sensor_init_do(struct i2c_client* client)
{
    int ret;

    /* initialize the sensor with default data */
    ret = gc0308db_write_array(client, gc0308db_sensor_init_regs);
    if (ret < 0)
        return -1;

    return 0;
}

static int gc0308db_sensor_init(struct v4l2_subdev *sd)
{
    int ret1,ret2;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct gc0308db_priv *priv   = to_gc0308db(client);
    struct double_camera_op* dbc = container_of(sd, struct double_camera_op, subdev);

    dev_info(&client->dev, "Init Sensors...\n");

    mutex_lock(&priv->cfg_lock);
    /*
     * the sensors frame rate is 25fps:
     * initialization the sensor register about 30ms.
     *
     * init first one sensor, after 40ms, init another sensor,
     * make sure the sensors pclk/vsync is complete synchornozation.
     */
    hrtimer_start(&priv->sensor_init_hrtimer, priv->sensor_init_delay,
                HRTIMER_MODE_REL);

    //double_channel_do(ret1, ret2, &dbc->cur_channel, gc0308db_sensor_init_do, client);
    printk("===============start =======1======\n");
    ret1 = gc0308db_sensor_init_do(client);
    if (ret1 != 0) {
        dev_err(&client->dev, "Failed to init first sensor.\n");
        mutex_unlock(&priv->cfg_lock);
        return -1;
    }
    printk("===============end =========1====\n");
    /* switch to another i2c channel */
    switch_channel(&priv->db_camera_op.cur_channel);

    ret1 = wait_for_completion_timeout(&priv->sensor_init_completion,
            msecs_to_jiffies(GC0308_FRAME_TIME_MS * 3));
    if (ret1 == 0) {
        dev_err(&client->dev, "wait for completion timeout %d ms.\n", GC0308_FRAME_TIME_MS * 3);
    }
    printk("===============start ======2=======\n");
    ret2 = gc0308db_sensor_init_do(client);
    if (ret2 < 0 ) {
        dev_err(&client->dev, "Failed to init second sensor.\n");
        mutex_unlock(&priv->cfg_lock);
        return -1;
    }
    printk("===============start ======2=======\n");
    mutex_unlock(&priv->cfg_lock);

    return 0;
}


static int gc0308db_s_fmt_do(struct v4l2_subdev *sd,
                      struct v4l2_mbus_framefmt *mf)
{
    /* current do not support set format, use unify format yuv422i */
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0308db_priv *priv = to_gc0308db(client);
    int ret;

    dev_info(&client->dev, "Set win of fmt : %dx%d\n",mf->width,mf->height);
    priv->win = gc0308db_select_win(&mf->width, &mf->height);
    /* set size win */
    ret = gc0308db_write_array(client, priv->win->regs);
    if (ret < 0) {
        dev_err(&client->dev, "%s: Error\n", __func__);
        return ret;
    }

    return 0;
}

static int gc0308db_s_fmt(struct v4l2_subdev *sd,
                   struct v4l2_mbus_framefmt *mf)
{
    int ret1,ret2;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct gc0308db_priv *priv   = to_gc0308db(client);
    struct double_camera_op* dbc = container_of(sd, struct double_camera_op, subdev);

    dev_info(&client->dev, "Set fmt\n");

    mutex_lock(&priv->cfg_lock);
    double_channel_do(ret1, ret2, &dbc->cur_channel, gc0308db_s_fmt_do, sd, mf);
    if (ret1 != 0 || ret2 != 0) {
        mutex_unlock(&priv->cfg_lock);
        return -1;
    }
    memcpy(&priv->mf, mf, sizeof(struct v4l2_mbus_framefmt));
    mutex_unlock(&priv->cfg_lock);

    return 0;
}


static int gc0308db_try_fmt(struct v4l2_subdev *sd,
                     struct v4l2_mbus_framefmt *mf)
{
    const struct gc0308db_win_size *win;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /*
     * select suitable win
     */
    win = gc0308db_select_win(&mf->width, &mf->height);

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

static int gc0308db_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
                                         enum v4l2_mbus_pixelcode *code)
{
    if (index >= ARRAY_SIZE(gc0308db_codes))
        return -EINVAL;

    *code = gc0308db_codes[index];
    return 0;
}


static char gc0308db_g_chip_id_do(struct i2c_client *client)
{
    int ret;

    ret = gc0308db_read_array(client, gc0308db_chip_id_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read chip id\n");
        return ret;
    }
    return gc0308db_chip_id_regs[1].value;
}

static void gc0308db_g_chip_id(char* id1, char* id2, struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    struct double_camera_op* dbc = container_of(sd, struct double_camera_op, subdev);

    dev_info(&client->dev, "Get chip id\n");

    double_channel_do(*id1, *id2, &dbc->cur_channel, gc0308db_g_chip_id_do, client);
}


static int gc0308db_s_power(struct v4l2_subdev *sd, int on)
{
    int ret;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct gc0308db_priv *priv = to_gc0308db(client);
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
        ret = gc0308db_sensor_init(sd);
        if (ret < 0) {
            dev_err(&client->dev, "Failed to init sensors.\n");
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

static int gc0308db_g_mbus_config(struct v4l2_subdev *sd,
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


static struct v4l2_subdev_core_ops gc0308db_subdev_core_ops = {
    .s_power      = gc0308db_s_power,
    .g_chip_ident = gc0308db_g_chip_ident,
};

static struct v4l2_subdev_video_ops gc0308db_subdev_video_ops = {
    .s_stream      = gc0308db_s_stream,
    .g_mbus_fmt    = gc0308db_g_fmt,
    .s_mbus_fmt    = gc0308db_s_fmt,
    .try_mbus_fmt  = gc0308db_try_fmt,
    .enum_mbus_fmt = gc0308db_enum_fmt,
    .g_mbus_config = gc0308db_g_mbus_config,
};

static struct v4l2_subdev_ops gc0308db_subdev_ops = {
    .core     = &gc0308db_subdev_core_ops,
    .video    = &gc0308db_subdev_video_ops,
};

static struct v4l2_ctrl_ops gc0308db_v4l2_ctrl_ops = {
    .g_volatile_ctrl = gc0308db_handler_g_volatile_ctrl,
    .s_ctrl          = gc0308db_handler_s_ctrl,
};

static int gc0308db_ctrl_handler_init(struct gc0308db_priv *priv)
{
    struct v4l2_ctrl* ctrl;
    struct i2c_client *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);

    v4l2_ctrl_handler_init(priv->db_camera_op.subdev.ctrl_handler, 16);

    ctrl = v4l2_ctrl_new_std(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0308db_v4l2_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_HFLIP\n");
    }

    ctrl = v4l2_ctrl_new_std(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0308db_v4l2_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_VFLIP\n");
    }

    ctrl = v4l2_ctrl_new_std_menu(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0308db_v4l2_ctrl_ops, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE,
                      V4L2_WHITE_BALANCE_SHADE, 0, V4L2_WHITE_BALANCE_AUTO);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE\n");
    }

    ctrl = v4l2_ctrl_new_std(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0308db_v4l2_ctrl_ops, V4L2_CID_DO_WHITE_BALANCE, 0, 0x00ffffff, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_EXPOSURE\n");
    }

    ctrl = v4l2_ctrl_new_std_menu(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0308db_v4l2_ctrl_ops, V4L2_CID_EXPOSURE_AUTO,
                      V4L2_EXPOSURE_APERTURE_PRIORITY, 0, V4L2_EXPOSURE_AUTO);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_EXPOSURE_AUTO\n");
    }

    ctrl = v4l2_ctrl_new_std(priv->db_camera_op.subdev.ctrl_handler,
                      &gc0308db_v4l2_ctrl_ops, V4L2_CID_EXPOSURE, 0, 4096, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_EXPOSURE\n");
    }

    return 0;
}

static int gc0308db_video_probe(struct i2c_client *client)
{
    int ret = 0;
    unsigned char chip_id1 = 0, chip_id2 = 0;
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    ret = soc_camera_power_on(&client->dev, ssdd);
    if (ret < 0){
        return -1;
    }

    /* check and show product ID and manufacturer ID */

    gc0308db_g_chip_id(&chip_id1, &chip_id2, client);

    if (chip_id1 != CHIP_ID_GC0308 || chip_id2 != CHIP_ID_GC0308) {
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
    struct gc0308db_priv* priv =
                    container_of(data, struct gc0308db_priv, resume_work);
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->db_camera_op.subdev);

    mdelay(GC0308_FRAME_TIME_MS + 10);
    /* active i2c switch */
    priv->db_camera_op.cur_channel = 1;
    switch_channel(&priv->db_camera_op.cur_channel);
    ret = gc0308db_sensor_init(&priv->db_camera_op.subdev);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to init sensor.\n");
    }

    ret = gc0308db_s_fmt(&priv->db_camera_op.subdev, &priv->mf);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to s sensor fmt.\n");
    }

    ctrl.handler = &priv->ctrl_handler;

    if (priv->exposure_mode != V4L2_EXPOSURE_AUTO) {
        ctrl.id  = V4L2_CID_EXPOSURE_AUTO;
        ctrl.val = priv->exposure_mode;
        gc0308db_handler_s_ctrl(&ctrl);
        if (priv->exposure_mode == V4L2_EXPOSURE_MANUAL) {
            ctrl.id  = V4L2_CID_EXPOSURE;
            ctrl.val = priv->exposure;
            gc0308db_handler_s_ctrl(&ctrl);
        }
    }

    if (priv->w_balance_mode != V4L2_WHITE_BALANCE_AUTO) {
        ctrl.id  = V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE;
        ctrl.val = priv->w_balance_mode;
        gc0308db_handler_s_ctrl(&ctrl);
        if (priv->w_balance_mode == V4L2_WHITE_BALANCE_MANUAL) {
            ctrl.id  = V4L2_CID_DO_WHITE_BALANCE;
            ctrl.val = priv->w_balance;
            gc0308db_handler_s_ctrl(&ctrl);
        }
    }

    if (priv->flag_vflip) {
        ctrl.id  = V4L2_CID_VFLIP;
        ctrl.val = priv->flag_vflip;
        gc0308db_handler_s_ctrl(&ctrl);
    }

    if (priv->flag_hflip) {
        ctrl.id  = V4L2_CID_HFLIP;
        ctrl.val = priv->flag_hflip;
        gc0308db_handler_s_ctrl(&ctrl);
    }


    gc0308db_stream_off(priv);
    gc0308db_stream_on(priv);
}

static enum hrtimer_restart fsync_sensor_init_hrtimer_handler(struct hrtimer *timer)
{
    int ret = HRTIMER_NORESTART;
    struct gc0308db_priv *priv = container_of(timer, struct gc0308db_priv, sensor_init_hrtimer);

    complete(&priv->sensor_init_completion);
    printk("fsync sensor hrtimer handler.\n");

    return ret;
}


static int gc0308db_probe(struct i2c_client *client,
                    const struct i2c_device_id *did)
{
    int ret = 0;
    struct gc0308db_priv *priv;
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

    priv = kzalloc(sizeof(struct gc0308db_priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&client->dev,
            "Failed to allocate memory for private data!\n");
        return -ENOMEM;
    }

    mutex_init(&priv->reg_lock);
    mutex_init(&priv->cfg_lock);

    priv->power_en                         = 0;
    priv->db_camera_op.switch_work_channel = gc0308db_switch_work_channel;
    priv->db_camera_op.cur_channel         = 1;
    priv->cfmt_code                        = V4L2_MBUS_FMT_YUYV8_2X8;

    priv->w_balance_mode                   = V4L2_WHITE_BALANCE_AUTO;
    priv->exposure_mode                    = V4L2_EXPOSURE_AUTO;
    priv->flag_vflip                       = 0;
    priv->flag_hflip                       = 0;

    priv->win = gc0308db_select_win(&default_wight, &default_height);

    priv->db_camera_op.subdev.ctrl_handler = &priv->ctrl_handler;

    v4l2_i2c_subdev_init(&priv->db_camera_op.subdev, client, &gc0308db_subdev_ops);

    ret = gc0308db_video_probe(client);
    if (ret < 0) {
        kfree(priv);
        return -1;
    }

    priv->gc0308db_work_queue = create_singlethread_workqueue("work_queue");
    if (priv->gc0308db_work_queue == NULL) {
        kfree(priv);
        dev_err(&client->dev,"Failed to create workquque.\n");
        return -1;
    }
    gc0308db_ctrl_handler_init(priv);

    INIT_WORK(&priv->switch_channel_work, switch_channel_handle);
    INIT_WORK(&priv->resume_work, resume_handle);

    hrtimer_init(&priv->sensor_init_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    priv->sensor_init_hrtimer.function = fsync_sensor_init_hrtimer_handler;
    priv->sensor_init_delay = ktime_set(0, GC0308_FRAME_TIME_MS *1000 * 1000);  /* 40ms */

    init_completion(&priv->sensor_init_completion);

    dev_info(&client->dev, "Probe successed\n");
    return 0;
}

static int gc0308db_remove(struct i2c_client *client)
{
    struct gc0308db_priv *priv = to_gc0308db(client);

    hrtimer_try_to_cancel(&priv->sensor_init_hrtimer);
    v4l2_ctrl_handler_free(&priv->ctrl_handler);
    destroy_workqueue(priv->gc0308db_work_queue);
    kfree(priv);

    return 0;
}

static int gc0308db_suspend(struct i2c_client *client, pm_message_t mesg)
{

    struct gc0308db_priv *priv = to_gc0308db(client);

    gc0308db_stream_off(priv);

    return 0;
}


static int gc0308db_resume(struct i2c_client *client)
{
    int ret;
    struct gc0308db_priv *priv = to_gc0308db(client);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    if (!priv->power_en) {
        return 0;
    }

    ret = soc_camera_set_power(&client->dev, ssdd, priv->power_en);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to set power.\n");
        return -1;
    }

    queue_work(priv->gc0308db_work_queue, &priv->resume_work);

    return 0;
}

static const struct i2c_device_id gc0308db_id[] = {
    { "gc0308_double",  0 },
    { }
};


MODULE_DEVICE_TABLE(i2c, gc0308db_id);

static struct i2c_driver gc0308db_i2c_driver = {
    .driver = {
        .name = "gc0308_double",
    },
    .suspend  = gc0308db_suspend,
    .resume   = gc0308db_resume,
    .probe    = gc0308db_probe,
    .remove   = gc0308db_remove,
    .id_table = gc0308db_id,
};

/*
 * Module functions
 */
static int __init gc0308db_module_init(void)
{
    return i2c_add_driver(&gc0308db_i2c_driver);
}

static void __exit gc0308db_module_exit(void)
{
    i2c_del_driver(&gc0308db_i2c_driver);
}

module_init(gc0308db_module_init);
module_exit(gc0308db_module_exit);

MODULE_DESCRIPTION("double gc0308 camera sensor driver");
MODULE_AUTHOR("Monk <rongjin.su@ingenic.com>");
MODULE_LICENSE("GPL");
