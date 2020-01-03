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
    {32*0+0,    GSS_OUTPUT_LOW   },    /* SLCD_D0 */
    {32*0+1,    GSS_OUTPUT_LOW   },    /* SLCD_D1 */
    {32*0+2,    GSS_OUTPUT_LOW   },    /* SLCD_D2 */
    {32*0+3,    GSS_OUTPUT_LOW   },    /* SLCD_D3 */
    {32*0+4,    GSS_OUTPUT_LOW   },    /* SLCD_D4 */
    {32*0+5,    GSS_OUTPUT_LOW   },    /* SLCD_D5 */
    {32*0+6,    GSS_OUTPUT_LOW   },    /* SLCD_D6 */
    {32*0+7,    GSS_OUTPUT_LOW   },    /* SLCD_D7 */
    {32*0+8,    GSS_OUTPUT_LOW   },    /* CIM_PCLK */
    {32*0+9,    GSS_OUTPUT_LOW   },    /* CIM_HSYN */
    {32*0+10,   GSS_OUTPUT_LOW   },    /* CIM_VSYN */
    {32*0+11,   GSS_OUTPUT_LOW   },    /* CIM_MCLK */
    {32*0+12,   GSS_OUTPUT_LOW   },    /* CIM_D7 */
    {32*0+13,   GSS_OUTPUT_LOW   },    /* CIM_D6 */
    {32*0+14,   GSS_OUTPUT_LOW   },    /* CIM_D5 */
    {32*0+15,   GSS_OUTPUT_LOW   },    /* CIM_D4 */
    {32*0+16,   GSS_OUTPUT_LOW   },    /* CIM_D3 */
    {32*0+17,   GSS_OUTPUT_LOW   },    /* CIM_D2 */
    {32*0+18,   GSS_OUTPUT_LOW   },    /* CIM_D1 */
    {32*0+19,   GSS_OUTPUT_LOW   },    /* CIM_D0 */
    {32*0+20,   GSS_OUTPUT_LOW  },    /* CIM_PWDN */
    {32*0+21,   GSS_OUTPUT_LOW   },    /* CIM_RST */
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
    {32*1+0,    GSS_OUTPUT_LOW   },    /* SLCD_BACKLIGHT */
    {32*1+1,    GSS_OUTPUT_HIGH  },    /* SLCD_VDD_EN */
    {32*1+2,    GSS_OUTPUT_LOW   },    /* PHY_PWR_EN */
    {32*1+3,    GSS_INPUT_PULL   },    /* USB_DETECT */
    {32*1+4,    GSS_OUTPUT_LOW   },    /* MAC_RST_N */
    {32*1+5,    GSS_OUTPUT_LOW   },    /* CPU_EN_QRLED */
    {32*1+6,    GSS_OUTPUT_LOW   },    /* MAC_PHY_CLK */
    {32*1+7,    GSS_OUTPUT_LOW   },    /* MAC_CRS_DV */
    {32*1+8,    GSS_OUTPUT_LOW   },    /* MAC_RXD1*/
    {32*1+9,    GSS_OUTPUT_LOW   },    /* MAC_RXD0 */
    {32*1+10,   GSS_OUTPUT_LOW   },    /* MAC_TXEN */
    {32*1+11,   GSS_OUTPUT_LOW   },    /* MAC_TXD1 */
    {32*1+12,   GSS_OUTPUT_LOW   },    /* MAC_TXD0 */
    {32*1+13,   GSS_OUTPUT_LOW   },    /* MAC_MDC */
    {32*1+14,   GSS_OUTPUT_LOW   },    /* MAC_MDIO */
    {32*1+15,   GSS_OUTPUT_LOW   },    /* MAC_REF_CLK */
    {32*1+16,   GSS_OUTPUT_HIGH   },    /* SLCD_RD */
    {32*1+17,   GSS_OUTPUT_LOW   },    /* SLCD_WR */
    {32*1+18,   GSS_OUTPUT_LOW   },    /* SLCD_CE */
    {32*1+19,   GSS_OUTPUT_HIGH   },    /* SLCD_TE */
    {32*1+20,   GSS_OUTPUT_LOW   },    /* SLCD_DC */
    {32*1+21,   GSS_OUTPUT_LOW   },    /* FUEL_GAUGE */
    {32*1+22,   GSS_INPUT_PULL       },    /* MCU_INT_CPU */
    {32*1+23,   GSS_OUTPUT_LOW   },    /* SMB0_SCK */
    {32*1+24,   GSS_OUTPUT_LOW   },    /* SMB0_SDA */
    {32*1+25,   GSS_OUTPUT_LOW   },    /* DRVVBUS */
    {32*1+26,   GSS_INPUT_PULL },    /* CHR_FINISH */
    {32*1+27,   GSS_OUTPUT_LOW   },    /* EXCLK */
    {32*1+28,   GSS_INPUT_PULL },    /* BOOT_SEL0 */
    {32*1+29,   GSS_INPUT_PULL },    /* BOOT_SEL1 */
    {32*1+30,   GSS_INPUT_PULL },    /* BOOT_SEL2 */
    {32*1+31,   GSS_INPUT_PULL },    /* WKUP */

    /* GPIO Group - C */
    {32*2+0,    GSS_IGNORE   },    /* MSC1_CLK */
    {32*2+1,    GSS_IGNORE },    /* MSC1_CMD */
    {32*2+2,    GSS_IGNORE },    /* MSC1_D0 */
    {32*2+3,    GSS_IGNORE },    /* MSC1_D1 */
    {32*2+4,    GSS_IGNORE },    /* MSC1_D2 */
    {32*2+5,    GSS_IGNORE },    /* MSC1_D3 */
    {32*2+6,    GSS_OUTPUT_LOW       },    /* MCU_BUSY_OUT */
    {32*2+7,    GSS_OUTPUT_LOW       },    /* CPU_RST_MCU */
    {32*2+8,    GSS_OUTPUT_LOW   },    /* NULL */
    {32*2+9,    GSS_OUTPUT_LOW   },    /* CPU_INT_MCU */
    {32*2+10,   GSS_OUTPUT_LOW   },    /* UART0_RXD */
    {32*2+11,   GSS_OUTPUT_LOW   },    /* UART0_TXD */
    {32*2+12,   GSS_OUTPUT_LOW   },    /* 2G_STATUS */
    {32*2+13,   GSS_OUTPUT_LOW       },    /* CPU_PWRON_2G */
    {32*2+16,   GSS_IGNORE       },    /* WL CHIP_EN */
    {32*2+17,   GSS_IGNORE       },    /* WL_IOPWR_EN */
    {32*2+18,   GSS_OUTPUT_HIGH  },    /* CIM_PWR_EN */
    {32*2+19,   GSS_IGNORE       },    /* WL_PWR_EN */
    {32*2+20,   GSS_OUTPUT_LOW       },    /* 2G_PWR_ON */
    {32*2+21,   GSS_IGNORE       },    /* WL_WAKEUP_CPU */
    {32*2+22,   GSS_INPUT_PULL       },    /* CPU_WAKEUP_2G */
    {32*2+23,   GSS_OUTPUT_LOW       },    /* CPU_RST_RST */
    {32*2+24,   GSS_OUTPUT_HIGH  },    /* MH_PWR_EN */
    {32*2+25,   GSS_OUTPUT_LOW   },    /* PWM0 */
    {32*2+26,   GSS_OUTPUT_LOW   },    /* SHUTDOWN_N */
    {32*2+27,   GSS_OUTPUT_LOW   },    /* NULL */
    {32*2+28,   GSS_INPUT_PULL   },    /* NULL */
    {32*2+29,   GSS_INPUT_PULL   },    /* NULL */
    {32*2+30,   GSS_IGNORE       },    /* TDI/UART2_RXD */
    {32*2+31,   GSS_IGNORE       },    /* TDO/UART2_TXD */

    /*GPIO Group -D */
    {32*3+0,    GSS_OUTPUT_LOW },    /* LED_DATA */
    {32*3+1,    GSS_OUTPUT_LOW },    /* CPU_EN_PWRB2 */
    {32*3+2,    GSS_OUTPUT_LOW       },    /* UART1_RXD */
    {32*3+3,    GSS_OUTPUT_LOW       },    /* UART1_TXD */
    {32*3+4,    GSS_OUTPUT_LOW  },    /* LED_STB */
    {32*3+5,    GSS_OUTPUT_LOW  },    /* LED_CLK */
    /* GPIO Group Set End */
    {GSS_TABLET_END,GSS_TABLET_END  }
};

