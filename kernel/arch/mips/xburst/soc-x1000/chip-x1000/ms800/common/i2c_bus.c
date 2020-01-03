#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/interrupt.h>
#include "board_base.h"
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/cypress_psoc4.h>
#include <linux/input/zinitix_ts.h>
#include <mach/jzsnd.h>

#ifdef CONFIG_EEPROM_AT24
#include <asm-generic/sizes.h>
#include <linux/i2c/at24.h>
#include <mach/platform.h>
#endif

#define SZ_16K	0x00004000


#if (defined(CONFIG_AKM4951_EXTERNAL_CODEC) || defined(CONFIG_SND_ASOC_JZ_EXTCODEC_AKM4951))
static struct snd_board_gpio power_down = {
    .gpio = GPIO_AKM4951_PDN,
    .active_level = LOW_ENABLE,
};

static struct akm4951_platform_data akm4951_data = {
    .pdn = &power_down,
};
#endif

#ifdef CONFIG_EEPROM_AT24
static struct at24_platform_data at24c16 = {
    .byte_len = SZ_16K / 8,
    .page_size = 16,

};
#endif
#ifdef CONFIG_WM8594_CODEC_V12
static struct snd_codec_data wm8594_codec_pdata = {
    .codec_sys_clk = 1200000,

};
#endif
#ifdef CONFIG_CYPRESS_PSOC4
static unsigned char keyscode[] = {
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_LEFT, KEY_RIGHT, KEY_WAKEUP,
};

static struct cypress_psoc4_platform_data cypress_psoc4_pdata = {
#ifdef CONFIG_CYPRESS_PSOC4_SYSFS
    .name = "cy8c4024",
#endif
    .cpu_int_pin = CYPRESS_PSOC4_CPU_INT_PIN,
    .mcu_int_pin = CYPRESS_PSOC4_MCU_INT_PIN,
    .mcu_rst_pin = CYPRESS_PSOC4_MCU_RST_PIN,

    .mcu_int_level = 0,
    .mcu_rst_level = RST_LOW_EN,
    .keyscode  = keyscode,
    .keyscode_num = ARRAY_SIZE(keyscode),
};
#endif


#ifdef CONFIG_TOUCHSCREEN_ZINITIX
static struct zinitix_ts_platform_data zinitix_ts_pdata = {
    .x_max = 720,
    .y_max = 1280,
    .ts_rst_pin = ZINITIX_TS_RST_PIN,
    .ts_int_pin = ZINITIX_TS_INT_PIN,
    .ts_pwr_pin = ZINITIX_TS_PWR_PIN,
    .pwr_en_level = ZINITIX_TS_PWR_EN_LEVEL,
};
#endif

#if (defined(CONFIG_SOFT_I2C0_GPIO_V12_JZ) || defined(CONFIG_I2C0_V12_JZ))
struct i2c_board_info jz_i2c0_devs[] __initdata = {
#ifdef CONFIG_SENSORS_BMA2X2
    {
        I2C_BOARD_INFO("bma2x2", 0x18),
        .irq = GPIO_GSENSOR_INTR,
    },
#endif
#ifdef CONFIG_I2C_MUX_PCA9543
    {
        I2C_BOARD_INFO("pca9543", 0x70),
    },
#endif
};
int jz_i2c0_devs_size = ARRAY_SIZE(jz_i2c0_devs);

struct i2c_board_info jz_v4l2_camera_devs[] __initdata = {
#ifdef CONFIG_SOC_CAMERA_OV5640
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("ov5640-front", 0x3c),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_OV7740
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("ov7740", 0x21),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_GC0308
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("gc0308", 0x21),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_GC0328
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("gc0328", 0x21),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_GC0328_DOUBLE
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("gc0328_double", 0x21),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_GC2155
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("gc2155", 0x3c),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_SC031GS
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("sc031gs", 0x30),
    },
#endif

};
int jz_v4l2_devs_size = ARRAY_SIZE(jz_v4l2_camera_devs);
#endif

