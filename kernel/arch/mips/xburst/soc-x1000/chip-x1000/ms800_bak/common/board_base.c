#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/input.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/interrupt.h>
#include <linux/jz_dwc.h>
#include <linux/delay.h>
#include <mach/jzsnd.h>
#include <mach/platform.h>
#include <mach/jzfb.h>
#include <mach/jzmmc.h>
#include <mach/jzssi.h>
#include <mach/jz_uart.h>
#include <gpio.h>
#include <mach/jz_efuse.h>
#include "board_base.h"


struct jz_platform_device
{
	struct platform_device *pdevices;
	void *pdata;
	int size;
};

static struct jz_platform_device platform_devices_array[] __initdata = {
#define DEF_DEVICE(DEVICE, DATA, SIZE)  \
	{ .pdevices = DEVICE,   \
		.pdata = DATA, .size = SIZE,}

#ifdef CONFIG_KEYBOARD_GPIO
	DEF_DEVICE(&jz_button_device, 0, 0),
#endif
#ifdef CONFIG_LOGO
	DEF_DEVICE(&jz_logo_device, 0, 0),
#endif

#ifdef CONFIG_KEYBOARD_MATRIX
	DEF_DEVICE(&matrix_keypad_device, 0, 0),
#endif
#ifdef CONFIG_LEDS_GPIO
	DEF_DEVICE(&jz_leds_gpio, 0, 0),
#endif
#ifdef CONFIG_LEDS_PWM
    DEF_DEVICE(&jz_leds_pwm, 0, 0),
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART0
	DEF_DEVICE(&jz_uart0_device, &jz_uart0_platform_data, sizeof(struct jz_uart_platform_data)),
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART1
	DEF_DEVICE(&jz_uart1_device, &jz_uart1_platform_data, sizeof(struct jz_uart_platform_data)),
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART2
	DEF_DEVICE(&jz_uart2_device, &jz_uart2_platform_data, sizeof(struct jz_uart_platform_data)),
#endif
#ifdef CONFIG_JZMMC_V11_MMC1
	DEF_DEVICE(&jz_msc1_device,&tf_pdata,sizeof(struct jzmmc_platform_data)),
#endif
#ifdef CONFIG_JZMMC_V12_MMC0
	DEF_DEVICE(&jz_msc0_device,&tf_pdata,sizeof(struct jzmmc_platform_data)),
#endif
#ifdef CONFIG_JZMMC_V12_MMC1
	DEF_DEVICE(&jz_msc1_device,&sdio_pdata,sizeof(struct jzmmc_platform_data)),
#endif
#ifdef CONFIG_JZMMC_V11_MMC2
	DEF_DEVICE(&jz_msc2_device,&sdio_pdata,sizeof(struct jzmmc_platform_data)),
#endif

#ifdef CONFIG_BROADCOM_RFKILL
	DEF_DEVICE(&bt_power_device,0,0),
	DEF_DEVICE(&bluesleep_device,0,0),
#endif

#ifdef CONFIG_JZ_INTERNAL_CODEC_V12
	DEF_DEVICE(&jz_codec_device, &codec_data, sizeof(struct snd_codec_data)),
#endif

#ifdef CONFIG_BCM_AP6212_RFKILL
	DEF_DEVICE(&bt_power_device,0,0),
#endif

#ifdef CONFIG_BCM_43438_RFKILL
	DEF_DEVICE(&bt_power_device,0,0),
#endif
/* JZ ALSA audio driver */

#if defined(CONFIG_SND_ASOC_JZ_AIC_V12)
    DEF_DEVICE(&jz_aic_device,0,0),
    DEF_DEVICE(&jz_aic_dma_device,0,0),
#endif

#if defined(CONFIG_SND_ASOC_JZ_ICDC_D3) && defined(CONFIG_SND_ASOC_JZ_AIC_I2S_V13)
    DEF_DEVICE(&jz_icdc_device,0,0),
#endif

#if defined(CONFIG_SND_ASOC_JZ_PCM_V13)
    DEF_DEVICE(&jz_pcm_device,0,0),
	DEF_DEVICE(&jz_pcm_dma_device,0,0),
#endif

#if defined(CONFIG_SND_ASOC_JZ_DMIC_V13)
    DEF_DEVICE(&jz_dmic_device,0,0),
	DEF_DEVICE(&jz_dmic_dma_device,0,0),
#endif

#if defined(CONFIG_SND_ASOC_JZ_PCM_DUMP_CDC)
	DEF_DEVICE(&jz_pcm_dump_cdc_device,0,0),
#endif

#if defined(CONFIG_SND_ASOC_JZ_SPDIF_DUMP_CDC)
    DEF_DEVICE(&jz_spdif_dump_cdc_device,0,0),
#endif

#if defined(CONFIG_SND_ASOC_JZ_DMIC_DUMP_CDC)
    DEF_DEVICE(&jz_dmic_dump_cdc_device,0,0),
#endif

#if defined(CONFIG_SND_ASOC_JZ_AIC_V12)
       DEF_DEVICE(&snd_alsa_device, NULL, 0),
#endif

/* end of ALSA audio driver */
#ifdef CONFIG_USB_JZ_DWC2
	DEF_DEVICE(&jz_dwc_otg_device,0,0),
#endif

#ifdef CONFIG_SOUND_JZ_I2S_V12
	DEF_DEVICE(&jz_i2s_device, &i2s_data, sizeof(struct snd_dev_data)),
	DEF_DEVICE(&jz_mixer0_device, &snd_mixer0_data, sizeof(struct snd_dev_data)),
#endif

#ifdef CONFIG_SOUND_JZ_SPDIF_V12
	DEF_DEVICE(&jz_spdif_device, &spdif_data, sizeof(struct snd_dev_data)),
	DEF_DEVICE(&jz_mixer2_device, &snd_mixer2_data, sizeof(struct snd_dev_data)),
#endif

#ifdef CONFIG_SOUND_JZ_PCM_V12
	DEF_DEVICE(&jz_pcm_device, &pcm_data, sizeof(struct snd_dev_data)),
	DEF_DEVICE(&jz_mixer1_device, &snd_mixer1_data, sizeof(struct snd_dev_data)),
#endif

#ifdef CONFIG_SOUND_JZ_DMIC_V12
     DEF_DEVICE(&jz_dmic_device, &dmic_data, sizeof(struct snd_dev_data)),
     DEF_DEVICE(&jz_mixer3_device, &snd_mixer3_data, sizeof(struct snd_dev_data)),
#endif

#ifdef CONFIG_JZ_PWM
       DEF_DEVICE(&jz_pwm_device, 0, 0),
#endif

#ifdef CONFIG_JZ_WDT
    DEF_DEVICE(&jz_wdt_device, 0, 0),
#endif

#ifdef CONFIG_FB_JZ_V13
	DEF_DEVICE(&jz_fb_device, &jzfb_pdata, sizeof(struct jzfb_platform_data)),
#endif
#ifdef CONFIG_BACKLIGHT_PWM
	DEF_DEVICE(&backlight_device, 0, 0),
#endif
#ifdef CONFIG_LCD_TRULY_TFT240240_2_E
	DEF_DEVICE(&truly_tft240240_device, 0, 0),
#endif

#ifdef CONFIG_LCD_TRULY_TFT240240_2_2E
	DEF_DEVICE(&truly_tft240240_device, 0, 0),
#endif

#ifdef CONFIG_LCD_DK_ST7789H2
    DEF_DEVICE(&dk_st7789h2_device, 0, 0),
#endif

#ifdef CONFIG_LCD_ZK_ST7789V
    DEF_DEVICE(&zk_st7789v_device, 0, 0),
#endif

#ifdef CONFIG_LCD_UG2864HLBEG01
    DEF_DEVICE(&ug2864hlbeg01_device, 0, 0),
#endif

#ifdef CONFIG_XBURST_DMAC_V13
	DEF_DEVICE(&jz_pdma_device, 0, 0),
#endif
#ifdef CONFIG_XBURST_DMAC
	DEF_DEVICE(&jz_pdma_device, 0, 0),
#endif

#ifdef CONFIG_USB_OHCI_HCD
	DEF_DEVICE(&jz_ohci_device,0,0),
#endif

#ifdef CONFIG_SPI_GPIO
	DEF_DEVICE(&jz_spi_gpio_device, 0,0),
#endif

#ifdef CONFIG_JZ_SPI0
	DEF_DEVICE(&jz_ssi0_device, &spi0_info_cfg, sizeof(struct jz_spi_info)),
#endif

#ifdef CONFIG_JZ_MAC
	DEF_DEVICE(&jz_mii_bus, 0, 0),
	DEF_DEVICE(&jz_mac_device, 0, 0),
#endif

#ifdef CONFIG_RTC_DRV_JZ
	DEF_DEVICE(&jz_rtc_device, 0, 0),
#endif
#ifdef CONFIG_JZ_EFUSE_V13
	DEF_DEVICE(&jz_efuse_device, &jz_efuse_pdata, sizeof(struct jz_efuse_platform_data)),
#endif
#ifdef CONFIG_JZ_SECURITY
	DEF_DEVICE(&jz_security_device, 0, 0),
#endif
#ifdef	CONFIG_JZ_SFC
	DEF_DEVICE(&jz_sfc_device,&sfc_info_cfg, sizeof(struct jz_sfc_info)),
#endif
#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
	DEF_DEVICE(&i2c0_gpio_device, 0, 0),
#endif
#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
	DEF_DEVICE(&i2c1_gpio_device, 0, 0),
#endif
#ifdef	CONFIG_I2C0_V12_JZ
	DEF_DEVICE(&jz_i2c0_device,0,0),
#endif
#ifdef	CONFIG_I2C1_V12_JZ
	DEF_DEVICE(&jz_i2c1_device,0,0),
#endif
#ifdef	CONFIG_I2C2_V12_JZ
	DEF_DEVICE(&jz_i2c2_device,0,0),
#endif
#ifdef	CONFIG_MFD_JZ_SADC_V12
	DEF_DEVICE(&jz_adc_device, 0, 0),
#endif
#ifdef	CONFIG_JZ_VPU_V13
	DEF_DEVICE(&jz_vpu_device, 0, 0),
#endif

#ifdef	CONFIG_I2C0_DMA_V12
	DEF_DEVICE(&jz_i2c0_dma_device, 0, 0),
#endif
#ifdef	CONFIG_I2C1_DMA_V12
	DEF_DEVICE(&jz_i2c1_dma_device, 0, 0),
#endif
#ifdef	CONFIG_I2C2_DMA_V12
	DEF_DEVICE(&jz_i2c2_dma_device, 0, 0),
#endif

#ifdef CONFIG_JZ_AES_V12
    DEF_DEVICE(&jz_aes_device, 0, 0),
#endif

#ifdef CONFIG_BATTERY_PWM
    DEF_DEVICE(&pwm_battery_device, 0, 0),
#endif

#ifdef CONFIG_SGM42609_MOTOR
    DEF_DEVICE(&sgm42609_motor_device, 0, 0),
#endif

#ifdef CONFIG_SGM42507
    DEF_DEVICE(&sgm42507_device, 0, 0),
#endif

#ifdef CONFIG_FXL_LOCK_BODY
    DEF_DEVICE(&fxl_lock_body_device, 0, 0),
#endif

#ifdef CONFIG_MILI_LOCK_BODY
    DEF_DEVICE(&mili_lock_body_device, 0, 0),
#endif
#ifdef CONFIG_LOCK_CYLINDER
    DEF_DEVICE(&lock_cylinder_dev, 0, 0),
#endif

#ifdef CONFIG_74HC595
    DEF_DEVICE(&jz_74hc595_dev, 0, 0),
#endif

#ifdef CONFIG_TM1620
    DEF_DEVICE(&tm1620_dev, 0, 0),
#endif

#ifdef CONFIG_ZIGBEE_CC2530
    DEF_DEVICE(&jz_cc2530_dev, 0, 0),
#endif

#ifdef CONFIG_JZ_PWM_GENERIC
    DEF_DEVICE(&jz_pwm_devs, 0, 0),
#endif
};

