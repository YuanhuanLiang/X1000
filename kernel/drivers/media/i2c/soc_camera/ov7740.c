/*
 * V4L2 Driver for camera sensor ov7740
 *
 * Copyright (C) 2012, Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * fix by xyfu@ingenic.com
 *
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
#include <mach/jz_camera.h>

#define REG_CHIP_ID_HIGH		0x0a
#define REG_CHIP_ID_LOW			0x0b
//#define REG_CHIP_REVISION		0x302a
#define CHIP_ID_HIGH			0x77
#define CHIP_ID_LOW				0x42 //0x40

//#define  ov7740_DEFAULT_WIDTH    320
//#define  ov7740_DEFAULT_HEIGHT   240


/* Private v4l2 controls */
#define V4L2_CID_PRIVATE_BALANCE  (V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_PRIVATE_EFFECT  (V4L2_CID_PRIVATE_BASE + 1)

/* In flip, the OV7740 does not need additional settings because the ISP block
 * will auto-detect whether the pixel is in the red line or blue line and make
 * the necessary adjustments.
 */
//#define REG_TC_VFLIP			0x3820
//#define REG_TC_MIRROR			0x3821
//#define OV7740_HFLIP			0x1
//#define OV7740_VFLIP			0x2
//#define OV7740_FLIP_VAL			((unsigned char)0x06)
//#define OV7740_FLIP_MASK		((unsigned char)0x06)

#define REG0C 0x0C /* Register 0C */
#define REG0C_HFLIP_IMG 0x40 /* Horizontal mirror image ON/OFF */
#define REG0C_VFLIP_IMG 0x80 /* Vertical flip image ON/OFF */

 /* whether sensor support high resolution (> vga) preview or not */
#define SUPPORT_HIGH_RESOLUTION_PRE		1

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
enum ov7740_width {
	W_VGA  = 640,
	W_QVGA = 320,
};

enum ov7740_height {
	H_VGA  = 480,
	H_QVGA = 240,
};

struct ov7740_win_size {
	char *name;
	enum ov7740_width width;
	enum ov7740_height height;
	const struct regval_list *regs;
};


struct ov7740_priv {
	struct v4l2_subdev		subdev;
	struct ov7740_camera_info	*info;
	enum v4l2_mbus_pixelcode	cfmt_code;
	struct ov7740_win_size	*win;
	int				model;
	u8				balance_value;
	u8				effect_value;
	u16				flag_vflip:1;
	u16				flag_hflip:1;
};

int cam_t_j = 0, cam_t_i = 0;
unsigned long long cam_t0_buf[10];
unsigned long long cam_t1_buf[10];
static int ov7740_s_power(struct v4l2_subdev *sd, int on);

static inline int ov7740_write_reg(struct i2c_client * client, unsigned char addr, unsigned char value)
{
	return i2c_smbus_write_byte_data(client, addr, value);
}
static inline char ov7740_read_reg(struct i2c_client *client, unsigned char addr)
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

