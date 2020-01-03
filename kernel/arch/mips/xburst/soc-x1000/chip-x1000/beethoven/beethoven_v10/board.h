#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>

#define GPIO_ACTIVE_LOW      1
#define GPIO_ACTIVE_HIGH     0

/* ****************************GPIO KEY START******************************** */
/* #define GPIO_HOME_KEY		GPIO_PD(18) */
/* #define ACTIVE_LOW_HOME		1 */

/* #define GPIO_VOLUMEDOWN_KEY         GPIO_PD(18) */
/* #define ACTIVE_LOW_VOLUMEDOWN	0 */

//#define GPIO_ENDCALL_KEY            GPIO_PB(31)
//#define ACTIVE_LOW_ENDCALL      0

 #define GPIO_F1_KEY        GPIO_PB(28)
 #define ACTIVE_LOW_F1      1
/*
 #define GPIO_F2_KEY        GPIO_PA(11)
 #define ACTIVE_LOW_F2      1
 #define GPIO_F3_KEY            GPIO_PA(10)
 #define ACTIVE_LOW_F3          1
*/
 #define GPIO_F5_KEY            GPIO_PB(31)
 #define ACTIVE_LOW_F5          1

/* ****************************GPIO KEY END********************************** */

#define GPIO_EFUSE_VDDQ GPIO_PB(27)

#ifdef CONFIG_BCMDHD_1_141_66

/**
 *  ** Bluetooth gpio
 *   **/
#define BLUETOOTH_UPORT_NAME    "ttyS0"
#define GPIO_BT_REG_ON          GPIO_PC(20)
#define GPIO_BT_WAKE            GPIO_PC(16)
#define GPIO_BT_INT             GPIO_PC(18)
#define GPIO_BT_UART_RTS        GPIO_PC(13)

#define GPIO_WIFI_RST_N		GPIO_PC(17)
#define GPIO_WIFI_WAKE 		GPIO_PC(19)
#define GPIO_WIFI_VDD       GPIO_PC(21)
#endif

#ifdef CONFIG_HALLEY2_MINI_CORE_V10        //notice:here there is a problem,but slove in the future!!!

#define GPIO_WIFI_WAKEUP	GPIO_PC(16)
#define GPIO_WIFI_RST_N		GPIO_PC(17)

#endif

/* MSC GPIO Definition */
#define GPIO_SD0_CD_N       GPIO_PB(14)

/*wifi  LED */
#ifdef CONFIG_LEDS_GPIO
#define	WL_LED_R	-1//GPIO_PC(5)
#define	WL_LED_G	-1//GPIO_PC(4)
#define	WL_LED_B	-1//GPIO_PC(13)
#endif

#ifdef CONFIG_LEDS_PWM
#define LED_ACTIVE_LOW                  false
#endif

#ifdef CONFIG_SPI_GPIO
#define GPIO_SPI_SCK  GPIO_PA(26)
#define GPIO_SPI_MOSI GPIO_PA(29)
#define GPIO_SPI_MISO GPIO_PA(28)
#endif

#if defined(CONFIG_JZ_SPI) || defined(CONFIG_JZ_SFC)
#define SPI_CHIP_ENABLE GPIO_PA(27)
#endif

/* ****************************GPIO BATTERY START******************************** */
#define GPIO_LI_ION_CHARGE      -1//GPIO_PB(12)
#define GPIO_BATTERY_STATUS     GPIO_PB(15)
/* ****************************GPIO BATTERY END********************************** */

/* ****************************GPIO USB START******************************** */
/*#define GPIO_USB_ID            	GPIO_PC(21)*/
#define GPIO_USB_ID_LEVEL       LOW_ENABLE
#ifdef CONFIG_BOARD_HAS_NO_DETE_FACILITY
#define GPIO_USB_DETE           -1 /*GPIO_PC(22)*/
#define GPIO_USB_DETE_LEVEL     LOW_ENABLE
#else
#define GPIO_USB_DETE                  GPIO_PB(8)
#define GPIO_USB_DETE_LEVEL     HIGH_ENABLE
#endif
#define GPIO_USB_DRVVBUS        GPIO_PB(25)
#define GPIO_USB_DRVVBUS_LEVEL      HIGH_ENABLE
/* ****************************GPIO USB END********************************** */

/* ****************************GPIO AUDIO START****************************** */
#define GPIO_HP_MUTE        -1  /*hp mute gpio*/
#define GPIO_HP_MUTE_LEVEL  -1  /*vaild level*/

#define GPIO_SPEAKER_MUTE   GPIO_PB(7)
#define GPIO_SPEAKER_MUTE_EN_LEVEL  1

#if 0
#define GPIO_FM_AUX_SEL     GPIO_PC(21)
#define GPIO_FM_AUX_SEL_FM      1
#define GPIO_FM_AUX_SEL_AUX_IN  0
#endif

