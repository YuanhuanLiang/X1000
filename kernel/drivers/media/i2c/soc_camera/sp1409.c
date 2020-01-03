/*
 * V4L2 Driver for camera sensor sp1409
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

#define REG_CUSTOMER_ID		0x04
#define REG_PID			0x05
#define CHIP_CUSTOMER_ID			0x14
#define CHIP_PID			0x09

#define  SP1409_DEFAULT_WIDTH    1280
#define  SP1409_DEFAULT_HEIGHT   720

//#define  SP1409_DEFAULT_WIDTH    240
//#define  SP1409_DEFAULT_HEIGHT   240

/* Private v4l2 controls */
#define V4L2_CID_PRIVATE_BALANCE  (V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_PRIVATE_EFFECT  (V4L2_CID_PRIVATE_BASE + 1)

/* In flip, the OV5640 does not need additional settings because the ISP block
 * will auto-detect whether the pixel is in the red line or blue line and make
 * the necessary adjustments.
 */
#define REG_TC_VFLIP			0x15
#define REG_TC_MIRROR			0x15
#define SP1409_VFLIP_VAL			((unsigned char)0x01)
#define SP1409_HFLIP_VAL			((unsigned char)0x02)
#define SP1409_VFLIP_MASK		((unsigned char)0x01)
#define SP1409_HFLIP_MASK		((unsigned char)0x02)
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
	u16 index;
	const struct regval_list *mode_regs;
};

/* Supported resolutions */
enum sp1409_width {
	W_720P	= 1280,
	W_752	= 752,
	W_640 = 640,
	W_240 = 240,
};

enum sp1409_height {
	H_720P	= 720,
	H_480	= 480,
	H_400 = 400,
	H_240 = 240,
};

struct sp1409_win_size {
	char *name;
	enum sp1409_width width;
	enum sp1409_height height;
	const struct regval_list *regs;
};


struct sp1409_priv {
	struct v4l2_subdev		subdev;
	struct sp1409_camera_info	*info;
	enum v4l2_mbus_pixelcode	cfmt_code;
	struct sp1409_win_size		*win;
	int				model;
	u16				balance_value;
	u16				effect_value;
	u16				flag_vflip:1;
	u16				flag_hflip:1;
};

int cam_t_j = 0, cam_t_i = 0;
unsigned long long cam_t0_buf[10];
unsigned long long cam_t1_buf[10];
static int sp1409_s_power(struct v4l2_subdev *sd, int on);
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


unsigned char sp1409_read_reg(struct i2c_client *client, u8 reg)
{
	int ret;
	unsigned char retval;
	unsigned char r = reg;

	ret = sensor_i2c_master_send(client,(u8 *)&r,1);

	if (ret < 0)
		return ret;
	if (ret != 1)
		return -EIO;

	ret = sensor_i2c_master_recv(client, &retval, 1);
	if (ret < 0)
		return ret;
	if (ret != 1)
		return -EIO;
	return retval;
}

int sp1409_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	unsigned char msg[2];
	int ret;

	reg = reg;

	memcpy(&msg[0], &reg, 1);
	memcpy(&msg[1], &val, 1);

	ret = sensor_i2c_master_send(client, msg, 2);

	if (ret < 0)
	{
		printk("RET<0\n");
		return ret;
	}
	if (ret < 2)
	{
		printk("RET<2\n");
		return -EIO;
	}

	return 0;
}

/*
 * Registers settings
 */

#define ENDMARKER { 0xff, 0xff }

