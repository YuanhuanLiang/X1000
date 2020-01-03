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

struct bmtf200_2_power{
	struct regulator *vlcdio;
	struct regulator *vlcdvcc;
	int inited;
};

static struct bmtf200_2_power lcd_power = {
	NULL,
	NULL,
	0
};

int bmtf200_2_power_init(struct lcd_device *ld)
{
	int ret ;
	printk("======bmtf200_2_power_init==============\n");
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

int bmtf200_2_power_reset(struct lcd_device *ld)
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

int bmtf200_2_power_on(struct lcd_device *ld, int enable)
{
	if (!lcd_power.inited && bmtf200_2_power_init(ld))
		return -EFAULT;

	if (enable) {
		gpio_direction_output(GPIO_LCD_CS, 1);
		gpio_direction_output(GPIO_LCD_RD, 1);

		bmtf200_2_power_reset(ld);

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

struct lcd_platform_data bmtf200_2_pdata = {
	.reset    = bmtf200_2_power_reset,
	.power_on = bmtf200_2_power_on,
};

/* LCD Panel Device */
struct platform_device bmtf200_2_device = {
	.name		= "bmtf200_2_slcd",
	.dev		= {
		.platform_data	= &bmtf200_2_pdata,
	},
};

static struct smart_lcd_data_table bmtf200_2_data_table[] = {
	{SMART_CONFIG_CMD,0xC8},       //Set EXTC
    {SMART_CONFIG_DATA,0xFF},
    {SMART_CONFIG_DATA,0x93},
    {SMART_CONFIG_DATA,0x42},

    {SMART_CONFIG_CMD,0x36},       //Memory Access Control
    {SMART_CONFIG_DATA,0xD8}, //MY,MX,MV,ML,BGR,MH
    
    {SMART_CONFIG_CMD,0x3A},       //Pixel Format Set
    {SMART_CONFIG_DATA,0x66}, //DPI [2:0],DBI [2:0]
    
    {SMART_CONFIG_CMD,0xC0},       //Power Control 1
    {SMART_CONFIG_DATA,0x15}, //VRH[5:0]
    {SMART_CONFIG_DATA,0x15}, //VC[3:0]
    
    {SMART_CONFIG_CMD,0xC1},       //Power Control 2
    {SMART_CONFIG_DATA,0x01}, //SAP[2:0],BT[3:0]
    
    {SMART_CONFIG_CMD,0xC5},       //VCOM
    {SMART_CONFIG_DATA,0xDA},
    
    {SMART_CONFIG_CMD,0xB1},      
    {SMART_CONFIG_DATA,0x00},     
    {SMART_CONFIG_DATA,0x1B},
    {SMART_CONFIG_CMD,0xB4},      
    {SMART_CONFIG_DATA,0x02},
    
    {SMART_CONFIG_CMD,0xE0},
    {SMART_CONFIG_DATA,0x0F},//P01-VP63   
    {SMART_CONFIG_DATA,0x13},//P02-VP62   
    {SMART_CONFIG_DATA,0x17},//P03-VP61   
    {SMART_CONFIG_DATA,0x04},//P04-VP59   
    {SMART_CONFIG_DATA,0x13},//P05-VP57   
    {SMART_CONFIG_DATA,0x07},//P06-VP50   
    {SMART_CONFIG_DATA,0x40},//P07-VP43   
    {SMART_CONFIG_DATA,0x39},//P08-VP27,36
    {SMART_CONFIG_DATA,0x4F},//P09-VP20   
    {SMART_CONFIG_DATA,0x06},//P10-VP13   
    {SMART_CONFIG_DATA,0x0D},//P11-VP6    
    {SMART_CONFIG_DATA,0x0A},//P12-VP4    
    {SMART_CONFIG_DATA,0x1F},//P13-VP2    
    {SMART_CONFIG_DATA,0x22},//P14-VP1    
    {SMART_CONFIG_DATA,0x00},//P15-VP0    
    
    {SMART_CONFIG_CMD,0xE1},
    {SMART_CONFIG_DATA,0x00},//P01
    {SMART_CONFIG_DATA,0x21},//P02
    {SMART_CONFIG_DATA,0x24},//P03
    {SMART_CONFIG_DATA,0x03},//P04
    {SMART_CONFIG_DATA,0x0F},//P05
    {SMART_CONFIG_DATA,0x05},//P06
    {SMART_CONFIG_DATA,0x38},//P07
    {SMART_CONFIG_DATA,0x32},//P08
    {SMART_CONFIG_DATA,0x49},//P09
    {SMART_CONFIG_DATA,0x00},//P10
    {SMART_CONFIG_DATA,0x09},//P11
    {SMART_CONFIG_DATA,0x08},//P12
    {SMART_CONFIG_DATA,0x32},//P13
    {SMART_CONFIG_DATA,0x35},//P14
    {SMART_CONFIG_DATA,0x0F},//P15

    {SMART_CONFIG_CMD,0x11},//Exit Sleep
    {SMART_CONFIG_UDELAY,120000},	
    //LCD_ClearAll_ILI9342bkground}, //added by chaibing for lcd issue 20140116 
    {SMART_CONFIG_CMD,0x29},//Display On
    {SMART_CONFIG_UDELAY,20000},
};

unsigned long cmd_buf[]= {
	0x2C2C2C2C,
};

struct fb_videomode jzfb0_videomode = {
	.name = "320x240",
	.refresh = 80,
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
	.smart_config.write_gram_cmd = cmd_buf,
	.smart_config.clkply_active_rising = 1,
	.smart_config.length_cmd = ARRAY_SIZE(cmd_buf),
	.smart_config.bus_width = 8,
	.smart_config.length_data_table =  ARRAY_SIZE(bmtf200_2_data_table),
	.smart_config.data_table = bmtf200_2_data_table,
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
