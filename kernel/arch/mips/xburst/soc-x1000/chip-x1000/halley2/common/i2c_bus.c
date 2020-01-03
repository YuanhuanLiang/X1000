#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/interrupt.h>
#include "board_base.h"
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <mach/jzsnd.h>
#ifdef CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP5
#include <linux/cyttsp5_core.h>
#include <linux/cyttsp5_platform.h>
#include <uapi/linux/input.h>
#endif

#ifdef CONFIG_EEPROM_AT24
#include <asm-generic/sizes.h>
#include <linux/i2c/at24.h>
#include <mach/platform.h>
#endif

#define SZ_16K  0x00004000

//extern struct platform_device jz_i2c2_device;

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
#ifdef CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP5
#define CYTTSP5_HID_DESC_REGISTER       1
#define CY_VKEYS_X                      720
#define CY_VKEYS_Y                      1280
#define CY_MAXX                         880
#define CY_MAXY                         1280
#define CY_MINX                         0
#define CY_MINY                         0
#define CY_ABS_MIN_X                    CY_MINX
#define CY_ABS_MIN_Y                    CY_MINY
#define CY_ABS_MAX_X                    CY_MAXX
#define CY_ABS_MAX_Y                    CY_MAXY
#define CY_ABS_MIN_P                    0
#define CY_ABS_MAX_P                    255
#define CY_ABS_MIN_W                    0
#define CY_ABS_MAX_W                    255
#define CY_PROXIMITY_MIN_VAL            0
#define CY_PROXIMITY_MAX_VAL            1
#define CY_ABS_MIN_T                    0
#define CY_ABS_MAX_T                    15
#define CYTTSP5_I2C_TCH_ADR             0x24
#define CYTTSP5_LDR_TCH_ADR             0x24

/* Button to keycode conversion */
static u16 cyttsp5_btn_keys[] = {
    /* use this table to map buttons to keycodes (see input.h) */
    KEY_HOMEPAGE,       /* 172 */ /* Previously was KEY_HOME (102) */
                /* New Android versions use KEY_HOMEPAGE */
    KEY_MENU,       /* 139 */
    KEY_BACK,       /* 158 */
    KEY_SEARCH,     /* 217 */
    KEY_VOLUMEDOWN,     /* 114 */
    KEY_VOLUMEUP,       /* 115 */
    KEY_CAMERA,     /* 212 */
    KEY_POWER       /* 116 */
};

static struct touch_settings cyttsp5_sett_btn_keys = {
    .data = (uint8_t *)&cyttsp5_btn_keys[0],
    .size = ARRAY_SIZE(cyttsp5_btn_keys),
    .tag = 0,
};

static struct cyttsp5_core_platform_data _cyttsp5_core_platform_data = {
    .irq_gpio = CYTTSP5_I2C_IRQ_GPIO,
    .rst_gpio = CYTTSP5_I2C_RST_GPIO,
    .hid_desc_register = CYTTSP5_HID_DESC_REGISTER,
    .xres = cyttsp5_xres,
    .init = cyttsp5_init,
    .power = cyttsp5_power,
    .detect = cyttsp5_detect,
    .irq_stat = cyttsp5_irq_stat,
    .sett = {
        NULL,   /* Reserved */
        NULL,   /* Command Registers */
        NULL,   /* Touch Report */
        NULL,   /* Parade Data Record */
        NULL,   /* Test Record */
        NULL,   /* Panel Configuration Record */
        NULL,   /* &cyttsp5_sett_param_regs, */
        NULL,   /* &cyttsp5_sett_param_size, */
        NULL,   /* Reserved */
        NULL,   /* Reserved */
        NULL,   /* Operational Configuration Record */
        NULL, /* &cyttsp5_sett_ddata, *//* Design Data Record */
        NULL, /* &cyttsp5_sett_mdata, *//* Manufacturing Data Record */
        NULL,   /* Config and Test Registers */
        &cyttsp5_sett_btn_keys, /* button-to-keycode table */
    },
    .flags = CY_CORE_FLAG_RESTORE_PARAMETERS,
    .easy_wakeup_gesture = CY_CORE_EWG_NONE,
};

