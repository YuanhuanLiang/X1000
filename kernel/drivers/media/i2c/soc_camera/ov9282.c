/*
 * V4L2 Driver for camera sensor ov9282
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

#define REG_CHIP_ID_HIGH		0x300a
#define REG_CHIP_ID_LOW			0x300b
#define CHIP_ID_HIGH			0x92
#define CHIP_ID_LOW			0x81

//#define  OV9282_DEFAULT_WIDTH    1280
//#define  OV9282_DEFAULT_HEIGHT   720

//#define  OV9282_DEFAULT_WIDTH    640
//#define  OV9282_DEFAULT_HEIGHT   400

//#define  OV9282_DEFAULT_WIDTH    752
//#define  OV9282_DEFAULT_HEIGHT   480

#define  OV9282_DEFAULT_WIDTH    640
#define  OV9282_DEFAULT_HEIGHT   480

/* Private v4l2 controls */
#define V4L2_CID_PRIVATE_BALANCE  (V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_PRIVATE_EFFECT  (V4L2_CID_PRIVATE_BASE + 1)

/* In flip, the OV5640 does not need additional settings because the ISP block
 * will auto-detect whether the pixel is in the red line or blue line and make
 * the necessary adjustments.
 */
#define REG_TC_VFLIP			0x3820
#define REG_TC_MIRROR			0x3821
#define OV9282_FLIP_VAL			((unsigned char)0x04)
#define OV9282_FLIP_MASK		((unsigned char)0x04)

 /* whether sensor support high resolution (> vga) preview or not */
#define SUPPORT_HIGH_RESOLUTION_PRE		1

/*
 * Struct
 */
struct regval_list {
	u16 reg_num;
	u16 value;
};

struct mode_list {
	u16 index;
	const struct regval_list *mode_regs;
};

/* Supported resolutions */
enum ov9282_width {
	W_720P	= 1280,
	W_752	= 752,
	W_640 = 640,
};

enum ov9282_height {
	H_720P	= 720,
	H_480	= 480,
	H_400 = 400,
};

struct ov9282_win_size {
	char *name;
	enum ov9282_width width;
	enum ov9282_height height;
	const struct regval_list *regs;
};


struct ov9282_priv {
	struct v4l2_subdev		subdev;
	struct ov9282_camera_info	*info;
	enum v4l2_mbus_pixelcode	cfmt_code;
	struct ov9282_win_size		*win;
	int				model;
	u16				balance_value;
	u16				effect_value;
	u16				flag_vflip:1;
	u16				flag_hflip:1;
};

int cam_t_j = 0, cam_t_i = 0;
unsigned long long cam_t0_buf[10];
unsigned long long cam_t1_buf[10];
static int ov9282_s_power(struct v4l2_subdev *sd, int on);
static inline int sensor_i2c_master_send(struct i2c_client *client,
		const char *buf ,int count)
{
	int ret;
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.len = count;
	msg.buf = (char *)buf;
	if (cam_t_i < 10)
		cam_t0_buf[cam_t_i] = cpu_clock(smp_processor_id());

	ret = i2c_transfer(adap, &msg, 1);
	if (cam_t_i < 10) {
		cam_t1_buf[cam_t_i] = cpu_clock(smp_processor_id());
		cam_t_i++;
	}
	if (cam_t_i == 10) {
		cam_t_j = cam_t_i;
		cam_t_i = 11;
		while(--cam_t_j)
			dprintk(7,"cam%d : i2c1_time 0  = %lld, i2c1_time 1"
					"= %lld, time = %lld\n",
					cam_t_j,
					cam_t0_buf[cam_t_j],
					cam_t1_buf[cam_t_j],
					cam_t1_buf[cam_t_j]
					- cam_t0_buf[cam_t_j]);
	}

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	   transmitted, else error code. */
	return (ret == 1) ? count : ret;
}

static inline int sensor_i2c_master_recv(struct i2c_client *client,
		char *buf ,int count)
{
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = buf;
	ret = i2c_transfer(adap, &msg, 1);

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	   transmitted, else error code. */
	return (ret == 1) ? count : ret;
}


