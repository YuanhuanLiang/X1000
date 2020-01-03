/*
 * Copyright (c) 2014 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * JZ-X1000 board lcd setup routines.
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
#include "../board_base.h"

static unsigned int lcd_inited = 0;

static int ug2864hlbeg01_power_init(struct lcd_device *ld)
{
    int ret ;

    if(GPIO_LCD_RST != -1) {
        ret = gpio_request(GPIO_LCD_RST, "lcd rst");
        if (ret) {
            printk(KERN_ERR "can's request lcd rst\n");
            return ret;
        }
    }
    if(GPIO_LCD_CS != -1) {
        ret = gpio_request(GPIO_LCD_CS, "lcd cs");
        if (ret) {
            printk(KERN_ERR "can's request lcd cs\n");
            return ret;
        }
    }
    if(GPIO_LCD_RD != -1) {
        ret = gpio_request(GPIO_LCD_RD, "lcd rd");
        if (ret) {
            printk(KERN_ERR "can's request lcd rd\n");
            return ret;
        }
    }

    if(GPIO_LCD_BACKLIGHT != -1) {
           ret = gpio_request(GPIO_LCD_BACKLIGHT, "lcd backlight");
           if (ret) {
               printk(KERN_ERR "can's request lcd backlight\n");
               return ret;
           }
      }

    if(GPIO_LCD_PWR_EN != -1) {
        ret = gpio_request(GPIO_LCD_PWR_EN, "lcd enable");
        if (ret) {
            printk(KERN_ERR "can's request lcd enable\n");
            return ret;
        }
   }
    printk("%s: set lcd_inited.inited = 1\n", __FUNCTION__);
    lcd_inited = 1;

    return 0;
}

static int ug2864hlbeg01_power_reset(struct lcd_device *ld)
{
    if (!lcd_inited && ug2864hlbeg01_power_init(ld))
        return -EFAULT;

    gpio_direction_output(GPIO_LCD_CS, 1);

    if (GPIO_LCD_RD != -1) {
        gpio_direction_output(GPIO_LCD_RD, 1);
    }

    if(GPIO_LCD_RST != -1) {
        gpio_direction_output(GPIO_LCD_RST, 0);
        mdelay(20);
        gpio_direction_output(GPIO_LCD_RST, 1);
        mdelay(20);
    }

    gpio_direction_output(GPIO_LCD_CS, 0);
    return 0;
}

int ug2864hlbeg01_power_on(struct lcd_device *ld, int enable)
{
    if (!lcd_inited && ug2864hlbeg01_power_init(ld))
        return -EFAULT;

    switch (enable) {
        /* backlight enable */
        case FB_BLANK_UNBLANK:
            break;

        /* power enable */
        case FB_BLANK_NORMAL:
            gpio_direction_output(GPIO_LCD_PWR_EN, 0);
            gpio_direction_output(GPIO_LCD_BACKLIGHT, 1);
            gpio_direction_output(GPIO_LCD_CS, 1);

            if (GPIO_LCD_RD != -1) {
                gpio_direction_output(GPIO_LCD_RD, 1);
            }

            if(GPIO_LCD_RST != -1) {
                gpio_direction_output(GPIO_LCD_RST, 0);
                mdelay(20);
                gpio_direction_output(GPIO_LCD_RST, 1);
                mdelay(20);
            } else {
                mdelay(40);
            }
            gpio_direction_output(GPIO_LCD_CS, 0);
            break;

        /* backlight disable */
        case FB_BLANK_VSYNC_SUSPEND:
        case FB_BLANK_HSYNC_SUSPEND:
            break;

        /* power and backlight disable */
        default:
            gpio_direction_output(GPIO_LCD_CS, 1);
            gpio_direction_output(GPIO_LCD_BACKLIGHT, 0);
            gpio_direction_output(GPIO_LCD_PWR_EN, 1);
            break;
    }

    return 0;
}

static struct lcd_platform_data ug2864hlbeg01_pdata = {
        .reset    = ug2864hlbeg01_power_reset,
        .power_on = ug2864hlbeg01_power_on,
};

