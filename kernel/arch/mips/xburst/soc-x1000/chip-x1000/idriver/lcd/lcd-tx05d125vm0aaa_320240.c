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
//#include "../board_base.h"

/*ifdef is 18bit,6-6-6 ,ifndef default 5-6-6*/
//#define CONFIG_SLCD_TRULY_18BIT

#ifdef	CONFIG_SLCD_TRULY_18BIT
static int slcd_inited = 1;
#else
static int slcd_inited = 0;
#endif

struct tx05d125vm0aaa_power{
	struct regulator *vlcdio;
	struct regulator *vlcdvcc;
	int inited;
};

static struct tx05d125vm0aaa_power lcd_power = {
	NULL,
	NULL,
	0
};

int tx05d125vm0aaa_power_init(struct lcd_device *ld)
{
	int ret ;
	printk("======tx05d125vm0aaa_power_init==============\n");
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

int tx05d125vm0aaa_power_reset(struct lcd_device *ld)
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

int tx05d125vm0aaa_power_on(struct lcd_device *ld, int enable)
{
	if (!lcd_power.inited && tx05d125vm0aaa_power_init(ld))
		return -EFAULT;

	if (enable) {
		gpio_direction_output(GPIO_LCD_CS, 1);
		gpio_direction_output(GPIO_LCD_RD, 1);

		tx05d125vm0aaa_power_reset(ld);

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

struct lcd_platform_data tx05d125vm0aaa_pdata = {
	.reset    = tx05d125vm0aaa_power_reset,
	.power_on = tx05d125vm0aaa_power_on,
};

/* LCD Panel Device */
struct platform_device tx05d125vm0aaa_device = {
	.name		= "tx05d125vm0aaa_slcd",
	.dev		= {
		.platform_data	= &tx05d125vm0aaa_pdata,
	},
};

static struct smart_lcd_data_table tx05d125vm0aaa_data_table[] = {
	{SMART_CONFIG_CMD,0x01},
	{SMART_CONFIG_UDELAY, 50000},
	{SMART_CONFIG_CMD,0x11},     //sleep out - wait at least 100 ms

	{SMART_CONFIG_UDELAY, 100000},
	{SMART_CONFIG_CMD,0x13},
	{SMART_CONFIG_UDELAY, 100000},

	{SMART_CONFIG_CMD,0xB0},
	{SMART_CONFIG_DATA,0x3C},
	{SMART_CONFIG_DATA,0x3C},
	{SMART_CONFIG_DATA,0x3C},
	{SMART_CONFIG_DATA,0x3C},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x3C},
	{SMART_CONFIG_DATA,0x20},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x01},
	{SMART_CONFIG_DATA,0x1C},
	{SMART_CONFIG_DATA,0x06},
	{SMART_CONFIG_DATA,0x66},
	{SMART_CONFIG_DATA,0x66},

	{SMART_CONFIG_CMD,0xB1},
	{SMART_CONFIG_DATA,0x14},
	{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x19},
	{SMART_CONFIG_DATA,0x57},
	{SMART_CONFIG_DATA,0x1F},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x80},
	{SMART_CONFIG_DATA,0x14},
	{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_DATA,0x80},
	{SMART_CONFIG_DATA,0x19},
	{SMART_CONFIG_DATA,0x57},
	{SMART_CONFIG_DATA,0x1F},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x80},
	{SMART_CONFIG_DATA,0x14},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x19},
	{SMART_CONFIG_DATA,0x57},
	{SMART_CONFIG_DATA,0x1F},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x34},
	{SMART_CONFIG_DATA,0x08},
	{SMART_CONFIG_DATA,0x19},
	{SMART_CONFIG_DATA,0x57},
	{SMART_CONFIG_DATA,0x1F},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},

	{SMART_CONFIG_CMD,0xD2},
	{SMART_CONFIG_DATA,0x01},


	{SMART_CONFIG_CMD,0xD3},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x57},

	{SMART_CONFIG_CMD,0xE0},
	{SMART_CONFIG_DATA,0x10},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x0F},
	{SMART_CONFIG_DATA,0x1B},
	{SMART_CONFIG_DATA,0x23},
	{SMART_CONFIG_DATA,0x25},
	{SMART_CONFIG_DATA,0x2D},
	{SMART_CONFIG_DATA,0x37},
	{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_DATA,0x3F},
	{SMART_CONFIG_DATA,0x47},
	{SMART_CONFIG_DATA,0x39},

	{SMART_CONFIG_CMD,0xE1},
	{SMART_CONFIG_DATA,0x10},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x0F},
	{SMART_CONFIG_DATA,0x1B},
	{SMART_CONFIG_DATA,0x23},
	{SMART_CONFIG_DATA,0x25},
	{SMART_CONFIG_DATA,0x2D},
	{SMART_CONFIG_DATA,0x37},
	{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_DATA,0x3F},
	{SMART_CONFIG_DATA,0x47},
	{SMART_CONFIG_DATA,0x39},

	{SMART_CONFIG_CMD,0xE2},
	{SMART_CONFIG_DATA,0x10},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x0F},
	{SMART_CONFIG_DATA,0x1B},
	{SMART_CONFIG_DATA,0x23},
	{SMART_CONFIG_DATA,0x25},
	{SMART_CONFIG_DATA,0x2D},
	{SMART_CONFIG_DATA,0x37},
	{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_DATA,0x3F},
	{SMART_CONFIG_DATA,0x47},
	{SMART_CONFIG_DATA,0x39},

	{SMART_CONFIG_CMD,0xE3},
	{SMART_CONFIG_DATA,0x10},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x0F},
	{SMART_CONFIG_DATA,0x1B},
	{SMART_CONFIG_DATA,0x23},
	{SMART_CONFIG_DATA,0x25},
	{SMART_CONFIG_DATA,0x2D},
	{SMART_CONFIG_DATA,0x37},
	{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_DATA,0x3F},
	{SMART_CONFIG_DATA,0x47},
	{SMART_CONFIG_DATA,0x39},

	{SMART_CONFIG_CMD,0xE4},
	{SMART_CONFIG_DATA,0x10},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x0F},
	{SMART_CONFIG_DATA,0x1B},
	{SMART_CONFIG_DATA,0x23},
	{SMART_CONFIG_DATA,0x25},
	{SMART_CONFIG_DATA,0x2D},
	{SMART_CONFIG_DATA,0x37},
	{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_DATA,0x3F},
	{SMART_CONFIG_DATA,0x47},
	{SMART_CONFIG_DATA,0x39},

	{SMART_CONFIG_CMD,0xE5},
	{SMART_CONFIG_DATA,0x10},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x0F},
	{SMART_CONFIG_DATA,0x1B},
	{SMART_CONFIG_DATA,0x23},
	{SMART_CONFIG_DATA,0x25},
	{SMART_CONFIG_DATA,0x2D},
	{SMART_CONFIG_DATA,0x37},
	{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_DATA,0x3F},
	{SMART_CONFIG_DATA,0x47},
	{SMART_CONFIG_DATA,0x39},


	{SMART_CONFIG_CMD,0x2A},  //Column address set
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x01},
	{SMART_CONFIG_DATA,0x3F},
	{SMART_CONFIG_CMD,0x2B},  // row address set
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x01},
	{SMART_CONFIG_DATA,0xEF},


	{SMART_CONFIG_CMD,0x3A}, // Memory Access Control
	{SMART_CONFIG_DATA,0x06}, //66

	{SMART_CONFIG_CMD,0x36}, // Memory Access Control
	{SMART_CONFIG_DATA,0xd8}, //08   48

	{SMART_CONFIG_CMD,0xB1},
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x15},///1f



	{SMART_CONFIG_CMD,0xF2}, // #Gamma function Disable
	{SMART_CONFIG_DATA,0x00},

	{SMART_CONFIG_CMD,0x26}, //Gamma curve selected
	{SMART_CONFIG_DATA,0x01},

	{SMART_CONFIG_CMD,0x35}, //Set TE ON  //ADD
	{SMART_CONFIG_DATA,0x00},

	{SMART_CONFIG_CMD,0x44}, //   ADD
	{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_DATA,0x3f},

	{SMART_CONFIG_CMD,0xb5}, // ADD
	{SMART_CONFIG_DATA,0x02},
	{SMART_CONFIG_DATA,0x02},
	{SMART_CONFIG_DATA,0xff},//0a
	{SMART_CONFIG_DATA,0xff},///14

	{SMART_CONFIG_CMD,0x11}, //Exit Sleep
	{SMART_CONFIG_UDELAY, 120000},
	{SMART_CONFIG_CMD,0x29}, //Display on
	{SMART_CONFIG_CMD,0x2c}, //memory write
};