static const struct regval_list ov7740_init_regs[] = {
#if 1
	//{0x12, 0x80},
	{0x13, 0x00},
	{0x11, 0x00},
	{0x12, 0x00},

	{0x11, 0x01},
	{0x0c, 0x12},
	{0xd5, 0x10},
	{0x0d, 0x34},
	{0x17, 0x25},
	{0x18, 0xa0},
	{0x19, 0x03},
	{0x1a, 0xf0},
	{0x1b, 0x89},
	{0x1e, 0x13},
	{0x22, 0x03},
	{0x29, 0x17},
	{0x2b, 0xf8},
	{0x2c, 0x01},

	//win_size 640 * 480
	{0x31, 0xa0},
	{0x32, 0xf0},
	{0x34, 0x00},

	{0x33, 0xc4},
	{0x3a, 0xb4},
	{0x36, 0x3f},
	{0x04, 0x60},
	{0x27, 0x80},
	{0x3d, 0x08},
	{0x3e, 0x82},
	{0x3f, 0x40},
	{0x40, 0x7f},
	{0x41, 0x6a},
	{0x42, 0x29},
	{0x44, 0xf5},
	{0x45, 0x41},
	{0x47, 0x42},
	{0x48, 0x00},
	{0x49, 0x61},
	{0x4a, 0xa1},
	{0x4b, 0x46},
	{0x4c, 0x18},
	{0x4d, 0x50},
	{0x4e, 0x13},
	{0x64, 0x00},
	{0x67, 0x88},
	{0x68, 0x1a},

	{0x14, 0x30}, //38
	{0x24, 0x78}, //3c
	{0x25, 0x68}, //30
	{0x26, 0xd4}, //72
	{0x50, 0xCA},
	{0x51, 0xA8},
	{0x52, 0x10},
	{0x53, 0x00},
	{0x20, 0x00},
	{0x21, 0x03},
	{0x38, 0x14},
	{0xe9, 0x00},
	{0x56, 0x55},
	{0x57, 0xff},
	{0x58, 0xff},
	{0x59, 0xff},
	{0x5f, 0x04},
	{0x13, 0xff},

	{0x80, 0x7d},
	{0x81, 0x3f},
	{0x82, 0x32},
	{0x83, 0x03},
	{0x38, 0x11},
	{0x84, 0x70},
	{0x85, 0x00},
	{0x86, 0x03},
	{0x87, 0x01},
	{0x88, 0x05},
	{0x89, 0x30},
	{0x8d, 0x30},
	{0x8f, 0x85},
	{0x93, 0x30},
	{0x95, 0x85},
	{0x99, 0x30},
	{0x9b, 0x85},

	{0x9c, 0x05}, //08
	{0x9d, 0x0F}, //12
	{0x9e, 0x20}, //23
	{0x9f, 0x43}, //45
	{0xa0, 0x54}, //55
	{0xa1, 0x63}, //64
	{0xa2, 0x72},
	{0xa3, 0x7f},
	{0xa4, 0x8b},
	{0xa5, 0x96}, //95
	{0xa6, 0xA9}, //a7
	{0xa7, 0xB8}, //b5
	{0xa8, 0xCF}, //cb
	{0xa9, 0xE3}, //dd
	{0xaa, 0xF4}, //ec
	{0xab, 0x10}, //1a

	{0xce, 0x78},
	{0xcf, 0x6e},
	{0xd0, 0x0a},
	{0xd1, 0x0c},
	{0xd2, 0x84},
	{0xd3, 0x90},
	{0xd4, 0x1e},

	{0x5a, 0x24},
	{0x5b, 0x1f},
	{0x5c, 0x88},
	{0x5d, 0x60},

	{0xac, 0x6e},
	{0xbe, 0xff},
	{0xbf, 0x00},

	{0x70, 0x00},
	{0x71, 0x34},
	{0x74, 0x28},
	{0x75, 0x98},
	{0x76, 0x00},
	{0x77, 0x08},
	{0x78, 0x01},
	{0x79, 0xc2},
	{0x7d, 0x02},
	{0x7a, 0x9c},
	{0x7b, 0x40},
	{0xec, 0x82},
	{0x7c, 0x0c},
	ENDMARKER,
#else
	//test pattern
	//{0x12, 0x80},
	{0x1B, 0x80},
	{0x29, 0x18},
	{0x38, 0x07},
	{0x84, 0x12},
	{0x38, 0x08},
	{0x84, 0x05},

	/* CLK = XVCLK1/2 */
	{0x11, 0x01},
	{0x36, 0x3f},

	{0x0c, 0x12},
	{0x12, 0x00},

	/* AGC - auto gain control */
	{0x86, 0x02},
	ENDMARKER,
#endif
};

static const struct regval_list ov7740_qvga_regs[] = {
	{0x31, 0x50},// win_size 320 * 240
	{0x32, 0x78},
	{0x34, 0x00},
	ENDMARKER,
};

static const struct regval_list ov7740_vga_regs[] = {
/* Initial registers is for vga format, no setting needed */
	{0x31, 0xa0},//win_size 640 * 480
	{0x32, 0xf0},
	{0x34, 0x00},
	ENDMARKER,
};