/* LCD Panel Device */
struct platform_device ug2864hlbeg01_device = {
        .name = "ug2864hlbeg01_slcd",
        .dev = {
                .platform_data = &ug2864hlbeg01_pdata,
        },
};

static struct smart_lcd_data_table ug2864hlbeg01_data_table[] = {
    /* LCD init code */

    {SMART_CONFIG_CMD, 0xae},//Set Display Off

    {SMART_CONFIG_CMD, 0xd5},//Set Display Clock Divide Ratio/Oscillator Frequency
    {SMART_CONFIG_CMD, 0x80},

    {SMART_CONFIG_CMD, 0xa8},//Set Multiplex Ratio
    {SMART_CONFIG_CMD, 0x3f},

    {SMART_CONFIG_CMD,0xd3},//Set Display Offset
    {SMART_CONFIG_CMD,0x00},

    {SMART_CONFIG_CMD,0x40},//Set Display Start Line

    {SMART_CONFIG_CMD,0x8d},//Set Charge Pump
    {SMART_CONFIG_CMD,0x14},//    {SMART_CONFIG_CMD,0x10},

    {SMART_CONFIG_CMD,0xa1},//Set Segment Re-Map
//    {SMART_CONFIG_CMD,0xa0},//Set Segment Re-Map

//    {SMART_CONFIG_CMD,0xc8},//Set COM Output Scan Direction
    {SMART_CONFIG_CMD,0xc0},//Set COM Output Scan Direction

    {SMART_CONFIG_CMD,0xda},//Set COM Pins Hardware Configuration
    {SMART_CONFIG_CMD,0x12},

    {SMART_CONFIG_CMD,0x81},//Set Contrast Control
    {SMART_CONFIG_CMD,0xcf},//    {SMART_CONFIG_CMD,0x9f},

    {SMART_CONFIG_CMD,0xd9},//Set Pre-Charge Period
    {SMART_CONFIG_CMD,0xf1},//    {SMART_CONFIG_CMD,0x22},

    {SMART_CONFIG_CMD,0xdb},//Set VCOMH Deselect Level
    {SMART_CONFIG_CMD,0x40},

    {SMART_CONFIG_CMD,0xa4},//Set Entire Display On/Off

    {SMART_CONFIG_CMD,0xa6},//Set Normal/Inverse Display

    {SMART_CONFIG_CMD,0x20},//Set RAM Mode
//    {SMART_CONFIG_CMD,0x00}, //Horizontal Addressing Mode
    {SMART_CONFIG_CMD,0x01}, //Vertical Addressing Mode

    {SMART_CONFIG_CMD,0x2e}, //Forbidden scroll

    {SMART_CONFIG_CMD,0x21}, //Set column start and end addresses
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x7f},

    {SMART_CONFIG_CMD,0x22}, //Set page start and end addresses
    {SMART_CONFIG_CMD,0x00},
    {SMART_CONFIG_CMD,0x07},

    {SMART_CONFIG_CMD,0xaf},//Set Display On
};


static unsigned long ug2864hlbeg01_cmd_buf[]= {
    0xe3e3e3e3,
};

static struct fb_videomode jzfb0_videomode = {
    .name = "128x64",
    .refresh = 60,
    .xres = 4,
    .yres = 128,
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

struct jzfb_platform_data jzfb_pdata = {
    .num_modes = 1,
    .modes = &jzfb0_videomode,
    .lcd_type = LCD_TYPE_SLCD,
    .bpp    = 16,
    .width = 31,
    .height = 31,
    .pinmd  = 0,

    .smart_config.rsply_cmd_high       = 0,
    .smart_config.csply_active_high    = 0,
    .smart_config.newcfg_fmt_conv =  1,
    .smart_config.write_gram_cmd = ug2864hlbeg01_cmd_buf,
    .smart_config.length_cmd = ARRAY_SIZE(ug2864hlbeg01_cmd_buf),
    .smart_config.bus_width = 8,
    .smart_config.length_data_table =  ARRAY_SIZE(ug2864hlbeg01_data_table),
    .smart_config.data_table = ug2864hlbeg01_data_table,
    .dither_enable = 0,
};
