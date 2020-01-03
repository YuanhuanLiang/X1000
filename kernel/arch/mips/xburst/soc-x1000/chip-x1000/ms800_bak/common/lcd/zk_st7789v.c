/*
 * Copyright (c) 2014 Engenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
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
#ifdef CONFIG_SYS_CONFIG
#include <sysconfig/sysconfig.h>
#endif
#include "../board_base.h"

#define CONFIG_SCREEN_HORIZONTAL    0

static int slcd_inited = 0;

static int zk_st7789v_power_init(struct lcd_device *ld)
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

    if(GPIO_LCD_VDD_EN != -1) {
        ret = gpio_request(GPIO_LCD_VDD_EN, "lcd vdd enable");
        if (ret) {
            printk(KERN_ERR "can's request lcd vdd enable\n");
            return ret;
        }
   }

    printk("%s: set slcd_inited = 1\n", __FUNCTION__);
    slcd_inited = 1;

    return 0;
}

static int zk_st7789v_power_reset(struct lcd_device *ld)
{
    if (!slcd_inited && zk_st7789v_power_init(ld))
        return -EFAULT;

    gpio_direction_output(GPIO_LCD_CS, 1);
    gpio_direction_output(GPIO_LCD_RD, 1);

    if(GPIO_LCD_RST != -1) {
        gpio_direction_output(GPIO_LCD_RST, 0);
        mdelay(20);
        gpio_direction_output(GPIO_LCD_RST, 1);
        mdelay(20);
    }

    gpio_direction_output(GPIO_LCD_CS, 0);
    return 0;
}

int zk_st7789v_power_on(struct lcd_device *ld, int enable)
{
    if (!slcd_inited && zk_st7789v_power_init(ld))
        return -EFAULT;

    switch (enable) {
        /* backlight enable */
        case FB_BLANK_UNBLANK:
            if(GPIO_LCD_BACKLIGHT != -1) {
                mdelay(50);
                gpio_direction_output(GPIO_LCD_BACKLIGHT, 1);
            }
            break;

        /* power enable */
        case FB_BLANK_NORMAL:
            if (GPIO_LCD_VDD_EN != -1)
                gpio_direction_output(GPIO_LCD_VDD_EN, 0);

            mdelay(140);

            gpio_direction_output(GPIO_LCD_CS, 1);
            gpio_direction_output(GPIO_LCD_RD, 1);

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
            if(GPIO_LCD_BACKLIGHT != -1) {
                gpio_direction_output(GPIO_LCD_BACKLIGHT, 0);
            }
            break;

        /* power and backlight disable */
        default:
            gpio_direction_output(GPIO_LCD_CS, 1);
            if(GPIO_LCD_BACKLIGHT != -1)
                gpio_direction_output(GPIO_LCD_BACKLIGHT, 0);
            break;

    }

    return 0;
}

static struct lcd_platform_data zk_st7789v_pdata = {
    .reset    = zk_st7789v_power_reset,
    .power_on = zk_st7789v_power_on,
};

/* LCD Panel Device */
struct platform_device zk_st7789v_device = {
    .name       = "zk_st7789v_slcd",
    .dev        = {
        .platform_data  = &zk_st7789v_pdata,
    },
};

static struct smart_lcd_data_table zk_st7789v_data_table[] = {
        /* LCD init code */
    {SMART_CONFIG_CMD, 0xe8},
    {SMART_CONFIG_DATA,0x85},
    {SMART_CONFIG_DATA,0x10},
    {SMART_CONFIG_DATA,0x78},

    {SMART_CONFIG_CMD, 0xb2},
    {SMART_CONFIG_DATA,0x0c},
    {SMART_CONFIG_DATA,0x0c},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x33},
    {SMART_CONFIG_DATA,0x33},

    {SMART_CONFIG_CMD, 0xb7},
    {SMART_CONFIG_DATA,0x35},

    {SMART_CONFIG_CMD, 0xbb},
    {SMART_CONFIG_DATA,0x35},  //vcom

    {SMART_CONFIG_CMD, 0xc0},
    {SMART_CONFIG_DATA,0x2c},

    {SMART_CONFIG_CMD, 0xc2},
    {SMART_CONFIG_DATA,0x01},

    {SMART_CONFIG_CMD, 0xc3},
    {SMART_CONFIG_DATA,0x10},

    {SMART_CONFIG_CMD, 0xc4},
    {SMART_CONFIG_DATA,0x20},

    {SMART_CONFIG_CMD, 0xc6},
    {SMART_CONFIG_DATA,0x0f},

    {SMART_CONFIG_CMD, 0xd0},
    {SMART_CONFIG_DATA,0xa4},
    {SMART_CONFIG_DATA,0xa1},

    {SMART_CONFIG_CMD, 0x36}, // Memory Access Control
#if (CONFIG_SCREEN_HORIZONTAL == 1)
    {SMART_CONFIG_DATA,0x60}, //0x00 RGB  0xc8 BGR
