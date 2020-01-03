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
    {32*0+0,    GSS_OUTPUT_LOW   },    /* SLCD_D0_NC */
    {32*0+1,    GSS_OUTPUT_LOW   },    /* SLCD_D1_NC */
    {32*0+2,    GSS_OUTPUT_LOW   },    /* SLCD_D2_NC */
    {32*0+3,    GSS_OUTPUT_LOW   },    /* SLCD_D3_NC */
    {32*0+4,    GSS_OUTPUT_LOW   },    /* SLCD_D4_NC */
    {32*0+5,    GSS_OUTPUT_LOW   },    /* MOTO_DIRECTION */
    {32*0+6,    GSS_OUTPUT_LOW   },    /* SLCD_D6_NC */
    {32*0+7,    GSS_OUTPUT_LOW   },    /* MOTO_DRIVER_EN */
    {32*0+8,    GSS_INPUT_NOPULL },    /* LOCK_SWS */
    {32*0+9,    GSS_INPUT_NOPULL },    /* LOCK_SWK */
    {32*0+10,   GSS_INPUT_NOPULL },    /* FP_PWR_EN */
    {32*0+11,   GSS_INPUT_NOPULL },    /* LOCK_SWF */
    {32*0+12,   GSS_OUTPUT_LOW   },    /* CIM_D7_NC */
    {32*0+13,   GSS_OUTPUT_LOW   },    /* CIM_D6_NC */
    {32*0+14,   GSS_OUTPUT_LOW   },    /* CIM_D5_NC */
    {32*0+15,   GSS_OUTPUT_LOW   },    /* CIM_D4_NC */

    {32*0+16,   GSS_OUTPUT_LOW   },    /* CIM_D3_NC */
    {32*0+17,   GSS_OUTPUT_LOW   },    /* CIM_D2_NC */
    {32*0+18,   GSS_OUTPUT_LOW   },    /* CIM_D1_NC */
    {32*0+19,   GSS_OUTPUT_LOW   },    /* CIM_D0_NC */
    {32*0+20,   GSS_OUTPUT_LOW   },    /* FP_INT_CPU */
    {32*0+21,   GSS_OUTPUT_LOW   },    /* SSIO_CE1 */
    {32*0+22,   GSS_OUTPUT_LOW   },    /* SSIO_DT */
    {32*0+23,   GSS_OUTPUT_LOW   },    /* SSIO_DR */
    {32*0+24,   GSS_OUTPUT_LOW   },    /* SSIO_CLK */
    {32*0+25,   GSS_OUTPUT_LOW   },    /* SSIO_CE0 */
#ifdef CONFIG_JZ_SFC_FLASH_POWER_CTRL
    {32*0+26,   GSS_OUTPUT_LOW   },    /* SFC_CLK */
    {32*0+27,   GSS_OUTPUT_LOW   },    /* SFC_CE */
    {32*0+28,   GSS_OUTPUT_LOW   },    /* SFC_DR */
    {32*0+29,   GSS_OUTPUT_LOW   },    /* SFC_DT */
    {32*0+30,   GSS_OUTPUT_LOW   },    /* SFC_WP */
    {32*0+31,   GSS_OUTPUT_LOW   },    /* SFC_HOLD */
#else
    {32*0+26,   GSS_INPUT_PULL   },    /* SFC_CLK */
    {32*0+27,   GSS_INPUT_PULL   },    /* SFC_CE */
    {32*0+28,   GSS_INPUT_PULL   },    /* SFC_DR */
    {32*0+29,   GSS_INPUT_PULL   },    /* SFC_DT */
    {32*0+30,   GSS_INPUT_PULL   },    /* SFC_WP */
    {32*0+31,   GSS_INPUT_PULL   },    /* SFC_HOLD */
