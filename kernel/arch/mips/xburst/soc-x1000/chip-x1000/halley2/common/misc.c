#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/leds.h>
//#include <linux/tsc.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
//#include <linux/android_pmem.h>
#include <mach/platform.h>
#include <mach/jzsnd.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <mach/jzscc.h>
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

#ifdef CONFIG_JZ_SCC
struct jz_scc_platform_data jz_scc_pdata = {
    .reset_pin = SC_RESET_PIN,
    .power_pin = SC_POWER_PIN,
    .pwr_en_level = 1, /* if power_pin is vaild, must set pwr_en_level */
};
#endif /* CONFIG_JZ_SCC */

#ifdef CONFIG_LEDS_GPIO
struct gpio_led jz_leds[] = {
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

};

struct gpio_led_platform_data  jz_led_pdata = {
	.num_leds = 3,
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

    .gpio_usb                   = -1,
    .gpio_usb_active_low        = GPIO_ACTIVE_LOW,

    .gpio_charger               = -1,
    .gpio_charger_active_low    = GPIO_ACTIVE_LOW,

    .pwm_id                     = CONFIG_BATTERY_PWM_INDEX,
    .pwm_active_low             = GPIO_ACTIVE_HIGH,

    .gpio_op                    = -1,
    .gpio_op_active_low         = GPIO_ACTIVE_LOW,

    .pwm_ref_voltage            = 3268,  /* unit mV: VDDIO voltage on Board */

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
static struct snd_codec_data snd_alsa_platform_data = {
        .gpio_spk_en        = {.gpio = GPIO_SPEAKER_EN, .active_level = GPIO_SPEAKER_EN_LEVEL},
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