unsigned char ov9282_read_reg(struct i2c_client *client, u16 reg)
{
	int ret;
	unsigned char retval;
	unsigned short r = cpu_to_be16(reg);

	ret = sensor_i2c_master_send(client,(u8 *)&r,2);

	if (ret < 0)
		return ret;
	if (ret != 2)
		return -EIO;

	ret = sensor_i2c_master_recv(client, &retval, 1);
	if (ret < 0)
		return ret;
	if (ret != 1)
		return -EIO;
	return retval;
}

int ov9282_write_reg(struct i2c_client *client, u16 reg, u8 val)
{
	unsigned char msg[3];
	int ret;

	reg = cpu_to_be16(reg);

	memcpy(&msg[0], &reg, 2);
	memcpy(&msg[2], &val, 1);

	ret = sensor_i2c_master_send(client, msg, 3);

	if (ret < 0)
	{
		printk("RET<0\n");
		return ret;
	}
	if (ret < 3)
	{
		printk("RET<3\n");
		return -EIO;
	}

	return 0;
}

/*
 * Registers settings
 */

#define ENDMARKER { 0xffff, 0xff }

static const struct regval_list ov9282_720p_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov9282_480_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov9282_640_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov9282_vga_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov9282_init_regs[] = {
	{ 0x0103, 0x01 },
	{ 0x0302, 0x30 },
	{ 0x030d, 0x60 },
	{ 0x030e, 0x06 },
	{ 0x3001, 0x62 },
	{ 0x3004, 0x01 },
	{ 0x3005, 0xff },
	{ 0x3006, 0xe2 },
	{ 0x3011, 0x0a },
	{ 0x3013, 0x18 },
	{ 0x3022, 0x07 },
	{ 0x3030, 0x10 },
	{ 0x3039, 0x2e },
	{ 0x303a, 0xf0 },
	{ 0x3500, 0x00 },
	{ 0x3501, 0x2a },
	{ 0x3502, 0x90 },
	{ 0x3503, 0x08 },
	{ 0x3505, 0x8c },
	{ 0x3507, 0x03 },
	{ 0x3508, 0x00 },
	{ 0x3509, 0x30 },
	{ 0x3610, 0x80 },
	{ 0x3611, 0xa0 },
	{ 0x3620, 0x6f },
	{ 0x3632, 0x56 },
	{ 0x3633, 0x78 },
	{ 0x3662, 0x03 },
	{ 0x3666, 0x5a },
	{ 0x366f, 0x7e },
	{ 0x3680, 0x84 },
	{ 0x3712, 0x80 },
	{ 0x372d, 0x22 },
	{ 0x3731, 0x80 },
	{ 0x3732, 0x30 },
	{ 0x3778, 0x00 },
	{ 0x377d, 0x22 },
	{ 0x3788, 0x02 },
	{ 0x3789, 0xa4 },
	{ 0x378a, 0x00 },
	{ 0x378b, 0x4a },
	{ 0x3799, 0x20 },
	{ 0x3800, 0x00 },
	{ 0x3801, 0x00 },
	{ 0x3802, 0x00 },
	{ 0x3803, 0x00 },
	{ 0x3804, 0x05 },
	{ 0x3805, 0x0f },
	{ 0x3806, 0x03 },
	{ 0x3807, 0x2f },
#if 0// 752X480 ok
	{ 0x3808, 0x02 },
	{ 0x3809, 0xf0 },
	{ 0x380a, 0x01 },
	{ 0x380b, 0xe0 },
	{ 0x380c, 0x02 },
	{ 0x380d, 0x40 },
	{ 0x380e, 0x03 },
	{ 0x380f, 0x55 },
#elif 0 // 1280X720 ok
	{ 0x3808, 0x05 },
	{ 0x3809, 0x00 },
	{ 0x380a, 0x02 },
	{ 0x380b, 0xd0 },
	{ 0x380c, 0x02 },
	{ 0x380d, 0xd8 },
	{ 0x380e, 0x03 },
	{ 0x380f, 0x54 },
#elif 1 // 640X480 ok
	{ 0x3808, 0x02 },
	{ 0x3809, 0x80 },
	{ 0x380a, 0x01 },
	{ 0x380b, 0xe0 },
	{ 0x380c, 0x02 },
	{ 0x380d, 0xd8 },
	{ 0x380e, 0x03 },
	{ 0x380f, 0x54 },
#endif
	{ 0x3810, 0x00 },
	{ 0x3811, 0x08 },
	{ 0x3812, 0x00 },
	{ 0x3813, 0x08 },
	{ 0x3814, 0x11 },
	{ 0x3815, 0x11 },
	{ 0x3820, 0x40 },
	{ 0x3821, 0x00 },
	{ 0x3881, 0x42 },
	{ 0x38b1, 0x00 },
	{ 0x3920, 0xff },
	{ 0x4003, 0x40 },
	{ 0x4008, 0x04 },
	{ 0x4009, 0x0b },
	{ 0x400c, 0x00 },
	{ 0x400d, 0x07 },
	{ 0x4010, 0x40 },
	{ 0x4043, 0x40 },
	{ 0x4307, 0x30 },
	{ 0x4317, 0x01 },
	{ 0x4501, 0x00 },
	{ 0x4507, 0x00 },
	{ 0x4509, 0x00 },
	{ 0x450a, 0x08 },
	{ 0x4601, 0x04 },
	{ 0x470f, 0xe0 },
	{ 0x4f07, 0x00 },
	{ 0x4800, 0x00 },
	{ 0x5000, 0x9f },
	{ 0x5001, 0x00 },
	{ 0x5e00, 0x00 },
	{ 0x5d00, 0x0b },
	{ 0x5d01, 0x02 },
	{ 0x0100, 0x01 },
#if 1
// triger snapshot mode
	{ 0x3006, 0xe4 },
	{ 0x3667, 0xda },
	{ 0x0100, 0x00 },
	{ 0x320c, 0x8f },
	{ 0x302c, 0x00 },
	{ 0x302d, 0x00 },
	{ 0x302e, 0x50 },
	{ 0x302f, 0x00 },
	{ 0x303f, 0x02 },
	{ 0x4242, 0x01 },
	{ 0x3030, 0x80 },
	{ 0x3030, 0x84 },
#endif
	ENDMARKER,	
};