static int __init board_base_init(void)
{
	int pdevices_array_size, i;

	pdevices_array_size = ARRAY_SIZE(platform_devices_array);
	for(i = 0; i < pdevices_array_size; i++) {
		if(platform_devices_array[i].size)
			platform_device_add_data(platform_devices_array[i].pdevices,
					platform_devices_array[i].pdata, platform_devices_array[i].size);
		platform_device_register(platform_devices_array[i].pdevices);
	}

#if defined(CONFIG_JZ_SPI0) || defined(CONFIG_SPI_GPIO)
	spi_register_board_info(jz_spi0_board_info, jz_spi0_devs_size);
#endif

#if (defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ))
     i2c_register_board_info(2, jz_i2c2_devs, jz_i2c2_devs_size);
#endif

#if (defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ))
     i2c_register_board_info(1, jz_i2c1_devs, jz_i2c1_devs_size);
#endif

#if (defined(CONFIG_SOFT_I2C0_GPIO_V12_JZ) || defined(CONFIG_I2C0_V12_JZ))
     i2c_register_board_info(0, jz_i2c0_devs, jz_i2c0_devs_size);
#endif

	return 0;
}


/*
 *  * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 *   * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 *    */
const char *get_board_type(void)
{
	return CONFIG_PRODUCT_NAME;
}


arch_initcall(board_base_init);
