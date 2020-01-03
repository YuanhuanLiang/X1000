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

struct oled_rgs15128128wr000_power{
	struct regulator *vlcdio;
	struct regulator *vlcdvcc;
	int inited;
};

static struct oled_rgs15128128wr000_power lcd_power = {
	NULL,
	NULL,
	0
};

int oled_rgs15128128wr000_power_init(struct lcd_device *ld)
{
	int ret ;
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
	if(GPIO_BL_PWR_EN > 0){
		ret = gpio_request(GPIO_BL_PWR_EN, "lcd backlight");
		if (ret) {
			printk(KERN_ERR "can's request lcd backlight\n");
			return ret;
		}
	}
    
	printk("set lcd_power.inited  =======1 \n");
	lcd_power.inited = 1;
	return 0;
}

int oled_rgs15128128wr000_power_reset(struct lcd_device *ld)
{
	if (!lcd_power.inited)
		return -EFAULT;
	gpio_direction_output(GPIO_LCD_RST, 0);
	mdelay(20);
	gpio_direction_output(GPIO_LCD_RST, 1);
	mdelay(10);

	return 0;
}

int oled_rgs15128128wr000_power_on(struct lcd_device *ld, int enable)
{
	if (!lcd_power.inited && oled_rgs15128128wr000_power_init(ld))
		return -EFAULT;

	if (enable) {
		gpio_direction_output(GPIO_BL_PWR_EN, 1);
	} else {
		gpio_direction_output(GPIO_BL_PWR_EN, 0);
	}
	return 0;
}

struct lcd_platform_data oled_rgs15128128wr000_pdata = {
	.reset    = oled_rgs15128128wr000_power_reset,
	.power_on = oled_rgs15128128wr000_power_on,
};

/* LCD Panel Device */
struct platform_device oled_rgs15128128wr000_device = {
	.name		= "oled_rgs15128128wr000_slcd",
	.dev		= {
		.platform_data	= &oled_rgs15128128wr000_pdata,
	},
};

static struct smart_lcd_data_table oled_rgs15128128wr000_data_table[] = {
	{SMART_CONFIG_CMD,0xae},//Set display off 
     
    {SMART_CONFIG_CMD,0xa0},//Set re-map 
    {SMART_CONFIG_CMD,0x42}, 
     
    {SMART_CONFIG_CMD,0xa1},//Set display start line 
    {SMART_CONFIG_CMD,0x00}, 
     
    {SMART_CONFIG_CMD,0xa2},//Set display offset 
    {SMART_CONFIG_CMD,0x00}, 
     
    {SMART_CONFIG_CMD,0xa4},//Normal Display 
     
    {SMART_CONFIG_CMD,0xa8},//Set multiplex ratio 
    {SMART_CONFIG_CMD,0x7f}, 
     
    {SMART_CONFIG_CMD,0xab},//Function Selection A 
    {SMART_CONFIG_CMD,0x01},//Enable internal VDD regulator 
     
    {SMART_CONFIG_CMD,0x81},//Set contrast 
    {SMART_CONFIG_CMD,0x77}, 
     
    {SMART_CONFIG_CMD,0xb1},//Set Phase Length 
    {SMART_CONFIG_CMD,0x31}, 
     
    {SMART_CONFIG_CMD,0xb3},//Set Front Clock Divider /Oscillator Frequency 
    {SMART_CONFIG_CMD,0xb1}, 
     
    {SMART_CONFIG_CMD,0xb4}, //For brightness enhancement 
    {SMART_CONFIG_CMD,0xb5}, 
     
    {SMART_CONFIG_CMD,0xb6},//Set Second pre-charge Period 
    {SMART_CONFIG_CMD,0x0d}, 

    {SMART_CONFIG_CMD,0xbc},//Set Pre-charge voltage 
    {SMART_CONFIG_CMD,0x07}, 
     
    {SMART_CONFIG_CMD,0xbe},//Set VCOMH 
    {SMART_CONFIG_CMD,0x07}, 
     
    {SMART_CONFIG_CMD,0xd5},//Function Selection B 
    {SMART_CONFIG_CMD,0x02},//Enable second pre-charge 
     
    {SMART_CONFIG_CMD,0xaf},//Display on 

	#if 1
    {SMART_CONFIG_CMD,0x15},//Set column address 
    {SMART_CONFIG_CMD,0x00},//Column Start Address 
    {SMART_CONFIG_CMD,0x3f},//Column End Address 
    {SMART_CONFIG_CMD,0x75},//Set row address 
    {SMART_CONFIG_CMD,0x00},//Row Start Address 
    {SMART_CONFIG_CMD,0x7f},//Row End Address 
    #endif
};


struct fb_videomode jzfb0_videomode = {
	.name = "64*128",
	.refresh = 80,
	.xres = 64,
	.yres = 128,
	.pixclock = KHZ2PICOS(2000),
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
	.smart_config.newcfg_fmt_conv =  0,
	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.write_gram_cmd = 0,
	.smart_config.length_cmd = 0,
	.smart_config.bus_width = 8,
	.smart_config.data_times = 1,
	.smart_config.length_data_table =  ARRAY_SIZE(oled_rgs15128128wr000_data_table),
	.smart_config.data_table = oled_rgs15128128wr000_data_table,
	.dither_enable = 0,
};
/**************************************************************************************************/
#ifdef CONFIG_BACKLIGHT_PWM

static int backlight_init(struct device *dev)
{
	int ret;

	printk("\t***backlight_init\n");
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
