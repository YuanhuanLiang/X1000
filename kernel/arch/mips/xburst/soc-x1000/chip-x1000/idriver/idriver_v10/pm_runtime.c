/*
 * Copyright (c) 2006-2010  Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <gpio.h>
#include "board.h"

int gpio_runtime_pm_table[][2] = {
    {32*0+0,    GSS_INPUT_PULL  },  /* SLCD_D0 */
    {32*0+1,    GSS_INPUT_PULL  },  /* SLCD_D1 */
    {32*0+2,    GSS_INPUT_PULL  },  /* SLCD_D2 */
    {32*0+3,    GSS_INPUT_PULL  },  /* SLCD_D3 */
    {32*0+4,    GSS_INPUT_PULL  },  /* SLCD_D4 */
    {32*0+5,    GSS_INPUT_PULL  },  /* SLCD_D5 */
    {32*0+6,    GSS_INPUT_PULL  },  /* SLCD_D6 */
    {32*0+7,    GSS_INPUT_PULL  },  /* SLCD_D7 */
    {32*0+8,    GSS_INPUT_PULL  },  /* NC */
    {32*0+9,    GSS_INPUT_PULL  },  /* LCD ENABLE */
    {32*0+10,   GSS_INPUT_NOPULL  },  /* AMPLIFIER ENABLE */
    {32*0+11,   GSS_INPUT_PULL  },  /* LCD RST */
    {32*0+12,   GSS_INPUT_PULL  },  /* NC */
    {32*0+13,   GSS_INPUT_PULL  },  /* NC */
    {32*0+14,   GSS_INPUT_NOPULL  },  /* HEADPHONE MUTE */
    {32*0+15,   GSS_INPUT_PULL  },  /* NC */
    {32*0+16,   GSS_INPUT_PULL  },  /* pmu irq */
    {32*0+17,   GSS_INPUT_PULL  },  /* NC */
    {32*0+18,   GSS_INPUT_PULL  },  /* NC */
    {32*0+19,   GSS_INPUT_NOPULL  },  /* mute */
    {32*0+20,   GSS_INPUT_PULL  },  /* MSC0_D3 */
    {32*0+21,   GSS_INPUT_PULL  },  /* MSC0_D2 */
    {32*0+22,   GSS_INPUT_PULL  },  /* MSC0_D1 */
    {32*0+23,   GSS_INPUT_PULL  },  /* MSC0_D0 */
    {32*0+24,   GSS_INPUT_PULL  },  /* MSC0_CLK */
    {32*0+25,   GSS_INPUT_PULL  },  /* MSC0_CMD */
    {32*0+26,   GSS_INPUT_PULL  },      /* SFC_CLK */
    {32*0+27,   GSS_INPUT_PULL  },      /* SFC_CE */
    {32*0+28,   GSS_INPUT_PULL  },      /* SFC_DR */
    {32*0+29,   GSS_INPUT_PULL  },      /* SFC_DT */
    {32*0+30,   GSS_INPUT_PULL  },      /* SFC_WP */
    {32*0+31,   GSS_INPUT_PULL  },      /* SFC_HOL */
    {32*1+0,    GSS_INPUT_PULL  },      /* I2S_MCLK */
    {32*1+1,    GSS_INPUT_PULL  },      /* I2S_BCLK */
    {32*1+2,    GSS_INPUT_PULL  },      /* I2S_LRCLK */
    {32*1+3,    GSS_INPUT_PULL  },      /* NC */
    {32*1+4,    GSS_INPUT_PULL  },      /* I2S_DO */
    {32*1+5,    GSS_INPUT_PULL  },  /* NC */
    {32*1+6,    GSS_INPUT_PULL  },  /* NC */
    {32*1+7,    GSS_INPUT_PULL  },  /* NC */
    {32*1+8,    GSS_INPUT_NOPULL},  /* USB-DET */
    {32*1+9,    GSS_INPUT_PULL  },  /* NC */
    {32*1+10,   GSS_INPUT_PULL  },  /* NC */
    {32*1+11,   GSS_INPUT_PULL},  /* sda */
    {32*1+12,   GSS_INPUT_PULL},  /* scl */
    {32*1+13,   GSS_INPUT_PULL  },  /* NC */
    {32*1+14,   GSS_INPUT_PULL  },  /* priv key */
    {32*1+15,   GSS_INPUT_PULL  },  /* next key */
    {32*1+16,   GSS_INPUT_PULL  },  /* SLCD_RD */
    {32*1+17,   GSS_INPUT_PULL  },  /* SLCD_WR */
    {32*1+18,   GSS_INPUT_PULL  },  /* SLCD_CS */
    {32*1+19,   GSS_INPUT_PULL  },  /* NC */
    {32*1+20,   GSS_INPUT_PULL  },  /* SLCD_DC */
    {32*1+21,   GSS_INPUT_PULL  },  /* NC */
    {32*1+22,   GSS_INPUT_PULL  },  /* NC */
    {32*1+23,   GSS_INPUT_NOPULL  },  /* tf enable */
    {32*1+24,   GSS_INPUT_PULL  },  /* NC */
    {32*1+25,   GSS_INPUT_PULL  },  /* NC */
    {32*1+26,   GSS_INPUT_NOPULL  },/* CLK32K */
    {32*1+27,   GSS_INPUT_PULL  },  /* efuse en */
    {32*1+28,   GSS_INPUT_NOPULL    },  /* BOOT_SEL0 */
    {32*1+29,   GSS_INPUT_NOPULL    },  /* BOOT_SEL1 */
    {32*1+30,   GSS_INPUT_NOPULL    },  /* BOOT_SEL2 */
    {32*1+31,   GSS_INPUT_PULL  },  /* WAKEUP */
    {32*2+0,    GSS_INPUT_PULL  },  /* MSC1_CLK */
    {32*2+1,    GSS_INPUT_PULL  },  /* MSC1_CMD */
    {32*2+2,    GSS_INPUT_PULL  },  /* MSC1_D0 */
    {32*2+3,    GSS_INPUT_PULL  },  /* MSC1_D1 */
    {32*2+4,    GSS_INPUT_PULL  },  /* MSC1_D2 */
    {32*2+5,    GSS_INPUT_PULL  },  /* MSC1_D3 */
    {32*2+6,    GSS_INPUT_PULL  },  /* NC */
    {32*2+7,    GSS_INPUT_PULL  },  /* NC */
    {32*2+8,    GSS_INPUT_PULL  },  /* NC */
    {32*2+9,    GSS_INPUT_PULL  },  /* NC */
    {32*2+10,   GSS_INPUT_PULL  },  /* UART0_RXD */
    {32*2+11,   GSS_INPUT_PULL  },  /* UART0_TXD */
    {32*2+12,   GSS_INPUT_PULL  },  /* UART0_CTS_N */
    {32*2+13,   GSS_INPUT_PULL  },  /* UART0_RTS_N */
    {32*2+16,   GSS_INPUT_PULL  },  /* WL_WAKE_HOST */
    {32*2+17,   GSS_INPUT_PULL  },  /* WL_REG_EN */
    {32*2+18,   GSS_INPUT_PULL    },  /* BT_REG_EN */
    {32*2+19,   GSS_INPUT_PULL    },  /* NC */
    {32*2+20,   GSS_INPUT_PULL    },  /* NC */
    {32*2+21,   GSS_INPUT_PULL    },  /* SD_CD_N */
    {32*2+22,   GSS_INPUT_PULL    },  /* NC */
    {32*2+23,   GSS_INPUT_PULL    },  /* NC */
    {32*2+24,   GSS_INPUT_PULL    },  /* NC */
    {32*2+25,   GSS_INPUT_PULL    },  /* NC */
    {32*2+26,   GSS_INPUT_PULL    },  /* SMB1_SCk */
    {32*2+27,   GSS_INPUT_PULL    },  /* SMB1_SDA */
    {32*2+31,   GSS_IGNORE  },  /* uart2 */
    {32*3+0,    GSS_INPUT_PULL  },  /* SMB2_SCk */
    {32*3+1,    GSS_INPUT_PULL  },  /* SMB2_SDA */
    {32*3+2,    GSS_INPUT_PULL  },  /* NC */
    {32*3+3,    GSS_INPUT_NOPULL  },  /* DAC-MUTE */
    {32*3+4,    GSS_INPUT_PULL  },  /* NC */
    {32*3+5,    GSS_INPUT_PULL  },  /* NC */
    {GSS_TABLET_END,GSS_TABLET_END  }   /* GPIO Group Set End */
};