static const struct regval_list ov9282_wb_auto_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov9282_wb_incandescence_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov9282_wb_daylight_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov9282_wb_fluorescent_regs[] = {
	ENDMARKER,
};

static const struct regval_list ov9282_wb_cloud_regs[] = {
	ENDMARKER,
};

static const struct mode_list ov9282_balance[] = {
	{0, ov9282_wb_auto_regs}, {1, ov9282_wb_incandescence_regs},
	{2, ov9282_wb_daylight_regs}, {3, ov9282_wb_fluorescent_regs},
	{4, ov9282_wb_cloud_regs},
};


static const struct regval_list ov9282_effect_normal_regs[] = {

	ENDMARKER,
};

static const struct regval_list ov9282_effect_grayscale_regs[] = {

	ENDMARKER,
};

static const struct regval_list ov9282_effect_sepia_regs[] = {

	ENDMARKER,
};

static const struct regval_list ov9282_effect_colorinv_regs[] = {

	ENDMARKER,
};

static const struct regval_list ov9282_effect_sepiabluel_regs[] = {

	ENDMARKER,
};

static const struct mode_list ov9282_effect[] = {
	{0, ov9282_effect_normal_regs},
	{1, ov9282_effect_grayscale_regs},
	{2, ov9282_effect_sepia_regs},
	{3, ov9282_effect_colorinv_regs},
	{4, ov9282_effect_sepiabluel_regs},
};

#define OV9282_SIZE(n, w, h, r) \
	{.name = n, .width = w , .height = h, .regs = r }

static struct ov9282_win_size ov9282_supported_win_sizes[] = {
	OV9282_SIZE("720P", W_720P, H_720P, ov9282_720p_regs),
	OV9282_SIZE("480P", W_752,H_480,ov9282_480_regs),
	OV9282_SIZE("vga", W_640,H_480,ov9282_vga_regs),
	OV9282_SIZE("640", W_640,H_400,ov9282_640_regs),
};

#define N_WIN_SIZES (ARRAY_SIZE(ov9282_supported_win_sizes))


static enum v4l2_mbus_pixelcode ov9282_codes[] = {
	V4L2_MBUS_FMT_Y8_1X8,
};

/*
 * Supported balance menus
 */