static const struct regval_list sp1409_720p_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_240_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_480_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_640_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_vga_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_init_regs[] = {
//S 时钟设置
	{ 0xfd , 0x00 }, //RegPage: 0
	{ 0x24 , 0x01 }, //PLL_EN, active high
	{ 0x27 , 0x3b }, //PLL_N
	{ 0x26 , 0x02 }, //PLL_M
	{ 0x28 , 0x00 }, //PLL_OUTDIV
	{ 0x40 , 0x00 }, //CLK_SEL
	{ 0x25 , 0x00 }, //PLL_SEL, == 1, bypass pll
	{ 0x41 , 0x01 }, //PLL_CLK_EN
	{ 0x2e , 0x01 }, //PLL_OUTEN
//;E 时钟设置

//;S MIPI设置
	{ 0x89 , 0x02 }, //TX_SPEED_AREA_SEL
	{ 0x90 , 0x01 }, //SHUTDOWN
	{ 0x69 , 0x05 }, //MIPI_HSIZE_H4
	{ 0x6a , 0x00 }, //MIPI_HSIZE_L8
	{ 0x6b , 0x02 }, //MIPI_VSIZE_H3
	{ 0x6c , 0xD0 }, //MIPI_VSIZE_L8
	{ 0xfd , 0x01 }, //RegPage
	{ 0x1f , 0x00 }, //MIPI EN
	{ 0x1e , 0x00 }, //MIPI OUTPUT，MIPI复用并口PAD
//;E MIPI设置

//;S 模拟静态设置
	{ 0xfd , 0x00 }, //RegPage
	{ 0x3d , 0x00 }, //RBG
	{ 0x43 , 0x03 }, //RTRIM
	{ 0x45 , 0x05 }, //REG_CNT
	{ 0x46 , 0x03 }, //ydec电压
	{ 0x47 , 0x07 }, //ICOMP_CTRL1，比较器电流值
	{ 0x49 , 0x00 }, //IPIX_CTRL，像素偏置电流
	{ 0x3f , 0x09 }, //eclipse vth，太阳黑子电路阈值电压
	{ 0x1d , 0x65 }, //DS, Note: Valid only in DVP mode
              //bit[7:6]: DATA[7:2]'s driving setting. Avaliable value: 2'b00,2'b01,2'b10,2'b11
              //bit[5:4]: PCLK's      driving setting. Avaliable value: 2'b00,2'b01,2'b10,2'b11
              //bit[3:2]: VSYNC's     driving setting. Avaliable value: 2'b00,2'b01,2'b10,2'b11
              //bit[1:0]: HSYNC's     driving setting. Avaliable value: 2'b00,2'b01,2'b10,2'b11
	{ 0x1e , 0x55 }, //DSH,  Note: Valid only in DVP mode
              //bit[5:4]: DATA[9:8]'s driving setting. Avaliable value: 2'b00,2'b01,2'b10,2'b11
              //bit[3:2]: DATA[1]'s   driving setting. Avaliable value: 2'b00,2'b01,2'b10,2'b11
              //bit[1:0]: DATA[0]'s   driving setting. Avaliable value: 2'b00,2'b01,2'b10,2'b11
	{ 0x58 , 0x08 }, //NCP setting
//;E 模拟静态设置

//;S Pixel Timing设置
	{ 0xfd , 0x01 }, //RegPage
	{ 0x15 , 0x02 }, //bit1: mirror_en; bit0, up_down en
              //0x15=0x02, 1st Pixel is Gr
	{ 0x0a , 0x00 }, //==1, FPS 1st; ==0, TEXP 1st
	{ 0x2e , 0x0a }, //P5
	{ 0x30 , 0x20 }, //P7
	{ 0x31 , 0x02 }, //P8
	{ 0x33 , 0x02 }, //P10
	{ 0x34 , 0x40 }, //P11
	{ 0x36 , 0x0A }, //P12_B
	{ 0x39 , 0x01 }, //P13_H6
	{ 0x3a , 0x67 }, //P13_L8
	{ 0x3c , 0x07 }, //P20_A
	{ 0x3d , 0x03 }, //P20_B
	{ 0x3e , 0x00 }, //P21_A_H4
	{ 0x3f , 0x0B }, //P21_A_L8
	{ 0x40 , 0x00 }, //P21_B_H6
	{ 0x41 , 0x3B }, //P21_B_L8
	{ 0x4a , 0x49 }, //P25_C
	{ 0x4b , 0x03 }, //P27_A_H4
	{ 0x4c , 0x90 }, //P27_A_L8
	{ 0x4f , 0x00 }, //P28_A_H4
	{ 0x50 , 0x10 }, //P28_A_L8
	{ 0x4d , 0x02 }, //P27_B_H3
	{ 0x4e , 0xD8 }, //P27_B_L8
	{ 0x5c , 0x03 }, //P30_B
	{ 0x64 , 0x01 }, //P34_H8
	{ 0x65 , 0xD0 }, //P34_L8
	{ 0x75 , 0x05 }, //row_sg & row_sel switch
	{ 0x7b , 0x01 }, //ncp line refresh
	{ 0x7c , 0x55 }, //line refresh
	{ 0x20 , 0x30 }, //RMP_D1_Code
	{ 0x7e , 0x01 }, //Double_shutter_en
	{ 0x1a , 0x02 }, //HBLANK_H7
	{ 0x1b , 0x37 }, //HBLANK_L8
	{ 0xfe , 0x01 },
//;E Pixel Timing设置

//;S On-Chip ISP 设置
	{ 0xfd , 0x01 }, //RegPage
	{ 0x77 , 0x00 }, //RD_DARKROWS
	{ 0x78 , 0x07 }, //RD_DARKROWE
	{ 0xfd , 0x02 }, //RegPage: 2
	{ 0x20 , 0x00 }, //Colorbar
	{ 0x50 , 0x03 }, //RNRC
              //bit[2]                   :   darkcol_outputen, == 1, output darkcol
              //bit[1:0]    black col sel:   == 2'b01, use 0~15 darkcol; == 2'b10, use 1312~1327 darkcol; == 2'b11, use both.
	{ 0x53 , 0x03 }, //RNR_EN
              //bit1: darkcol median5 en, active high
              //bit0: derownoise_en, active high
	{ 0x51 , 0x03 }, //BLC_DARKROW0
	{ 0x52 , 0x04 }, //BLC_DARKROW1
	{ 0x31 , 0x01 }, //BLC_DARKCOLS_H3
	{ 0x32 , 0x81 }, //BLC_DARKCOLS_L8
	{ 0x33 , 0x03 }, //BLC_DARKCOLE_H3
	{ 0x34 , 0x80 }, //BLC_DARKCOLE_L8
	{ 0x35 , 0x00 }, //BLC_AVE_SEL,==0, 使用4个通道的OB；==1、2,使用一行两个通道的OB
	{ 0x30 , 0x09 }, //BLCC
              //bit[3:2]: blc_mode
              //bit[1]: blc_dr_oen
              //bit[0]: blc_en
	{ 0x80 , 0x0F }, //LSC_BPC_EN
	      //BIT3: RAW_DENOISE_EN
	      //BIT2: BPC_DPIX_EN
	      //BIT1: BPC_EN
	      //BIT0: DEMO_EN
              //Note: Should write 0x80=0x07, when Noise Test
              //Note2: Xchip的ISP中存在BPC算法，因此关闭我们自己的。

	{ 0x81 , 0x1b }, //BPC_VT_EFF
              //选择判断坏点时，坏点与正常点的相对差异。（为了保护细节，别低于18）
              //bit[5:3]是用来判断亮判断阈值，而bit[2:0]是用来判断暗判断阈值。[5:3]与[2:0]不同的取值将决定相对差异的大小：
              //  3'd0: 1/32
              //  3'd1: 1/16
              //  3'd2: 2/16
              //  3'd3: 3/16
              //  3'd4: 4/16
              //  3'd5: 5/16
              //  3'd6: 6/16
              //  3'd7: 7/16

	{ 0x92 , 0x20 }, //WHITE_RANGE_THR
              //低于这个区间的像素值，判定坏点时不用相对差异，而是绝对差异

//;E On-Chip ISP 设置

//;S Vblank Setting
	{ 0xfd , 0x01 }, //RegPage
	{ 0x16 , 0x00 }, //VBLANK_H7
	{ 0x17 , 0x08 }, //VBLANK_L8
//;E Vblank Setting

#if 1 // 1280X720
//;S SIZE Setting
	{ 0xfd , 0x01 }, //RegPage
	{ 0x06 , 0x05 }, //HSIZE_H3
	{ 0x07 , 0x00 }, //HSIZE_L8
	{ 0x08 , 0x02 }, //VSIZE_H2
	{ 0x09 , 0xD0 }, //VSIZE_L2
//;E SIZE Setting


//;start location
	{ 0xfd , 0x01 },
	{ 0x02 , 0x00 }, //HSTART_H3
	{ 0x03 , 0x18 }, //HSTART_L8
	{ 0x04 , 0x00 }, //VSTART_H2
	{ 0x05 , 0x10 }, //VSTART_L8
#else  //240X240
//;S SIZE Setting
	{ 0xfd , 0x01 }, //RegPage
	{ 0x06 , 0x00 }, //HSIZE_H3
	{ 0x07 , 0xf0 }, //HSIZE_L8
	{ 0x08 , 0x00 }, //VSIZE_H2
	{ 0x09 , 0xf0 }, //VSIZE_L2
//;E SIZE Setting

//;start location
	{ 0xfd , 0x01 },
	{ 0x02 , 0x02 }, //HSTART_H3
	{ 0x03 , 0x20 }, //HSTART_L8
	{ 0x04 , 0x00 }, //VSTART_H2
	{ 0x05 , 0xfc }, //VSTART_L8
#endif
//;S TEXP setting
//;Mclkg=20MHz, Rowtime=899 ~ 45us;  --> 10ms ~ 222 rows.
	{ 0xfd , 0x01 }, //RegPage
	{ 0x0c , 0x02 }, //TEXP_H8
	{ 0x0d , 0x9a }, //TEXP_L8
//;E TEXP setting

//;S Analog Gain Setting
	{ 0xfd , 0x01 }, //RegPage
	{ 0x24 , 0x20 }, //RPC_GAIN[5:0], RPC_real=RPC*RPC_GAIN; ADC range adjustment
// ;RPC setting (Analog Gain): real gain=RPC_real/128
	{ 0x22 , 0x00 }, //RPC_H2
	{ 0x23 , 0x10 }, //RPC_L8
//;E Analog Gain Setting

//;S Dgain setting
	{ 0xfd , 0x02 }, //RegPage: 2

	{ 0x70 , 0x10 }, //DG_PRE_GAIN:  real_gain=DG_PRE_GAIN/16

	{ 0x60 , 0x00 }, //DG_GR_GAIN_H3:  dg_gr_gain=DG_GR_GAIN/128
	{ 0x61 , 0x80 }, //DG_GR_GAIN_L8:

	{ 0x62 , 0x00 }, //DG_R_GAIN_H3:  dg_r_gain=DG_R_GAIN/128
	{ 0x63 , 0x80 }, //DG_R_GAIN_L8:

	{ 0x64 , 0x00 }, //DG_B_GAIN_H3:  dg_b_gain=DG_B_GAIN/128
	{ 0x65 , 0x80 }, //DG_B_GAIN_L8:

	{ 0x66 , 0x00 }, //DG_GB_GAIN_H3:  dg_gb_gain=DG_GB_GAIN/128
	{ 0x67 , 0x80 }, //DG_GB_GAIN_L8:

	{ 0x68 , 0x00 }, //GR_OFFSET_H8
	{ 0x69 , 0x20 }, //GR_OFFSET_L8

	{ 0x6a , 0x00 }, //R_OFFSET_H8
	{ 0x6b , 0x20 }, //R_OFFSET_L8

	{ 0x6c , 0x00 }, //B_OFFSET_H8
	{ 0x6d , 0x20 }, //B_OFFSET_L8

	{ 0x6e , 0x00 }, //GB_OFFSET_H8
	{ 0x6f , 0x20 }, //GB_OFFSET_L8

//;E Dgain setting

	{ 0xfe , 0x01 },
	{ 0xfe , 0x01 }, //Restart twice
	{ 0xfd , 0x00 },
	{ 0x58 , 0x02 }, //NCP setting
	{ 0xfd , 0x01 },
	ENDMARKER,
};