#else
    {SMART_CONFIG_DATA,0x00}, //0x00 RGB  0xc8 BGR
#endif

    {SMART_CONFIG_CMD, 0x3A},
    {SMART_CONFIG_DATA,0x05},

    {SMART_CONFIG_CMD, 0xe0},
    {SMART_CONFIG_DATA,0xd0},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x02},
    {SMART_CONFIG_DATA,0x07},
    {SMART_CONFIG_DATA,0x0a},
    {SMART_CONFIG_DATA,0x28},
    {SMART_CONFIG_DATA,0x32},
    {SMART_CONFIG_DATA,0x44},
    {SMART_CONFIG_DATA,0x42},
    {SMART_CONFIG_DATA,0x06},
    {SMART_CONFIG_DATA,0x0e},
    {SMART_CONFIG_DATA,0x12},
    {SMART_CONFIG_DATA,0x14},
    {SMART_CONFIG_DATA,0x17},

    {SMART_CONFIG_CMD, 0xe1},
    {SMART_CONFIG_DATA,0xd0},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x02},
    {SMART_CONFIG_DATA,0x07},
    {SMART_CONFIG_DATA,0x0a},
    {SMART_CONFIG_DATA,0x28},
    {SMART_CONFIG_DATA,0x31},
    {SMART_CONFIG_DATA,0x54},
    {SMART_CONFIG_DATA,0x47},
    {SMART_CONFIG_DATA,0x0e},
    {SMART_CONFIG_DATA,0x1c},
    {SMART_CONFIG_DATA,0x17},
    {SMART_CONFIG_DATA,0x1b},
    {SMART_CONFIG_DATA,0x1e},

    {SMART_CONFIG_CMD, 0x35},
    {SMART_CONFIG_DATA,0x00},

#if (CONFIG_SCREEN_HORIZONTAL == 1)
    {SMART_CONFIG_CMD, 0x2a},
#else
    {SMART_CONFIG_CMD, 0x2b},
#endif
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x01}, // 320
    {SMART_CONFIG_DATA,0x3f},

#if (CONFIG_SCREEN_HORIZONTAL == 1)
    {SMART_CONFIG_CMD, 0x2b},
#else
    {SMART_CONFIG_CMD, 0x2a},
#endif
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00},
    {SMART_CONFIG_DATA,0x00}, // 240
    {SMART_CONFIG_DATA,0xef},

    {SMART_CONFIG_CMD, 0x11}, //Exit Sleep

    {SMART_CONFIG_UDELAY, 120000},

    {SMART_CONFIG_CMD, 0x29},         //Display on

};


static unsigned long zk_st7789v_cmd_buf[]= {
    0x2C2C2C2C,
};

static struct fb_videomode jzfb0_videomode = {
    .name = "320x240",
    .refresh = 60,
#if (CONFIG_SCREEN_HORIZONTAL == 1)
    .xres = 320,
    .yres = 240,
#else
    .xres = 240,
    .yres = 320,
#endif
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
    /* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
    .smart_config.write_gram_cmd = zk_st7789v_cmd_buf,
    .smart_config.length_cmd = ARRAY_SIZE(zk_st7789v_cmd_buf),
    .smart_config.bus_width = 8,
    .smart_config.length_data_table =  ARRAY_SIZE(zk_st7789v_data_table),
    .smart_config.data_table = zk_st7789v_data_table,
    .dither_enable = 0,
};

#ifdef CONFIG_BACKLIGHT_PWM
static int backlight_init(struct device *dev)
{
    int ret;

    if (gpio_is_valid(GPIO_BL_PWR_EN)) {
        ret = gpio_request(GPIO_BL_PWR_EN, "BL PWR");
        if (ret) {
            printk(KERN_ERR "failed to reqeust BL PWR\n");
            return ret;
        }
        gpio_direction_output(GPIO_BL_PWR_EN, BL_PWR_EN_LEVEL);
    }

    return 0;
}

static int backlight_notify(struct device *dev, int brightness)
{
    if (gpio_is_valid(GPIO_BL_PWR_EN)) {
        if (brightness)
            gpio_direction_output(GPIO_BL_PWR_EN, BL_PWR_EN_LEVEL);
        else
            gpio_direction_output(GPIO_BL_PWR_EN, !BL_PWR_EN_LEVEL);
    }

    return brightness;
}

static void backlight_exit(struct device *dev)
{
    gpio_free(GPIO_BL_PWR_EN);
}

static struct platform_pwm_backlight_data backlight_data = {
    .pwm_id     = 2,
    .max_brightness = 255,
    .dft_brightness = 120,
    .pwm_period_ns  = 30000,
    .active_level   = 1,
    .init       = backlight_init,
    .exit       = backlight_exit,
    .notify     = backlight_notify,
};

struct platform_device backlight_device = {
    .name       = "pwm-backlight",
    .dev        = {
        .platform_data  = &backlight_data,
    },
};
#endif