static const struct v4l2_querymenu ov9282_balance_menus[] = {
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
static const struct v4l2_querymenu ov9282_effect_menus[] = {
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
static struct ov9282_priv *to_ov9282(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct ov9282_priv,
			    subdev);
}

static int ov9282_write_array(struct i2c_client *client,
			      const struct regval_list *vals)
{
	int ret;

	while ((vals->reg_num != 0xffff) || (vals->value != 0xff)) {
		ret = ov9282_write_reg(client, vals->reg_num, vals->value);
		dev_vdbg(&client->dev, "array: 0x%02x, 0x%02x",
			 vals->reg_num, vals->value);

		if (ret < 0)
			return ret;
		vals++;
	}
	return 0;
}


static int ov9282_mask_set(struct i2c_client *client,
			   u16  reg, u16  mask, u16  set)
{
	s32 val = ov9282_read_reg(client, reg);
	if (val < 0)
		return val;

	val &= ~mask;
	val |= set & mask;

	dev_vdbg(&client->dev, "masks: 0x%02x, 0x%02x", reg, val);

	return ov9282_write_reg(client, reg, val);
}

/*
 * soc_camera_ops functions
 */
static int ov9282_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	return 0;
}

static int ov9282_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov9282_priv *priv = to_ov9282(client);

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
static int ov9282_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov9282_priv *priv = to_ov9282(client);
	int ret = 0;
	int i = 0;
	u16 value;

	int balance_count = ARRAY_SIZE(ov9282_balance);
	int effect_count = ARRAY_SIZE(ov9282_effect);


	switch (ctrl->id) {
	case V4L2_CID_PRIVATE_BALANCE:
		if(ctrl->value > balance_count)
			return -EINVAL;

		for(i = 0; i < balance_count; i++) {
			if(ctrl->value == ov9282_balance[i].index) {
				ret = ov9282_write_array(client,
						ov9282_balance[ctrl->value].mode_regs);
				priv->balance_value = ctrl->value;
				break;
			}
		}
		break;

	case V4L2_CID_PRIVATE_EFFECT:
		if(ctrl->value > effect_count)
			return -EINVAL;

		for(i = 0; i < effect_count; i++) {
			if(ctrl->value == ov9282_effect[i].index) {
				ret = ov9282_write_array(client,
						ov9282_effect[ctrl->value].mode_regs);
				priv->effect_value = ctrl->value;
				break;
			}
		}
		break;

	case V4L2_CID_VFLIP:
		value = ctrl->value ? OV9282_FLIP_VAL : 0x00;
		priv->flag_vflip = ctrl->value ? 1 : 0;
		ret = ov9282_mask_set(client, REG_TC_VFLIP, OV9282_FLIP_MASK, value);
		break;

	case V4L2_CID_HFLIP:
		value = ctrl->value ? OV9282_FLIP_VAL : 0x00;
		priv->flag_hflip = ctrl->value ? 1 : 0;
		ret = ov9282_mask_set(client, REG_TC_MIRROR, OV9282_FLIP_MASK, value);
		break;

	default:
		dev_err(&client->dev, "no V4L2 CID: 0x%x ", ctrl->id);
		return -EINVAL;
	}

	return ret;
}

static int ov9282_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	id->ident    = SUPPORT_HIGH_RESOLUTION_PRE;
	id->revision = 0;

	return 0;
}


static int ov9282_querymenu(struct v4l2_subdev *sd,
					struct v4l2_querymenu *qm)
{
	switch (qm->id) {
	case V4L2_CID_PRIVATE_BALANCE:
		memcpy(qm->name, ov9282_balance_menus[qm->index].name,
				sizeof(qm->name));
		break;

	case V4L2_CID_PRIVATE_EFFECT:
		memcpy(qm->name, ov9282_effect_menus[qm->index].name,
				sizeof(qm->name));
		break;
	}

	return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov9282_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	reg->size = 1;
	if (reg->reg > 0xff)
		return -EINVAL;

	ret = ov9282_read_reg(client, reg->reg);
	if (ret < 0)
		return ret;

	reg->val = ret;

	return 0;
}

static int ov9282_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->reg > 0xff ||
	    reg->val > 0xff)
		return -EINVAL;

	return ov9282_write_reg(client, reg->reg, reg->val);
}
#endif