static const struct regval_list sp1409_wb_auto_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_wb_incandescence_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_wb_daylight_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_wb_fluorescent_regs[] = {
	ENDMARKER,
};

static const struct regval_list sp1409_wb_cloud_regs[] = {
	ENDMARKER,
};

static const struct mode_list sp1409_balance[] = {
	{0, sp1409_wb_auto_regs}, {1, sp1409_wb_incandescence_regs},
	{2, sp1409_wb_daylight_regs}, {3, sp1409_wb_fluorescent_regs},
	{4, sp1409_wb_cloud_regs},
};


static const struct regval_list sp1409_effect_normal_regs[] = {

	ENDMARKER,
};

static const struct regval_list sp1409_effect_grayscale_regs[] = {

	ENDMARKER,
};

static const struct regval_list sp1409_effect_sepia_regs[] = {

	ENDMARKER,
};

static const struct regval_list sp1409_effect_colorinv_regs[] = {

	ENDMARKER,
};

static const struct regval_list sp1409_effect_sepiabluel_regs[] = {

	ENDMARKER,
};

static const struct mode_list sp1409_effect[] = {
	{0, sp1409_effect_normal_regs},
	{1, sp1409_effect_grayscale_regs},
	{2, sp1409_effect_sepia_regs},
	{3, sp1409_effect_colorinv_regs},
	{4, sp1409_effect_sepiabluel_regs},
};

