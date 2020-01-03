/*
 * Copyright (c) 2014 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * JZ-M200 orion board lcd setup routines.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/lcd.h>
#include <linux/regulator/consumer.h>
#include <mach/jzfb.h>
#include <board.h>
#include <linux/interrupt.h>
//#include "../board_base.h"

//#define dprintf printk
#define dprintf(...)
/*ifdef is 18bit,6-6-6 ,ifndef default 5-6-6*/
//#define CONFIG_SLCD_TRULY_18BIT

#ifdef	CONFIG_SLCD_TRULY_18BIT
static int slcd_inited = 1;
#else
static int slcd_inited = 0;
#endif

struct slcd24252_2_power{
	struct regulator *vlcdio;
	struct regulator *vlcdvcc;
	int inited;
};

static struct slcd24252_2_power lcd_power = {
	NULL,
	NULL,
	0
};

int slcd24252_2_power_init(struct lcd_device *ld)
{
	int ret ;
	printk("======slcd24252_2_power_init==============\n");
	if(GPIO_LCD_RST > 0){
		ret = gpio_request(GPIO_LCD_RST, "lcd rst");
		if (ret) {
			printk(KERN_ERR "can's request lcd rst\n");
			return ret;
		}
	}
	if(GPIO_LCD_CS > 0){
		ret = gpio_request(GPIO_LCD_CS, "lcd cs");
		if (ret) {
			printk(KERN_ERR "can's request lcd cs\n");
			return ret;
		}
	}
	if(GPIO_LCD_RD > 0){
		ret = gpio_request(GPIO_LCD_RD, "lcd rd");
		if (ret) {
			printk(KERN_ERR "can's request lcd rd\n");
			return ret;
		}
	}
	printk("set lcd_power.inited  =======1 \n");
	lcd_power.inited = 1;
	return 0;
}

int slcd24252_2_power_reset(struct lcd_device *ld)
{
	if (!lcd_power.inited)
		return -EFAULT;
	printk("\t***RESET LCD\n");	
	gpio_direction_output(GPIO_LCD_RST, 0);
	mdelay(20);
	gpio_direction_output(GPIO_LCD_RST, 1);
	mdelay(10);

	return 0;
}

int slcd24252_2_power_on(struct lcd_device *ld, int enable)
{
	if (!lcd_power.inited && slcd24252_2_power_init(ld))
		return -EFAULT;

	if (enable) {
		gpio_direction_output(GPIO_LCD_CS, 1);
		gpio_direction_output(GPIO_LCD_RD, 1);

		slcd24252_2_power_reset(ld);

		mdelay(5);
		gpio_direction_output(GPIO_LCD_CS, 0);

	} else {
		mdelay(5);
		gpio_direction_output(GPIO_LCD_CS, 0);
		gpio_direction_output(GPIO_LCD_RD, 0);
		gpio_direction_output(GPIO_LCD_RST, 0);
		slcd_inited = 0;
	}
	return 0;
}

struct lcd_platform_data slcd24252_2_pdata = {
	.reset    = slcd24252_2_power_reset,
	.power_on = slcd24252_2_power_on,
};

/* LCD Panel Device */
struct platform_device slcd24252_2_device = {
	.name		= "slcd24252_2_slcd",
	.dev		= {
		.platform_data	= &slcd24252_2_pdata,
	},
};

static struct smart_lcd_data_table slcd24252_2_data_table[] = {
	{SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0xbe},
    {SMART_CONFIG_DATA,0xc3},
    {SMART_CONFIG_DATA,0x29},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x04},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_DATA,0x00},

#if (SLCD_24252_2_XRES==360) && !defined(SLCD_ROTATE_180)
	{SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_DATA,0x00},
