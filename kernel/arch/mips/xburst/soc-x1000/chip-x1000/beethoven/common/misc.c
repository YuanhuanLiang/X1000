#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/leds.h>
#include <linux/leds_pwm.h>
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
#include <linux/interrupt.h>
//#include <sound/jz-aic.h>
#include "board_base.h"

#ifdef CONFIG_JZ_MAC
#ifndef CONFIG_MDIO_GPIO
#ifdef CONFIG_JZGPIO_PHY_RESET
static struct jz_gpio_phy_reset gpio_phy_reset = {
	.gpio = GMAC_PHY_PORT_GPIO,
	.active_level = GMAC_PHY_ACTIVE_HIGH,
	.crtl_port = GMAC_CRLT_PORT,
	.crtl_pins = GMAC_CRLT_PORT_PINS,
	.set_func = GMAC_CRTL_PORT_SET_FUNC,
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
	.mdc = MDIO_MDIO_MDC_GPIO,
	.mdio = MDIO_MDIO_GPIO,
	.phy_mask = 0,
	.irqs = { 0 },
};

struct platform_device jz_mii_bus = {
	.name = "mdio-gpio",
	.dev.platform_data = &mdio_gpio_data,
};
#endif /* CONFIG_MDIO_GPIO */
struct platform_device jz_mac_device = {
	.name = "jz_mac",
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
#if 0
	[0]={
		.name = "wl_led_r",
		.gpio = WL_LED_R,
		.active_low = 0,
	},
	[1]={
		.name = "wl_led_g",
		.gpio = WL_LED_G,
		.active_low = 0,
	},

	[2]={
		.name = "wl_led_b",
		.gpio = WL_LED_B,
		.active_low = 0,
	},
#endif
};

struct gpio_led_platform_data  jz_led_pdata = {
	.num_leds = ARRAY_SIZE(jz_leds),
	.leds = jz_leds,
};

struct platform_device jz_led_rgb = {
	.name       = "leds-gpio",
	.id     = -1,
	.dev        = {
		.platform_data  = &jz_led_pdata,
	}
};
#endif

#ifdef CONFIG_LEDS_PWM
static struct led_pwm leds_pwm[] = {
    {
        .name = "led_rgb0",
        .default_trigger = NULL,
        .pwm_id     = 0,
        .active_low = LED_ACTIVE_LOW,
        .brightness = 0,
        .max_brightness = 4095,
        .pwm_period_ns  = 30000,
    },
    {
        .name = "led_rgb1",
        .default_trigger = NULL,
        .pwm_id     = 2,
        .active_low = LED_ACTIVE_LOW,
        .brightness = 0,
        .max_brightness = 4095,
        .pwm_period_ns  = 30000,
    },
    {
        .name = "led_rgb2",
        .default_trigger = NULL,
        .pwm_id     = 1,
        .active_low = LED_ACTIVE_LOW,
        .brightness = 4095,
        .max_brightness = 4095,
        .pwm_period_ns  = 30000,
    },
    {
        .name = "led_mire",
        .default_trigger = NULL,
        .pwm_id     = 4,
        .active_low = LED_ACTIVE_LOW,
        .brightness = 0,
        .max_brightness = 4095,
        .pwm_period_ns  = 30000,
    },
};

static struct led_pwm_platform_data led_pwm_info = {
    .num_leds = ARRAY_SIZE(leds_pwm),
    .leds = leds_pwm,
};