#define SP1409_SIZE(n, w, h, r) \
	{.name = n, .width = w , .height = h, .regs = r }

static struct sp1409_win_size sp1409_supported_win_sizes[] = {
	SP1409_SIZE("720P", W_720P, H_720P, sp1409_720p_regs),
	SP1409_SIZE("240", W_240, H_240, sp1409_240_regs),
	SP1409_SIZE("480P", W_752,H_480,sp1409_480_regs),
	SP1409_SIZE("vga", W_640,H_480,sp1409_vga_regs),
	SP1409_SIZE("640", W_640,H_400,sp1409_640_regs),
};

#define N_WIN_SIZES (ARRAY_SIZE(sp1409_supported_win_sizes))


static enum v4l2_mbus_pixelcode sp1409_codes[] = {
	V4L2_MBUS_FMT_Y8_1X8,
};

/*
 * Supported balance menus
 */
static const struct v4l2_querymenu sp1409_balance_menus[] = {
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
static const struct v4l2_querymenu sp1409_effect_menus[] = {
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
static struct sp1409_priv *to_sp1409(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct sp1409_priv,
			    subdev);
}

static int sp1409_write_array(struct i2c_client *client,
			      const struct regval_list *vals)
{
	int ret;

	while ((vals->reg_num != 0xff) || (vals->value != 0xff)) {
		ret = sp1409_write_reg(client, vals->reg_num, vals->value);
		dev_vdbg(&client->dev, "array: 0x%02x, 0x%02x",
			 vals->reg_num, vals->value);

		if (ret < 0)
			return ret;
		vals++;
	}
	return 0;
}


static int sp1409_mask_set(struct i2c_client *client,
			   u16  reg, u16  mask, u16  set)
{
	s32 val = sp1409_read_reg(client, reg);
	if (val < 0)
		return val;

	val &= ~mask;
	val |= set & mask;

	dev_vdbg(&client->dev, "masks: 0x%02x, 0x%02x", reg, val);

	return sp1409_write_reg(client, reg, val);
}

/*
 * soc_camera_ops functions
 */
static int sp1409_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	return 0;
}

