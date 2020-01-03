/* Copyright (c) 2008 -2014 Espressif System.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 *  sdio stub code template
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mmc/core.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>


extern int jzmmc_manual_detect(int index, int on);
extern void mmc1_gpio_set_io(void);
extern void mmc1_gpio_restore_io(void);
extern void jzmmc_rescan_card(unsigned int id, int insert);


void sif_platform_rescan_card(unsigned insert)
{
    jzmmc_manual_detect(0, insert);
}

void sif_platform_target_speed(int high_speed)
{

}

void sif_platform_check_r1_ready(struct esp_pub *epub)
{

}

#ifdef CONFIG_ESP_ACK_INTERRUPT
void sif_platform_ack_interrupt(struct esp_pub *epub)
{
    struct esp_sdio_ctrl *sctrl = NULL;
    struct sdio_func *func = NULL;

    sctrl = (struct esp_sdio_ctrl *)epub->sif;
    func = sctrl->func;
}
#endif /* CONFIG_ESP_ACK_INTERRUPT */