unsigned long cmd_buf[]= {
	0x2C2C2C2C,
};

struct fb_videomode jzfb0_videomode = {
	.name = "320x240",
	.refresh = 160,
	.xres = 320,
	.yres = 240,
	.pixclock = KHZ2PICOS(20000),
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
	.smart_config.write_gram_cmd = cmd_buf,
	.smart_config.length_cmd = ARRAY_SIZE(cmd_buf),
	.smart_config.bus_width = 8,
	.smart_config.length_data_table =  ARRAY_SIZE(tx05d125vm0aaa_data_table),
	.smart_config.data_table = tx05d125vm0aaa_data_table,
	.dither_enable = 0,
};
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

	printk("\t***backlight_init\n");
	//while(1);
	ret = gpio_request(GPIO_BL_PWR_EN, "BL PWR");
	if (ret) {
		printk(KERN_ERR "failed to reqeust BL PWR\n");
		return ret;
	}
	gpio_direction_output(GPIO_BL_PWR_EN, 1);
	return 0;
}

static int backlight_notify(struct device *dev, int brightness)
{
	if (brightness)
		gpio_direction_output(GPIO_BL_PWR_EN, 1);
	else
		gpio_direction_output(GPIO_BL_PWR_EN, 0);

	return brightness;
}

static void backlight_exit(struct device *dev)
{
	gpio_free(GPIO_LCD_PWM);
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
