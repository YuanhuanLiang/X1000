#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>

#define GPIO_ACTIVE_LOW				(1)
#define GPIO_ACTIVE_HIGH			(0)

/*
 * gpio keys f1
 */
#define GPIO_MENU_KEY				GPIO_PB(28)
#define GPIO_POWER_KEY				GPIO_PB(31)

/*
 * WiFi
 */
#ifdef CONFIG_ESP8089
#define ESP8089_CHIP_EN				GPIO_PC(13)
#define ESP8089_IOPWR_EN			(-1)
#define ESP8089_IOPWR_EN_LEVEL		0
#define ESP8089_PWR_EN				(-1)
#define ESP8089_PWR_EN_LEVEL		1
#define ESP8089_WKUP_CPU			(-1)
#endif

#ifdef CONFIG_RTL8189FS
#define RTL8189FS_CHIP_EN			(-1)//GPIO_PC(13)
#define RTL8189FS_IOPWR_EN			(-1)
#define RTL8189FS_IOPWR_EN_LEVEL	0
#define RTL8189FS_PWR_EN			(-1)
#define RTL8189FS_PWR_EN_LEVEL		1
#define RTL8189FS_WKUP_CPU			(-1)
#endif

/*
 * cc1101 RF (Temporarily useless)
 */
#define CC1101_GD0					GPIO_PD(5)

/*
 * MSC GPIO Definition
 */
#define GPIO_SD0_CD_N                   (-1)

/*
 * Efuse VDD Enable
 */
#define GPIO_EFUSE_VDDQ                 (-1)


/*
 * SPI GPIO
 */
#ifdef CONFIG_SPI_GPIO
#define GPIO_SPI_SCK                    GPIO_PA(26)
#define GPIO_SPI_MOSI                   GPIO_PA(29)
#define GPIO_SPI_MISO                   GPIO_PA(28)
#endif

#if defined(CONFIG_MTD_JZ_SPI_NOR) || defined(CONFIG_MTD_JZ_SPI_NAND)
#define SPI_CHIP_ENABLE                 GPIO_PA(27)
#endif

#if defined(CONFIG_JZ_SPI0)
#define SPI0_CHIP_SELECT0               GPIO_PD(1)//(-1) 
#define SPI0_CHIP_SELECT1               (-1)
#endif

/*
 * PWM LED
 */
//#define GPIO_PWM_BATTERY_POWER          (-1)
//#define GPIO_LI_ION_CHARGE              (-1) /* GPIO_PB(26) */
//#define GPIO_BATTERY_STATUS             GPIO_PC(24)

/*
 * USB
 */
#define GPIO_USB_ID_LEVEL               LOW_ENABLE
#ifdef CONFIG_BOARD_HAS_NO_DETE_FACILITY
#define GPIO_USB_DETE                   (-1) /* GPIO_PC(22) */
#define GPIO_USB_DETE_LEVEL             LOW_ENABLE
#else
#define GPIO_USB_DETE                   (-1)
#define GPIO_USB_DETE_LEVEL             HIGH_ENABLE
#endif

#define GPIO_USB_DRVVBUS                GPIO_PB(25)
#define GPIO_USB_DRVVBUS_LEVEL          HIGH_ENABLE



/*
 * i2c gpio0
 */
#ifndef CONFIG_I2C0_V12_JZ
#define GPIO_I2C0_SDA                       GPIO_PB(24)
#define GPIO_I2C0_SCK                       GPIO_PB(23)
#endif

/*
 * i2c gpio1
 */
#ifndef CONFIG_I2C1_V12_JZ
#define GPIO_I2C1_SDA                       GPIO_PC(27)
#define GPIO_I2C1_SCK                       GPIO_PC(26)
#endif

/*
 * i2c gpio2
 */
#ifndef CONFIG_I2C2_V12_JZ
#define GPIO_I2C2_SDA                       GPIO_PD(1)
#define GPIO_I2C2_SCK                       GPIO_PD(0)
#endif

/*
 * Camera
 */
#ifdef CONFIG_SENSORS_BMA2X2
#define GPIO_GSENSOR_INTR                   GPIO_PB(2)
#endif

#ifdef CONFIG_VIDEO_JZ_CIM_HOST_V13
#define FRONT_CAMERA_INDEX                  (0)
#define BACK_CAMERA_INDEX                   (1)

#define FRONT_CAMERA_SENSOR_RESET                 (-1)//GPIO_PA(21) -- liangyh
#define FRONT_CAMERA_SENSOR_PWDN                  GPIO_PB(5)// when LEVEL=0 SC031GS  sleep
#define FRONT_CAMERA_VDD_EN                       (-1)
#define FRONT_CAMERA_SENSOR_RESET_LEVEL           0
#define FRONT_CAMERA_SENSOR_PWDN_LEVEL            1
#define FRONT_CAMERA_VDD_EN_LEVEL                 0 

#define FRONT_CAMERA_IR_POWER_EN                  (-1)
#define FRONT_CAMERA_IR_POWER_EN__LEVEL           (1)
#endif

/*
 * PMU RN5T567
 */
#ifdef CONFIG_REGULATOR_RN5T567
#define PMU_IRQ_N                           (-1)
#define PMU_SLP_N                           (-1)
#define SLP_PIN_DISABLE_VALUE               (1)
#endif /* CONFIG_REGULATOR_RN5T567 */



/*
 * SFC Flash
 */
#ifdef CONFIG_JZ_SFC_FLASH_POWER_CTRL
#define GPIO_FLASH_POWER                    GPIO_PC(25)
#define GPIO_FLASH_POWER_EN_LEVEL           (0)
#define FLASH_POWER_ON_DELAY                (5)
#endif /* CONFIG_JZ_SFC_FLASH_POWER_CTRL */


/*
 * leds gpio
 */
#ifdef CONFIG_LEDS_GPIO

/* leds matrix */
#ifdef CONFIG_LEDS_GPIO_MATRIX
#define MATRIX_LED_COL_ACTIVE_LOW           1
#define MATRIX_LED_ROW_ACTIVE_LOW           0
#define MATRIX_LEDS_COL_D1                  GPIO_PB(15)
#define MATRIX_LEDS_COL_D2                  GPIO_PB(19)
#define MATRIX_LEDS_COL_D3                  GPIO_PB(11)
#define MATRIX_LEDS_ROW_D1                  GPIO_PB(8)
#define MATRIX_LEDS_ROW_D2                  GPIO_PB(13)
#define MATRIX_LEDS_ROW_D3                  GPIO_PB(9)
#define MATRIX_LEDS_ROW_D4                  GPIO_PB(06)
#endif

#endif /* end of CONFIG_LEDS_GPIO */

#endif /* __BOARD_H__ */
