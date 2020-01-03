/*
 * gc0308 Camera Driver
 *
 * Copyright (C) 2018, Ingenic Semiconductor Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

//#define DEBUG 1
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

#define DEFAULT_WIDTH                   640
#define DEFAULT_HEIGHT                  480

#define REG_PAGE                        0xfe

#define CHIP_ID_GC0308                  0x9b

#define REG_HFLIP_IMG                   0x01 /* Horizontal mirror image reg bit */
#define REG_VFLIP_IMG                   0x02 /* Vertical flip image reg bit */

/*
 * Struct
 */
struct regval_list {
    unsigned char reg_num;
    unsigned char value;
};

/* Supported resolutions */
enum gc0308_width {
    W_QCIF  = 176,
    W_QVGA  = 320,
    W_CIF   = 352,
    W_VGA   = 640,
};

enum gc0308_height {
    H_QCIF  = 144,
    H_QVGA  = 240,
    H_CIF   = 288,
    H_VGA   = 480,
};

struct gc0308_win_size {
    char *name;
    enum gc0308_width width;
    enum gc0308_height height;
    const struct regval_list *regs;
};


struct gc0308_priv {
    struct v4l2_subdev subdev;
    struct v4l2_ctrl_handler ctrl_handler;
    enum v4l2_mbus_pixelcode cfmt_code;
    struct v4l2_mbus_framefmt mf;
    const struct gc0308_win_size *win;

    unsigned char power_en;
    unsigned int  exposure;
    unsigned char exposure_mode;
    unsigned char flag_vflip:1;
    unsigned char flag_hflip:1;

    struct work_struct resume_work;
};

static inline int gc0308_write_reg(struct i2c_client * client, unsigned char addr, unsigned char value)
{
    return i2c_smbus_write_byte_data(client, addr, value);
}
static inline char gc0308_read_reg(struct i2c_client *client, unsigned char addr)
{
    return i2c_smbus_read_byte_data(client, addr);;
}
/*
 * Registers settings
 */

#define ENDMARKER { 0xff, 0xff }

static const struct regval_list gc0308_init_regs[] = {
    {0xfe, 0x80},
    {0xfe, 0x00},   // set page0
    {0xd2, 0x10},   // close AEC
    {0x22, 0x55},   // close AWB

    {0x03, 0x01},
    {0x04, 0x2c},
    {0x5a, 0x56},
    {0x5b, 0x40},
    {0x5c, 0x4a},

    {0x22, 0x57},   // Open AWB

    {0xfe, 0x00},
    {0x01, 0x6a},
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
    {0x10, 0x22},
    {0x11, 0xfd},
    {0x12, 0x2a},
    {0x13, 0x00},

    {0x15, 0x0a},
    {0x16, 0x05},
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
#if defined(CONFIG_FMT_Y8_1X8)
    {0x24, 0xb1}, // only Y
#elif defined(CONFIG_FMT_YUYV8_2X8)
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
    {0x67, 0x86},
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

    {0x14, 0x10},
    //{0x25, 0x0f}, // enable output
    ENDMARKER,
};


