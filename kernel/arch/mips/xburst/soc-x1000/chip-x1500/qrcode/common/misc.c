#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/leds.h>
#include <linux/leds_pwm.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
//#include <linux/android_pmem.h>
#include <mach/platform.h>
#include <mach/jzsnd.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <mach/jz_efuse.h>
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

#ifdef  CONFIG_LEDS_PWM
static struct led_pwm leds_pwm[] = {
	{
		.name = "led_state",
		.default_trigger = NULL,
		.pwm_id     = 4,
		.active_low = true,
		.max_brightness = 4095,
		.pwm_period_ns  = 30000,
	},
};

static struct led_pwm_platform_data led_pwm_info = {
    .leds = leds_pwm,
    .num_leds = ARRAY_SIZE(leds_pwm),
};

struct platform_device jz_leds_pwm = {
    .name = "leds_pwm",
    .id = -1,
    .dev = {
        .platform_data = &led_pwm_info,
    },
};
#endif

#ifdef CONFIG_INPUT_PWM_BEEPER
struct platform_device pwm_beeper_device = {
    .name = "pwm-beeper",
    .dev  = {
        .platform_data = (unsigned long *)BEEPER_PORT_PWM_ID,
    },
};
#endif

/*
 *  USB
 */
#if defined(GPIO_USB_ID) && defined(GPIO_USB_ID_LEVEL)
struct jzdwc_pin dwc2_id_pin = {
    .num                    = GPIO_USB_ID,
    .enable_level           = GPIO_USB_ID_LEVEL,
};
#endif


#if defined(GPIO_USB_DETE) && defined(GPIO_USB_DETE_LEVEL)
struct jzdwc_pin dwc2_dete_pin = {
    .num                    = GPIO_USB_DETE,
    .enable_level           = GPIO_USB_DETE_LEVEL,
};
#endif


#if defined(GPIO_USB_DRVVBUS) && defined(GPIO_USB_DRVVBUS_LEVEL) && !defined(USB_DWC2_DRVVBUS_FUNCTION_PIN)
struct jzdwc_pin dwc2_drvvbus_pin = {
    .num                    = GPIO_USB_DRVVBUS,
    .enable_level           = GPIO_USB_DRVVBUS_LEVEL,
};
#endif

#ifdef CONFIG_JZ_USB_REMOTE_WKUP_PIN
/* usb switch pin */
struct jzdwc_pin dwc2_switch_pin = {
    .num                    = GPIO_USB_SWITCH,
    .enable_level           = GPIO_USB_SWITCH_LEVEL, /* suspend status level */
};

/* usb remote wake up pin */
struct jzdwc_pin dwc2_remote_wkup_pin = {
    .num                    = GPIO_USB_REMOTE_WKUP_DETE,
    .enable_level           = GPIO_USB_REMOTE_WKUP_DETE_LEVEL,
};

#endif
/*
 * Audio
 */
#if defined(CONFIG_SND_ASOC_INGENIC)
static struct snd_codec_data snd_alsa_platform_data = {
    .gpio_spk_en        = {.gpio = GPIO_SPEAKER_EN,     .active_level = GPIO_SPEAKER_EN_LEVEL},
    .gpio_spk_mute      = {.gpio = GPIO_SPEAKER_MUTE,   .active_level = GPIO_SPEAKER_MUTE_EN_LEVEL},
    .gpio_amp_pwr       = {.gpio = GPIO_AMP_POWER,      .active_level = GPIO_AMP_POWER_LEVEL},
    .gpio_amp_mute      = {.gpio = GPIO_AMP_MUTE,       .active_level = GPIO_AMP_MUTE_LEVEL},
};

struct platform_device snd_alsa_device = {
    .name = "ingenic-alsa",
    .dev = {
        .platform_data  = &snd_alsa_platform_data,
    },
};
#endif