/* Select the nearest higher resolution for capture */
static struct ov9282_win_size *ov9282_select_win(u32 *width, u32 *height)
{
	int i, default_size = ARRAY_SIZE(ov9282_supported_win_sizes) - 1;
	for (i = 0; i < ARRAY_SIZE(ov9282_supported_win_sizes); i++) {
		if ((*width >= ov9282_supported_win_sizes[i].width) &&
		    (*height >= ov9282_supported_win_sizes[i].height)) {
			*width = ov9282_supported_win_sizes[i].width;
			*height = ov9282_supported_win_sizes[i].height;
			printk("===>%s i = %d, width = %d,height = %d\n",__func__,i,*width,*height);
			return &ov9282_supported_win_sizes[i];
		}
	}

	*width = ov9282_supported_win_sizes[default_size].width;
	*height = ov9282_supported_win_sizes[default_size].height;

	return &ov9282_supported_win_sizes[default_size];
}

static int ov9282_g_fmt(struct v4l2_subdev *sd,
			struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov9282_priv *priv = to_ov9282(client);

	mf->width = OV9282_DEFAULT_WIDTH;//priv->win->width;
	mf->height = OV9282_DEFAULT_HEIGHT;//priv->win->height;
	mf->code = priv->cfmt_code;

	mf->colorspace = V4L2_COLORSPACE_JPEG;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int ov9282_s_fmt(struct v4l2_subdev *sd,
			struct v4l2_mbus_framefmt *mf)
{
	/* current do not support set format, use unify format yuv422i */
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct ov9282_priv *priv = to_ov9282(client);
	int ret;
	printk("===>%s,width = %d,height = %d\n",__func__,mf->width,mf->height);
	priv->win = ov9282_select_win(&mf->width, &mf->height);
	/* set size win */
	ret = ov9282_write_array(client, priv->win->regs);
	if (ret < 0) {
		dev_err(&client->dev, "%s: Error\n", __func__);
		return ret;
	}
	return 0;
}

static int ov9282_try_fmt(struct v4l2_subdev *sd,
			  struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	/*
	 * select suitable win
	 */
	const struct ov9282_win_size *win;

	printk("===>%s,width = %d,height = %d\n",__func__,mf->width,mf->height);
	win = ov9282_select_win(&mf->width, &mf->height);

	if(mf->field == V4L2_FIELD_ANY) {
		mf->field = V4L2_FIELD_NONE;
	} else if (mf->field != V4L2_FIELD_NONE) {
		dev_err(&client->dev, "Field type invalid.\n");
		return -ENODEV;
	}
	mf->code = V4L2_MBUS_FMT_Y8_1X8;
	mf->colorspace = V4L2_COLORSPACE_JPEG;
	return 0;
}


static int ov9282_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(ov9282_codes))
		return -EINVAL;
	*code = ov9282_codes[index];
	return 0;
}

static int ov9282_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	return 0;
}

static int ov9282_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	return 0;
}


/*
 * Frame intervals.  Since frame rates are controlled with the clock
 * divider, we can only do 30/n for integer n values.  So no continuous
 * or stepwise options.  Here we just pick a handful of logical values.
 */


static int ov9282_enum_frameintervals(struct v4l2_subdev *sd,
		struct v4l2_frmivalenum *interval)
{
	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	interval->discrete.numerator = 1;
	interval->discrete.denominator = 60;//ov9282_frame_rates[interval->index];
	return 0;
}

/*
 * Frame size enumeration
 */
static int ov9282_enum_framesizes(struct v4l2_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	int i;
	int num_valid = -1;
	__u32 index = fsize->index;
	for (i = 0; i < N_WIN_SIZES; i++) {
		struct ov9282_win_size *win = &ov9282_supported_win_sizes[index];
		if (index == ++num_valid) {
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width = win->width;
			fsize->discrete.height = win->height;
			return 0;
		}
	}

	return -EINVAL;
}

static int ov9282_video_probe(struct i2c_client *client)
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
	retval_high = ov9282_read_reg(client, REG_CHIP_ID_HIGH);
	if (retval_high != CHIP_ID_HIGH) {
		dev_err(&client->dev, "read sensor %s chip_id high %x is error\n",
				client->name, retval_high);
		return -1;
	}

	retval_low = ov9282_read_reg(client, REG_CHIP_ID_LOW);
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