static int sp1409_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct sp1409_priv *priv = to_sp1409(client);

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
static int sp1409_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct sp1409_priv *priv = to_sp1409(client);
	int ret = 0;
	int i = 0;
	u16 value;

	int balance_count = ARRAY_SIZE(sp1409_balance);
	int effect_count = ARRAY_SIZE(sp1409_effect);


	switch (ctrl->id) {
	case V4L2_CID_PRIVATE_BALANCE:
		if(ctrl->value > balance_count)
			return -EINVAL;

		for(i = 0; i < balance_count; i++) {
			if(ctrl->value == sp1409_balance[i].index) {
				ret = sp1409_write_array(client,
						sp1409_balance[ctrl->value].mode_regs);
				priv->balance_value = ctrl->value;
				break;
			}
		}
		break;

	case V4L2_CID_PRIVATE_EFFECT:
		if(ctrl->value > effect_count)
			return -EINVAL;

		for(i = 0; i < effect_count; i++) {
			if(ctrl->value == sp1409_effect[i].index) {
				ret = sp1409_write_array(client,
						sp1409_effect[ctrl->value].mode_regs);
				priv->effect_value = ctrl->value;
				break;
			}
		}
		break;

	case V4L2_CID_VFLIP:
		value = ctrl->value ? SP1409_VFLIP_VAL : 0x00;
		priv->flag_vflip = ctrl->value ? 1 : 0;
		ret = sp1409_mask_set(client, REG_TC_VFLIP, SP1409_VFLIP_MASK, value);
		break;

	case V4L2_CID_HFLIP:
		value = ctrl->value ? SP1409_HFLIP_VAL : 0x00;
		priv->flag_hflip = ctrl->value ? 1 : 0;
		ret = sp1409_mask_set(client, REG_TC_MIRROR, SP1409_HFLIP_MASK, value);
		break;

	default:
		dev_err(&client->dev, "no V4L2 CID: 0x%x ", ctrl->id);
		return -EINVAL;
	}

	return ret;
}