struct platform_device jz_leds_pwm = {
    .name = "leds_pwm",
    .id = -1,
    .dev = {
        .platform_data = &led_pwm_info,
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
        .num = -1,
        .trigger_edge = IRQF_TRIGGER_FALLING,
        .def_func = GPIO_FUNC_1,
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
    .gpio_power                 = -1,
    .gpio_power_active_low      = GPIO_ACTIVE_LOW,

    .gpio_usb                   = GPIO_USB_DETE,
    .gpio_usb_active_low        = GPIO_ACTIVE_HIGH,

    .gpio_charger               = GPIO_LI_ION_CHARGE,
    .gpio_charger_active_low    = GPIO_ACTIVE_LOW,
    .charger_debounce           = 20,

    .pwm_id                     = CONFIG_BATTERY_PWM_INDEX,
    .pwm_active_low             = GPIO_ACTIVE_HIGH,

    .gpio_op                    = GPIO_BATTERY_STATUS,
    .gpio_op_active_low         = GPIO_ACTIVE_HIGH,

    .pwm_ref_voltage            = 3300,  /* unit mV: VDDIO voltage on Board */

    .battery_ref_resistor1      = 2000,  /* unit KΩ: Resistance value on Board */
    .battery_ref_resistor2      = 1000,  /* unit KΩ: Resistance value on Board */

    .battery_info = {
            .battery_max_cpt    = 4000,
            .sleep_current      = 20,
    },
};

struct platform_device pwm_battery_device = {
    .name = "pwm-battery",
    .dev = {
        .platform_data = &pwm_battery_platform_data,
    },
};
#endif

#if defined(GPIO_USB_ID) && defined(GPIO_USB_ID_LEVEL)
struct jzdwc_pin dwc2_id_pin = {
	.num = GPIO_USB_ID,
	.enable_level = GPIO_USB_ID_LEVEL,
};
#endif


#if defined(GPIO_USB_DETE) && defined(GPIO_USB_DETE_LEVEL) && (!defined(CONFIG_BATTERY_PWM))
struct jzdwc_pin dwc2_dete_pin = {
	.num = GPIO_USB_DETE,
	.enable_level = GPIO_USB_DETE_LEVEL,
};
#endif


#if defined(GPIO_USB_DRVVBUS) && defined(GPIO_USB_DRVVBUS_LEVEL) && !defined(USB_DWC2_DRVVBUS_FUNCTION_PIN)
struct jzdwc_pin dwc2_drvvbus_pin = {
	.num = GPIO_USB_DRVVBUS,
	.enable_level = GPIO_USB_DRVVBUS_LEVEL,
};
#endif

#if defined(CONFIG_SND_ASOC_INGENIC)

#if defined(CONFIG_SND_ASOC_JZ_EXTCODEC_AKM4951)
struct snd_codec_data snd_alsa_platform_data = {
    .gpio_spk_en = {.gpio = GPIO_AKM4951_SPEAKER_EN, .active_level = GPIO_AKM4951_SPEAKER_EN_LEVEL},
    .gpio_amp_pwr = {.gpio = GPIO_AKM4951_AMP_POWER_EN, .active_level = GPIO_AKM4951_AMP_POWER_EN_LEVEL},
    .gpio_linein_detect = {.gpio = GPIO_AKM4951_LINEIN_DETECT, .active_level = GPIO_AKM4951_LINEIN_INSERT_LEVEL},
    .gpio_spk_mute = {.gpio = GPIO_SPEAKER_MUTE, .active_level = GPIO_SPEAKER_MUTE_EN_LEVEL},
};

struct platform_device snd_alsa_device = {
    .name = "ingenic-beethoven",
    .dev = {
        .platform_data = &snd_alsa_platform_data,
    },
};
#else
static struct snd_codec_data snd_alsa_platform_data = {
    .gpio_spk_en        = {.gpio = GPIO_SPEAKER_EN,     .active_level = GPIO_SPEAKER_EN_LEVEL},
    .gpio_linein_detect = {.gpio = GPIO_LINEIN_DETECT,  .active_level = GPIO_LINEIN_INSERT_LEVEL},
    .gpio_spk_mute      = {.gpio = GPIO_SPEAKER_MUTE, .active_level = GPIO_SPEAKER_MUTE_EN_LEVEL},
    .gpio_amp_pwr       = {.gpio = GPIO_AMP_POWER,  .active_level = GPIO_AMP_POWER_LEVEL},
    .gpio_amp_mute      = {.gpio = GPIO_AMP_MUTE,   .active_level = GPIO_AMP_MUTE_LEVEL},
};

struct platform_device snd_alsa_device = {
    .name = "ingenic-alsa",
    .dev = {
        .platform_data = &snd_alsa_platform_data,
    },
};
#endif

#endif

#ifdef CONFIG_BK9522_KEY
#include <linux/input/bk9522_key.h>

static struct bk9522_keys bk9522_board_keys[] = {
    {
        .data = 1,
        .code = KEY_VIDEO_PREV,
    },
    {
        .data = 2,
        .code = KEY_VIDEO_NEXT,
    },
    {
        .data = 3,
        .code = KEY_PLAY,
    },
    {
        .data = 4,
        .code = KEY_MENU,
    },
    {
        .data = 5,
        .code = KEY_SOUND,
    },
};

struct bk9522_platform_data bk9522_key_pdata = {

    .sda_gpio = BK9522_SDA_GPIO,
    .sck_gpio = BK9522_SCK_GPIO,

    .irq = -1,
    .irqflags = 0,

    .power = -1,
    .power_level_en = -1,

    .keys = bk9522_board_keys,
    .key_num = ARRAY_SIZE(bk9522_board_keys),

};

struct platform_device bk9522_key_device = {
    .name = BK9522_NAME,
    .dev = {
        .platform_data = &bk9522_key_pdata,
    },
};


#endif