static const struct regval_list ov7740_wb_auto_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_wb_incandescence_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_wb_daylight_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_wb_fluorescent_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_wb_cloud_regs[] = {
	ENDMARKER,
};

static const struct mode_list ov7740_balance[] = {
	{0, ov7740_wb_auto_regs}, {1, ov7740_wb_incandescence_regs},
	{2, ov7740_wb_daylight_regs}, {3, ov7740_wb_fluorescent_regs},
	{4, ov7740_wb_cloud_regs},
};


static const struct regval_list ov7740_effect_normal_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_effect_grayscale_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_effect_sepia_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_effect_colorinv_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_effect_sepiabluel_regs[] = {
	ENDMARKER,
};

static const struct mode_list ov7740_effect[] = {
	{0, ov7740_effect_normal_regs}, {1, ov7740_effect_grayscale_regs},
	{2, ov7740_effect_sepia_regs}, {3, ov7740_effect_colorinv_regs},
	{4, ov7740_effect_sepiabluel_regs},
};

#define OV7740_SIZE(n, w, h, r) \
	{.name = n, .width = w , .height = h, .regs = r }

static struct ov7740_win_size ov7740_supported_win_sizes[] = {
	OV7740_SIZE("VGA", W_VGA, H_VGA, ov7740_vga_regs),
	OV7740_SIZE("QVGA", W_QVGA, H_QVGA, ov7740_qvga_regs),
};

#define N_WIN_SIZES (ARRAY_SIZE(ov7740_supported_win_sizes))

static const struct regval_list ov7740_yuv422_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov7740_rgb565_regs[] = {
	ENDMARKER,
};

static enum v4l2_mbus_pixelcode ov7740_codes[] = {
	V4L2_MBUS_FMT_YUYV8_2X8,
	//V4L2_MBUS_FMT_YUYV8_1_5X8,
	//V4L2_MBUS_FMT_JZYUYV8_1_5X8,
};

/*
 * Supported balance menus
 */
static const struct v4l2_querymenu ov7740_balance_menus[] = {
	{
		.id		= V4L2_CID_PRIVATE_BALANCE,
		.index		= 0,
		.name		= "auto",
	}, {
		.id		= V4L2_CID_PRIVATE_BALANCE,
		.index		= 1,
		.name		= "incandescent",
	}, {
		.id		= V4L2_CID_PRIVATE_BALANCE,
		.index		= 2,
		.name		= "fluorescent",
	},  {
		.id		= V4L2_CID_PRIVATE_BALANCE,
		.index		= 3,
		.name		= "daylight",
	},  {
		.id		= V4L2_CID_PRIVATE_BALANCE,
		.index		= 4,
		.name		= "cloudy-daylight",
	},

};

/*
 * Supported effect menus
 */
static const struct v4l2_querymenu ov7740_effect_menus[] = {
	{
		.id		= V4L2_CID_PRIVATE_EFFECT,
		.index		= 0,
		.name		= "none",
	}, {
		.id		= V4L2_CID_PRIVATE_EFFECT,
		.index		= 1,
		.name		= "mono",
	}, {
		.id		= V4L2_CID_PRIVATE_EFFECT,
		.index		= 2,
		.name		= "sepia",
	},  {
		.id		= V4L2_CID_PRIVATE_EFFECT,
		.index		= 3,
		.name		= "negative",
	}, {
		.id		= V4L2_CID_PRIVATE_EFFECT,
		.index		= 4,
		.name		= "aqua",
	},
};

/*
 * General functions
 */
static struct ov7740_priv *to_ov7740(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov7740_priv,
			    subdev);
}

static int ov7740_write_array(struct i2c_client *client, const struct regval_list *vals)
{
    int ret;
    while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
        ret = i2c_smbus_write_byte_data(client, vals->reg_num, vals->value);
        dev_vdbg(&client->dev, "array: 0x%02x, 0x%02x", vals->reg_num, vals->value);
        if (ret < 0) {
            dev_err(&client->dev, "array: 0x%02x, 0x%02x write failed",
            vals->reg_num, vals->value);
            return ret;
        }
        vals++;
    }
    return 0;
}

