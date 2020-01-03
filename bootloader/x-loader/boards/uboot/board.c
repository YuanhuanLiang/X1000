/*
 *  Copyright (C) 2016 Ingenic Semiconductor Co.,Ltd
 *
 *  X1000 series bootloader for u-boot/rtos/linux
 *
 *  Zhang YanMing <yanming.zhang@ingenic.com, jamincheung@126.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <common.h>

void board_early_init(void) {
#ifdef CONFIG_JZ_CIM_CORE
	gpio_set_func(GPIO_PA(8), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(9), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(10), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(11), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(12), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(13), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(14), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(15), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(16), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(17), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(18), GPIO_FUNC_2);
	gpio_set_func(GPIO_PA(19), GPIO_FUNC_2);
#endif
}

void board_init(void) {
#ifdef CONFIG_RTCCLK_SEL
    rtc_change_sel();
#endif
}
