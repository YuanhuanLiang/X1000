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
    {GSS_TABLET_END,GSS_TABLET_END  },
};
__initdata int gpio_ss_table[][2] = {
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
    {32*0+10,   GSS_INPUT_PULL  },  /* AMPLIFIER ENABLE */
    {32*0+11,   GSS_INPUT_PULL  },  /* LCD RST */
    {32*0+12,   GSS_INPUT_PULL  },  /* NC */
    {32*0+13,   GSS_INPUT_PULL  },  /* NC */
    {32*0+14,   GSS_INPUT_PULL  },  /* HEADPHONE MUTE */
    {32*0+15,   GSS_INPUT_PULL  },  /* NC */
    {32*0+16,   GSS_INPUT_PULL  },  /* pmu irq */
    {32*0+17,   GSS_INPUT_PULL  },  /* NC */
    {32*0+18,   GSS_INPUT_PULL  },  /* NC */
    {32*0+19,   GSS_INPUT_PULL  },  /* mute */
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
    #if 1
    {32*1+0,    GSS_INPUT_PULL  },      /* I2S_MCLK */
    {32*1+1,    GSS_INPUT_PULL  },      /* I2S_BCLK */
    {32*1+2,    GSS_INPUT_PULL  },      /* I2S_LRCLK */
    {32*1+3,    GSS_INPUT_PULL  },      /* NC */
    {32*1+4,    GSS_INPUT_PULL  },      /* I2S_DO */
    #endif
    {32*1+5,    GSS_INPUT_PULL  },  /* NC */
    {32*1+6,    GSS_INPUT_PULL  },  /* NC */
    {32*1+7,    GSS_INPUT_PULL  },  /* NC */
    {32*1+8,    GSS_INPUT_PULL},  /* USB-DET */
    {32*1+9,    GSS_INPUT_PULL  },  /* NC */
    {32*1+10,   GSS_INPUT_PULL  },  /* NC */
    {32*1+11,   GSS_INPUT_PULL},  /* sda */
    {32*1+12,   GSS_INPUT_PULL},  /* scl */
    {32*1+13,   GSS_INPUT_PULL  },  /* NC */
    {32*1+14,   GSS_INPUT_PULL  },  /* priv key */          //ignore
    {32*1+15,   GSS_INPUT_PULL  },  /* next key */          //ignore
    {32*1+16,   GSS_INPUT_PULL  },  /* SLCD_RD */
    {32*1+17,   GSS_INPUT_PULL  },  /* SLCD_WR */
    {32*1+18,   GSS_INPUT_PULL  },  /* SLCD_CS */
    {32*1+19,   GSS_INPUT_PULL  },  /* NC */
    {32*1+20,   GSS_INPUT_PULL  },  /* SLCD_DC */
    {32*1+21,   GSS_INPUT_PULL  },  /* NC */
    {32*1+22,   GSS_INPUT_PULL  },  /* NC */
    {32*1+23,   GSS_INPUT_PULL  },  /* tf enable */
    {32*1+24,   GSS_INPUT_PULL  },  /* NC */
    {32*1+25,   GSS_INPUT_PULL  },  /* NC */
    {32*1+26,   GSS_INPUT_PULL  },/* CLK32K */
    {32*1+27,   GSS_INPUT_PULL  },  /* efuse en */
    {32*1+28,   GSS_INPUT_PULL    },  /* BOOT_SEL0 */
    {32*1+29,   GSS_INPUT_PULL    },  /* BOOT_SEL1 */
    {32*1+30,   GSS_INPUT_PULL    },  /* BOOT_SEL2 */
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
    {32*3+3,    GSS_INPUT_PULL  },  /* DAC-MUTE */
    {32*3+4,    GSS_INPUT_PULL  },  /* NC */
    {32*3+5,    GSS_INPUT_PULL  },  /* NC */
    {GSS_TABLET_END,GSS_TABLET_END	}	/* GPIO Group Set End */
};