#if (defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ))
struct i2c_board_info jz_i2c2_devs[] __initdata = {
#ifdef CONFIG_EEPROM_AT24
    {
        I2C_BOARD_INFO("at24",0x57),
        .platform_data = &at24c16,
    },
#endif
#ifdef CONFIG_WM8594_CODEC_V12
    {
        I2C_BOARD_INFO("wm8594", 0x1a),
        .platform_data = &wm8594_codec_pdata,
    },
#endif
#if (defined(CONFIG_AKM4951_EXTERNAL_CODEC) || defined(CONFIG_SND_ASOC_JZ_EXTCODEC_AKM4951))
    {
        I2C_BOARD_INFO("akm4951", 0x12),
        .platform_data = &akm4951_data,
    },
#endif
#ifdef CONFIG_CYPRESS_PSOC4
    {
        I2C_BOARD_INFO("cypress_psoc4", 0x08),
        .platform_data = &cypress_psoc4_pdata,
    },
#endif
#ifdef CONFIG_TOUCHSCREEN_ZINITIX
    {
        I2C_BOARD_INFO("zinitix", 0x20),
        .platform_data = &zinitix_ts_pdata,
    },
#endif
};
#endif

#if (defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ))
struct i2c_board_info jz_i2c1_devs[] __initdata = {
#ifdef CONFIG_EEPROM_AT24
    {
        I2C_BOARD_INFO("at24",0x57),
        .platform_data = &at24c16,
    },
#endif
#ifdef CONFIG_WM8594_CODEC_V12
    {
        I2C_BOARD_INFO("wm8594", 0x1a),
        .platform_data = &wm8594_codec_pdata,
    },
#endif
#ifdef CONFIG_CYPRESS_PSOC4
    {
        I2C_BOARD_INFO("cypress_psoc4", 0x08),
        .platform_data = &cypress_psoc4_pdata,
    },
#endif
#ifdef CONFIG_MM9932
	{
        I2C_BOARD_INFO("mm9932", 0x5B),
        .platform_data = NULL,
	},
#endif
};
#endif

#if defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ)
int jz_i2c2_devs_size = ARRAY_SIZE(jz_i2c2_devs);
#endif

#if defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ)
int jz_i2c1_devs_size = ARRAY_SIZE(jz_i2c1_devs);
#endif

#ifdef CONFIG_EEPROM_AT24

struct i2c_client *at24_client;

static int at24_dev_init(void)
{
    struct i2c_adapter *i2c_adap;

#if defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ)
    i2c_adap = i2c_get_adapter(2);
    at24_client = i2c_new_device(i2c_adap, jz_i2c2_devs);
#endif

#if defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ)
    i2c_adap = i2c_get_adapter(1);
    at24_client = i2c_new_device(i2c_adap, jz_i2c1_devs);
#endif
    i2c_put_adapter(i2c_adap);

    return 0;
}

static void at24_dev_exit(void)
{
    i2c_unregister_device(at24_client);
}

module_init(at24_dev_init);
module_exit(at24_dev_exit);

MODULE_LICENSE("GPL");
#endif
#ifdef CONFIG_I2C_GPIO
#define DEF_GPIO_I2C(NO)                        \
    static struct i2c_gpio_platform_data i2c##NO##_gpio_data = {    \
        .sda_pin    = GPIO_I2C##NO##_SDA,           \
        .scl_pin    = GPIO_I2C##NO##_SCK,           \
        .udelay 	= 1,                            \
        .timeout	= 100,                          \
    };                              \
    struct platform_device i2c##NO##_gpio_device = {        \
        .name   = "i2c-gpio",                   \
        .id = NO,                       \
        .dev    = { .platform_data = &i2c##NO##_gpio_data,},    \
    };
#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
DEF_GPIO_I2C(1);
#endif
#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
DEF_GPIO_I2C(0);
#endif
#endif /*CONFIG_I2C_GPIO*/
