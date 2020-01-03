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
	{GSS_TABLET_END,GSS_TABLET_END	},
};

__initdata int gpio_ss_table[][2] = {
	/* GPIO Group - A */
	{32*0+0,    GSS_INPUT_PULL},	/* NC */
	{32*0+1,    GSS_INPUT_PULL},	/* NC */
	{32*0+2,    GSS_INPUT_PULL},	/* NC */
	{32*0+3,    GSS_INPUT_PULL},	/* NC */
	{32*0+4,    GSS_INPUT_PULL},	/* NC */
	{32*0+5,    GSS_INPUT_PULL},	/* NC */
	{32*0+6,    GSS_INPUT_PULL},	/* NC */
	{32*0+7,    GSS_INPUT_PULL},	/* NC */
	{32*0+8,    GSS_INPUT_PULL},	/* CIM_PCLK */
	{32*0+9,    GSS_INPUT_PULL},	/* CIM_HSYN */
	{32*0+10,   GSS_INPUT_PULL},	/* CIM_VSYN */
	{32*0+11,   GSS_INPUT_PULL},	/* CIM_MCLK */
	{32*0+12,   GSS_INPUT_PULL},	/* CIM_D7 */
	{32*0+13,   GSS_INPUT_PULL},	/* CIM_D6 */
	{32*0+14,   GSS_INPUT_PULL},	/* CIM_D5 */
	{32*0+15,   GSS_INPUT_PULL},	/* CIM_D4 */
	{32*0+16,   GSS_INPUT_PULL},	/* CIM_D3 */
	{32*0+17,   GSS_INPUT_PULL},	/* CIM_D2 */
	{32*0+18,   GSS_INPUT_PULL},	/* CIM_D1 */
	{32*0+19,   GSS_INPUT_PULL},	/* CIM_D0 */
	{32*0+20,   GSS_INPUT_PULL},	/* NC */
	{32*0+21,   GSS_INPUT_PULL},	/* NC */
	{32*0+22,   GSS_INPUT_PULL},	/* NC */
	{32*0+23,   GSS_INPUT_PULL},	/* NC */
	{32*0+24,   GSS_INPUT_PULL},	/* NC */
	{32*0+25,   GSS_INPUT_PULL},	/* NC */
	{32*0+26,   GSS_INPUT_PULL},    /* SFC_CLK */
	{32*0+27,   GSS_INPUT_PULL},    /* SFC_CE */
	{32*0+28,   GSS_INPUT_PULL},    /* SFC_DR */
	{32*0+29,   GSS_INPUT_PULL},    /* SFC_DT */
	{32*0+30,   GSS_INPUT_PULL},    /* SFC_WP */
	{32*0+31,   GSS_INPUT_PULL},    /* SFC_HOLD */

	/* GPIO Group - B */
	{32*1+0,    GSS_INPUT_PULL},	/* NC */
	{32*1+1,    GSS_INPUT_PULL},	/* NC */
	{32*1+2,    GSS_INPUT_PULL},	/* NC */
	{32*1+3,    GSS_INPUT_PULL},	/* NC */
	{32*1+4,    GSS_INPUT_PULL},	/* NC */
	{32*1+5,    GSS_INPUT_PULL},	/* NC */
	{32*1+6,    GSS_INPUT_PULL},	/* NC */
	{32*1+7,    GSS_INPUT_PULL},	/* NC */
	{32*1+8,    GSS_INPUT_PULL},	/* EFUSE_EN*/
	{32*1+9,    GSS_INPUT_PULL},	/* NC */
	{32*1+10,   GSS_INPUT_PULL},	/* NC */
	{32*1+11,   GSS_INPUT_PULL},	/* NC */
	{32*1+12,   GSS_INPUT_PULL},	/* NC */
	{32*1+13,   GSS_INPUT_PULL},	/* NC */
	{32*1+14,   GSS_INPUT_PULL},	/* NC */
	{32*1+15,   GSS_INPUT_PULL},	/* NC */
	{32*1+16,   GSS_INPUT_PULL},	/* NC */
	{32*1+17,   GSS_INPUT_PULL},	/* NC */
	{32*1+18,   GSS_INPUT_PULL},	/* NC */
	{32*1+19,   GSS_INPUT_PULL},	/* NC */
	{32*1+20,   GSS_INPUT_PULL},	/* NC */
	{32*1+21,   GSS_INPUT_PULL},	/* NC */
	{32*1+22,   GSS_INPUT_PULL},	/* NC */
	{32*1+23,   GSS_INPUT_NOPULL},	/* SMB0_SDA */
	{32*1+24,   GSS_INPUT_NOPULL},  /* SMB0_SCK */
	{32*1+25,   GSS_INPUT_PULL},	/* NC */
	{32*1+26,   GSS_INPUT_PULL},	/* NC */
	{32*1+27,   GSS_INPUT_PULL},	/* NC */
	{32*1+28,   GSS_INPUT_NOPULL},  /* BOOT_SEL0 */
	{32*1+29,   GSS_INPUT_NOPULL},  /* BOOT_SEL1 */
	{32*1+30,   GSS_INPUT_NOPULL},  /* BOOT_SEL2 */
	{32*1+31,   GSS_INPUT_PULL  },  /* WKUP */

	/* GPIO Group - C */
	{32*2+0,    GSS_INPUT_PULL},	/* NC */
	{32*2+1,    GSS_INPUT_PULL},	/* NC */
	{32*2+2,    GSS_INPUT_PULL},	/* NC */
	{32*2+3,    GSS_INPUT_PULL},	/* NC */
	{32*2+4,    GSS_INPUT_PULL},	/* NC */
	{32*2+5,    GSS_INPUT_PULL},	/* NC */
	{32*2+6,    GSS_INPUT_PULL},	/* NC */
	{32*2+7,    GSS_INPUT_PULL},	/* NC */
	{32*2+8,    GSS_INPUT_PULL},	/* NC */
	{32*2+9,    GSS_INPUT_PULL},	/* NC */
	{32*2+10,   GSS_INPUT_PULL},    /* UART0_RXD */
	{32*2+11,   GSS_INPUT_PULL},    /* UART0_TXD */
	{32*2+12,   GSS_INPUT_PULL},  	/* NC */
	{32*2+13,   GSS_INPUT_PULL},  	/* NC */
	{32*2+16,   GSS_INPUT_PULL }, 	/* NC */
	{32*2+17,   GSS_INPUT_PULL},  	/* LED_Blue*/
	{32*2+18,   GSS_INPUT_PULL}, 	/* NC */
	{32*2+19,   GSS_INPUT_PULL},  	/* NC */
	{32*2+20,   GSS_INPUT_PULL},  	/* NC */
	{32*2+21,   GSS_INPUT_NOPULL},  /* OTG_ID */
	{32*2+22,   GSS_INPUT_NOPULL},  /* USB_DETECT */
	{32*2+23,   GSS_INPUT_PULL},  	/* NC */
	{32*2+24,   GSS_INPUT_PULL}, 	/* NC */
	{32*2+25,   GSS_INPUT_PULL}, 	/* NC */
	{32*2+26,   GSS_INPUT_NOPULL},  /* SMB1_SDA */
	{32*2+27,   GSS_INPUT_NOPULL},  /* SMB1_SCk */
	{32*2+31,   GSS_INPUT_PULL},    /* JTAG/UART2 */

	/*GPIO Group -D */
	{32*3+0,    GSS_INPUT_NOPULL}, 	/* SMB2_SCK*/
	{32*3+1,    GSS_INPUT_NOPULL}, 	/* SMB2_SDA*/
	{32*3+2,    GSS_INPUT_PULL  },  /* NC */
	{32*3+3,    GSS_INPUT_PULL  },  /* NC */
	{32*3+4,    GSS_INPUT_PULL  },  /* CIM_PWDN */
	{32*3+5,    GSS_INPUT_PULL  },  /* CIM_EN */

	/* GPIO Group Set End */
	{GSS_TABLET_END,GSS_TABLET_END	}
};