#ifdef CONFIG_HALLEY2_MINI_CORE_V10
#define GPIO_SPEAKER_EN    GPIO_PC(23)         /*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL   0
#else
#define GPIO_SPEAKER_EN    -1         /*speaker enable gpio*/
#define GPIO_SPEAKER_EN_LEVEL   0
#endif

#define GPIO_AMP_POWER                  (-1)
#define GPIO_AMP_POWER_LEVEL            (1)

#define GPIO_AMP_MUTE                   (-1)                /* amp mute */
#define GPIO_AMP_MUTE_LEVEL             (1)

#define GPIO_HANDSET_EN     -1  /*handset enable gpio*/
#define GPIO_HANDSET_EN_LEVEL   -1

#define GPIO_HP_DETECT		-1/*hp detect gpio*/
#define GPIO_HP_INSERT_LEVEL    1
#define GPIO_MIC_SELECT     -1  /*mic select gpio*/
#define GPIO_BUILDIN_MIC_LEVEL  -1  /*builin mic select level*/
#define GPIO_MIC_DETECT     -1
#define GPIO_MIC_INSERT_LEVEL   -1
#define GPIO_MIC_DETECT_EN  -1  /*mic detect enable gpio*/
#define GPIO_MIC_DETECT_EN_LEVEL -1 /*mic detect enable gpio*/

#define HP_SENSE_ACTIVE_LEVEL   1
#define HOOK_ACTIVE_LEVEL       -1

#define GPIO_SPEAKER_SHUTDOWN          GPIO_PB(10)

#if (defined(CONFIG_AKM4951_EXTERNAL_CODEC) || defined(CONFIG_SND_ASOC_JZ_EXTCODEC_AKM4951))
#define GPIO_AKM4951_PDN                GPIO_PC(23)             /* AKM4951 PDN pin */
#define GPIO_AKM4951_SPEAKER_EN         -1       /* amp shutdown pin */
#define GPIO_AKM4951_SPEAKER_EN_LEVEL   1
#define GPIO_AKM4951_AMP_POWER_EN       -1              /* amp power enable pin */
#define GPIO_AKM4951_AMP_POWER_EN_LEVEL 1
#define GPIO_AKM4951_LINEIN_DETECT          GPIO_PB(11)   /*linein detect gpio*/
#define GPIO_AKM4951_LINEIN_INSERT_LEVEL    0
#define GPIO_AKM4951_HP_DETECT          -1              /*hp detect gpio*/
#define GPIO_AKM4951_HP_INSERT_LEVEL    -1
#endif

#ifdef CONFIG_RADIO_RDA5807M
#define GPIO_FM_PWREN          GPIO_PB(9) /* FM power enable gpio */
#define FM_PWREN_LEVEL         0           /* power enable level */
#define FM_I2C_ADAPTER         2           /* i2c adapter 0 */
#endif
/* ****************************GPIO AUDIO END******************************** */

/* ****************************GPIO GMAC START******************************* */
#ifdef CONFIG_JZ_MAC
#ifndef CONFIG_MDIO_GPIO
#ifdef CONFIG_JZGPIO_PHY_RESET
#define GMAC_PHY_PORT_GPIO GPIO_PC(23)
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
#define GPIO_I2C0_SDA GPIO_PB(23)
#define GPIO_I2C0_SCK GPIO_PB(24)
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

/* ****************************GPIO CAMERA START***************************** */
#ifdef CONFIG_SENSORS_BMA2X2
#define GPIO_GSENSOR_INTR	GPIO_PB(2)
#endif

#ifdef CONFIG_VIDEO_JZ_CIM_HOST_V13
#define FRONT_CAMERA_INDEX  0
#define BACK_CAMERA_INDEX   1

#define FRONT_CAMERA_SENSOR_RESET                 GPIO_PD(5)
#define FRONT_CAMERA_SENSOR_PWDN                  GPIO_PD(4)
#define FRONT_CAMERA_VDD_EN                       GPIO_PD(3)
#define FRONT_CAMERA_SENSOR_RESET_LEVEL           0
#define FRONT_CAMERA_SENSOR_PWDN_LEVEL            0
#define FRONT_CAMERA_VDD_EN_LEVEL                 1

#define FRONT_CAMERA_IR_POWER_EN                  (-1)
#define FRONT_CAMERA_IR_POWER_EN__LEVEL           (1)
#endif
/* ****************************GPIO CAMERA END******************************* */


//#define GPIO_EFUSE_VDDQ		GPIO_PB(27) /* EFUSE must be -ENODEV or a gpio */

/* PMU RN5T567 */
#ifdef CONFIG_REGULATOR_RN5T567
#define PMU_IRQ_N       -1
#define PMU_SLP_N       -1
#define SLP_PIN_DISABLE_VALUE  1
#endif /* CONFIG_REGULATOR_RN5T567 */

#endif /* __BOARD_H__ */
