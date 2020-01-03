/*
 *  Copyright (C) 2016, Wang Qiuwei <qiuwei.wang@ingenic.com, panddio@163.com>
 *
 *  Ingenic Linux plarform SDK project
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/spi/spidev.h>

#include <gpio/gpio_manager.h>
#include <spi/spi_manager.h>
#include <mh1902_manager.h>


#define LOG_TAG    "mh1902"

static struct gpio_manager *gpio;
static struct spi_manager *spi;

static inline void mh1902_set_cs(unsigned char sta)
{
    gpio->set_value(MCU_CS_PIN, !!sta);
}

int mh1902_spi_read(unsigned char *rxbuf, unsigned int count)
{
    int i;

    for(i = 0; i < count; i++, rxbuf++) {
        mh1902_set_cs(0);
        if (spi->read(SPI_DEV0, rxbuf, 1) < 0) {
            mh1902_set_cs(1);
            return -1;
        }
        mh1902_set_cs(1);
        usleep(1000);
    }

    return 0;
}

int mh1902_spi_write(unsigned char *txbuf, unsigned int count)
{
    int i;

    for (i = 0; i < count; i++, txbuf++) {
        mh1902_set_cs(0);
        if (spi->write(SPI_DEV0, txbuf, 1) < 0) {
            mh1902_set_cs(1);
            return -1;
        }
        mh1902_set_cs(1);
        usleep(1000);
    }

    return 0;
}

int mh1902_spi_transfer(unsigned char *txbuf, unsigned char *rxbuf, unsigned int len)
{
    int i;

    for (i = 0; i < len; i++, txbuf++, rxbuf++) {
        mh1902_set_cs(0);
        //if (spi->transfer(SPI_DEV0, txbuf, rxbuf, 1) < 0) {
        if (spi->transfer(SPI_DEV0, txbuf, 1, rxbuf, 1) < 0) {
            mh1902_set_cs(1);
            return -1;
        }
        mh1902_set_cs(1);
        usleep(1000);
    }
}

int mh1902_spi_init(uint8_t mode, uint8_t bits, uint32_t speed)
{
    spi = get_spi_manager();
    return spi->init(SPI_DEV0, mode, bits, speed);
}

void mh1902_spi_deinit(void)
{
    spi->deinit(SPI_DEV0);
}

int mh1902_gpio_init(void)
{
    gpio = get_gpio_manager();
    if (gpio->init() < 0)
        return -1;

    if (gpio->open(MCU_CS_PIN) < 0)
        goto err1;
    if (gpio->set_direction(MCU_CS_PIN, GPIO_OUT) < 0)
        goto err2;
    if (gpio->set_value(MCU_CS_PIN, GPIO_HIGH) < 0)
        goto err2;

    if (gpio->open(MCU_BUSY_OUT) < 0)
        goto err2;
    if (gpio->set_direction(MCU_BUSY_OUT, GPIO_IN) < 0)
        goto err3;

    if (gpio->open(CPU_INT_MCU) < 0)
        goto err3;
    if (gpio->set_direction(CPU_INT_MCU, GPIO_OUT) < 0)
        goto err4;
    if (gpio->set_value(CPU_INT_MCU, 1) < 0)
        goto err4;

    return 0;

err4:
    gpio->close(CPU_INT_MCU);
err3:
    gpio->close(MCU_BUSY_OUT);
err2:
    gpio->close(MCU_CS_PIN);
err1:
    gpio->deinit();
    return -1;
}

void mh1902_gpio_deinit(void)
{
    gpio->close(MCU_CS_PIN);
    gpio->close(MCU_BUSY_OUT);
    gpio->close(CPU_INT_MCU);
    gpio->deinit();
}
