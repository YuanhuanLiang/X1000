#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>

#define GPIO_ACTIVE_LOW                 1
#define GPIO_ACTIVE_HIGH                0

/*
 * GPIO KEYBOAED
 */
#ifdef CONFIG_KEYBOARD_GPIO
#define GPIO_F1_KEY                     GPIO_PB(28)
#define ACTIVE_LOW_F1                   (1)
#endif

/*
 * MSC GPIO Definition
 */
#define GPIO_SD0_CD_N                   (-1)

/*
 * Efuse VDD Enable
 */
#define GPIO_EFUSE_VDDQ                 (-1)

/*
 *MOTO
 */
#ifdef CONFIG_SGM42507
#define SGM42507_POWER_EN               (-1)
#define SGM42507_DRIVER_EN              GPIO_PA(7)
#define SGM42507_DIRECTION              GPIO_PA(5)
#endif


/*
 * Lock body
 */

#ifdef CONFIG_LOCK_BODY

#ifdef CONFIG_MILI_LOCK_BODY
#define MILI_LOCK_SQUARE_MASTER_PIN     GPIO_PA(8)      //LOCK_SWK
#define MILI_LOCK_SQUARE_ANTI_PIN       GPIO_PA(11)     //LOCK_SWF
#define MILI_LOCK_CYLINDER_ROTATION_PIN GPIO_PA(9)      //LOCK_SWS
#endif

#ifdef CONFIG_FXL_LOCK_BODY
#define FXL_LOCK_BOLIQUE_TONGUE_SHRINK  GPIO_PA(8)
#define FXL_LOCK_SQUARE_TONGUE_STRETCH  GPIO_PA(9)
#define FXL_LOCK_SQUARE_TONGUE_SHRINK   GPIO_PA(11)
#endif

#endif


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
#define SPI0_CHIP_SELECT0               GPIO_PA(25)
#define SPI0_CHIP_SELECT1               GPIO_PA(21)
#endif

/*
 * PWM Battery
 */
#define GPIO_PWM_BATTERY_POWER          GPIO_PC(22)
#define GPIO_LI_ION_CHARGE              (-1)
#define GPIO_BATTERY_STATUS             GPIO_PC(21)


/*
 * USB
 */
#define GPIO_USB_ID_LEVEL               LOW_ENABLE
#ifdef CONFIG_BOARD_HAS_NO_DETE_FACILITY
#define GPIO_USB_DETE                   (-1)
#define GPIO_USB_DETE_LEVEL             LOW_ENABLE
#else
#define GPIO_USB_DETE                   (-1)
#define GPIO_USB_DETE_LEVEL             HIGH_ENABLE
#endif

#define GPIO_USB_DRVVBUS                (-1)
#define GPIO_USB_DRVVBUS_LEVEL          HIGH_ENABLE

/*
 * Audio
 */
#define GPIO_HP_MUTE                    (-1)                /* hp mute gpio */
#define GPIO_HP_MUTE_LEVEL              (-1)                /* vaild level */

#define GPIO_SPEAKER_MUTE               (-1)
#define GPIO_SPEAKER_MUTE_EN_LEVEL      (1)

#define GPIO_SPEAKER_EN                 GPIO_PB(5)          /* speaker enable gpio */
#define GPIO_SPEAKER_EN_LEVEL           (1)

#define GPIO_AMP_POWER                  (-1)
#define GPIO_AMP_POWER_LEVEL            (1)

#define GPIO_AMP_MUTE                   (-1)                /* amp mute */
#define GPIO_AMP_MUTE_LEVEL             (1)

#define GPIO_HANDSET_EN                 (-1)                /* handset enable gpio */
#define GPIO_HANDSET_EN_LEVEL           (-1)

#define GPIO_HP_DETECT                  (-1)                /* hp detect gpio */
#define GPIO_HP_INSERT_LEVEL            (1)
#define GPIO_MIC_SELECT                 (-1)                /* mic select gpio */
#define GPIO_BUILDIN_MIC_LEVEL          (-1)                /* builin mic select level */
#define GPIO_MIC_DETECT                 (-1)
#define GPIO_MIC_INSERT_LEVEL           (-1)
#define GPIO_MIC_DETECT_EN              (-1)                /* mic detect enable gpio */
#define GPIO_MIC_DETECT_EN_LEVEL        (-1)                /* mic detect enable gpio */

#define HP_SENSE_ACTIVE_LEVEL           (1)
#define HOOK_ACTIVE_LEVEL               (-1)


/*
 * i2c gpio2
 */
#ifndef CONFIG_I2C2_V12_JZ
#define GPIO_I2C2_SDA                   GPIO_PD(1)
#define GPIO_I2C2_SCK                   GPIO_PD(0)
#endif

/*
 *Finger Print(Microarray)
 */
#ifdef CONFIG_FINGERPRINT_MICROARRAY
#define FINGERPRINT_POWER_EN            GPIO_PA(10)
#define FINGERPRINT_POWER_2V8           (-1)
#define FINGERPRINT_POWER_1V8           (-1)
#define FINGERPRINT_INT                 GPIO_PA(20)
#define FINGERPRINT_RESET               (-1)
#endif

/*
 *Finger Print(Goodix)
 */
#ifdef CONFIG_FINGERPRINT_GOODIX_GF5X
#define GOODIX_FP_PWR_EN                GPIO_PA(10)
#define GOODIX_FP_INT                   GPIO_PA(20)
#define GOODIX_FP_RESET                 (-1)
#endif


/*
 * Touch(Cypress)
 */
#ifdef CONFIG_CYPRESS_PSOC4
#define CYPRESS_PSOC4_CPU_INT_PIN       GPIO_PD(3)
#define CYPRESS_PSOC4_MCU_RST_PIN       GPIO_PD(2)
#define CYPRESS_PSOC4_MCU_INT_PIN       GPIO_PD(4)
#endif /* CONFIG_CYPRESS_PSOC4 */

/*
 * SFC Flash
 */
#ifdef CONFIG_JZ_SFC_FLASH_POWER_CTRL
#define GPIO_FLASH_POWER                GPIO_PC(25)
#define GPIO_FLASH_POWER_EN_LEVEL       (0)
#define FLASH_POWER_ON_DELAY            (5)
#endif /* CONFIG_JZ_SFC_FLASH_POWER_CTRL */


#define CARD_CPU_RST_CARD               GPIO_PC(18)
#define CARD_CARD_INT_CPU               GPIO_PC(17)
#define CARD_CPU_PD_CARD                GPIO_PC(16)


#define BROKEN_ALARM                    GPIO_PB(0)
/*
 * leds gpio
 */
#ifdef CONFIG_LEDS_GPIO
#ifdef CONFIG_LEDS_GPIO_BACKLIGHT
#define BACKLIGHT_LED_ACTIVE_LOW        1
#define BACKLIGHT_LEDS_D1               GPIO_PB(1)
#endif


#ifdef CONFIG_LEDS_GPIO_INDICATION
#define INDICATION_LED_ACTIVE_LOW       1
#define INDICATION_LEDS_DR              GPIO_PB(4)
#define INDICATION_LEDS_DG              GPIO_PB(2)
#define INDICATION_LEDS_DB              GPIO_PB(3)
#endif
#endif

#ifdef CONFIG_TOUCHSCREEN_ZINITIX
#define ZINITIX_TS_RST_PIN              GPIO_PD(3)
#define ZINITIX_TS_INT_PIN              GPIO_PD(4)
#define ZINITIX_TS_PWR_PIN              -1
#define ZINITIX_TS_PWR_EN_LEVEL          1
#endif

#endif /* __BOARD_H__ */