static const int16_t cyttsp5_abs[] = {
    ABS_MT_POSITION_Y, CY_ABS_MIN_X, CY_ABS_MAX_X, 0, 0,
    ABS_MT_POSITION_X, CY_ABS_MIN_Y, CY_ABS_MAX_Y, 0, 0,
    ABS_MT_PRESSURE, CY_ABS_MIN_P, CY_ABS_MAX_P, 0, 0,
    CY_IGNORE_VALUE, CY_ABS_MIN_W, CY_ABS_MAX_W, 0, 0,
    ABS_MT_TRACKING_ID, CY_ABS_MIN_T, CY_ABS_MAX_T, 0, 0,
    ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0,
    ABS_MT_TOUCH_MINOR, 0, 255, 0, 0,
    ABS_MT_ORIENTATION, -127, 127, 0, 0,
    ABS_MT_TOOL_TYPE, 0, MT_TOOL_MAX, 0, 0,
    ABS_MT_DISTANCE, 0, 255, 0, 0,  /* Used with hover */
};

struct touch_framework cyttsp5_framework = {
    .abs = (uint16_t *)&cyttsp5_abs[0],
    .size = ARRAY_SIZE(cyttsp5_abs),
    .enable_vkeys = 0,
};

static struct cyttsp5_mt_platform_data _cyttsp5_mt_platform_data = {
    .frmwrk = &cyttsp5_framework,
    .flags = CY_MT_FLAG_INV_X | CY_MT_FLAG_INV_Y,
    .inp_dev_name = CYTTSP5_MT_NAME,
    .vkeys_x = CY_VKEYS_X,
    .vkeys_y = CY_VKEYS_Y,
};

static struct cyttsp5_btn_platform_data _cyttsp5_btn_platform_data = {
    .inp_dev_name = CYTTSP5_BTN_NAME,
};

static const int16_t cyttsp5_prox_abs[] = {
    ABS_DISTANCE, CY_PROXIMITY_MIN_VAL, CY_PROXIMITY_MAX_VAL, 0, 0,
};

struct touch_framework cyttsp5_prox_framework = {
    .abs = (uint16_t *)&cyttsp5_prox_abs[0],
    .size = ARRAY_SIZE(cyttsp5_prox_abs),
};

static struct cyttsp5_proximity_platform_data
        _cyttsp5_proximity_platform_data = {
    .frmwrk = &cyttsp5_prox_framework,
    .inp_dev_name = CYTTSP5_PROXIMITY_NAME,
};

static struct cyttsp5_platform_data _cyttsp5_platform_data = {
    .core_pdata = &_cyttsp5_core_platform_data,
    .mt_pdata = &_cyttsp5_mt_platform_data,
    .loader_pdata = &_cyttsp5_loader_platform_data,
    .btn_pdata = &_cyttsp5_btn_platform_data,
    .prox_pdata = &_cyttsp5_proximity_platform_data,
};


#endif
#ifdef CONFIG_TOUCHSCREEN_FT6X06
#include <linux/input/ft6x06_ts.h>
struct ft6x06_platform_data ft6x06_tsc_pdata = {
    .x_max          = 240,
    .y_max          = 240,
    .va_x_max   = 240,
    .va_y_max   = 240,
    .irqflags = IRQF_TRIGGER_FALLING|IRQF_DISABLED,
    .irq = GPIO_TP_INT,
    .reset = GPIO_TP_RESET,
    .power = GPIO_TP_POWER,
    .power_level_en = GPIO_TP_POWER_LEVEL,
};
#endif

#ifdef CONFIG_IR_EM20918
#include <linux/input/em20918.h>
static struct em20918_platform_data em20918_pdata = {
    .int_pin                    = EM20918_INT_PIN,
    .interrupt_key_code         = 1,
    .capture_key_code           = 2,
};
#endif


#if (defined(CONFIG_SOFT_I2C0_GPIO_V12_JZ) || defined(CONFIG_I2C0_V12_JZ))
struct i2c_board_info jz_i2c0_devs[] __initdata = {
/*
 * sensor i2c info for jz sensor drivers
 */
#ifdef CONFIG_CIM_SENSOR_GC2155
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("gc2155", 0x3c),
        .platform_data = &cim_sensor_pdata,
    },