static int sp1409_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	id->ident    = SUPPORT_HIGH_RESOLUTION_PRE;
	id->revision = 0;

	return 0;
}


static int sp1409_querymenu(struct v4l2_subdev *sd,
					struct v4l2_querymenu *qm)
{
	switch (qm->id) {
	case V4L2_CID_PRIVATE_BALANCE:
		memcpy(qm->name, sp1409_balance_menus[qm->index].name,
				sizeof(qm->name));
		break;

	case V4L2_CID_PRIVATE_EFFECT:
		memcpy(qm->name, sp1409_effect_menus[qm->index].name,
				sizeof(qm->name));
		break;
	}

	return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int sp1409_g_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	reg->size = 1;
	if (reg->reg > 0xff)
		return -EINVAL;

	ret = sp1409_read_reg(client, reg->reg);
	if (ret < 0)
		return ret;

	reg->val = ret;

	return 0;
}

static int sp1409_s_register(struct v4l2_subdev *sd,
			     struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->reg > 0xff ||
	    reg->val > 0xff)
		return -EINVAL;

	return sp1409_write_reg(client, reg->reg, reg->val);
}
#endif

/* Select the nearest higher resolution for capture */
static struct sp1409_win_size *sp1409_select_win(u32 *width, u32 *height)
{
	int i, default_size = ARRAY_SIZE(sp1409_supported_win_sizes) - 1;
	for (i = 0; i < ARRAY_SIZE(sp1409_supported_win_sizes); i++) {
		if ((*width >= sp1409_supported_win_sizes[i].width) &&
		    (*height >= sp1409_supported_win_sizes[i].height)) {
			*width = sp1409_supported_win_sizes[i].width;
			*height = sp1409_supported_win_sizes[i].height;
			printk("===>%s i = %d, width = %d,height = %d\n",__func__,i,*width,*height);
			return &sp1409_supported_win_sizes[i];
		}
	}

	*width = sp1409_supported_win_sizes[default_size].width;
	*height = sp1409_supported_win_sizes[default_size].height;

