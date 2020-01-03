#ifndef __BOARD_H__
#define __BOARD_H__
#include <gpio.h>
#include <soc/gpio.h>

#define GPIO_ACTIVE_LOW                 (1)
#define GPIO_ACTIVE_HIGH                (0)


/*
 * gpio keys f1
 */
#define GPIO_SCAN_KEY                   GPIO_PB(0)
#define GPIO_POWER_KEY                  GPIO_PB(31)
#define GPIO_CHARGE_KEY                  GPIO_PB(03)
//#define GPIO_CHARGING_LIGHT                -1
//#define GPIO_CHARGEFULL_LIGHT                 -1


/*
 * matrix keypad
 */
#define MATRIX_KEYPAD_ROW0              GPIO_PB(10)
#define MATRIX_KEYPAD_ROW1              GPIO_PB(11)
#define MATRIX_KEYPAD_ROW2              GPIO_PB(12)
#define MATRIX_KEYPAD_ROW3              GPIO_PB(13)
#define MATRIX_KEYPAD_ROW4              GPIO_PB(14)

#define MATRIX_KEYPAD_COL0              GPIO_PB(7)
#define MATRIX_KEYPAD_COL1              GPIO_PB(8)
#define MATRIX_KEYPAD_COL2              GPIO_PB(9)

/*
 * WiFi
 */
#ifdef CONFIG_ESP8089
#define ESP8089_CHIP_EN                 GPIO_PC(16)
#define ESP8089_IOPWR_EN                GPIO_PC(17)
#define ESP8089_IOPWR_EN_LEVEL          0
#define ESP8089_PWR_EN                  GPIO_PC(19)
#define ESP8089_PWR_EN_LEVEL            1
#define ESP8089_WKUP_CPU                GPIO_PC(21)
#endif

#ifdef CONFIG_BCMDHD_1_141_66
#define GPIO_WIFI_RST_N                 GPIO_PC(17)
#define GPIO_WIFI_WAKE                  GPIO_PC(16)
#define GPIO_WIFI_VDD                   (-1)
#endif

/*
 * Bluetooth
 */
#ifdef CONFIG_BCM_43438_RFKILL
#define BLUETOOTH_UPORT_NAME            "ttyS0"
#define GPIO_BT_REG_ON                  GPIO_PC(18)
#define GPIO_BT_WAKE                    GPIO_PC(19)
#define GPIO_BT_INT                     GPIO_PC(20)
#define GPIO_BT_UART_RTS                GPIO_PC(13)
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
#define SGM42507_DRIVER_EN              GPIO_PB(1)
#define SGM42507_DIRECTION              GPIO_PB(0)
#endif


/*
 * Lock body
 */

#ifdef CONFIG_LOCK_BODY

#ifdef CONFIG_MILI_LOCK_BODY
#define MILI_LOCK_SQUARE_MASTER_PIN     GPIO_PC(26)     //LOCK_SWK
#define MILI_LOCK_SQUARE_ANTI_PIN       GPIO_PC(27)     //LOCK_SWF
#define MILI_LOCK_CYLINDER_ROTATION_PIN GPIO_PB(3)      //LOCK_SWS
#endif

#ifdef CONFIG_FXL_LOCK_BODY
#define FXL_LOCK_BOLIQUE_TONGUE_SHRINK  GPIO_PB(3)
#define FXL_LOCK_SQUARE_TONGUE_STRETCH  GPIO_PC(26)
#define FXL_LOCK_SQUARE_TONGUE_SHRINK   GPIO_PC(27)
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
#define SPI0_CHIP_SELECT0               (-1)
#define SPI0_CHIP_SELECT1               (-1)
#endif

/*
 * PWM Battery
 */
#define GPIO_PWM_BATTERY_POWER          (-1)
#define GPIO_LI_ION_CHARGE              (-1) /* GPIO_PB(26) */
#define GPIO_BATTERY_STATUS             GPIO_PB(21)

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
 * Audio
 */
#define GPIO_HP_MUTE                    (-1)                /* hp mute gpio */
#define GPIO_HP_MUTE_LEVEL              (-1)                /* vaild level */

#define GPIO_SPEAKER_MUTE               (-1)
#define GPIO_SPEAKER_MUTE_EN_LEVEL      (1)

#define GPIO_SPEAKER_EN                 GPIO_PC(26)          /* speaker enable gpio */
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
 * external codec
 */