#endif

    /* GPIO Group - B */
    {32*1+0,    GSS_OUTPUT_LOW   },    /* BORKEN_ALARM */
    {32*1+1,    GSS_OUTPUT_HIGH  },    /* KEYB_BL_EN */
    {32*1+2,    GSS_OUTPUT_HIGH  },    /* FP_LED_G */
    {32*1+3,    GSS_OUTPUT_HIGH  },    /* FP_LED_B */
    {32*1+4,    GSS_OUTPUT_HIGH  },    /* FP_LED_R */
    {32*1+5,    GSS_OUTPUT_LOW   },    /* SHUTDOWN_N */
    {32*1+6,    GSS_OUTPUT_LOW   },    /* MAC_NC */
    {32*1+7,    GSS_OUTPUT_LOW   },    /* MAC_NC */
    {32*1+8,    GSS_OUTPUT_LOW   },    /* MAC_NC*/
    {32*1+9,    GSS_OUTPUT_LOW   },    /* MAC_NC */
    {32*1+10,   GSS_OUTPUT_LOW   },    /* MAC_NC */
    {32*1+11,   GSS_OUTPUT_LOW   },    /* MAC_NC */
    {32*1+12,   GSS_OUTPUT_LOW   },    /* MAC_NC */
    {32*1+13,   GSS_OUTPUT_LOW   },    /* MAC_NC */
    {32*1+14,   GSS_OUTPUT_LOW   },    /* MAC_NC */
    {32*1+15,   GSS_OUTPUT_LOW   },    /* MAC_NC */

    {32*1+16,   GSS_OUTPUT_LOW   },    /* SLCD_RD_NC */
    {32*1+17,   GSS_OUTPUT_LOW   },    /* SLCD_WR_NC */
    {32*1+18,   GSS_OUTPUT_LOW   },    /* SLCD_CE_NC */
    {32*1+19,   GSS_OUTPUT_LOW   },    /* SLCD_TE_NC */
    {32*1+20,   GSS_OUTPUT_LOW   },    /* SLCD_DC_NC */
    {32*1+21,   GSS_OUTPUT_LOW   },    /* DMIC_CLK_NC */
    {32*1+22,   GSS_OUTPUT_LOW   },    /* DMIC0_IN_NC */
    {32*1+23,   GSS_OUTPUT_LOW   },    /* SMB0_SCK_NC */
    {32*1+24,   GSS_OUTPUT_LOW   },    /* SMB0_SDA_NC */
    {32*1+25,   GSS_OUTPUT_LOW   },    /* DRVVBUS_NC */
    {32*1+26,   GSS_OUTPUT_LOW   },    /* CLK32K_NC */
    {32*1+27,   GSS_INPUT_PULL   },    /* EXCLK_NC */
    {32*1+28,   GSS_INPUT_NOPULL },    /* BOOT_SEL0 */
    {32*1+29,   GSS_INPUT_NOPULL },    /* BOOT_SEL1 */
    {32*1+30,   GSS_INPUT_NOPULL },    /* BOOT_SEL2 */
    {32*1+31,   GSS_INPUT_NOPULL },    /* WKUP */

    /* GPIO Group - C */
    {32*2+0,    GSS_OUTPUT_LOW   },    /* MSC1_CLK_NC */
    {32*2+1,    GSS_OUTPUT_LOW   },    /* MSC1_CMD_NC */
    {32*2+2,    GSS_OUTPUT_LOW   },    /* MSC1_D0_NC */
    {32*2+3,    GSS_OUTPUT_LOW   },    /* MSC1_D1_NC */
    {32*2+4,    GSS_OUTPUT_LOW   },    /* MSC1_D2_NC */
    {32*2+5,    GSS_OUTPUT_LOW   },    /* MSC1_D3_NC */
    {32*2+6,    GSS_OUTPUT_LOW   },    /* PCM_CLK_NC */
    {32*2+7,    GSS_OUTPUT_LOW   },    /* PCM_DO_NC */
    {32*2+8,    GSS_OUTPUT_LOW   },    /* PCM_DI_NC */
    {32*2+9,    GSS_OUTPUT_LOW   },    /* PCM_SYN_NC */
    {32*2+10,   GSS_OUTPUT_LOW   },    /* UART0_RXD_NC */
    {32*2+11,   GSS_OUTPUT_LOW   },    /* UART0_TXD_NC */
    {32*2+12,   GSS_OUTPUT_LOW   },    /* UART0_CTS_N_NC */
    {32*2+13,   GSS_OUTPUT_LOW   },    /* UART0_RTS_N_NC */

    {32*2+16,   GSS_OUTPUT_LOW   },    /* CPU_PD_CARD */
    {32*2+17,   GSS_OUTPUT_LOW   },    /* CARD_INT_CPU */
    {32*2+18,   GSS_OUTPUT_LOW   },    /* CPU_RST_CARD */
    {32*2+19,   GSS_OUTPUT_LOW   },    /* NC */
    {32*2+20,   GSS_OUTPUT_LOW   },    /* FP_WAKE_CPU */
    {32*2+21,   GSS_OUTPUT_LOW   },    /* BAT_GUAGE */
    {32*2+22,   GSS_OUTPUT_HIGH  },    /* FG_PW EN */
    {32*2+23,   GSS_OUTPUT_LOW   },    /* NC */
    {32*2+24,   GSS_OUTPUT_LOW   },    /* PWM4 */
#ifdef CONFIG_JZ_SFC_FLASH_POWER_CTRL
    {32*2+25,   GSS_OUTPUT_HIGH  },    /* FLASH_PWEN */
#else
    {32*2+25,   GSS_OUTPUT_LOW   },    /* FLASH_PWEN */
#endif
    {32*2+26,   GSS_OUTPUT_LOW   },    /* SMB1_SCK/PWM1_NC */
    {32*2+27,   GSS_OUTPUT_LOW   },    /* SMB1_SDA/PWM2_NC */
    {32*2+28,   GSS_INPUT_PULL   },    /* NC */
    {32*2+29,   GSS_INPUT_PULL   },    /* NC */
    {32*2+30,   GSS_IGNORE       },    /* TDI/UART2_RXD */
    {32*2+31,   GSS_IGNORE       },    /* TDO/UART2_TXD */

    /*GPIO Group -D */
    {32*3+0,    GSS_INPUT_PULL   },    /* SSI0_CLK/SMB2_SCK*/
    {32*3+1,    GSS_INPUT_PULL   },    /* SSI0_CE0/SMB2_SDA*/
    {32*3+2,    GSS_INPUT_PULL   },    /* CPU_RST_TOUCH */
    {32*3+3,    GSS_IGNORE       },    /* TOUCH_INT_CPU */
    {32*3+4,    GSS_IGNORE       },    /* CPU_INT_TOUCH */
    {32*3+5,    GSS_OUTPUT_LOW   },    /* NC */
    /* GPIO Group Set End */
    {GSS_TABLET_END,GSS_TABLET_END  }
};

