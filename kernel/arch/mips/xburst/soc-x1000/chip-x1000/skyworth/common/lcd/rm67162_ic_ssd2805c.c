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

struct rm67162_ic_ssd2805c_power{
    struct regulator *vlcdio;
    struct regulator *vlcdvcc;
    int inited;
};

static struct rm67162_ic_ssd2805c_power lcd_power = {
    NULL,
    NULL,
    0
};

int rm67162_ic_ssd2805c_power_init(struct lcd_device *ld)
{
    int ret ;

    if(GPIO_LCD_CLOCK > 0){
        ret = gpio_request(GPIO_LCD_CLOCK, "lcd clock");
        if (ret) {
            printk(KERN_ERR "can's request lcd cs\n");
            return ret;
        }
        jz_gpio_set_func(GPIO_LCD_CLOCK, GPIO_CLOCK_FUNC);
        mdelay(10);
    }

    if(GPIO_LCD_CS > 0){
        ret = gpio_request(GPIO_LCD_CS, "lcd cs");
        if (ret) {
            printk(KERN_ERR "can's request lcd cs\n");
            return ret;
        }
    }

    if(GPIO_LCD_RST > 0){
        ret = gpio_request(GPIO_LCD_RST, "lcd rst");
        if (ret) {
            printk(KERN_ERR "can's request lcd rst\n");
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

    lcd_power.inited = 1;
    return 0;
}

int rm67162_ic_ssd2805c_power_reset(struct lcd_device *ld)
{
    if (!lcd_power.inited && rm67162_ic_ssd2805c_power_init(ld))
        return -EFAULT;

    gpio_direction_output(GPIO_LCD_CS, 1);
    gpio_direction_output(GPIO_LCD_RD, 1);

    gpio_direction_output(GPIO_LCD_RST, 0);
    mdelay(50);
    gpio_direction_output(GPIO_LCD_RST, 1);
    mdelay(120);

    gpio_direction_output(GPIO_LCD_CS, 0);

    return 0;
}

int rm67162_ic_ssd2805c_power_on(struct lcd_device *ld, int enable)
{
    if (!lcd_power.inited && rm67162_ic_ssd2805c_power_init(ld))
        return -EFAULT;

    switch (enable) {
        /* backlight enable */
        case FB_BLANK_UNBLANK:
            break;

        /* power enable */
        case FB_BLANK_NORMAL:
            gpio_direction_output(GPIO_LCD_CS, 1);
            gpio_direction_output(GPIO_LCD_RD, 1);

            gpio_direction_output(GPIO_LCD_RST, 0);
            mdelay(50);
            gpio_direction_output(GPIO_LCD_RST, 1);
            mdelay(120);

            gpio_direction_output(GPIO_LCD_CS, 0);
            break;

        /* backlight disable */
        case FB_BLANK_VSYNC_SUSPEND:
        case FB_BLANK_HSYNC_SUSPEND:
            break;

        /* power and backlight disable */
        default:
            gpio_direction_output(GPIO_LCD_CS, 1);
            break;

    }

    return 0;
}

struct lcd_platform_data rm67162_ic_ssd2805c_pdata = {
    .reset    = rm67162_ic_ssd2805c_power_reset,
    .power_on = rm67162_ic_ssd2805c_power_on,
};

struct platform_device rm67162_ic_ssd2805c_device = {
    .name = "rm67162_ic_ssd2805c",
    .dev = {
        .platform_data = &rm67162_ic_ssd2805c_pdata,
    },
};

static struct smart_lcd_data_table rm67162_ic_ssd2805c_data_table[] = {

    {SMART_CONFIG_CMD, 0xb9},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xde},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xb7},
    {SMART_CONFIG_DATA, 0x42},
    {SMART_CONFIG_DATA, 0x01},

    {SMART_CONFIG_CMD, 0xba},
    {SMART_CONFIG_DATA, 0x0a},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xb9},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xb8},
    {SMART_CONFIG_DATA, 0x0},
    {SMART_CONFIG_DATA, 0x0},

    {SMART_CONFIG_CMD, 0xbb},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0x1},
    {SMART_CONFIG_DATA, 0x0},
    {SMART_CONFIG_CMD, 0xbd},
    {SMART_CONFIG_DATA, 0x0},
    {SMART_CONFIG_DATA, 0x0},


    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x06},
    {SMART_CONFIG_DATA, 0x62},
    {SMART_CONFIG_CMD, 0x0a},
    {SMART_CONFIG_DATA, 0xe8},
    {SMART_CONFIG_CMD, 0x0e},
    {SMART_CONFIG_DATA, 0x80},
    {SMART_CONFIG_CMD, 0x0f},
    {SMART_CONFIG_DATA, 0x80},
    {SMART_CONFIG_CMD, 0x10},
    {SMART_CONFIG_DATA, 0x71},
    {SMART_CONFIG_CMD, 0x13},
    {SMART_CONFIG_DATA, 0x81},
    {SMART_CONFIG_CMD, 0x14},
    {SMART_CONFIG_DATA, 0x81},
    {SMART_CONFIG_CMD, 0x15},
    {SMART_CONFIG_DATA, 0x82},
    {SMART_CONFIG_CMD, 0x16},
    {SMART_CONFIG_DATA, 0x82},
    {SMART_CONFIG_CMD, 0x18},
    {SMART_CONFIG_DATA, 0x88},
    {SMART_CONFIG_CMD, 0x19},
    {SMART_CONFIG_DATA, 0x55},
    {SMART_CONFIG_CMD, 0x1a},
    {SMART_CONFIG_DATA, 0x10},
    {SMART_CONFIG_CMD, 0x1c},
    {SMART_CONFIG_DATA, 0x99},
    {SMART_CONFIG_CMD, 0x1d},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x1e},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x1f},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x20},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x25},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x26},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x36},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x37},
    {SMART_CONFIG_DATA, 0x10},
    {SMART_CONFIG_CMD, 0x3a},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x3b},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x3d},
    {SMART_CONFIG_DATA, 0x20},
    {SMART_CONFIG_CMD, 0x3f},
    {SMART_CONFIG_DATA, 0x3a},
    {SMART_CONFIG_CMD, 0x40},
    {SMART_CONFIG_DATA, 0x30},
    {SMART_CONFIG_CMD, 0x41},
    {SMART_CONFIG_DATA, 0x30},
    {SMART_CONFIG_CMD, 0x42},
    {SMART_CONFIG_DATA, 0x33},
    {SMART_CONFIG_CMD, 0x43},
    {SMART_CONFIG_DATA, 0x22},
    {SMART_CONFIG_CMD, 0x44},
    {SMART_CONFIG_DATA, 0x11},
    {SMART_CONFIG_CMD, 0x45},
    {SMART_CONFIG_DATA, 0x66},
    {SMART_CONFIG_CMD, 0x46},
    {SMART_CONFIG_DATA, 0x55},
    {SMART_CONFIG_CMD, 0x47},
    {SMART_CONFIG_DATA, 0x44},
    {SMART_CONFIG_CMD, 0x4c},
    {SMART_CONFIG_DATA, 0x33},
    {SMART_CONFIG_CMD, 0x4d},
    {SMART_CONFIG_DATA, 0x22},
    {SMART_CONFIG_CMD, 0x4e},
    {SMART_CONFIG_DATA, 0x11},
    {SMART_CONFIG_CMD, 0x4f},
    {SMART_CONFIG_DATA, 0x66},
    {SMART_CONFIG_CMD, 0x50},
    {SMART_CONFIG_DATA, 0x55},
    {SMART_CONFIG_CMD, 0x51},
    {SMART_CONFIG_DATA, 0x44},
    {SMART_CONFIG_CMD, 0x57},
    {SMART_CONFIG_DATA, 0xb3},
    {SMART_CONFIG_CMD, 0x6b},
    {SMART_CONFIG_DATA, 0x19},
    {SMART_CONFIG_CMD, 0x70},
    {SMART_CONFIG_DATA, 0x55},
    {SMART_CONFIG_CMD, 0x74},
    {SMART_CONFIG_DATA, 0x0c},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x9b},
    {SMART_CONFIG_DATA, 0x40},
    {SMART_CONFIG_CMD, 0x9c},
    {SMART_CONFIG_DATA, 0x67},
    {SMART_CONFIG_CMD, 0x9d},
    {SMART_CONFIG_DATA, 0x20},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x9b},
    {SMART_CONFIG_DATA, 0x40},
    {SMART_CONFIG_CMD, 0x9c},
    {SMART_CONFIG_DATA, 0x67},
    {SMART_CONFIG_CMD, 0x9d},
    {SMART_CONFIG_DATA, 0x20},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x5d},
    {SMART_CONFIG_DATA, 0x10},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x00},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x01},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x02},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x03},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x04},
    {SMART_CONFIG_DATA, 0x10},
    {SMART_CONFIG_CMD, 0x05},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x06},
    {SMART_CONFIG_DATA, 0xa7},
    {SMART_CONFIG_CMD, 0x07},
    {SMART_CONFIG_DATA, 0x20},
    {SMART_CONFIG_CMD, 0x08},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x09},
    {SMART_CONFIG_DATA, 0xc2},
    {SMART_CONFIG_CMD, 0x0a},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x0b},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x0c},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x0d},
    {SMART_CONFIG_DATA, 0x40},
    {SMART_CONFIG_CMD, 0x0e},
    {SMART_CONFIG_DATA, 0x06},
    {SMART_CONFIG_CMD, 0x0f},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x10},
    {SMART_CONFIG_DATA, 0xa7},
    {SMART_CONFIG_CMD, 0x11},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x12},
    {SMART_CONFIG_DATA, 0xc2},
    {SMART_CONFIG_CMD, 0x13},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x14},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x15},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x16},
    {SMART_CONFIG_DATA, 0x40},
    {SMART_CONFIG_CMD, 0x17},
    {SMART_CONFIG_DATA, 0x07},
    {SMART_CONFIG_CMD, 0x18},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x19},
    {SMART_CONFIG_DATA, 0xa7},
    {SMART_CONFIG_CMD, 0x1a},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x1b},
    {SMART_CONFIG_DATA, 0x82},
    {SMART_CONFIG_CMD, 0x1c},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x1d},
    {SMART_CONFIG_DATA, 0xff},
    {SMART_CONFIG_CMD, 0x1e},
    {SMART_CONFIG_DATA, 0x05},
    {SMART_CONFIG_CMD, 0x1f},
    {SMART_CONFIG_DATA, 0x60},
    {SMART_CONFIG_CMD, 0x20},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x21},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x22},
    {SMART_CONFIG_DATA, 0x7c},
    {SMART_CONFIG_CMD, 0x23},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x24},
    {SMART_CONFIG_DATA, 0xc2},
    {SMART_CONFIG_CMD, 0x25},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x26},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x27},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x28},
    {SMART_CONFIG_DATA, 0x70},
    {SMART_CONFIG_CMD, 0x29},
    {SMART_CONFIG_DATA, 0x05},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x74},
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x2d},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x2f},
    {SMART_CONFIG_DATA, 0xc2},
    {SMART_CONFIG_CMD, 0x30},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x31},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x32},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x33},
    {SMART_CONFIG_DATA, 0x70},
    {SMART_CONFIG_CMD, 0x34},
    {SMART_CONFIG_DATA, 0x07},
    {SMART_CONFIG_CMD, 0x35},
    {SMART_CONFIG_DATA, 0x74},
    {SMART_CONFIG_CMD, 0x36},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x37},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x5e},
    {SMART_CONFIG_DATA, 0x20},
    {SMART_CONFIG_CMD, 0x5f},
    {SMART_CONFIG_DATA, 0x31},
    {SMART_CONFIG_CMD, 0x60},
    {SMART_CONFIG_DATA, 0x54},
    {SMART_CONFIG_CMD, 0x61},
    {SMART_CONFIG_DATA, 0x76},
    {SMART_CONFIG_CMD, 0x62},
    {SMART_CONFIG_DATA, 0x98},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x05},
    {SMART_CONFIG_CMD, 0x05},
    {SMART_CONFIG_DATA, 0x17},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x91},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x35},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x06},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_DATA, 0x8b},
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_DATA, 0x85},

    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x11},
    {SMART_CONFIG_UDELAY, 200000},
    {SMART_CONFIG_CMD, 0x29},

    {SMART_CONFIG_UDELAY, 10000},
    {SMART_CONFIG_CMD, 0xb7},
    {SMART_CONFIG_DATA, 0x43},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0x6c},
    {SMART_CONFIG_DATA, 0xff},
    {SMART_CONFIG_CMD, 0xbd},
    {SMART_CONFIG_DATA, 0x06},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0xbe},
    {SMART_CONFIG_DATA, 0x0c},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x2c},