static const struct regval_list gc0308_qcif_regs[] = {
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

static const struct regval_list gc0308_qvga_regs[] = {
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

static const struct regval_list gc0308_cif_regs[] = {
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

static const struct regval_list gc0308_vga_regs[] = {
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


#define GC0308_SIZE(n, w, h, r) \
    {.name = n, .width = w , .height = h, .regs = r }

static const struct gc0308_win_size gc0308_supported_win_sizes[] = {
    GC0308_SIZE("QCIF", W_QCIF, H_QCIF, gc0308_qcif_regs),
    GC0308_SIZE("QVGA", W_QVGA, H_QVGA, gc0308_qvga_regs),
    GC0308_SIZE("CIF", W_CIF, H_CIF, gc0308_cif_regs),
    GC0308_SIZE("VGA", W_VGA, H_VGA, gc0308_vga_regs),
};

static const struct regval_list gc0308_enable_output_regs[] = {
    {0xfe, 0x00},
    {0x25, 0x0f},
    ENDMARKER,
};

static const struct regval_list gc0308_disable_output_regs[] = {
    {0xfe, 0x00},
    {0x25, 0x00},
    ENDMARKER,
};

static struct regval_list gc0308_mirror_flip_regs[] = {
    {0xfe, 0x00},
    {0x14, 0x10},
    ENDMARKER,
};

static struct regval_list gc0308_chip_id_regs[] = {
    {0xfe, 0x00},
    {0x00, 0x00},
    ENDMARKER,
};

static const struct regval_list gc0308_enable_auto_exposure_regs[] = {
    {0xfe, 0x00},
    {0xd2, 0x90},
    ENDMARKER,
};

static const struct regval_list gc0308_disable_auto_exposure_regs[] = {
    {0xfe, 0x00},
    {0xd2, 0x10},
    ENDMARKER,
};

static struct regval_list gc0308_set_exposure_time_regs[] = {
    {0xfe, 0x00},
    {0x03, 0x00},
    {0x04, 0x00},
    ENDMARKER,
};

static enum v4l2_mbus_pixelcode gc0308_codes[] = {
    V4L2_MBUS_FMT_Y8_1X8,
    V4L2_MBUS_FMT_YUYV8_2X8,
    V4L2_MBUS_FMT_YUYV8_1_5X8,
    V4L2_MBUS_FMT_JZYUYV8_1_5X8,
};


/*
 * General functions
 */
static struct gc0308_priv *to_gc0308(const struct i2c_client *client)
{
    return container_of(i2c_get_clientdata(client), struct gc0308_priv,
                subdev);
}

static int gc0308_write_array(struct i2c_client *client,
                  const struct regval_list *vals)
{
    int ret;

    while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
        dev_vdbg(&client->dev, "write array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
        ret = gc0308_write_reg(client, vals->reg_num, vals->value);
        if (ret < 0) {
            return ret;
        }
        vals++;
    }
    return 0;
}

static int gc0308_read_array(struct i2c_client  *client,
                               struct regval_list *vals)
{
    int ret;

    while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
        if (vals->reg_num == REG_PAGE) {
            dev_vdbg(&client->dev, "write array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
            ret = gc0308_write_reg(client,vals->reg_num, vals->value);
            if (ret < 0) {
                return ret;
            }
        } else {
            vals->value = gc0308_read_reg(client,vals->reg_num);
            dev_vdbg(&client->dev, "read array: 0x%02x, 0x%02x",vals->reg_num, vals->value);
        }
        vals++;
    }

    return 0;
}

static void gc0308_stream_on(struct gc0308_priv* priv)
{
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->subdev);

    gc0308_write_array(client, gc0308_enable_output_regs);
}

static void gc0308_stream_off(struct gc0308_priv* priv)
{
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->subdev);

    gc0308_write_array(client, gc0308_disable_output_regs);
}



/*
 * soc_camera_ops functions
 */
static int gc0308_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0308_priv *priv = to_gc0308(client);

    if (enable) {
        gc0308_stream_on(priv);
    } else {
        gc0308_stream_off(priv);
    }
    return 0;
}


static int gc0308_g_chip_ident(struct v4l2_subdev *sd,
                       struct v4l2_dbg_chip_ident *id)
{
    return CHIP_ID_GC0308;
}

static int s_ctrl_auto_exposure_handle(struct i2c_client *client, int mode)
{
    int ret;

    switch(mode) {
    case V4L2_EXPOSURE_AUTO:
        ret = gc0308_write_array(client, gc0308_enable_auto_exposure_regs);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to write enable_auto_exposure regs\n");
            return -1;
        }
        break;

    case V4L2_EXPOSURE_MANUAL:
        ret = gc0308_write_array(client, gc0308_disable_auto_exposure_regs);
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

static int s_ctrl_exposure_handle(struct i2c_client *client, int value)
{
    int ret;
    struct gc0308_priv *priv = to_gc0308(client);
    int exposure = value & 0xfff;

    if (priv->exposure_mode != V4L2_EXPOSURE_MANUAL){
        dev_err(&client->dev,"Not in manual mode.\n");
        return -1;
    }

    gc0308_set_exposure_time_regs[1].value = (unsigned char)(0x0f   & (exposure>>8));
    gc0308_set_exposure_time_regs[2].value = (unsigned char)(0x00ff & exposure);

    ret = gc0308_write_array(client, gc0308_set_exposure_time_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write set_exposure_time regs\n");
        return -1;
    }

    return 0;
}

static int s_ctrl_mirror_vflip(struct i2c_client *client, int value)
{
    int ret,val = 0;

    val = value ? REG_VFLIP_IMG : 0;

#if 1
    msleep(30);
    ret = gc0308_read_array(client, gc0308_mirror_flip_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read mirror regs\n");
        return -1;
    }
    gc0308_mirror_flip_regs[1].value |= 0x10;
#endif

    gc0308_mirror_flip_regs[1].value &= ~REG_VFLIP_IMG;
    gc0308_mirror_flip_regs[1].value |= val;
    ret = gc0308_write_array(client, gc0308_mirror_flip_regs);
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

#if 1
    msleep(30);
    ret = gc0308_read_array(client, gc0308_mirror_flip_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read mirror regs\n");
        return -1;
    }
    gc0308_mirror_flip_regs[1].value |= 0x10;
#endif

    gc0308_mirror_flip_regs[1].value &= ~REG_HFLIP_IMG;
    gc0308_mirror_flip_regs[1].value |= val;
    ret = gc0308_write_array(client, gc0308_mirror_flip_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to write mirror regs\n");
        return -1;
    }

    return 0;
}


static int gc0308_s_ctrl_do(struct i2c_client *client, struct v4l2_control *ctrl)
{
    int ret = 0;

    switch (ctrl->id) {
    case V4L2_CID_EXPOSURE_AUTO:
        dev_err(&client->dev,"V4L2_CID_EXPOSURE_AUTO.\n");
        ret = s_ctrl_auto_exposure_handle(client, ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set exposure mode.\n");
        }
        break;

    case V4L2_CID_EXPOSURE:
        dev_err(&client->dev,"V4L2_CID_EXPOSURE.\n");
        ret = s_ctrl_exposure_handle(client,ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set exposure\n");
        }
        break;

    case V4L2_CID_VFLIP:
        dev_err(&client->dev,"V4L2_CID_VFLIP.\n");
        ret = s_ctrl_mirror_vflip(client, ctrl->value);
        if (ret < 0) {
            dev_err(&client->dev,"Failed to set h mirror.\n");
        }
        break;

    case V4L2_CID_HFLIP:
        dev_err(&client->dev,"V4L2_CID_HFLIP.\n");
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

static void set_ctrl_mode(struct gc0308_priv *priv, unsigned int cid, unsigned int mode)
{
    switch (cid) {
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

static int gc0308_handler_s_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret;
    struct v4l2_control control;
    struct gc0308_priv *priv   = container_of(ctrl->handler,
                                 struct gc0308_priv, ctrl_handler);
    struct v4l2_subdev *sd     = &priv->subdev;
    struct i2c_client  *client = v4l2_get_subdevdata(sd);

    dev_info(&client->dev, "Set ctrl\n");

    control.id    = ctrl->id;
    control.value = ctrl->val;
    ret = gc0308_s_ctrl_do(client,&control);
    if (ret < 0) {
        dev_err(&client->dev, "Set ctrl error!\n");
        return -1;
    }
    set_ctrl_mode(priv, control.id, control.value);
    return 0;
}

static int gc0308_g_volatile_ctrl_do(struct i2c_client *client, struct v4l2_control *ctrl)
{
    int ret = 0;

    switch (ctrl->id) {
    default:
        dev_err(&client->dev, "Has no V4L2 CID: 0x%x ", ctrl->id);
        return -EINVAL;
    }

    return ret;
}

static int gc0308_handler_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
    int ret;
    struct gc0308_priv *priv   = container_of(ctrl->handler, struct gc0308_priv, ctrl_handler);
    struct v4l2_subdev *sd       = &priv->subdev;
    struct i2c_client  *client   = v4l2_get_subdevdata(sd);
    struct v4l2_control control;

    control.id = ctrl->id;

    ret = gc0308_g_volatile_ctrl_do(client, &control);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to g ctrl\n");
        return -1;
    }
    ctrl->val = control.value;
    return 0;
}


/* Select the nearest higher resolution for capture */
static const struct gc0308_win_size *gc0308_select_win(u32 *width, u32 *height)
{
    int i, default_size = ARRAY_SIZE(gc0308_supported_win_sizes) - 1;

    for (i = 0; i < ARRAY_SIZE(gc0308_supported_win_sizes); i++) {
        if (gc0308_supported_win_sizes[i].width  >= *width &&
            gc0308_supported_win_sizes[i].height >= *height) {
            *width = gc0308_supported_win_sizes[i].width;
            *height = gc0308_supported_win_sizes[i].height;
            return &gc0308_supported_win_sizes[i];
        }
    }

    *width = gc0308_supported_win_sizes[default_size].width;
    *height = gc0308_supported_win_sizes[default_size].height;
    return &gc0308_supported_win_sizes[default_size];
}

static int gc0308_sensor_init(struct v4l2_subdev *sd)
{
    int ret;
    struct i2c_client  *client = v4l2_get_subdevdata(sd);

    /* initialize the sensor with default data */
    ret = gc0308_write_array(client, gc0308_init_regs);
    if (ret < 0) {
        dev_err(&client->dev, "%s: Error %d", __func__, ret);
        return -1;
    }

    dev_info(&client->dev, "%s: Init default", __func__);
    return 0;
}


static int gc0308_g_fmt(struct v4l2_subdev *sd,
                 struct v4l2_mbus_framefmt *mf)
{
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0308_priv *priv   = to_gc0308(client);

    mf->width      = priv->win->width;
    mf->height     = priv->win->height;
    mf->code       = priv->cfmt_code;

    mf->colorspace = V4L2_COLORSPACE_JPEG;
    mf->field      = V4L2_FIELD_NONE;

    return 0;
}

static int gc0308_s_fmt(struct v4l2_subdev *sd,
                 struct v4l2_mbus_framefmt *mf)
{
    /* current do not support set format, use unify format yuv422i */
    int ret;
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct gc0308_priv *priv   = to_gc0308(client);

    priv->win = gc0308_select_win(&mf->width, &mf->height);
    /* set size win */
    ret = gc0308_write_array(client, priv->win->regs);
    if (ret < 0) {
        dev_err(&client->dev, "%s: Error\n", __func__);
        return ret;
    }

    memcpy(&priv->mf, mf, sizeof(struct v4l2_mbus_framefmt));
    return 0;
}

static int gc0308_try_fmt(struct v4l2_subdev *sd,
                   struct v4l2_mbus_framefmt *mf)
{
    const struct gc0308_win_size *win;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    /*
     * select suitable win
     */
    win = gc0308_select_win(&mf->width, &mf->height);

    if(mf->field == V4L2_FIELD_ANY) {
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

static int gc0308_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
                                       enum v4l2_mbus_pixelcode *code)
{
    if (index >= ARRAY_SIZE(gc0308_codes))
        return -EINVAL;

    *code = gc0308_codes[index];
    return 0;
}


static int gc0308_video_probe(struct i2c_client *client)
{
    int ret = 0;

    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    ret = soc_camera_power_on(&client->dev, ssdd);
    if (ret < 0) {
        return ret;
    }

    /*
     * check and show product ID and manufacturer ID
     */
    ret = gc0308_read_array(client, gc0308_chip_id_regs);
    if (ret < 0) {
        dev_err(&client->dev,"Failed to read chip id\n");
        return ret;
    }

    if (gc0308_chip_id_regs[1].value != CHIP_ID_GC0308) {
        dev_err(&client->dev, "read sensor %s chip_id is error\n",
                client->name);
        return -1;
    }

    dev_info(&client->dev, "read sensor %s id : 0x%x, successed!\n",
             client->name, gc0308_chip_id_regs[1].value);

    ret = soc_camera_power_off(&client->dev, ssdd);

    return 0;
}

static int gc0308_s_power(struct v4l2_subdev *sd, int on)
{
    int ret;
    struct v4l2_ctrl* ctrl;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct gc0308_priv *priv   = to_gc0308(client);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    dev_info(&client->dev,"s_power %s\n",on?"on":"off");

    ret = soc_camera_set_power(&client->dev, ssdd, on);

    if (on) {
        ret = gc0308_sensor_init(sd);
        if (ret < 0) {
            dev_err(&client->dev, "Failed to init sensor.\n");
            soc_camera_set_power(&client->dev, ssdd, !on);
            return -1;
        }
    } else {
        ctrl = v4l2_ctrl_find(priv->subdev.ctrl_handler,
                              V4L2_CID_EXPOSURE_AUTO);
        if (ctrl) {
            priv->exposure_mode = V4L2_EXPOSURE_AUTO;
            ctrl->cur.val       = V4L2_EXPOSURE_AUTO;
        }

        ctrl = v4l2_ctrl_find(priv->subdev.ctrl_handler,
                              V4L2_CID_EXPOSURE);
        if (ctrl) {
            priv->exposure      = 0;
            ctrl->cur.val       = 0;
        }

        ctrl = v4l2_ctrl_find(priv->subdev.ctrl_handler,
                              V4L2_CID_VFLIP);
        if (ctrl) {
            priv->flag_vflip     = 0;
            ctrl->cur.val        = 0;
        }

        ctrl = v4l2_ctrl_find(priv->subdev.ctrl_handler,
                              V4L2_CID_HFLIP);
        if (ctrl) {
            priv->flag_hflip     = 0;
            ctrl->cur.val        = 0;
        }
    }
    priv->power_en = on;
    return 0;
}

static int gc0308_g_mbus_config(struct v4l2_subdev *sd,
                          struct v4l2_mbus_config *cfg)
{

    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    cfg->flags = V4L2_MBUS_PCLK_SAMPLE_RISING | V4L2_MBUS_MASTER |
                 V4L2_MBUS_VSYNC_ACTIVE_HIGH  | V4L2_MBUS_HSYNC_ACTIVE_HIGH |
                 V4L2_MBUS_DATA_ACTIVE_HIGH;
    cfg->type  = V4L2_MBUS_PARALLEL;

    cfg->flags = soc_camera_apply_board_flags(ssdd, cfg);

    return 0;
}

static struct v4l2_subdev_core_ops gc0308_subdev_core_ops = {
    .s_power      = gc0308_s_power,
    .g_chip_ident = gc0308_g_chip_ident,
};

static struct v4l2_subdev_video_ops gc0308_subdev_video_ops = {
    .s_stream      = gc0308_s_stream,
    .g_mbus_fmt    = gc0308_g_fmt,
    .s_mbus_fmt    = gc0308_s_fmt,
    .try_mbus_fmt  = gc0308_try_fmt,
    .enum_mbus_fmt = gc0308_enum_fmt,
    .g_mbus_config = gc0308_g_mbus_config,
};

static struct v4l2_subdev_ops gc0308_subdev_ops = {
    .core   = &gc0308_subdev_core_ops,
    .video  = &gc0308_subdev_video_ops,
};


static struct v4l2_ctrl_ops gc0308_v4l2_ctrl_ops = {
    .g_volatile_ctrl = gc0308_handler_g_volatile_ctrl,
    .s_ctrl          = gc0308_handler_s_ctrl,
};

static int gc0308_ctrl_handler_init(struct gc0308_priv *priv)
{
    struct v4l2_ctrl* ctrl;
    struct i2c_client *client = v4l2_get_subdevdata(&priv->subdev);

    v4l2_ctrl_handler_init(priv->subdev.ctrl_handler, 16);

    ctrl = v4l2_ctrl_new_std(priv->subdev.ctrl_handler,
                      &gc0308_v4l2_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_HFLIP\n");
    }

    ctrl = v4l2_ctrl_new_std(priv->subdev.ctrl_handler,
                      &gc0308_v4l2_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_VFLIP\n");
    }

    ctrl = v4l2_ctrl_new_std_menu(priv->subdev.ctrl_handler,
                      &gc0308_v4l2_ctrl_ops, V4L2_CID_EXPOSURE_AUTO,
                      V4L2_EXPOSURE_APERTURE_PRIORITY, 0, V4L2_EXPOSURE_AUTO);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_EXPOSURE_AUTO\n");
    }

    ctrl = v4l2_ctrl_new_std(priv->subdev.ctrl_handler,
                      &gc0308_v4l2_ctrl_ops, V4L2_CID_EXPOSURE, 0, 4096, 1, 0);
    if (!ctrl) {
        dev_err(&client->dev, "Failed to add ctrl: V4L2_CID_EXPOSURE\n");
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
    struct gc0308_priv* priv =
                    container_of(data, struct gc0308_priv, resume_work);
    struct i2c_client  *client = v4l2_get_subdevdata(&priv->subdev);

    ret = gc0308_sensor_init(&priv->subdev);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to init sensor.\n");
    }

    ret = gc0308_s_fmt(&priv->subdev, &priv->mf);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to s sensor fmt.\n");
    }

    ctrl.handler = &priv->ctrl_handler;

    if (priv->exposure_mode != V4L2_EXPOSURE_AUTO) {
        ctrl.id  = V4L2_CID_EXPOSURE_AUTO;
        ctrl.val = priv->exposure_mode;
        gc0308_handler_s_ctrl(&ctrl);
        if (priv->exposure_mode == V4L2_EXPOSURE_MANUAL) {
            ctrl.id  = V4L2_CID_EXPOSURE;
            ctrl.val = priv->exposure;
            gc0308_handler_s_ctrl(&ctrl);
        }
    }

    if (priv->flag_vflip) {
        ctrl.id  = V4L2_CID_VFLIP;
        ctrl.val = priv->flag_vflip;
        gc0308_handler_s_ctrl(&ctrl);
    }

    if (priv->flag_hflip) {
        ctrl.id  = V4L2_CID_HFLIP;
        ctrl.val = priv->flag_hflip;
        gc0308_handler_s_ctrl(&ctrl);
    }

    gc0308_stream_on(priv);
}



static int gc0308_probe(struct i2c_client *client,
                  const struct i2c_device_id *did)
{
    int ret;
    struct gc0308_priv *priv;
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
    struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
    int default_wight  = DEFAULT_WIDTH;
    int default_height = DEFAULT_HEIGHT;

    dev_info(&client->dev, "Probe ...\n");

    if (!ssdd) {
        dev_err(&client->dev, "gc0308: missing platform data!\n");
        return -EINVAL;
    }

    if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE
            | I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&client->dev, "client not i2c capable\n");
        return -ENODEV;
    }

    priv = kzalloc(sizeof(struct gc0308_priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&adapter->dev,
            "Failed to allocate memory for private data!\n");
        return -ENOMEM;
    }

    priv->win                 = gc0308_select_win(&default_wight, &default_height);
#if defined(CONFIG_FMT_Y8_1X8)
    priv->cfmt_code           = V4L2_MBUS_FMT_Y8_1X8; //only y
#elif defined(CONFIG_FMT_YUYV8_2X8)
    priv->cfmt_code           = V4L2_MBUS_FMT_YUYV8_2X8;
#endif
    priv->subdev.ctrl_handler = &priv->ctrl_handler;

    priv->power_en            = 0;
    priv->exposure_mode       = V4L2_EXPOSURE_AUTO;
    priv->flag_vflip          = 0;
    priv->flag_hflip          = 0;

    ret = gc0308_video_probe(client);
    if (ret) {
        kfree(priv);
        return -1;
    }

    gc0308_ctrl_handler_init(priv);
    v4l2_i2c_subdev_init(&priv->subdev, client, &gc0308_subdev_ops);

    INIT_WORK(&priv->resume_work, resume_handle);

    dev_info(&client->dev, "Probe successed\n");
    return 0;
}