#if 0
int gpio_runtime_pm_table[][2] = {
#ifdef CONFIG_KEYBOARD_GPIO
    //keyboard
    {GPIO_NEXT_KEY, GSS_IGNORE},
    {GPIO_PREV_KEY, GSS_IGNORE},
//    {GPIO_LINE_CTRL_KEY, -1},
    {GPIO_PLAY_KEY, GSS_IGNORE}, //GPIO_PB(31)
#endif

#ifdef CONFIG_BCM_AP6212_RFKILL
    //Bluetooth && wlan gpio
    {GPIO_WIFI_REG_ON, GSS_IGNORE},  //GPIO_PC(17)  [connect directly]
    {GPIO_WIFI_WAKE_HOST, GSS_IGNORE},//GPIO_PC(16) [connect directly]
    {GPIO_BT_REG_ON, GSS_IGNORE},    //GPIO_PC(18)  [connect directly]
    {GPIO_HOST_WAKE_BT, GSS_IGNORE}, //GPIO_PC(20)  [pull up]
    {GPIO_BT_WAKE_HOST, GSS_IGNORE}, //GPIO_PC(19)  [connect directly]
//  {#define GPIO_BT_UART_RTS, -1}     //GPIO_PC(13)[pull up]
#ifdef CONFIG_JZMMC_V12_MMC1
    {GPIO_PC(0), GSS_IGNORE},   //MSC1_CLK
    {GPIO_PC(1), GSS_IGNORE},   //MSC1_CMD
    {GPIO_PC(2), GSS_IGNORE},   //MSC1_D0
    {GPIO_PC(3), GSS_IGNORE},   //MSC1_D1
    {GPIO_PC(4), GSS_IGNORE},   //MSC1_D2
    {GPIO_PC(5), GSS_IGNORE},   //MSC1_D3
#endif
#endif
#ifdef CONFIG_SERIAL_JZ47XX_UART0
    //uart
    {GPIO_PC(10), GSS_IGNORE},   //UART0_RXD
    {GPIO_PC(11), GSS_IGNORE},   //UART0_TXD
    {GPIO_PC(12), GSS_IGNORE},   //UART0_CTS_N
    {GPIO_PC(13), GSS_IGNORE},   //UART0_RTS_N
#endif
#ifdef SERIAL_JZ47XX_UART2_PC
    {GPIO_PC(31), GSS_IGNORE},   //UART2_RXD, UART2_TXD
#endif

    //TF card
    {GPIO_SD0_CD_N, GSS_IGNORE},  //GPIO_PC(21)  [pull up]
    {GPIO_SD0_PWR,  GSS_IGNORE},  //GPIO_PB(23)  [pull down]
#ifdef CONFIG_JZMMC_V12_MMC0
    {GPIO_PA(20), GSS_IGNORE},    //MSC0_D3      [pull up]
    {GPIO_PA(21), GSS_IGNORE},    //MSC0_D2      [pull up]
    {GPIO_PA(22), GSS_IGNORE},    //MSC0_D1      [pull up]
    {GPIO_PA(23), GSS_IGNORE},    //MSC0_D0      [pull up]
    {GPIO_PA(24), GSS_IGNORE},    //MSC0_CLK     [connect directly]
    {GPIO_PA(25), GSS_IGNORE},    //MSC0_CMD     [pull up]
#endif

    //sfc flash
    {GPIO_PA(26), GSS_IGNORE},    //SFC_CLK      [connect directly]
    {GPIO_PA(27), GSS_IGNORE},    //SFC_CE       [connect directly]
    {GPIO_PA(28), GSS_IGNORE},    //SFC_DR       [connect directly]
    {GPIO_PA(29), GSS_IGNORE},    //SFC_DT       [connect directly]
    {GPIO_PA(30), GSS_IGNORE},    //SFC_WP       [connect directly]
    {GPIO_PA(31), GSS_IGNORE},    //SFC_HOLD     [connect directly]
#ifdef GPIO_USB_DETE
    //USB
    {GPIO_USB_DETE, GSS_IGNORE},    //GPIO_PB(8)    [pull up]
#endif
//    {GPIO_USB_DRVVBUS, -1}        //GPIO_PB(25)
    //otg pins reserved

    //audio
//    {GPIO_SPEAKER_EN, -1}
//    {GPIO_HP_DETECT, -1},         //GPIO_PA(17)
#if 0
#ifdef GPIO_PO_PWR_EN
    {GPIO_PO_PWR_EN, GSS_IGNORE},   //GPIO_PA(10) [connect directly]
#endif
#endif
    {GPIO_HP_MUTE, GSS_IGNORE},     //GPIO_PA(14) [pull down]
//    {GPIO_LINE_MUTE, GSS_IGNORE}, //GPIO_PA(19) [pull down]
    {GPIO_DAC_N_MUTE, GSS_IGNORE},  //GPIO_PD(3)  [pull down]
    {GPIO_PB(0), GSS_IGNORE},       //I2S_MCLK    [connect directly]
    {GPIO_PB(1), GSS_IGNORE},       //I2S_BCLK    [connect directly]
    {GPIO_PB(2), GSS_IGNORE},       //I2S_LRCLK   [connect directly]
    {GPIO_PB(4), GSS_IGNORE},       //I2S_SDO     [connect directly]

    //I2C
#ifndef CONFIG_I2C0_V12_JZ
    {GPIO_I2C0_SDA, GSS_IGNORE},      //GPIO_PB(11)   [pull up]
    {GPIO_I2C0_SCK, GSS_IGNORE},      //GPIO_PB(12)   [pull up]
#endif
#ifdef CONFIG_I2C1_V12_JZ
    {GPIO_PC(27), GSS_IGNORE},      //I2C1_SDA     [pull up]
    {GPIO_PC(26), GSS_IGNORE},      //I2C1_SCK     [pull up]
#endif

    {GPIO_PD(1), GSS_IGNORE},       //I2C2_SCK     [pull up]
    {GPIO_PD(0), GSS_IGNORE},       //I2C2_SDA     [pull up]

    //LCD
#ifdef CONFIG_LCD_V13_SLCD_8BIT
    {GPIO_PA(0), GSS_IGNORE},       //LCD_D0      [connect directly]
    {GPIO_PA(1), GSS_IGNORE},       //LCD_D1      [connect directly]
    {GPIO_PA(2), GSS_IGNORE},       //LCD_D2
    {GPIO_PA(3), GSS_IGNORE},       //LCD_D3
    {GPIO_PA(4), GSS_IGNORE},       //LCD_D4
    {GPIO_PA(5), GSS_IGNORE},       //LCD_D5
    {GPIO_PA(6), GSS_IGNORE},       //LCD_D6
    {GPIO_PA(7), GSS_IGNORE},       //LCD_D7
    {GPIO_PB(17), GSS_IGNORE},      //LCD_WR
    {GPIO_PB(19), GSS_IGNORE},      //LCD_TE
    {GPIO_PB(20), GSS_IGNORE},      //LCD_DC
#endif
#ifdef CONFIG_OLED_RGS15128128WR000
    {GPIO_BL_PWR_EN, GSS_IGNORE},   //GPIO_PA(9)  [connect directly]
    {GPIO_LCD_RST, GSS_IGNORE},     //GPIO_PA(11) [connect directly]
    {GPIO_LCD_CS, GSS_IGNORE},      //GPIO_PB(18) [connect directly]
    {GPIO_LCD_RD, GSS_IGNORE},      //GPIO_PB(16) [connect directly]
#endif
    //EFUSE
    {GPIO_EFUSE_VDDQ, GSS_IGNORE},  //GPIO_PB(27)   [pull up]
    //other functionality
//    {PMU_IRQ_N, GSS_IGNORE},

	{GSS_TABLET_END, GSS_TABLET_END	},	/* GPIO Group Set End */
};
#endif