	return &sp1409_supported_win_sizes[default_size];
}

static int sp1409_g_fmt(struct v4l2_subdev *sd,
			struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct sp1409_priv *priv = to_sp1409(client);

	mf->width = SP1409_DEFAULT_WIDTH;//priv->win->width;
	mf->height = SP1409_DEFAULT_HEIGHT;//priv->win->height;
	mf->code = priv->cfmt_code;

	mf->colorspace = V4L2_COLORSPACE_JPEG;
	mf->field	= V4L2_FIELD_NONE;

	return 0;
}

static int sp1409_s_fmt(struct v4l2_subdev *sd,
			struct v4l2_mbus_framefmt *mf)
{
	/* current do not support set format, use unify format yuv422i */
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	struct sp1409_priv *priv = to_sp1409(client);
	int ret;
	printk("===>%s,width = %d,height = %d\n",__func__,mf->width,mf->height);
	priv->win = sp1409_select_win(&mf->width, &mf->height);
	/* set size win */
	ret = sp1409_write_array(client, priv->win->regs);
	if (ret < 0) {
		dev_err(&client->dev, "%s: Error\n", __func__);
		return ret;
	}
	return 0;
}

static int sp1409_try_fmt(struct v4l2_subdev *sd,
			  struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client  *client = v4l2_get_subdevdata(sd);
	/*
	 * select suitable win
	 */
	const struct sp1409_win_size *win;

	printk("===>%s,width = %d,height = %d\n",__func__,mf->width,mf->height);
	win = sp1409_select_win(&mf->width, &mf->height);

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


static int sp1409_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(sp1409_codes))
		return -EINVAL;
	*code = sp1409_codes[index];
	return 0;
}

static int sp1409_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	return 0;
}

static int sp1409_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	return 0;
}


/*
 * Frame intervals.  Since frame rates are controlled with the clock
 * divider, we can only do 30/n for integer n values.  So no continuous
 * or stepwise options.  Here we just pick a handful of logical values.
 */


static int sp1409_enum_frameintervals(struct v4l2_subdev *sd,
		struct v4l2_frmivalenum *interval)
{
	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	interval->discrete.numerator = 1;
	interval->discrete.denominator = 60;//sp1409_frame_rates[interval->index];
	return 0;
}

/*
 * Frame size enumeration
 */
static int sp1409_enum_framesizes(struct v4l2_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	int i;
	int num_valid = -1;
	__u32 index = fsize->index;
	for (i = 0; i < N_WIN_SIZES; i++) {
		struct sp1409_win_size *win = &sp1409_supported_win_sizes[index];
		if (index == ++num_valid) {
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width = win->width;
			fsize->discrete.height = win->height;
			return 0;
		}
	}

	return -EINVAL;
}

static int sp1409_video_probe(struct i2c_client *client)
{
	unsigned char retval_high = 0, retval_low = 0;
	int ret = 0;

	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	ret = soc_camera_power_on(&client->dev, ssdd);
	if (ret < 0)
		return ret;

	/*
	 * check and show product ID and manufacturer ID
	 */
	retval_high = sp1409_read_reg(client, REG_CUSTOMER_ID);
	if (retval_high != CHIP_CUSTOMER_ID) {
		dev_err(&client->dev, "read sensor %s CHIP_CUSTOMER_ID %x is error\n",
				client->name, retval_high);
		return -1;
	}

	retval_low = sp1409_read_reg(client, REG_PID);
	if (retval_low != CHIP_PID) {
		dev_err(&client->dev, "read sensor %s CHIP_PID %x is error\n",
				client->name, retval_low);
		return -1;
	}

	dev_info(&client->dev, "read sensor %s REG_CUSTOMER_ID:0x%x,REG_PID:%x successed!\n",
			client->name, retval_high, retval_low);

	ret = soc_camera_power_off(&client->dev, ssdd);

	return 0;
}

