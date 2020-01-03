/*
 * Copyright (c) 2006-2010  Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <gpio.h>

// default gpio state is input pull;
 __initdata int gpio_nc_table[][2] = {
    {GSS_TABLET_END,GSS_TABLET_END  }   /* GPIO Group Set End */
};


__initdata int gpio_ss_table[][2] = {
    /* GPIO Group - A */
    {32*0+0,    GSS_INPUT_NOPULL  },  /* IN_LOCK_N  */
    {32*0+1,    GSS_INPUT_NOPULL  },  /* S_LOCK_TAB_N  */
    {32*0+2,    GSS_OUTPUT_LOW  },    /* NULL  */
    {32*0+3,    GSS_OUTPUT_HIGH  },   /* LED_D5  */
    {32*0+4,    GSS_INPUT_NOPULL  },  /* MAG_SW _N  */
    {32*0+5,    GSS_INPUT_NOPULL  },  /* W IFI_EN  */
    {32*0+6,    GSS_OUTPUT_LOW  },    /* NULL  */
    {32*0+7,    GSS_INPUT_NOPULL  },  /* MENU  */
    {32*0+8,    GSS_INPUT_NOPULL  },  /* FACTORY_RESET_EN  */
    {32*0+9,    GSS_OUTPUT_HIGH  },   /* LED_D10  */
    {32*0+10,   GSS_OUTPUT_HIGH  },   /* LED_D8  */
    {32*0+11,   GSS_OUTPUT_LOW  },    /* FPC_AEC_CLK  */
    {32*0+12,   GSS_OUTPUT_HIGH  },   /* LED_D9  */
    {32*0+13,   GSS_OUTPUT_HIGH  },   /* LED_D6  */
    {32*0+14,   GSS_INPUT_PULL  },    /* CPU_RST_TOUCH   */
    {32*0+15,   GSS_OUTPUT_LOW  },    /* ROTATE_SW _DE  */
    {32*0+16,   GSS_IGNORE  },        /* TOUCH_INT_CPU   -->  CHECK*/
    {32*0+17,   GSS_OUTPUT_LOW  },    /* FPW KUP_RING_SEL  */
    {32*0+18,   GSS_OUTPUT_HIGH  },   /* LED_D11  */
    {32*0+19,   GSS_IGNORE  },        /* CPU_INT_TOUCH  */
    {32*0+20,   GSS_INPUT_NOPULL  },  /* BT_EN  */
    {32*0+21,   GSS_OUTPUT_HIGH  },   /* SENSOR_PW R_EN  */
    {32*0+22,   GSS_OUTPUT_LOW  },    /* SPI_DT  */
    {32*0+23,   GSS_OUTPUT_LOW  },    /* SPI_DR  */
    {32*0+24,   GSS_OUTPUT_LOW  },    /* SPI_CLK  */
    {32*0+25,   GSS_OUTPUT_LOW  },    /* SPI_CE  */
#ifdef CONFIG_JZ_SFC_FLASH_POWER_CTRL
    {32*0+26,   GSS_OUTPUT_LOW  },    /* SFC_CLK */
    {32*0+27,   GSS_OUTPUT_LOW  },    /* SFC_CE */
    {32*0+28,   GSS_OUTPUT_LOW  },    /* SFC_DR */
    {32*0+29,   GSS_OUTPUT_LOW  },    /* SFC_DT */
    {32*0+30,   GSS_OUTPUT_LOW  },    /* SFC_WP */
    {32*0+31,   GSS_OUTPUT_LOW  },    /* SFC_HOLD */
#else
    {32*0+26,   GSS_INPUT_NOPULL  },    /* SFC_CLK */
    {32*0+27,   GSS_INPUT_PULL  },    /* SFC_CE */
    {32*0+28,   GSS_INPUT_PULL  },    /* SFC_DR */
    {32*0+29,   GSS_INPUT_PULL  },    /* SFC_DT */
    {32*0+30,   GSS_INPUT_PULL  },    /* SFC_WP */
    {32*0+31,   GSS_INPUT_PULL  },    /* SFC_HOLD */
#endif

    /* GPIO Group - B */
    {32*1+0,    GSS_OUTPUT_HIGH  },    /* LOCK_CORE_PW R_EN  */
    {32*1+1,    GSS_IGNORE  },         /* KEY_DET_N  */
    {32*1+2,    GSS_INPUT_NOPULL  },    /* KEY_R  */
    {32*1+3,    GSS_INPUT_NOPULL  },    /* KEY_L  */
    {32*1+4,    GSS_OUTPUT_LOW  },    /* LOCK_ID  */
    {32*1+5,    GSS_OUTPUT_LOW  },    /* MOTO_DIRECTION  */
    {32*1+6,    GSS_OUTPUT_LOW  },    /* LED-R  */
    {32*1+7,    GSS_OUTPUT_LOW  },   /* LED-W  */
    {32*1+8,    GSS_OUTPUT_HIGH  },   /* LED_D3  */
    {32*1+9,    GSS_OUTPUT_LOW  },   /* F_SHUTDOWN_N  */
    {32*1+10,   GSS_OUTPUT_LOW  },   /* LED-G  */
    {32*1+11,   GSS_OUTPUT_HIGH  },   /* LED_D2  */
    {32*1+12,   GSS_INPUT_NOPULL  },    /* M_LOCK_TAB_N  */
    {32*1+13,   GSS_OUTPUT_HIGH  },   /* LED_D1  */
    {32*1+14,   GSS_OUTPUT_LOW  },    /* NULL  */
    {32*1+15,   GSS_OUTPUT_LOW  },   /* B_SHUTDOWN_N  */
    {32*1+16,   GSS_OUTPUT_HIGH  },   /* LED_D7  */
    {32*1+17,   GSS_IGNORE  },        /* PHOTO_SW _BOLT  */
    {32*1+18,   GSS_OUTPUT_LOW  },    /* NULL  */
    {32*1+19,   GSS_INPUT_NOPULL  },   /* BROKEN_ALARM  */
    {32*1+20,   GSS_OUTPUT_HIGH  },    /* LED_D4  */
    {32*1+21,   GSS_OUTPUT_LOW  },    /* MOTO_DRIVER_EN  */
    {32*1+22,   GSS_OUTPUT_HIGH  },    /* GAUG_PW R_EN  */
    {32*1+23,   GSS_OUTPUT_LOW  },    /* SENSOR_RST  */
    {32*1+24,   GSS_OUTPUT_HIGH  },   /* LED_D12  */
    {32*1+25,   GSS_OUTPUT_LOW  },    /* NULL  */
    {32*1+26,   GSS_INPUT_PULL  },    /* CLK32K  */
    {32*1+27,   GSS_OUTPUT_LOW  },    /* NULL  */
    {32*1+28,   GSS_INPUT_NOPULL},    /* BOOT_SEL0 */
    {32*1+29,   GSS_INPUT_NOPULL},    /* BOOT_SEL1 */
    {32*1+30,   GSS_INPUT_NOPULL},    /* BOOT_SEL2 */
    {32*1+31,   GSS_INPUT_NOPULL},    /* WKUP */

    /* GPIO Group - C */
    {32*2+0,    GSS_INPUT_NOPULL  },   /* MSC1_CLK */
    {32*2+1,    GSS_INPUT_NOPULL},     /* MSC1_CMD */
    {32*2+2,    GSS_INPUT_NOPULL},     /* MSC1_D0 */
    {32*2+3,    GSS_INPUT_NOPULL},     /* MSC1_D1 */
    {32*2+4,    GSS_INPUT_NOPULL},     /* MSC1_D2 */
    {32*2+5,    GSS_INPUT_NOPULL},     /* MSC1_D3 */
    {32*2+6,    GSS_INPUT_PULL  },     /* PCM_CLK */
    {32*2+7,    GSS_INPUT_PULL  },     /* PCM_DO */
    {32*2+8,    GSS_INPUT_PULL  },     /* PCM_DI */
    {32*2+9,    GSS_INPUT_PULL  },     /* PCM_SYN */
    {32*2+10,   GSS_INPUT_PULL  },     /* UART0_RXD */
    {32*2+11,   GSS_INPUT_PULL  },     /* UART0_TXD */
    {32*2+12,   GSS_INPUT_PULL  },     /* UART0_CTS_N */
    {32*2+13,   GSS_INPUT_PULL  },     /* UART0_RTS_N */
    {32*2+16,   GSS_INPUT_PULL  },     /* WL_WAKE_HOST */
    {32*2+17,   GSS_OUTPUT_LOW  },     /* WL_REG_EN */
    {32*2+18,   GSS_OUTPUT_LOW  },     /* BT_REG_EN */
    {32*2+19,   GSS_INPUT_PULL  },     /* HOST_WAKE_BT */
    {32*2+20,   GSS_INPUT_PULL  },     /* BT_WAKE_HOST */
    {32*2+21,   GSS_OUTPUT_LOW  },     /* BAT_GUAGE */
    {32*2+22,   GSS_OUTPUT_LOW  },     /* SEN_IRQ */
    {32*2+23,   GSS_INPUT_PULL  },     /* W L_W AKEUP_MCU */
    {32*2+24,   GSS_OUTPUT_LOW  },     /* PW M4 */
#ifdef CONFIG_JZ_SFC_FLASH_POWER_CTRL
    {32*2+25,   GSS_OUTPUT_HIGH  },     /* FLASH_PW EN */
#else
    {32*2+25,   GSS_OUTPUT_LOW  },     /* FLASH_PW EN */
#endif
    {32*2+26,   GSS_OUTPUT_LOW  },     /* PW M1 */
    {32*2+27,   GSS_OUTPUT_HIGH  },     /* PHOTO_SW _PW R_EN */
    {32*2+28,   GSS_OUTPUT_LOW  },     /* SMB1_SCk */
    {32*2+29,   GSS_OUTPUT_LOW  },     /* SMB1_SCk */
    {32*2+30,   GSS_OUTPUT_LOW  },     /* SMB1_SCk */
    {32*2+31,   GSS_INPUT_PULL  },     /* JTAG/UART2 */

    /*GPIO Group -D */
    {32*3+0,    GSS_INPUT_NOPULL  },     /* SSI0_CLK/SMB2_SCK    */
    {32*3+1,    GSS_INPUT_NOPULL  },     /* SSI0_CE0/SMB2_SDA    */
    {32*3+2,    GSS_OUTPUT_LOW  },     /* SSI0_DT/UART1_RXD */
    {32*3+3,    GSS_OUTPUT_LOW  },     /* SSI0_DR/UART1_TXD */
    {32*3+4,    GSS_OUTPUT_LOW },     /* FIRE_ALARM_N */
    {32*3+5,    GSS_OUTPUT_LOW  },     /* FPC_AEC_RST */

    /* GPIO Group Set End */
    {GSS_TABLET_END,GSS_TABLET_END  }
};