static int ov7740_mask_set(struct i2c_client *client,
			   u8  reg, u8  mask, u8  set)
{
	s32 val = ov7740_read_reg(client, reg);
	if (val < 0)
		return val;

	val &= ~mask;
	val |= set & mask;

	dev_vdbg(&client->dev, "masks: 0x%02x, 0x%02x", reg, val);

	return ov7740_write_reg(client, reg, val);
}

static int ov7740_reset(struct i2c_client *client)
{
    int ret;
    const struct regval_list reset_seq[] = {
        {0x12 ,0x80},
        ENDMARKER,
    };
    ret = ov7740_write_array(client, reset_seq);
    if (ret)
        goto err;
    msleep(5);
err:
    dev_dbg(&client->dev, "%s: (ret %d)", __func__, ret);
    return ret;
}

/*
 * soc_camera_ops functions
 */
static int ov7740_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
#if 0

	if (!enable ) {
		dev_info(&client->dev, "stream down\n");
		//ov7740_write_reg(client, 0x3008, 0x42);
		ov7740_write_reg(client, 0x4202, 0x0f);
		return 0;
	}

	dev_info(&client->dev, "stream on\n");
	//ov7740_write_reg(client, 0x3008, 0x02);
	ov7740_write_reg(client, 0x4202, 0x00);
    #endif
	dev_info(&client->dev, "-----%s----", __func__);
	return 0;
}

static int ov7740_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov7740_priv *priv = to_ov7740(client);
	dev_info(&client->dev, "-----%s----", __func__);

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

/* FIXME: Flip function should be update according to specific sensor */
static int ov7740_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov7740_priv *priv = to_ov7740(client);
	int ret = 0;
	int i = 0;
	u8 value;

	int balance_count = ARRAY_SIZE(ov7740_balance);
	int effect_count = ARRAY_SIZE(ov7740_effect);

	dev_info(&client->dev, "-----%s----", __func__);

	switch (ctrl->id) {
	case V4L2_CID_PRIVATE_BALANCE:
		if(ctrl->value > balance_count)
			return -EINVAL;

		for(i = 0; i < balance_count; i++) {
			if(ctrl->value == ov7740_balance[i].index) {
				ret = ov7740_write_array(client,
						ov7740_balance[ctrl->value].mode_regs);
				priv->balance_value = ctrl->value;
				break;
			}
		}
		break;

	case V4L2_CID_PRIVATE_EFFECT:
		if(ctrl->value > effect_count)
			return -EINVAL;

		for(i = 0; i < effect_count; i++) {
			if(ctrl->value == ov7740_effect[i].index) {
				ret = ov7740_write_array(client,
						ov7740_effect[ctrl->value].mode_regs);
				priv->effect_value = ctrl->value;
				break;
			}
		}
		break;

    case V4L2_CID_VFLIP:
        value = ctrl->value ? REG0C_VFLIP_IMG : 0x00;
        return ov7740_mask_set(client, REG0C, REG0C_VFLIP_IMG, value);
    case V4L2_CID_HFLIP:
        value = ctrl->value ? REG0C_HFLIP_IMG : 0x00;
        return ov7740_mask_set(client, REG0C, REG0C_HFLIP_IMG, value);

	default:
		dev_err(&client->dev, "no V4L2 CID: 0x%x ", ctrl->id);
		return -EINVAL;
	}

	return ret;
}

static int ov7740_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_info(&client->dev, "-----%s----", __func__);
	id->ident    = SUPPORT_HIGH_RESOLUTION_PRE;
	id->revision = 0;

	return 0;
}


static int ov7740_querymenu(struct v4l2_subdev *sd,
					struct v4l2_querymenu *qm)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_info(&client->dev, "-----%s----", __func__);
	switch (qm->id) {
	case V4L2_CID_PRIVATE_BALANCE:
		memcpy(qm->name, ov7740_balance_menus[qm->index].name,
				sizeof(qm->name));
		break;

	case V4L2_CID_PRIVATE_EFFECT:
		memcpy(qm->name, ov7740_effect_menus[qm->index].name,
				sizeof(qm->name));
		break;
	}

	return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov7740_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	reg->size = 1;
	if (reg->reg > 0xff)
		return -EINVAL;

	ret = ov7740_read_reg(client, reg->reg);
	if (ret < 0)
		return ret;

	reg->val = ret;

	return 0;
}