static int gc0308_remove(struct i2c_client *client)
{
    struct gc0308_priv *priv = to_gc0308(client);

    v4l2_ctrl_handler_free(&priv->ctrl_handler);
    kfree(priv);

    return 0;
}

static int gc0308_suspend(struct i2c_client *client, pm_message_t mesg)
{
    return 0;
}

static int gc0308_resume(struct i2c_client *client)
{
    int ret;
    struct gc0308_priv *priv = to_gc0308(client);
    struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

    if (!priv->power_en) {
        return 0;
    }

    ret = soc_camera_set_power(&client->dev, ssdd, priv->power_en);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to set power.\n");
        return -1;
    }

    schedule_work(&priv->resume_work);

    return 0;
}

static const struct i2c_device_id gc0308_id[] = {
    { "gc0308",  0 },
    { }
};


MODULE_DEVICE_TABLE(i2c, gc0308_id);

static struct i2c_driver gc0308_i2c_driver = {
    .driver = {
        .name = "gc0308",
    },
    .suspend  = gc0308_suspend,
    .resume   = gc0308_resume,
    .probe    = gc0308_probe,
    .remove   = gc0308_remove,
    .id_table = gc0308_id,
};

/*
 * Module functions
 */
static int __init gc0308_module_init(void)
{
    return i2c_add_driver(&gc0308_i2c_driver);
}

static void __exit gc0308_module_exit(void)
{
    i2c_del_driver(&gc0308_i2c_driver);
}

module_init(gc0308_module_init);
module_exit(gc0308_module_exit);

MODULE_DESCRIPTION("camera sensor gc0308 driver");
MODULE_AUTHOR("qipengzhen <aric.pzqi@ingenic.com>,Monk <rongjin.su@ingenic.com>");
MODULE_LICENSE("GPL");