#endif
#ifdef CONFIG_CIM_SENSOR_GC0308
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("gc0308", 0x21),
        .platform_data = &cim_sensor_pdata,
    },
#endif
#ifdef CONFIG_CIM_SENSOR_OV7725
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("ov7725", 0x21),
        .platform_data = &cim_sensor_pdata,
    },
#endif
#ifdef CONFIG_SENSORS_BMA2X2
    {
        I2C_BOARD_INFO("bma2x2", 0x18),
        .irq = GPIO_GSENSOR_INTR,
    },
#endif
#ifdef CONFIG_TOUCHSCREEN_FT6X06
    {
        I2C_BOARD_INFO("ft6x06_ts", 0x38),
        .platform_data = &ft6x06_tsc_pdata,
    },
#endif
#ifdef CONFIG_IR_EM20918
    {
        I2C_BOARD_INFO("ir_em20918", 0x24),
        .platform_data = &em20918_pdata,
    },
#endif
};
int jz_i2c0_devs_size = ARRAY_SIZE(jz_i2c0_devs);

#ifdef CONFIG_SOC_CAMERA
/*
 * sensor i2c info for v4l2 camera drivers
 */
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
#ifdef CONFIG_SOC_CAMERA_OV7725
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("ov7725", 0x21),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_GC0308
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("gc0308", 0x21),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_GC2155
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("gc2155", 0x3c),
    },
#endif
#ifdef CONFIG_SOC_CAMERA_OV9282
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("ov9282", 0x60), //0xc0 or 0xe0
    },
#endif
#ifdef CONFIG_SOC_CAMERA_SP1409
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("sp1409", 0x34), //0xc0 or 0xe0
    },
#endif

#ifdef CONFIG_SOC_CAMERA_OV2640
    [FRONT_CAMERA_INDEX] = {
        I2C_BOARD_INFO("ov2640", 0x30),
    },
#endif
};
int jz_v4l2_devs_size = ARRAY_SIZE(jz_v4l2_camera_devs);
#endif
#endif /* CONFIG_SOC_CAMERA */

#if (defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ))
struct i2c_board_info jz_i2c2_devs[] __initdata = {
#ifdef CONFIG_EEPROM_AT24
    {
        I2C_BOARD_INFO("at24",0x57),
        .platform_data  = &at24c16,
    },
#endif
#ifdef CONFIG_WM8594_CODEC_V12
    {
        I2C_BOARD_INFO("wm8594", 0x1a),
        .platform_data  = &wm8594_codec_pdata,
    },
#endif
};
#endif

#if (defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ))
struct i2c_board_info jz_i2c1_devs[] __initdata = {
#ifdef CONFIG_EEPROM_AT24
    {
        I2C_BOARD_INFO("at24",0x57),
        .platform_data  = &at24c16,
    },
#endif
#ifdef CONFIG_WM8594_CODEC_V12
    {
        I2C_BOARD_INFO("wm8594", 0x1a),
        .platform_data  = &wm8594_codec_pdata,
    },
#endif
#ifdef CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP5
    {
        I2C_BOARD_INFO(CYTTSP5_I2C_NAME, CYTTSP5_I2C_TCH_ADR),
        .platform_data = &_cyttsp5_platform_data,
    },
#endif

};
#endif


#if     defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ)
int jz_i2c2_devs_size = ARRAY_SIZE(jz_i2c2_devs);
#endif

#if     defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ)
int jz_i2c1_devs_size = ARRAY_SIZE(jz_i2c1_devs);
#endif


#ifdef CONFIG_EEPROM_AT24

struct i2c_client *at24_client;

static int  at24_dev_init(void)
{
    struct i2c_adapter *i2c_adap;


#if     defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ)
    i2c_adap = i2c_get_adapter(2);
    at24_client = i2c_new_device(i2c_adap, jz_i2c2_devs);
#endif
#if     defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ)
    i2c_adap = i2c_get_adapter(1);
    at24_client = i2c_new_device(i2c_adap, jz_i2c1_devs);
#endif
    i2c_put_adapter(i2c_adap);

    return 0;
}


static void  at24_dev_exit(void)
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