static int ov7740_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->reg > 0xff ||
	    reg->val > 0xff)
		return -EINVAL;

	return ov7740_write_reg(client, reg->reg, reg->val);
}
#endif

/* Select the nearest higher resolution for capture */
static struct ov7740_win_size *ov7740_select_win(u32 *width, u32 *height)
{
	int i, default_size = ARRAY_SIZE(ov7740_supported_win_sizes) - 1;

	for (i = 0; i < ARRAY_SIZE(ov7740_supported_win_sizes); i++) {
		if ((*width >= ov7740_supported_win_sizes[i].width) &&
		    (*height >= ov7740_supported_win_sizes[i].height)) {
			*width = ov7740_supported_win_sizes[i].width;
			*height = ov7740_supported_win_sizes[i].height;
			return &ov7740_supported_win_sizes[i];
		}
	}

	*width = ov7740_supported_win_sizes[default_size].width;
	*height = ov7740_supported_win_sizes[default_size].height;


	return &ov7740_supported_win_sizes[default_size];
}

static int ov7740_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov7740_priv *priv = to_ov7740(client);

	int bala_index = priv->balance_value;
	int effe_index = priv->effect_value;

	ov7740_reset(client);

	/* initialize the sensor with default data */
	ret = ov7740_write_array(client, ov7740_init_regs);
	ret = ov7740_write_array(client, ov7740_balance[bala_index].mode_regs);
	ret = ov7740_write_array(client, ov7740_effect[effe_index].mode_regs);
	if (ret < 0)
		goto err;

	dev_info(&client->dev, "%s: Init default", __func__);
	return 0;

err:
	dev_err(&client->dev, "%s: Error %d", __func__, ret);
	return ret;
}

static int ov7740_g_fmt(struct v4l2_subdev *sd,
			struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov7740_priv *priv = to_ov7740(client);
	dev_info(&client->dev, "-----%s----", __func__);

	mf->width = priv->win->width;
	mf->height = priv->win->height;
	mf->code = priv->cfmt_code;

	mf->colorspace = V4L2_COLORSPACE_JPEG;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int ov7740_s_fmt(struct v4l2_subdev *sd,
			struct v4l2_mbus_framefmt *mf)
{
	/* current do not support set format, use unify format yuv422i */
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov7740_priv *priv = to_ov7740(client);
	int ret;
	dev_info(&client->dev, "-----%s----", __func__);

	ov7740_init(sd, 1);

	priv->win = ov7740_select_win(&mf->width, &mf->height);
	/* set size win */
	ret = ov7740_write_array(client, priv->win->regs);
	if (ret < 0) {
		dev_err(&client->dev, "%s: Error\n", __func__);
		return ret;
	}
	return 0;
}

static int ov7740_try_fmt(struct v4l2_subdev *sd,
			  struct v4l2_mbus_framefmt *mf)
{
	const struct ov7740_win_size *win;
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	/*
	 * select suitable win
	 */
        dev_info(&client->dev, "-----%s----", __func__);
	win = ov7740_select_win(&mf->width, &mf->height);

	if(mf->field == V4L2_FIELD_ANY) {
		mf->field = V4L2_FIELD_NONE;
	} else if (mf->field != V4L2_FIELD_NONE) {
		dev_err(&client->dev, "Field type invalid.\n");
		return -ENODEV;
	}

	switch (mf->code) {
	case V4L2_MBUS_FMT_YUYV8_2X8:
	//case V4L2_MBUS_FMT_YUYV8_1_5X8:
	//case V4L2_MBUS_FMT_JZYUYV8_1_5X8:
		mf->colorspace = V4L2_COLORSPACE_JPEG;
		break;

	default:
		mf->code = V4L2_MBUS_FMT_YUYV8_2X8;
		mf->colorspace = V4L2_COLORSPACE_JPEG;
		break;
	}

	return 0;
}


static int ov7740_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_info(&client->dev, "-----%s----", __func__);
	if (index >= ARRAY_SIZE(ov7740_codes))
		return -EINVAL;

	*code = ov7740_codes[index];
	return 0;
}