// test 8080 16bit
#if 0
    {SMART_CONFIG_CMD, 0xb9},
    {SMART_CONFIG_DATA, 0x0},

    {SMART_CONFIG_CMD, 0xde},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xb7},
    {SMART_CONFIG_DATA, 0x0142},

    {SMART_CONFIG_CMD, 0xba},
    {SMART_CONFIG_DATA, 0x000a},

    {SMART_CONFIG_CMD, 0xbb},
    {SMART_CONFIG_DATA, 0x0040},

    {SMART_CONFIG_CMD, 0xb9},
    {SMART_CONFIG_DATA, 0x0001},

    {SMART_CONFIG_CMD, 0xb8},
    {SMART_CONFIG_DATA, 0x0},

    {SMART_CONFIG_CMD, 0xbb},
    {SMART_CONFIG_DATA, 0x0004},

    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0x1},
    {SMART_CONFIG_CMD, 0xbd},
    {SMART_CONFIG_DATA, 0x0},


    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x06},
    {SMART_CONFIG_DATA, 0x62},
    {SMART_CONFIG_CMD, 0x0a},
    {SMART_CONFIG_DATA, 0xe8},
    {SMART_CONFIG_CMD, 0x0e},
    {SMART_CONFIG_DATA, 0x80},
    {SMART_CONFIG_CMD, 0x0f},
    {SMART_CONFIG_DATA, 0x80},
    {SMART_CONFIG_CMD, 0x10},
    {SMART_CONFIG_DATA, 0x71},
    {SMART_CONFIG_CMD, 0x13},
    {SMART_CONFIG_DATA, 0x81},
    {SMART_CONFIG_CMD, 0x14},
    {SMART_CONFIG_DATA, 0x81},
    {SMART_CONFIG_CMD, 0x15},
    {SMART_CONFIG_DATA, 0x82},
    {SMART_CONFIG_CMD, 0x16},
    {SMART_CONFIG_DATA, 0x82},
    {SMART_CONFIG_CMD, 0x18},
    {SMART_CONFIG_DATA, 0x88},
    {SMART_CONFIG_CMD, 0x19},
    {SMART_CONFIG_DATA, 0x55},
    {SMART_CONFIG_CMD, 0x1a},
    {SMART_CONFIG_DATA, 0x10},
    {SMART_CONFIG_CMD, 0x1c},
    {SMART_CONFIG_DATA, 0x99},
    {SMART_CONFIG_CMD, 0x1d},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x1e},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x1f},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x20},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x25},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x26},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x36},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x37},
    {SMART_CONFIG_DATA, 0x10},
    {SMART_CONFIG_CMD, 0x3a},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x3b},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x3d},
    {SMART_CONFIG_DATA, 0x20},
    {SMART_CONFIG_CMD, 0x3f},
    {SMART_CONFIG_DATA, 0x3a},
    {SMART_CONFIG_CMD, 0x40},
    {SMART_CONFIG_DATA, 0x30},
    {SMART_CONFIG_CMD, 0x41},
    {SMART_CONFIG_DATA, 0x30},
    {SMART_CONFIG_CMD, 0x42},
    {SMART_CONFIG_DATA, 0x33},
    {SMART_CONFIG_CMD, 0x43},
    {SMART_CONFIG_DATA, 0x22},
    {SMART_CONFIG_CMD, 0x44},
    {SMART_CONFIG_DATA, 0x11},
    {SMART_CONFIG_CMD, 0x45},
    {SMART_CONFIG_DATA, 0x66},
    {SMART_CONFIG_CMD, 0x46},
    {SMART_CONFIG_DATA, 0x55},
    {SMART_CONFIG_CMD, 0x47},
    {SMART_CONFIG_DATA, 0x44},
    {SMART_CONFIG_CMD, 0x4c},
    {SMART_CONFIG_DATA, 0x33},
    {SMART_CONFIG_CMD, 0x4d},
    {SMART_CONFIG_DATA, 0x22},
    {SMART_CONFIG_CMD, 0x4e},
    {SMART_CONFIG_DATA, 0x11},
    {SMART_CONFIG_CMD, 0x4f},
    {SMART_CONFIG_DATA, 0x66},
    {SMART_CONFIG_CMD, 0x50},
    {SMART_CONFIG_DATA, 0x55},
    {SMART_CONFIG_CMD, 0x51},
    {SMART_CONFIG_DATA, 0x44},
    {SMART_CONFIG_CMD, 0x57},
    {SMART_CONFIG_DATA, 0xb3},
    {SMART_CONFIG_CMD, 0x6b},
    {SMART_CONFIG_DATA, 0x19},
    {SMART_CONFIG_CMD, 0x70},
    {SMART_CONFIG_DATA, 0x55},
    {SMART_CONFIG_CMD, 0x74},
    {SMART_CONFIG_DATA, 0x0c},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x9b},
    {SMART_CONFIG_DATA, 0x40},
    {SMART_CONFIG_CMD, 0x9c},
    {SMART_CONFIG_DATA, 0x67},
    {SMART_CONFIG_CMD, 0x9d},
    {SMART_CONFIG_DATA, 0x20},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x03},
    {SMART_CONFIG_CMD, 0x9b},
    {SMART_CONFIG_DATA, 0x40},
    {SMART_CONFIG_CMD, 0x9c},
    {SMART_CONFIG_DATA, 0x67},
    {SMART_CONFIG_CMD, 0x9d},
    {SMART_CONFIG_DATA, 0x20},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x5d},
    {SMART_CONFIG_DATA, 0x10},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x00},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x01},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x02},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x03},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x04},
    {SMART_CONFIG_DATA, 0x10},
    {SMART_CONFIG_CMD, 0x05},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x06},
    {SMART_CONFIG_DATA, 0xa7},
    {SMART_CONFIG_CMD, 0x07},
    {SMART_CONFIG_DATA, 0x20},
    {SMART_CONFIG_CMD, 0x08},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x09},
    {SMART_CONFIG_DATA, 0xc2},
    {SMART_CONFIG_CMD, 0x0a},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x0b},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x0c},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x0d},
    {SMART_CONFIG_DATA, 0x40},
    {SMART_CONFIG_CMD, 0x0e},
    {SMART_CONFIG_DATA, 0x06},
    {SMART_CONFIG_CMD, 0x0f},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x10},
    {SMART_CONFIG_DATA, 0xa7},
    {SMART_CONFIG_CMD, 0x11},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x12},
    {SMART_CONFIG_DATA, 0xc2},
    {SMART_CONFIG_CMD, 0x13},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x14},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x15},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x16},
    {SMART_CONFIG_DATA, 0x40},
    {SMART_CONFIG_CMD, 0x17},
    {SMART_CONFIG_DATA, 0x07},
    {SMART_CONFIG_CMD, 0x18},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x19},
    {SMART_CONFIG_DATA, 0xa7},
    {SMART_CONFIG_CMD, 0x1a},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x1b},
    {SMART_CONFIG_DATA, 0x82},
    {SMART_CONFIG_CMD, 0x1c},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x1d},
    {SMART_CONFIG_DATA, 0xff},
    {SMART_CONFIG_CMD, 0x1e},
    {SMART_CONFIG_DATA, 0x05},
    {SMART_CONFIG_CMD, 0x1f},
    {SMART_CONFIG_DATA, 0x60},
    {SMART_CONFIG_CMD, 0x20},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x21},
    {SMART_CONFIG_DATA, 0x01},
    {SMART_CONFIG_CMD, 0x22},
    {SMART_CONFIG_DATA, 0x7c},
    {SMART_CONFIG_CMD, 0x23},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x24},
    {SMART_CONFIG_DATA, 0xc2},
    {SMART_CONFIG_CMD, 0x25},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x26},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x27},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x28},
    {SMART_CONFIG_DATA, 0x70},
    {SMART_CONFIG_CMD, 0x29},
    {SMART_CONFIG_DATA, 0x05},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x74},
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x2d},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x2f},
    {SMART_CONFIG_DATA, 0xc2},
    {SMART_CONFIG_CMD, 0x30},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x31},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x32},
    {SMART_CONFIG_DATA, 0x02},
    {SMART_CONFIG_CMD, 0x33},
    {SMART_CONFIG_DATA, 0x70},
    {SMART_CONFIG_CMD, 0x34},
    {SMART_CONFIG_DATA, 0x07},
    {SMART_CONFIG_CMD, 0x35},
    {SMART_CONFIG_DATA, 0x74},
    {SMART_CONFIG_CMD, 0x36},
    {SMART_CONFIG_DATA, 0x8d},
    {SMART_CONFIG_CMD, 0x37},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x5e},
    {SMART_CONFIG_DATA, 0x20},
    {SMART_CONFIG_CMD, 0x5f},
    {SMART_CONFIG_DATA, 0x31},
    {SMART_CONFIG_CMD, 0x60},
    {SMART_CONFIG_DATA, 0x54},
    {SMART_CONFIG_CMD, 0x61},
    {SMART_CONFIG_DATA, 0x76},
    {SMART_CONFIG_CMD, 0x62},
    {SMART_CONFIG_DATA, 0x98},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x05},
    {SMART_CONFIG_CMD, 0x05},
    {SMART_CONFIG_DATA, 0x17},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x91},
    {SMART_CONFIG_DATA, 0x00},

    {SMART_CONFIG_CMD, 0xfe},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x35},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0x04},
    {SMART_CONFIG_CMD, 0x2a},
    {SMART_CONFIG_DATA, 0x0600},
    {SMART_CONFIG_DATA, 0x8b01},
    {SMART_CONFIG_CMD, 0x2b},
    {SMART_CONFIG_DATA, 0x0000},
    {SMART_CONFIG_DATA, 0x8501},

    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0x00},
    {SMART_CONFIG_CMD, 0x11},
    {SMART_CONFIG_UDELAY, 200000},
    {SMART_CONFIG_CMD, 0x29},

    {SMART_CONFIG_UDELAY, 10000},
    {SMART_CONFIG_CMD, 0xb7},
    {SMART_CONFIG_DATA, 0x0143},
    {SMART_CONFIG_CMD, 0xbc},
    {SMART_CONFIG_DATA, 0xff6c},
    {SMART_CONFIG_CMD, 0xbd},
    {SMART_CONFIG_DATA, 0x06},
    {SMART_CONFIG_CMD, 0xbe},
    {SMART_CONFIG_DATA, 0x030c},
    {SMART_CONFIG_CMD, 0x2c},

#endif

};


unsigned long rm67162_ic_ssd2805c_cmd_buf[]= {
    0x2c2c2c2c,
};

struct fb_videomode jzfb0_videomode = {
    .name = "390x390",
    .refresh = 60,
    .xres = 390,
    .yres = 390,
    .init_pixclock = KHZ2PICOS(10000),
    .pixclock = KHZ2PICOS(50000),
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
    .smart_config.write_gram_cmd = rm67162_ic_ssd2805c_cmd_buf,
    .smart_config.length_cmd = ARRAY_SIZE(rm67162_ic_ssd2805c_cmd_buf),
    .smart_config.bus_width = 8,
    .smart_config.length_data_table =  ARRAY_SIZE(rm67162_ic_ssd2805c_data_table),
    .smart_config.data_table = rm67162_ic_ssd2805c_data_table,
    .dither_enable = 0,
};