#else
#if (SLCD_24252_2_XRES==360)
    //旋转0度或180度时用如下设置，解析度设为360*400
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x80}, //0X30:上到下。 0x80:下到上
#else
    //旋转90度或270度时用如下设置，解析度调整为400*360
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0xEC}, //0X3C：左到右  0XEC：右到左
#endif
#endif
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x06},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x07},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_DATA,0x03},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x08},
    {SMART_CONFIG_DATA,0x03},
    {SMART_CONFIG_DATA,0x03},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x0d},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x10},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0xc1},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x11},
    {SMART_CONFIG_DATA,0xb1},
    {SMART_CONFIG_DATA,0x08},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x12},
    {SMART_CONFIG_DATA,0xb1},
    {SMART_CONFIG_DATA,0x08},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x13},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x0f},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x14},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x14},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x15},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x04},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x16},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_UDELAY,200},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x22},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x23},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x30},
    {SMART_CONFIG_DATA,0x7c},
    {SMART_CONFIG_DATA,0x3f},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x32},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x70},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x91},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0xe0},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0xe1},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x61},

    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_DATA,0x10},
    {SMART_CONFIG_DATA,0x30},

    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_DATA,0xc6},
    {SMART_CONFIG_DATA,0x33},

    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_DATA,0x50},
    {SMART_CONFIG_DATA,0x1f},

    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x25},

    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x08},
    {SMART_CONFIG_DATA,0x03},
    {SMART_CONFIG_DATA,0x60},


    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x11},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_UDELAY,200}, 

    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x35},
    {SMART_CONFIG_DATA,0x76},
    {SMART_CONFIG_DATA,0x66},

    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x39},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x26},
    {SMART_CONFIG_UDELAY,200},

    {SMART_CONFIG_CMD,0x04},
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0xc7},

    {SMART_CONFIG_CMD,0x04},
    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x06},
    {SMART_CONFIG_CMD,0x06},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_DATA,0x0d},
    {SMART_CONFIG_DATA,0x0e},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x03},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_DATA,0x08},
    {SMART_CONFIG_DATA,0x08},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_DATA,0x02},
    {SMART_CONFIG_DATA,0x01},


    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x04},
    {SMART_CONFIG_DATA,0x03},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x04},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x06},
    {SMART_CONFIG_DATA,0x1b},
    {SMART_CONFIG_DATA,0x21},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x07},
    {SMART_CONFIG_DATA,0x0f},
    {SMART_CONFIG_DATA,0x0e},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x08},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_DATA,0x04},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x09},
    {SMART_CONFIG_DATA,0x08},
    {SMART_CONFIG_DATA,0x08},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x0a},
    {SMART_CONFIG_DATA,0x02},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x0b},
    {SMART_CONFIG_DATA,0x03},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x0c},
    {SMART_CONFIG_UDELAY,200},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x03},

    {SMART_CONFIG_CMD,0x03},
    {SMART_CONFIG_CMD,0x0d},
    {SMART_CONFIG_DATA,0x31},
    {SMART_CONFIG_DATA,0x34},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x81},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x06},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x83},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x3f},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x85},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x3a},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x89},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x90},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x82},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x84},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x3f},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x86},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x3f},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x8a},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x10},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x87},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x88},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x1f},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x90},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x91},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x92},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x93},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x1d},
    {SMART_CONFIG_UDELAY,200},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x94},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x2a},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x95},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x4d},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x96},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x7a},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x97},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0xb1},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x98},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0xf2},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0xa0},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0xa1},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0xff},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0xa2},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01},


    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0xa3},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x18},

    {SMART_CONFIG_CMD,0x05},
    {SMART_CONFIG_CMD,0x80},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x03},
    {SMART_CONFIG_UDELAY,200},

    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x00},


    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_CMD,0x10},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x1e},

    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_CMD,0x11},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_DATA,0x85},

    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_CMD,0x12},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_CMD,0x13},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_DATA,0x8f},

    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_DATA,0x85},

    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},

    {SMART_CONFIG_CMD,0x01},
    {SMART_CONFIG_CMD,0x11},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01},
    {SMART_CONFIG_UDELAY,200},
    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_CMD,0x02},
    {SMART_CONFIG_UDELAY,200},
};

struct fb_videomode jzfb0_videomode = {
	.name = "slcd24252_2_slcd",
	.refresh = 80,
	.xres = SLCD_24252_2_XRES,
	.yres = SLCD_24252_2_YRES,
	.pixclock = KHZ2PICOS(30000),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

unsigned long truly_cmd_buf[]= {
    0x0,
};

static int init_vsync(void* vsync_handler);

struct jzfb_platform_data jzfb_pdata = {
	.num_modes = 1,
	.modes = &jzfb0_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp    = 24,
	.width = 31,
	.height = 31,
	.pinmd  = 0,

