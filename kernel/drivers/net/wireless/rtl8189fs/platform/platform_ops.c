/******************************************************************************
 *
 * Copyright(c) 2013 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

extern int jzmmc_manual_detect(int index, int on);
extern void mmc1_gpio_set_io(void);
extern void mmc1_gpio_restore_io(void);
extern void jzmmc_rescan_card(unsigned int id, int insert);

void sif_platform_target_poweron(void);
void sif_platform_target_poweroff(void);

#ifndef CONFIG_PLATFORM_OPS
/*
 * Return:
 *	0:	power on successfully
 *	others: power on failed
 */
int platform_wifi_power_on(void)
{
	int ret = 0;
	sif_platform_target_poweron();
    jzmmc_manual_detect(0, 1);


	return ret;
}

void platform_wifi_power_off(void)
{
    jzmmc_manual_detect(0, 0);
	sif_platform_target_poweroff();

}
#endif /* !CONFIG_PLATFORM_OPS */