static int sp1409_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	int ret;

	if (!on)
		return soc_camera_power_off(&client->dev, ssdd);

	ret = soc_camera_power_on(&client->dev, ssdd);
	if (ret < 0)
		return ret;

	///* initialize the sensor with default data */
	ret = sp1409_write_array(client, sp1409_init_regs);
	if (ret < 0)
		goto err;
	dev_info(&client->dev, "%s: Init default", __func__);
	return 0;
err:
	dev_err(&client->dev, "%s: Error %d", __func__, ret);
	return ret;
}

static int sp1409_g_mbus_config(struct v4l2_subdev *sd,
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

static struct v4l2_subdev_core_ops sp1409_subdev_core_ops = {
	.s_power    	= sp1409_s_power,
	.g_ctrl		= sp1409_g_ctrl,
	.s_ctrl		= sp1409_s_ctrl,
	.g_chip_ident	= sp1409_g_chip_ident,
	.querymenu  	= sp1409_querymenu,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register	= sp1409_g_register,
	.s_register	= sp1409_s_register,
#endif
};

static struct v4l2_subdev_video_ops sp1409_subdev_video_ops = {
	.s_stream	= sp1409_s_stream,
	.g_mbus_fmt	= sp1409_g_fmt,
	.s_mbus_fmt	= sp1409_s_fmt,
	.try_mbus_fmt	= sp1409_try_fmt,
	.cropcap	= sp1409_cropcap,
	.g_crop		= sp1409_g_crop,
	.enum_mbus_fmt	= sp1409_enum_fmt,
	.enum_framesizes = sp1409_enum_framesizes,
	.enum_frameintervals = sp1409_enum_frameintervals,
	.g_mbus_config  = sp1409_g_mbus_config,
};

static struct v4l2_subdev_ops sp1409_subdev_ops = {
	.core	= &sp1409_subdev_core_ops,
	.video	= &sp1409_subdev_video_ops,
};

/*
 * i2c_driver functions
 */

static int sp1409_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct sp1409_priv *priv;
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret = 0;
	int default_width = SP1409_DEFAULT_WIDTH;
	int default_height = SP1409_DEFAULT_HEIGHT;
	if (!ssdd) {
		dev_err(&client->dev, "sp1409: missing platform data!\n");
		return -EINVAL;
	}


	if(!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE
			| I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "client not i2c capable\n");
		return -ENODEV;
	}

	priv = kzalloc(sizeof(struct sp1409_priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&adapter->dev,
			"Failed to allocate memory for private data!\n");
		return -ENOMEM;
	}


	v4l2_i2c_subdev_init(&priv->subdev, client, &sp1409_subdev_ops);
	priv->win = sp1409_select_win(&default_width, &default_height);

	priv->cfmt_code  = V4L2_MBUS_FMT_Y8_1X8;
	ret = sp1409_video_probe(client);
	if (ret) {
		kfree(priv);
	}
	return ret;
}

static int sp1409_remove(struct i2c_client *client)
{
	struct sp1409_priv       *priv = to_sp1409(client);

	kfree(priv);
	return 0;
}

static const struct i2c_device_id sp1409_id[] = {
	{ "sp1409",  0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, sp1409_id);

static struct i2c_driver sp1409_i2c_driver = {
	.driver = {
		.name = "sp1409",
	},
	.probe    = sp1409_probe,
	.remove   = sp1409_remove,
	.id_table = sp1409_id,
};

/*
 * Module functions
 */
static int __init sp1409_module_init(void)
{
	return i2c_add_driver(&sp1409_i2c_driver);
}

static void __exit sp1409_module_exit(void)
{
	i2c_del_driver(&sp1409_i2c_driver);
}

module_init(sp1409_module_init);
module_exit(sp1409_module_exit);

MODULE_DESCRIPTION("camera sensor sp1409 driver");
MODULE_AUTHOR("zhangxiaoyan <xiaoyan.zhang@ingenic.com>");
MODULE_LICENSE("GPL");
