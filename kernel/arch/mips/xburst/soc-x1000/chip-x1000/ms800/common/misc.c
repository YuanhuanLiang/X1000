#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/leds.h>
#include <linux/leds_pwm.h>
#include <linux/74hc595.h>
#include <linux/zb_cc2530.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/power/pwm_battery.h>
//#include <linux/android_pmem.h>
#include <mach/platform.h>
#include <mach/jzsnd.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <mach/jz_efuse.h>
#include <mach/jz_uart.h>
#include <gpio.h>
#include <linux/jz_dwc.h>
#include <linux/tm1620.h>
#include <linux/sgm42609_motor.h>
#include <linux/sgm42507.h>
#include <linux/fxl_lock_body.h>
#include <linux/mili_lock_body.h>
#include <linux/lock_cylinder.h>
#include <linux/interrupt.h>
//#include <sound/jz-aic.h>
#include "board_base.h"

#ifdef CONFIG_JZ_MAC
#ifndef CONFIG_MDIO_GPIO
#ifdef CONFIG_JZGPIO_PHY_RESET
static struct jz_gpio_phy_reset gpio_phy_reset = {
    .gpio           = GMAC_PHY_PORT_GPIO,
    .active_level   = GMAC_PHY_ACTIVE_HIGH,
    .crtl_port      = GMAC_CRLT_PORT,
    .crtl_pins      = GMAC_CRLT_PORT_PINS,
    .set_func       = GMAC_CRTL_PORT_SET_FUNC,
    .delaytime_msec = GMAC_PHY_DELAYTIME,
};
#endif
struct platform_device jz_mii_bus = {
    .name = "jz_mii_bus",
#ifdef CONFIG_JZGPIO_PHY_RESET
    .dev.platform_data = &gpio_phy_reset,
#endif
};
#else /* CONFIG_MDIO_GPIO */
static struct mdio_gpio_platform_data mdio_gpio_data = {
    .mdc      = MDIO_MDIO_MDC_GPIO,
    .mdio     = MDIO_MDIO_GPIO,
    .phy_mask = 0,
    .irqs     = { 0 },
};

struct platform_device jz_mii_bus = {
    .name              = "mdio-gpio",
    .dev.platform_data = &mdio_gpio_data,
};
#endif /* CONFIG_MDIO_GPIO */
struct platform_device jz_mac_device = {
    .name              = "jz_mac",
    .dev.platform_data = &jz_mii_bus,
};
#endif /* CONFIG_JZ_MAC */


#ifdef CONFIG_JZ_EFUSE_V13
struct jz_efuse_platform_data jz_efuse_pdata = {
        /* supply 2.5V to VDDQ */
        .gpio_vddq_en_n = GPIO_EFUSE_VDDQ,
};
#endif