#if (defined(CONFIG_AKM4951_EXTERNAL_CODEC) || defined(CONFIG_SND_ASOC_JZ_EXTCODEC_AKM4951))

#define GPIO_AKM4951_PDN                    GPIO_PA(25) /* AKM4951 PDN pin */
#define GPIO_AKM4951_SPEAKER_EN             GPIO_PA(23) /* amp shutdown pin */
#define GPIO_AKM4951_SPEAKER_EN_LEVEL       (1)
#define GPIO_AKM4951_AMP_POWER_EN           (-1)        /* amp power enable pin */
#define GPIO_AKM4951_AMP_POWER_EN_LEVEL     (1)
#define GPIO_AKM4951_LINEIN_DETECT          GPIO_PA(20) /* linein detect gpio*/
#define GPIO_AKM4951_LINEIN_INSERT_LEVEL    (0)
#define GPIO_AKM4951_HP_DETECT              GPIO_PA(21) /* hp detect gpio */
#define GPIO_AKM4951_HP_INSERT_LEVEL        (0)
#define GPIO_AKM4951_HP_MUTE                GPIO_PA(24) /* hp detect gpio */
#define GPIO_AKM4951_HP_MUTE_EN_LEVEL       (1)

#endif

/*
 * FM
 */
#ifdef CONFIG_RADIO_RDA5807M
#define GPIO_FM_PWREN                       GPIO_PB(9)  /* FM power enable gpio */
#define FM_PWREN_LEVEL                      (0)         /* power enable level */
#define FM_I2C_ADAPTER                      (2)         /* i2c adapter 0 */
#endif

/*
 * GMAC
 */
#ifdef CONFIG_JZ_MAC
#ifndef CONFIG_MDIO_GPIO
#ifdef CONFIG_JZGPIO_PHY_RESET
#define GMAC_PHY_PORT_GPIO                  GPIO_PB(4)
#define GMAC_PHY_ACTIVE_HIGH                (1)
#define GMAC_CRLT_PORT                      GPIO_PORT_B
#define GMAC_CRLT_PORT_PINS                 (0x7 << 7)
#define GMAC_CRTL_PORT_INIT_FUNC            GPIO_FUNC_1
#define GMAC_CRTL_PORT_SET_FUNC             GPIO_OUTPUT0
#define GMAC_PHY_DELAYTIME                  (10)
#endif
#else /* CONFIG_MDIO_GPIO */
#define MDIO_MDIO_MDC_GPIO                  GPIO_PF(13)
#define MDIO_MDIO_GPIO                      GPIO_PF(14)
#endif
#endif /* CONFIG_JZ4775_MAC */


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

#define FRONT_CAMERA_SENSOR_RESET                 GPIO_PA(21)
#define FRONT_CAMERA_SENSOR_PWDN                  GPIO_PA(20)
#define FRONT_CAMERA_VDD_EN                       GPIO_PC(18)
#define FRONT_CAMERA_SENSOR_RESET_LEVEL           0
#define FRONT_CAMERA_SENSOR_PWDN_LEVEL            0
#define FRONT_CAMERA_VDD_EN_LEVEL                 0

#define FRONT_CAMERA_IR_POWER_EN                  (-1)
#define FRONT_CAMERA_IR_POWER_EN__LEVEL           (1)
#endif


/*
 * LCD
 */
#ifdef  CONFIG_LCD_TRULY_TFT240240_2_E
#define GPIO_LCD_CS                         GPIO_PB(18)
#define GPIO_LCD_RD                         GPIO_PB(16)
#define GPIO_LCD_RST                        GPIO_PD(0)
#define GPIO_BL_PWR_EN                      GPIO_PD(1)
#define GPIO_LCD_PWM                        GPIO_PC(25)
#endif

#ifdef  CONFIG_LCD_TRULY_TFT240240_2_2E
#define GPIO_LCD_CS                         GPIO_PB(18)
#define GPIO_LCD_RD                         GPIO_PB(16)
#define GPIO_LCD_RST                        GPIO_PC(12)
#define GPIO_BL_PWR_EN                      GPIO_PC(24)
#define GPIO_LCD_PWM                        GPIO_PC(25)
#endif

#ifdef  CONFIG_LCD_DK_ST7789H2
#define GPIO_LCD_CS                         GPIO_PB(18)
#define GPIO_LCD_RD                         GPIO_PB(16)
#define GPIO_LCD_RST                        GPIO_PC(23)
#define GPIO_BL_PWR_EN                      GPIO_PB(19)
#define GPIO_LCD_PWM                        (-1)
#endif