static int ov7740_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	return 0;
}

static int ov7740_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	return 0;
}


/*
 * Frame intervals.  Since frame rates are controlled with the clock
 * divider, we can only do 30/n for integer n values.  So no continuous
 * or stepwise options.  Here we just pick a handful of logical values.
 */

static int ov7740_frame_rates[] = { 30, 15, 10, 5, 1 };

static int ov7740_enum_frameintervals(struct v4l2_subdev *sd,
		struct v4l2_frmivalenum *interval)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_info(&client->dev, "-----%s----", __func__);
	if (interval->index >= ARRAY_SIZE(ov7740_frame_rates))
		return -EINVAL;
	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	interval->discrete.numerator = 1;
	interval->discrete.denominator = ov7740_frame_rates[interval->index];
	return 0;
}


/*
 * Frame size enumeration
 */
static int ov7740_enum_framesizes(struct v4l2_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	int i;
	int num_valid = -1;
	__u32 index = fsize->index;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	dev_info(&client->dev, "-----%s----", __func__);
	for (i = 0; i < N_WIN_SIZES; i++) {
		struct ov7740_win_size *win = &ov7740_supported_win_sizes[index];
		if (index == ++num_valid) {
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width = win->width;
			fsize->discrete.height = win->height;
			return 0;
		}
	}

	return -EINVAL;
}

static int ov7740_video_probe(struct i2c_client *client)
{
	unsigned char retval = 0, retval_high = 0, retval_low = 0;
	struct v4l2_subdev *subdev = i2c_get_clientdata(client);
	int ret = 0;

	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	dev_info(&client->dev, "-----%s----", __func__);

	ret = soc_camera_power_on(&client->dev, ssdd);
	if (ret < 0)
		return ret;

	/*
	 * check and show product ID and manufacturer ID
	 */
	 #if 0
	retval = ov7740_write_reg(client, 0x3008, 0x80);
	if(retval) {
		dev_err(&client->dev, "i2c write failed!\n");
		return -1;
	}
    #endif

	retval_high = ov7740_read_reg(client, REG_CHIP_ID_HIGH);
	if (retval_high != CHIP_ID_HIGH) {
		dev_err(&client->dev, "read sensor %s chip_id high %x is error\n",
				client->name, retval_high);
		return -1;
	}

	retval_low = ov7740_read_reg(client, REG_CHIP_ID_LOW);
	if (retval_low != CHIP_ID_LOW) {
		dev_err(&client->dev, "read sensor %s chip_id low %x is error\n",
				client->name, retval_low);
		return -1;
	}

	dev_info(&client->dev, "read sensor %s id high:0x%x,low:%x successed!\n",
			client->name, retval_high, retval_low);

	ret = soc_camera_power_off(&client->dev, ssdd);

	return 0;
}

static int ov7740_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	struct ov7740_priv *priv = to_ov7740(client);

	dev_info(&client->dev, "-----%s----", __func__);
	return soc_camera_set_power(&client->dev, ssdd, on);

   #if 0
	int ret;
	int bala_index = priv->balance_value;
	int effe_index = priv->effect_value;

	if (!on)
		return soc_camera_power_off(&client->dev, ssdd);

	ret = soc_camera_power_on(&client->dev, ssdd);
	if (ret < 0)
		return ret;

	///* initialize the sensor with default data */
	ret = ov7740_write_array(client, ov7740_init_regs);
	ret = ov7740_write_array(client, ov7740_balance[bala_index].mode_regs);
	ret = ov7740_write_array(client, ov7740_effect[effe_index].mode_regs);
	if (ret < 0)
		goto err;

	dev_info(&client->dev, "%s: Init default", __func__);
	return 0;

