#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>
#include "pmu.h"


/* ****************************GPIO KEY START******************************** */
#define GPIO_NEXT_KEY		GPIO_PB(15)
#define ACTIVE_LOW_NEXT		1

#define GPIO_PREV_KEY		GPIO_PB(14)
#define ACTIVE_LOW_PREV		1

#define GPIO_LINE_CTRL_KEY          -1 //GPIO_PA(15)
#define ACTIVE_LINE_CTRL      0

#define GPIO_PLAY_KEY		GPIO_PB(31)
#define ACTIVE_LOW_PLAY		1

/* ****************************GPIO KEY END********************************** */
/**
 *  ** Bluetooth && wlan gpio
 *   **/
#define BLUETOOTH_UPORT_NAME    "ttyS0"
#define GPIO_BT_REG_ON          GPIO_PC(18)
#define GPIO_HOST_WAKE_BT       GPIO_PC(20)
#define GPIO_BT_WAKE_HOST       GPIO_PC(19)
//#define GPIO_BT_UART_RTS        GPIO_PC(13)

#define GPIO_WIFI_WAKE_HOST	    GPIO_PC(16)
#define GPIO_WIFI_REG_ON		GPIO_PC(17)
#define WLAN_SDIO_INDEX			1

#define RESET               		0
#define NORMAL              		1

/* MSC GPIO Definition */
#define GPIO_SD0_CD_N       GPIO_PC(21)
#define GPIO_SD0_PWR	    GPIO_PB(23)

#if 0
/*wifi  LED */
#ifdef CONFIG_LEDS_GPIO
#define	LED_P1 GPIO_PB(8)
#define	LED_P2 GPIO_PB(9)
#define	LED_P3 GPIO_PB(7)
#define	LED_N1 GPIO_PB(12)
#define	LED_N2 GPIO_PB(10)
#define	LED_N3 GPIO_PB(11)
#define	LED_N4 GPIO_PB(13)
#endif
#endif
#if 0
#ifdef CONFIG_SPI_GPIO
#define GPIO_SPI_SCK  GPIO_PA(26)
#define GPIO_SPI_MOSI GPIO_PA(29)
#define GPIO_SPI_MISO GPIO_PA(28)
#endif
#endif

#if defined(CONFIG_JZ_SPI) || defined(CONFIG_JZ_SFC)
#define SPI_CHIP_ENABLE GPIO_PA(27)
#endif
/* ****************************GPIO USB START******************************** */
#define GPIO_USB_ID             -1 //GPIO_PD(3)/*GPIO_PB(4)*/
#define GPIO_USB_ID_LEVEL       -1 //LOW_ENABLE

/* note : when the usb detect pin don't be connected, -1 should be set to GPIO_USB_DETE */
#if 0
    #define GPIO_USB_DETE           GPIO_PB(6) 
    #define GPIO_USB_DETE_LEVEL     LOW_ENABLE
#endif
#if 1
    #define GPIO_USB_DETE           GPIO_PB(8)
    #define GPIO_USB_DETE_LEVEL     HIGH_ENABLE
#endif
#if 0
    #define GPIO_USB_DETE           -2
    #define GPIO_USB_DETE_LEVEL     -1
#endif

#define GPIO_USB_DRVVBUS        -1//GPIO_PB(25)
#define GPIO_USB_DRVVBUS_LEVEL      HIGH_ENABLE
/* ****************************GPIO USB END********************************** */

/* ****************************GPIO AUDIO START****************************** */
#define GPIO_SPEAKER_MUTE               (-1)
#define GPIO_SPEAKER_MUTE_EN_LEVEL      (1)
#define GPIO_SPEAKER_EN_LEVEL	0
#define GPIO_SPEAKER_EN			-1
#define GPIO_HP_INSERT_LEVEL    0
#define GPIO_HP_DETECT			GPIO_PA(17)
#define GPIO_PO_PWR_EN       	GPIO_PA(10)
#define GPIO_HP_MUTE			GPIO_PA(14)
#define GPIO_HP_MUTE_LEVEL		1
#define GPIO_LINE_MUTE			GPIO_PA(19)
#define GPIO_DAC_N_MUTE			GPIO_PD(3)
/* ****************************GPIO AUDIO END******************************** */

/* ****************************GPIO GMAC START******************************* */
#ifdef CONFIG_JZ_MAC
#ifndef CONFIG_MDIO_GPIO
#ifdef CONFIG_JZGPIO_PHY_RESET
#define GMAC_PHY_PORT_GPIO  -1//GPIO_PB(3)
#define GMAC_PHY_ACTIVE_HIGH 1
#define GMAC_CRLT_PORT GPIO_PORT_B
#define GMAC_CRLT_PORT_PINS (0x7 << 7)
#define GMAC_CRTL_PORT_INIT_FUNC GPIO_FUNC_1
#define GMAC_CRTL_PORT_SET_FUNC GPIO_OUTPUT0
#define GMAC_PHY_DELAYTIME 10
#endif
#else /* CONFIG_MDIO_GPIO */
#define MDIO_MDIO_MDC_GPIO GPIO_PF(13)
#define MDIO_MDIO_GPIO GPIO_PF(14)
#endif
#endif /* CONFIG_JZ4775_MAC */
/* ****************************GPIO GMAC END********************************* */


/* ****************************GPIO I2C START******************************** */
#ifndef CONFIG_I2C0_V12_JZ
#define GPIO_I2C0_SDA GPIO_PB(11)
#define GPIO_I2C0_SCK GPIO_PB(12)
#endif
#ifndef CONFIG_I2C1_V12_JZ
#define GPIO_I2C1_SDA GPIO_PC(27)
#define GPIO_I2C1_SCK GPIO_PC(26)
#endif
#ifndef CONFIG_I2C2_V12_JZ
#define GPIO_I2C2_SDA GPIO_PD(1)
#define GPIO_I2C2_SCK GPIO_PD(0)
#endif
/* ****************************GPIO I2C END********************************** */

#define GPIO_LCD_CS     GPIO_PB(18)
#define GPIO_LCD_RD     GPIO_PB(16)
#define GPIO_LCD_RST    GPIO_PA(11)
#define GPIO_BL_PWR_EN  GPIO_PA(9)

#define GPIO_EFUSE_VDDQ	GPIO_PB(27)		/* EFUSE must be -ENODEV or a gpio */


/* PMU AXP */
#ifdef CONFIG_KP_AXP
//#define PMU_IRQ_N       GPIO_PD(2)
#define PMU_IRQ_N       GPIO_PA(16)
#endif /* CONFIG_KP_AXP */
#endif /* __BOARD_H__ */

