#ifndef __BOARD_BASE_H__
#define __BOARD_BASE_H__

#include <board.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <mach/sfc_flash.h>

#ifdef CONFIG_SGM42609_MOTOR
extern struct platform_device sgm42609_motor_device;
#endif
#ifdef CONFIG_LOGO
extern struct platform_device jz_logo_device;
#endif

#ifdef CONFIG_SGM42507
extern struct platform_device sgm42507_device;
#endif
#ifdef CONFIG_FXL_LOCK_BODY
extern struct platform_device fxl_lock_body_device;
#endif
#ifdef CONFIG_MILI_LOCK_BODY
extern struct platform_device mili_lock_body_device;
#endif
#ifdef CONFIG_KEYBOARD_GPIO
extern struct platform_device jz_button_device;
#endif
#ifdef CONFIG_KEYBOARD_MATRIX
extern struct platform_device matrix_keypad_device;
#endif
#ifdef CONFIG_LEDS_GPIO
extern struct platform_device jz_leds_gpio;
#endif
#ifdef CONFIG_LEDS_PWM
extern struct platform_device jz_leds_pwm;
#endif
#ifdef CONFIG_74HC595
extern struct platform_device jz_74hc595_dev;
#endif
#ifdef CONFIG_TM1620
extern struct platform_device tm1620_dev;
#endif
#ifdef CONFIG_ZIGBEE_CC2530
extern struct platform_device jz_cc2530_dev;
#endif
#ifdef CONFIG_LOCK_CYLINDER
extern struct platform_device lock_cylinder_dev;
#endif
#ifdef CONFIG_JZ_INTERNAL_CODEC_V12
extern struct snd_codec_data codec_data;
#endif

#ifdef CONFIG_JZMMC_V12_MMC0
//extern struct jzmmc_platform_data tf_pdata;
extern struct jzmmc_platform_data sdio_pdata;
#endif
#ifdef CONFIG_JZMMC_V12_MMC1
extern struct jzmmc_platform_data sdio_pdata;
#endif

#ifdef CONFIG_JZMMC_V11_MMC2
extern struct jzmmc_platform_data sdio_pdata;
#endif

#ifdef CONFIG_BROADCOM_RFKILL
extern struct platform_device	bt_power_device;
extern struct platform_device	bluesleep_device;
#endif

#ifdef CONFIG_BCM_AP6212_RFKILL
extern struct platform_device   bt_power_device;
#endif

#ifdef CONFIG_BCM_43438_RFKILL
extern struct platform_device   bt_power_device;
#endif

#ifdef CONFIG_USB_JZ_DWC2
extern struct platform_device   jz_dwc_otg_device;
#endif
#ifdef CONFIG_USB_OHCI_HCD
extern struct platform_device jz_ohci_device;
#endif

#ifdef CONFIG_JZ_SPI0
extern struct jz_spi_info spi0_info_cfg;
#endif
#ifdef CONFIG_JZ_SFC
extern struct jz_sfc_info sfc_info_cfg;
#endif
#if defined(CONFIG_JZ_SPI0) || defined(CONFIG_SPI_GPIO)
extern struct spi_board_info jz_spi0_board_info[];
extern int jz_spi0_devs_size;
#endif
#ifdef CONFIG_SPI_GPIO
extern struct platform_device jz_spi_gpio_device;
#endif

#ifdef CONFIG_JZ_EFUSE_V13
extern struct jz_efuse_platform_data jz_efuse_pdata;
#endif

#ifdef CONFIG_SOFT_I2C0_GPIO_V12_JZ
extern  struct platform_device i2c0_gpio_device;
#endif

#ifdef CONFIG_SOFT_I2C1_GPIO_V12_JZ
extern struct platform_device i2c1_gpio_device;
#endif

#if (defined(CONFIG_SOFT_I2C0_GPIO_V12_JZ) || defined(CONFIG_I2C0_V12_JZ))
extern struct platform_device jz_i2c0_device;
extern struct i2c_board_info jz_i2c0_devs[];
extern int jz_i2c0_devs_size;
extern struct i2c_board_info jz_v4l2_camera_devs[];
extern int jz_v4l2_devs_size;
#endif
#if (defined(CONFIG_SOFT_I2C1_GPIO_V12_JZ) || defined(CONFIG_I2C1_V12_JZ))
extern struct platform_device jz_i2c1_device;
extern struct i2c_board_info jz_i2c1_devs[];
extern int jz_i2c1_devs_size;
#endif
#if (defined(CONFIG_SOFT_I2C2_GPIO_V12_JZ) || defined(CONFIG_I2C2_V12_JZ))
extern struct platform_device jz_i2c2_device;
extern struct i2c_board_info jz_i2c2_devs[];
extern int jz_i2c2_devs_size;
#endif
#ifdef CONFIG_BACKLIGHT_PWM
extern struct platform_device backlight_device;
#endif
#ifdef CONFIG_I2C3_V12_JZ
extern struct platform_device jz_i2c3_device;
#endif
#ifdef CONFIG_MFD_JZ_SADC_V12
extern struct platform_device jz_adc_device;
#endif
#ifdef CONFIG_JZ_MAC
extern struct platform_device jz_mii_bus;
extern struct platform_device jz_mac_device;
#endif
/* lcd pdata and display panel */
#ifdef CONFIG_FB_JZ_V13
extern struct jzfb_platform_data jzfb_pdata;
#endif
#ifdef CONFIG_LCD_TRULY_TFT240240_2_E
extern struct platform_device truly_tft240240_device;
#endif
#ifdef CONFIG_LCD_TRULY_TFT240240_2_2E
extern struct platform_device truly_tft240240_device;
#endif
#if defined(CONFIG_SND_ASOC_JZ_AIC_V12)
extern struct platform_device snd_alsa_device;
#endif
#if defined(CONFIG_LCD_DK_ST7789H2)
extern struct platform_device dk_st7789h2_device;
#endif
#if defined(CONFIG_LCD_ZK_ST7789V)
extern struct platform_device zk_st7789v_device;
#endif
#if defined(CONFIG_LCD_UG2864HLBEG01)
extern struct platform_device ug2864hlbeg01_device;
#endif

#ifdef CONFIG_JZ_PWM_GENERIC
extern struct platform_device jz_pwm_devs;
#endif

#ifdef CONFIG_BATTERY_PWM
extern struct platform_device pwm_battery_device;
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART0
extern struct jz_uart_platform_data jz_uart0_platform_data;
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART1
extern struct jz_uart_platform_data jz_uart1_platform_data;
#endif

#ifdef CONFIG_SERIAL_JZ47XX_UART2
extern struct jz_uart_platform_data jz_uart2_platform_data;
#endif


#endif