#ifdef CONFIG_LEDS_GPIO
struct gpio_led jz_leds[] = {
#ifdef CONFIG_LEDS_GPIO_KEYBOARD
#ifdef KEYBOARD_LEDS_D1
    {
        .name          = "keyboard-D1",
        .gpio          = KEYBOARD_LEDS_D1,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D2
    {
        .name          = "keyboard-D2",
        .gpio          = KEYBOARD_LEDS_D2,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D3
    {
        .name          = "keyboard-D3",
        .gpio          = KEYBOARD_LEDS_D3,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D4
    {
        .name          = "keyboard-D4",
        .gpio          = KEYBOARD_LEDS_D4,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D5
    {
        .name          = "keyboard-D5",
        .gpio          = KEYBOARD_LEDS_D5,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D6
    {
        .name          = "keyboard-D6",
        .gpio          = KEYBOARD_LEDS_D6,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D7
    {
        .name          = "keyboard-D7",
        .gpio          = KEYBOARD_LEDS_D7,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D8
    {
        .name          = "keyboard-D8",
        .gpio          = KEYBOARD_LEDS_D8,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D9
    {
        .name          = "keyboard-D9",
        .gpio          = KEYBOARD_LEDS_D9,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D10
    {
        .name          = "keyboard-D10",
        .gpio          = KEYBOARD_LEDS_D10,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D11
    {
        .name          = "keyboard-D11",
        .gpio          = KEYBOARD_LEDS_D11,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#ifdef KEYBOARD_LEDS_D12
    {
        .name          = "keyboard-D12",
        .gpio          = KEYBOARD_LEDS_D12,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = KEYBOARD_LED_ACTIVE_LOW,
    },
#endif
#endif/* CONFIG_LEDS_GPIO_KEYBOARD */

#ifdef CONFIG_LEDS_GPIO_BACKLIGHT
#ifdef BACKLIGHT_LEDS_D1
    {
        .name          = "backlight-D1",
        .gpio          = BACKLIGHT_LEDS_D1,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = BACKLIGHT_LED_ACTIVE_LOW,
    },
#endif
#endif /* CONFIG_LEDS_GPIO_BACKLIGHT */

#ifdef CONFIG_LEDS_GPIO_INDICATION
#ifdef INDICATION_LEDS_DW
    {
        .name          = "indication-DW",
        .gpio          = INDICATION_LEDS_DW,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = INDICATION_LED_ACTIVE_LOW,
    },
#endif
#ifdef INDICATION_LEDS_DG
    {
        .name          = "indication-DG",
        .gpio          = INDICATION_LEDS_DG,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = INDICATION_LED_ACTIVE_LOW,
    },
#endif
#ifdef INDICATION_LEDS_DR
    {
        .name          = "indication-DR",
        .gpio          = INDICATION_LEDS_DR,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = INDICATION_LED_ACTIVE_LOW,
    },
#endif
#ifdef INDICATION_LEDS_DB
    {
        .name          = "indication-DB",
        .gpio          = INDICATION_LEDS_DB,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = INDICATION_LED_ACTIVE_LOW,
    },
#endif
#endif /* CONFIG_LEDS_GPIO_INDICATION */

#ifdef CONFIG_LEDS_GPIO_MATRIX
#ifdef MATRIX_LEDS_COL_D1
    {
        .name          = "matrix-col-D1",
        .gpio          = MATRIX_LEDS_COL_D1,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = MATRIX_LED_COL_ACTIVE_LOW,
    },
#endif
#ifdef MATRIX_LEDS_COL_D2
    {
        .name          = "matrix-col-D2",
        .gpio          = MATRIX_LEDS_COL_D2,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = MATRIX_LED_COL_ACTIVE_LOW,
    },
#endif
#ifdef MATRIX_LEDS_COL_D3
    {
        .name          = "matrix-col-D3",
        .gpio          = MATRIX_LEDS_COL_D3,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = MATRIX_LED_COL_ACTIVE_LOW,
    },
#endif
#ifdef MATRIX_LEDS_ROW_D1
    {
        .name          = "matrix-row-D1",
        .gpio          = MATRIX_LEDS_ROW_D1,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = MATRIX_LED_ROW_ACTIVE_LOW,
    },
#endif
#ifdef MATRIX_LEDS_ROW_D2
    {
        .name          = "matrix-row-D2",
        .gpio          = MATRIX_LEDS_ROW_D2,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = MATRIX_LED_ROW_ACTIVE_LOW,
    },
#endif
#ifdef MATRIX_LEDS_ROW_D3
    {
        .name          = "matrix-row-D3",
        .gpio          = MATRIX_LEDS_ROW_D3,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = MATRIX_LED_ROW_ACTIVE_LOW,
    },
#endif
#ifdef MATRIX_LEDS_ROW_D4
    {
        .name          = "matrix-row-D4",
        .gpio          = MATRIX_LEDS_ROW_D4,
        .default_state = LEDS_GPIO_DEFSTATE_OFF,
        .active_low    = MATRIX_LED_ROW_ACTIVE_LOW,
    },
#endif
#endif /* CONFIG_LEDS_GPIO_INDICATION */
};

struct gpio_led_platform_data  jz_led_pdata = {
    .num_leds = ARRAY_SIZE(jz_leds),
    .leds     = jz_leds,
};

struct platform_device jz_leds_gpio = {
    .name       = "leds-gpio",
    .id         = -1,
    .dev        = {
        .platform_data  = &jz_led_pdata,
    }
};
#endif

#ifdef CONFIG_LEDS_PWM
static struct led_pwm leds_pwm[] = {
    {
        .name            = "led_rgb0",
        .default_trigger = NULL,
        .pwm_id          = 0,
        .active_low      = false,
        .brightness      = 0,
        .max_brightness  = 4095,
        .pwm_period_ns   = 30000,
    },
    {
        .name            = "led_rgb1",
        .default_trigger = NULL,
        .pwm_id          = 2,
        .active_low      = false,
        .brightness      = 0,
        .max_brightness  = 4095,
        .pwm_period_ns   = 30000,
    },
    {
        .name            = "led_rgb2",
        .default_trigger = NULL,
        .pwm_id          = 1,
        .active_low      = false,
        .brightness      = 4095,
        .max_brightness  = 4095,
        .pwm_period_ns   = 30000,
    },
    {
        .name            = "led_mire",
        .default_trigger = NULL,
        .pwm_id          = 4,
        .active_low      = false,
        .brightness      = 0,
        .max_brightness  = 4095,
        .pwm_period_ns   = 30000,
    },
};

static struct led_pwm_platform_data led_pwm_info = {
    .num_leds = ARRAY_SIZE(leds_pwm),
    .leds     = leds_pwm,
};

struct platform_device jz_leds_pwm = {
    .name = "leds_pwm",
    .id   = -1,
    .dev  = {
        .platform_data = &led_pwm_info,
    },
};
#endif

#ifdef CONFIG_SGM42609_MOTOR
struct sgm42609_motor_platform_data sgm42609_motor_platform_data0 = {
    .in1_gpio           = SGM42609_MOTOR_IN1,
    .in2_gpio           = SGM42609_MOTOR_IN2,
    .fault_gpio         = SGM42609_MOTOR_FAULT,
    .power_gpio         = SGM42609_MOTOR_POWER,
    .power_active_level = SGM42609_MOTOR_POWER_LEVEL,
};

struct platform_device sgm42609_motor_device = {
    .name = "sgm42609_motor",
    .id   = 0,
    .dev  = {
        .platform_data = &sgm42609_motor_platform_data0,
    },
};
#endif

#ifdef CONFIG_SGM42507
struct sgm42507_platform_data sgm42507_platform_data0 = {
    .id            = 0,
    .pwr_en_pin    = SGM42507_POWER_EN,
    .dri_en_pin    = SGM42507_DRIVER_EN,
    .direction_pin = SGM42507_DIRECTION,
};

struct platform_device sgm42507_device = {
    .name = "sgm42507",
    .id   = 0,
    .dev  = {
        .platform_data = &sgm42507_platform_data0,
    },
};
#endif

#ifdef CONFIG_FXL_LOCK_BODY
struct fxl_lock_body_platform_data fxl_lock_body_platform_data = {
    .id                        = 0,
    .bolique_tongue_shrink_pin = FXL_LOCK_BOLIQUE_TONGUE_SHRINK,
    .square_tongue_stretch_pin = FXL_LOCK_SQUARE_TONGUE_STRETCH,
    .square_tongue_shrink_pin  = FXL_LOCK_SQUARE_TONGUE_SHRINK,
};

struct platform_device fxl_lock_body_device = {
    .name = "fxl_lock_body",
    .id   = 0,
    .dev  = {
        .platform_data = &fxl_lock_body_platform_data,
    },
};
#endif

#ifdef CONFIG_MILI_LOCK_BODY
struct mili_lock_body_platform_data mili_lock_body_platform_data = {
    .id                            = 0,
    .square_tongue_master_lock_pin = MILI_LOCK_SQUARE_MASTER_PIN,
    .square_tongue_anti_lock_pin   = MILI_LOCK_SQUARE_ANTI_PIN,
    .lock_cylinder_rotation_pin    = MILI_LOCK_CYLINDER_ROTATION_PIN,
};

struct platform_device mili_lock_body_device = {
    .name = "mili_lock_body",
    .id   = 0,
    .dev  = {
        .platform_data = &mili_lock_body_platform_data,
    },
};
#endif


#ifdef CONFIG_74HC595
static struct sn74hc595_platform_data jz_74hc595_pdata = {
    .en_level = 0,
    .out_bits = 16,
    .init_val = 0xffff,
    .data_pin = GPIO_74HC595_DATA,
    .rclk_pin = GPIO_74HC595_RCLK,
    .sclk_pin = GPIO_74HC595_SCLK,
    .sclr_pin = GPIO_74HC595_SCLR,
    .oe_pin   = GPIO_74HC595_OE,
};

struct platform_device jz_74hc595_dev = {
    .name = "sn74hc595",
    .id   = 0,
    .dev  = {
        .platform_data = &jz_74hc595_pdata,
    },
};
#endif

#ifdef CONFIG_TM1620
static struct tm1620_platform_data tm1620_pdata = {
    .stb_pin = TM1620_STB_PIN,
    .clk_pin = TM1620_CLK_PIN,
    .dio_pin = TM1620_DIO_PIN,
};

struct platform_device tm1620_dev = {
    .name = "tm1620",
    .id = 0,
    .dev = {
        .platform_data = &tm1620_pdata,
    },
};
#endif

#ifdef CONFIG_ZIGBEE_CC2530

static void zb_restore_pin_status(void)
{
    jz_gpio_set_func(GPIO_CC2530_UART_RXD, GPIO_FUNC_1);
    jz_gpio_set_func(GPIO_CC2530_UART_TXD, GPIO_FUNC_1);
}

static void zb_set_pin_status(void)
{
    jz_gpio_set_func(GPIO_CC2530_UART_RXD, GPIO_OUTPUT0);
    jz_gpio_set_func(GPIO_CC2530_UART_TXD, GPIO_OUTPUT0);
}
static struct zb_cc2530_platform_data jz_cc2530_pdata = {
    .en_level   = 1,
    .rst_level  = 0,
    .wake_level = 1,
    .en_pin     = GPIO_CC2530_EN,
    .int_pin    = GPIO_CC2530_INT,
    .wake_pin   = GPIO_CC2530_WAKE,
    .rst_pin    = GPIO_CC2530_RST,
    .restore_pin_status = zb_restore_pin_status,
    .set_pin_status     = zb_set_pin_status,
};

struct platform_device jz_cc2530_dev = {
    .name = "zb_cc2530",
    .dev  = {
        .platform_data = &jz_cc2530_pdata,
    },
};
#endif

#ifdef CONFIG_LOCK_CYLINDER
static struct lock_cylinder_platform_data lock_cylinder_pdata = {
    .reverse_endian = true,
    .inserted_level = 0,
    .pwr_en_level   = 0,
    .id_pin         = LOCK_CYLINDER_LOCK_ID_PIN,
    .pwr_pin        = LOCK_CYLINDER_PWR_PIN,
    .detect_pin     = LOCK_CYLINDER_DETECT_PIN,
};

struct platform_device lock_cylinder_dev = {
    .name = "lock_cylinder",
    .id   = 0,
    .dev  = {
        .platform_data = &lock_cylinder_pdata,
    },
};
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART0
struct jz_uart_platform_data jz_uart0_platform_data = {
        .wakeup_pin = NULL,
};
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART1
struct uart_wakeup_pin uart1_wakeup_pin = {
        .num          = -1,
        .trigger_edge = IRQF_TRIGGER_FALLING,
        .def_func     = GPIO_FUNC_1,
};

struct jz_uart_platform_data jz_uart1_platform_data = {
        .wakeup_pin = NULL,
};
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART2
struct jz_uart_platform_data jz_uart2_platform_data = {
        .wakeup_pin = NULL,
};
#endif

#ifdef CONFIG_BATTERY_PWM
struct pwm_battery_platform_data pwm_battery_platform_data = {
    .gpio_power                 = GPIO_PWM_BATTERY_POWER,
    .gpio_power_active_low      = GPIO_ACTIVE_LOW,

    .gpio_usb                   = GPIO_USB_DETE,
    .gpio_usb_active_low        = GPIO_ACTIVE_HIGH,

    .gpio_charger               = GPIO_LI_ION_CHARGE,
    .gpio_charger_active_low    = GPIO_ACTIVE_HIGH,
    .charger_debounce           = 0,

    .pwm_id                     = CONFIG_BATTERY_PWM_INDEX,
    .pwm_active_low             = GPIO_ACTIVE_HIGH,

    .gpio_op                    = GPIO_BATTERY_STATUS,
    .gpio_op_active_low         = GPIO_ACTIVE_HIGH,

    .pwm_ref_voltage            = 2897,  /* unit mV: VDDIO voltage on Board */

    .battery_ref_resistor1      = 1000,  /* unit KΩ: Resistance value on Board */
    .battery_ref_resistor2      = 470,   /* unit KΩ: Resistance value on Board */

    .battery_info = {
            .battery_max_cpt    = 4000,
            .sleep_current      = 20,
    },
};

struct platform_device pwm_battery_device = {
    .name = "pwm-battery",
    .dev  = {
        .platform_data = &pwm_battery_platform_data,
    },
};
#endif

#if defined(GPIO_USB_ID) && defined(GPIO_USB_ID_LEVEL)
struct jzdwc_pin dwc2_id_pin = {
    .num          = GPIO_USB_ID,
    .enable_level = GPIO_USB_ID_LEVEL,
};
#endif


#if defined(GPIO_USB_DETE) && defined(GPIO_USB_DETE_LEVEL) && (!defined(CONFIG_BATTERY_PWM))
struct jzdwc_pin dwc2_dete_pin = {
    .num          = GPIO_USB_DETE,
    .enable_level = GPIO_USB_DETE_LEVEL,
};
#endif


#if defined(GPIO_USB_DRVVBUS) && defined(GPIO_USB_DRVVBUS_LEVEL) && !defined(USB_DWC2_DRVVBUS_FUNCTION_PIN)
struct jzdwc_pin dwc2_drvvbus_pin = {
    .num          = GPIO_USB_DRVVBUS,
    .enable_level = GPIO_USB_DRVVBUS_LEVEL,
};
#endif

#if defined(CONFIG_SND_ASOC_INGENIC)

#if defined(CONFIG_SND_ASOC_JZ_EXTCODEC_AKM4951)
struct snd_codec_data snd_alsa_platform_data = {
    .gpio_spk_en        = {.gpio = GPIO_AKM4951_SPEAKER_EN, .active_level = GPIO_AKM4951_SPEAKER_EN_LEVEL},
    .gpio_amp_pwr       = {.gpio = GPIO_AKM4951_AMP_POWER_EN, .active_level = GPIO_AKM4951_AMP_POWER_EN_LEVEL},
    .gpio_linein_detect = {.gpio = GPIO_AKM4951_LINEIN_DETECT, .active_level = GPIO_AKM4951_LINEIN_INSERT_LEVEL},
    .gpio_spk_mute      = {.gpio = GPIO_SPEAKER_MUTE, .active_level = GPIO_SPEAKER_MUTE_EN_LEVEL},
    .gpio_hp_detect     = {.gpio = GPIO_AKM4951_HP_DETECT, .active_level = GPIO_AKM4951_HP_INSERT_LEVEL},
    .gpio_hp_mute       = {.gpio = GPIO_AKM4951_HP_MUTE, .active_level = GPIO_AKM4951_HP_MUTE_EN_LEVEL},
};

struct platform_device snd_alsa_device = {
    .name = "ingenic-ilock",
    .dev  = {
        .platform_data = &snd_alsa_platform_data,
    },
};
#else
static struct snd_codec_data snd_alsa_platform_data = {
    .gpio_spk_en        = {.gpio = GPIO_SPEAKER_EN, .active_level = GPIO_SPEAKER_EN_LEVEL},
    .gpio_amp_pwr       = {.gpio = GPIO_AMP_POWER,  .active_level = GPIO_AMP_POWER_LEVEL},
    .gpio_amp_mute      = {.gpio = GPIO_AMP_MUTE,   .active_level = GPIO_AMP_MUTE_LEVEL},
};

struct platform_device snd_alsa_device = {
    .name = "ingenic-alsa",
    .dev  = {
        .platform_data = &snd_alsa_platform_data,
    },
};
#endif

#endif