	.smart_config.rsply_cmd_high       = 0,
	.smart_config.csply_active_high    = 0,
	.smart_config.newcfg_fmt_conv =  1,
	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.write_gram_cmd = truly_cmd_buf,
	.smart_config.length_cmd = ARRAY_SIZE(truly_cmd_buf),
	.smart_config.bus_width = 8,
	.smart_config.data_times = 2,
	.smart_config.length_data_table =  ARRAY_SIZE(slcd24252_2_data_table),
	.smart_config.data_table = slcd24252_2_data_table,
	.dither_enable = 0,
	
	.lcd_callback_ops.not_block_sync = 1,
	.lcd_callback_ops.sync_init = init_vsync,
};


static int vsync_inited;
static int init_vsync(void* vsync_handler)
{
	int irq, ret;
	printk("%s\n",__FUNCTION__);
	if(vsync_inited)
		return 0;

	ret = gpio_request(GPIO_LCD_VSYNC, "lcd vsync");
	//ret = gpio_request_one(GPIO_LCD_VSYNC, GPIOF_IN, 0);
	if (ret < 0) {
		printk("Failed to request GPIO %d, error %d\n",
			GPIO_LCD_VSYNC, ret);
		goto error;
	}

	irq = gpio_to_irq(GPIO_LCD_VSYNC);
	if (irq < 0) {
		ret = irq;
		printk("Unable to get irq number for GPIO %d, error %d\n",
			GPIO_LCD_VSYNC, ret);
		goto error;
	}

	ret = request_irq(irq, (irq_handler_t)vsync_handler, 0, "slcd_vsync", 0);
	if (ret < 0) {
		printk(KERN_ALERT "%s: request_irg failed with %d\n",
		__func__, ret);
		goto error;
	}
	vsync_inited = 1;
	return 0;
error:
	gpio_free(GPIO_LCD_VSYNC);
	return -1;
}
/**************************************************************************************************/
#ifdef CONFIG_BACKLIGHT_PWM

static int backlight_init(struct device *dev)
{
	int ret;
#if 1
	ret = gpio_request(GPIO_LCD_PWM, "lcd power");
	if (ret) {
		printk(KERN_ERR "failed to request GPF for PWM-OUT1\n");
		return ret;
	}
#endif
	gpio_direction_output(GPIO_LCD_PWM, 1);

	jzgpio_set_func(GPIO_PORT_C, GPIO_OUTPUT1, (0x1 << 24));

	ret = gpio_request(GPIO_BL_PWR_EN, "BL PWR");
	if (ret) {
		printk(KERN_ERR "failed to reqeust BL PWR\n");
		return ret;
	}
	//gpio_direction_output(GPIO_BL_PWR_EN, 1);
    jz_gpio_set_func(GPIO_BL_PWR_EN, GPIO_FUNC_0);

	ret = gpio_request(GPIO_LCD_VSYNC, "lcd vsync");
	if (ret < 0) {
		printk("Failed to request GPIO %d, error %d\n",
			GPIO_LCD_VSYNC, ret);
	}
	else{
		gpio_free(GPIO_LCD_VSYNC);
	}
	return 0;
}

static int backlight_notify(struct device *dev, int brightness)
{
#if 0
	if (brightness)
		gpio_direction_output(GPIO_BL_PWR_EN, 1);
	else
		gpio_direction_output(GPIO_BL_PWR_EN, 0);
#endif

    dprintf("notify brightness:%d\n", brightness);
	return brightness;
}

static void backlight_exit(struct device *dev)
{
	gpio_free(GPIO_BL_PWR_EN);
}

static struct platform_pwm_backlight_data backlight_data = {
	.pwm_id		= 0,
	.max_brightness	= 255,
	.dft_brightness	= 120,
	.pwm_period_ns	= 30000,
	.init		= backlight_init,
	.exit		= backlight_exit,
	.notify		= backlight_notify,
};

struct platform_device backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.platform_data	= &backlight_data,
	},
};
#endif