err:
	dev_err(&client->dev, "%s: Error %d", __func__, ret);

	ov7740_reset(client);
	return ret;
#endif

}

static int ov7740_g_mbus_config(struct v4l2_subdev *sd, struct v4l2_mbus_config *cfg)
{

	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	dev_info(&client->dev, "-----%s----", __func__);

	cfg->flags = V4L2_MBUS_PCLK_SAMPLE_FALLING | V4L2_MBUS_MASTER |
			V4L2_MBUS_VSYNC_ACTIVE_HIGH| V4L2_MBUS_HSYNC_ACTIVE_HIGH |
			V4L2_MBUS_DATA_ACTIVE_HIGH;

	cfg->type = V4L2_MBUS_PARALLEL;
	cfg->flags = soc_camera_apply_board_flags(ssdd, cfg);

	return 0;
}

static struct v4l2_subdev_core_ops ov7740_subdev_core_ops = {
	.s_power    = ov7740_s_power,
	.g_ctrl		= ov7740_g_ctrl,
	.s_ctrl		= ov7740_s_ctrl,
	.g_chip_ident	= ov7740_g_chip_ident,
	.querymenu  = ov7740_querymenu,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register	= ov7740_g_register,
	.s_register	= ov7740_s_register,
#endif
};

static struct v4l2_subdev_video_ops ov7740_subdev_video_ops = {
	.s_stream	= ov7740_s_stream,
	.g_mbus_fmt	= ov7740_g_fmt,
	.s_mbus_fmt	= ov7740_s_fmt,
	.try_mbus_fmt	= ov7740_try_fmt,
	.cropcap	= ov7740_cropcap,
	.g_crop		= ov7740_g_crop,
	.enum_mbus_fmt	= ov7740_enum_fmt,
//	.enum_framesizes = ov7740_enum_framesizes,
//	.enum_frameintervals = ov7740_enum_frameintervals,
	.g_mbus_config  = ov7740_g_mbus_config,
};

static struct v4l2_subdev_ops ov7740_subdev_ops = {
	.core	= &ov7740_subdev_core_ops,
	.video	= &ov7740_subdev_video_ops,
};

/*
 * i2c_driver functions
 */

static int ov7740_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct ov7740_priv *priv;
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret = 0,default_wight = 640,default_height = 480;

	if (!ssdd) {
		dev_err(&client->dev, "ov7740: missing platform data!\n");
		return -EINVAL;
	}


	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE
			| I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "client not i2c capable\n");
		return -ENODEV;
	}

	priv = kzalloc(sizeof(struct ov7740_priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&adapter->dev,
			"Failed to allocate memory for private data!\n");
		return -ENOMEM;
	}


	v4l2_i2c_subdev_init(&priv->subdev, client, &ov7740_subdev_ops);

	priv->win = ov7740_select_win(&default_wight, &default_height);

	priv->cfmt_code  =  V4L2_MBUS_FMT_YUYV8_2X8;


	ret = ov7740_video_probe(client);
	if (ret) {
		kfree(priv);
	}
	return ret;
}

static int ov7740_remove(struct i2c_client *client)
{
	struct ov7740_priv       *priv = to_ov7740(client);

	kfree(priv);
	return 0;
}

static const struct i2c_device_id ov7740_id[] = {
	{ "ov7740",  0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, ov7740_id);

static struct i2c_driver ov7740_i2c_driver = {
	.driver = {
		.name = "ov7740",
	},
	.probe    = ov7740_probe,
	.remove   = ov7740_remove,
	.id_table = ov7740_id,
};

/*
 * Module functions
 */
static int __init ov7740_module_init(void)
{
	return i2c_add_driver(&ov7740_i2c_driver);
}

static void __exit ov7740_module_exit(void)
{
	i2c_del_driver(&ov7740_i2c_driver);
}

module_init(ov7740_module_init);
module_exit(ov7740_module_exit);

MODULE_DESCRIPTION("camera sensor ov7740 driver");
MODULE_AUTHOR("Zack Yu");
MODULE_LICENSE("GPL");