#ifdef  CONFIG_LCD_ZK_ST7789V
#define GPIO_LCD_CS                         GPIO_PB(18)
#define GPIO_LCD_RD                         GPIO_PB(16)
#define GPIO_LCD_RST                        GPIO_PB(19)
#define GPIO_LCD_BACKLIGHT                  (-1)
#define GPIO_LCD_VDD_EN                     (-1)
#define GPIO_BL_PWR_EN                      (-1)
#define BL_PWR_EN_LEVEL                     (1)
#endif

#ifdef CONFIG_LCD_UG2864HLBEG01
#define GPIO_LCD_CS                         GPIO_PB(18)
#define GPIO_LCD_RD                         GPIO_PB(16)
#define GPIO_LCD_RST                        (-1)
#define GPIO_LCD_PWR_EN                     GPIO_PB(12)
#define GPIO_LCD_BACKLIGHT                  GPIO_PB(14)
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
 * 74ch595
 */
#ifdef CONFIG_74HC595
#define GPIO_74HC595_DATA                   GPIO_PB(5)
#define GPIO_74HC595_RCLK                   GPIO_PB(21)
#define GPIO_74HC595_SCLK                   GPIO_PB(22)
#define GPIO_74HC595_SCLR                   (-1)
#define GPIO_74HC595_OE                     (-1)
#endif /* CONFIG_74HC595 */

/*
 * TM1620
 */
#ifdef CONFIG_TM1620
#define TM1620_STB_PIN                     GPIO_PD(4)
#define TM1620_CLK_PIN                     GPIO_PD(5)
#define TM1620_DIO_PIN                     GPIO_PD(0)
#endif /* CONFIG_TM1620 */

/*
 *Finger Print(Microarray)
 */
#ifdef CONFIG_FINGERPRINT_MICROARRAY
#define FINGERPRINT_POWER_EN                GPIO_PB(22)
#define FINGERPRINT_POWER_2V8               (-1)
#define FINGERPRINT_POWER_1V8               (-1)
#define FINGERPRINT_INT                     GPIO_PB(02)
#define FINGERPRINT_RESET                   (-1)
#endif

/*
 *Finger Print(Goodix)
 */
#ifdef CONFIG_FINGERPRINT_GOODIX_GF5X
#define GOODIX_FP_PWR_EN                    GPIO_PB(22)
#define GOODIX_FP_INT                       GPIO_PB(02)
#define GOODIX_FP_RESET                     GPIO_PA(20)
#endif

/*
 *Finger Print(FPC)
 */
#ifdef CONFIG_FINGERPRINT_FPC
#define FPC_FP_PWR_EN                       GPIO_PB(22)
#define FPC_FP_INT                          GPIO_PB(02)
#define FPC_FP_RESET                        GPIO_PA(20)
#define FPC_FP_CS                           GPIO_PA(25)
#define ENCRYPT_IC_RST                      GPIO_PD(0)
#ifdef CONFIG_MCLK_PROVIDED_TO_ENCRYPT_IC
#define ENCRYPT_IC_CLK                      GPIO_PA(11)
#endif
#endif

#ifdef CONFIG_INPUT_BYD_FPS_SPI
#define BYD_FP_PWR_EN                       GPIO_PB(22)
#define BYD_FP_INT                          GPIO_PB(02)
#define BYD_FP_RESET                        GPIO_PA(20)
#endif
/*
 * Contactless cards IC
 */
#ifdef CONFIG_SKY1311S
#define SKY1311S_SPI_CS_PIN                 GPIO_PA(21)
#define SKY1311S_IC_INT_PIN                 GPIO_PB(7)
#define SKY1311S_IC_PD2_PIN                 GPIO_PB(4)
#endif

#ifdef CONFIG_SKY1311T
#define SKY1311T_SPI_CS_PIN                 GPIO_PA(21)
#define SKY1311T_IC_INT_PIN                 GPIO_PB(7)
#define SKY1311T_IC_PD2_PIN                 GPIO_PB(4)
#endif

/*
 * Touch(Cypress)
 */
#ifdef CONFIG_CYPRESS_PSOC4
#define CYPRESS_PSOC4_CPU_INT_PIN           GPIO_PD(3)
#define CYPRESS_PSOC4_MCU_RST_PIN           GPIO_PD(2)
#define CYPRESS_PSOC4_MCU_INT_PIN           GPIO_PB(10)
#endif /* CONFIG_CYPRESS_PSOC4 */

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