static int ov9282_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	struct ov9282_priv *priv = to_ov9282(client);
	int ret;

	int bala_index = priv->balance_value;
	int effe_index = priv->effect_value;

	if (!on)
		return soc_camera_power_off(&client->dev, ssdd);

	ret = soc_camera_power_on(&client->dev, ssdd);
	if (ret < 0)
		return ret;

	///* initialize the sensor with default data */
	ret = ov9282_write_array(client, ov9282_init_regs);
	if (ret < 0)
		goto err;
	dev_info(&client->dev, "%s: Init default", __func__);
	return 0;
err:
	dev_err(&client->dev, "%s: Error %d", __func__, ret);
	return ret;
}

static int ov9282_g_mbus_config(struct v4l2_subdev *sd,
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

static struct v4l2_subdev_core_ops ov9282_subdev_core_ops = {
	.s_power    	= ov9282_s_power,
	.g_ctrl		= ov9282_g_ctrl,
	.s_ctrl		= ov9282_s_ctrl,
	.g_chip_ident	= ov9282_g_chip_ident,
	.querymenu  	= ov9282_querymenu,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register	= ov9282_g_register,
	.s_register	= ov9282_s_register,
#endif
};

static struct v4l2_subdev_video_ops ov9282_subdev_video_ops = {
	.s_stream	= ov9282_s_stream,
	.g_mbus_fmt	= ov9282_g_fmt,
	.s_mbus_fmt	= ov9282_s_fmt,
	.try_mbus_fmt	= ov9282_try_fmt,
	.cropcap	= ov9282_cropcap,
	.g_crop		= ov9282_g_crop,
	.enum_mbus_fmt	= ov9282_enum_fmt,
	.enum_framesizes = ov9282_enum_framesizes,
	.enum_frameintervals = ov9282_enum_frameintervals,
	.g_mbus_config  = ov9282_g_mbus_config,
};

static struct v4l2_subdev_ops ov9282_subdev_ops = {
	.core	= &ov9282_subdev_core_ops,
	.video	= &ov9282_subdev_video_ops,
};

/*
 * i2c_driver functions
 */

static int ov9282_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct ov9282_priv *priv;
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret = 0;
	int default_width = OV9282_DEFAULT_WIDTH;
	int default_height = OV9282_DEFAULT_HEIGHT;
	if (!ssdd) {
		dev_err(&client->dev, "ov9282: missing platform data!\n");
		return -EINVAL;
	}


	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE
			| I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "client not i2c capable\n");
		return -ENODEV;
	}

	priv = kzalloc(sizeof(struct ov9282_priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&adapter->dev,
			"Failed to allocate memory for private data!\n");
		return -ENOMEM;
	}


	v4l2_i2c_subdev_init(&priv->subdev, client, &ov9282_subdev_ops);
	priv->win = ov9282_select_win(&default_width, &default_height);

	priv->cfmt_code  = V4L2_MBUS_FMT_Y8_1X8;
	ret = ov9282_video_probe(client);
	if (ret) {
		kfree(priv);
	}
	return ret;
}

static int ov9282_remove(struct i2c_client *client)
{
	struct ov9282_priv       *priv = to_ov9282(client);

	kfree(priv);
	return 0;
}

static const struct i2c_device_id ov9282_id[] = {
	{ "ov9282",  0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, ov9282_id);

static struct i2c_driver ov9282_i2c_driver = {
	.driver = {
		.name = "ov9282",
	},
	.probe    = ov9282_probe,
	.remove   = ov9282_remove,
	.id_table = ov9282_id,
};

/*
 * Module functions
 */
static int __init ov9282_module_init(void)
{
	return i2c_add_driver(&ov9282_i2c_driver);
}

static void __exit ov9282_module_exit(void)
{
	i2c_del_driver(&ov9282_i2c_driver);
}

module_init(ov9282_module_init);
module_exit(ov9282_module_exit);

MODULE_DESCRIPTION("camera sensor ov9282 driver");
MODULE_AUTHOR("YeFei <feiye@ingenic.cn>");
MODULE_LICENSE("GPL");
