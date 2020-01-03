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
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>

#include <types.h>
#include <utils/log.h>
#include <utils/assert.h>

#include <gpio/gpio_manager.h>
#include <uart/uart_manager.h>
#include <l718_manager.h>


#define L718_PWR_ON_PIN          GPIO_PC(20)
#define L718_WAKEUP_PIN          GPIO_PC(22)
#define L718_RESET_PIN           GPIO_PC(23)

#define L718_UART_NAME           "/dev/ttyS0"
#define L718_UART_BAUDRATE       115200
#define L718_UART_DATABITS       8
#define L718_UART_PARITY         UART_PARITY_NONE
#define L718_UART_STOPBITS       1
#define L718_UART_TRANS_TIMEOUT  1000 //ms

#define LOG_TAG    "l718"

static struct gpio_manager *gpio;
static struct uart_manager *uart;

int l718_gpio_init(void)
{
    gpio = get_gpio_manager();
    if (gpio->init() < 0) {
        LOGE("Failed to init gpio\n");
        return -1;
    }

    if (gpio->open(L718_PWR_ON_PIN) < 0)
        goto err1;
    if (gpio->set_direction(L718_PWR_ON_PIN, GPIO_OUT) < 0)
        goto err2;
    if (gpio->set_value(L718_PWR_ON_PIN, GPIO_LOW) < 0)
        goto err2;

    if (gpio->open(L718_RESET_PIN) < 0)
        goto err2;
    if (gpio->set_direction(L718_RESET_PIN, GPIO_IN) < 0)
        goto err3;

    if (gpio->open(L718_WAKEUP_PIN) < 0)
        goto err3;
    if (gpio->set_direction(L718_WAKEUP_PIN, GPIO_OUT) < 0)
        goto err4;
    if (gpio->set_value(L718_WAKEUP_PIN, GPIO_LOW) < 0)
        goto err4;

    return 0;

err4:
    gpio->close(L718_WAKEUP_PIN);
err3:
    gpio->close(L718_RESET_PIN);
err2:
    gpio->close(L718_PWR_ON_PIN);
err1:
    gpio->deinit();
    gpio = NULL;
    return -1;
}

void l718_gpio_deinit(void)
{
    gpio->close(L718_WAKEUP_PIN);
    gpio->close(L718_RESET_PIN);
    gpio->close(L718_PWR_ON_PIN);
    gpio->deinit();
    gpio = NULL;
}

int l718_uart_init(void)
{
    uart = get_uart_manager();
    if (uart->init(L718_UART_NAME,
                   L718_UART_BAUDRATE,
                   L718_UART_DATABITS,
                   L718_UART_PARITY,
                   L718_UART_STOPBITS) < 0) {
        LOGE("Failed to init %s\n", L718_UART_NAME);
        return -1;
    }
    return 0;
}

void l718_uart_deinit(void)
{
    uart->deinit(L718_UART_NAME);
}

int l718_uart_read(void *buf, uint32_t count)
{
    return uart->read(L718_UART_NAME, buf, count,
                L718_UART_TRANS_TIMEOUT);
}

int l718_uart_write(void *buf)
{
    return uart->write(L718_UART_NAME, buf, strlen(buf),
                L718_UART_TRANS_TIMEOUT);
}

int l718_power_on(void)
{
    char rxbuf[32];

    if (gpio->set_value(L718_PWR_ON_PIN, GPIO_HIGH) < 0)
        goto err_power_on;
    sleep(1);

    while(1) {
        bzero(rxbuf, sizeof(rxbuf));
        if (l718_uart_write("AT+IPR=115200\r\n") < 0) {
            LOGE("%s: uart failed to write\n", __FUNCTION__);
            goto err_power_on;
        }
        sleep(1);
        if (l718_uart_read(rxbuf, sizeof(rxbuf)) < 0) {
            LOGE("%s: uart failed to read\n", __FUNCTION__);
            goto err_power_on;
        }
        LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);
        if (strstr(rxbuf, "OK"))
            break;
        msleep(64);
    }

    LOGI("power on OK!!!\n");
    return 0;

err_power_on:
    gpio->set_value(L718_PWR_ON_PIN, GPIO_LOW);
    LOGE("L718 power failed\n");
    return -1;
}

void l718_power_off(void)
{
    gpio->set_value(L718_PWR_ON_PIN, GPIO_LOW);
    msleep(100);
}

int l718_wakeup(void)
{
    return 0;
}

int l718_reset(void)
{
    if (gpio->set_direction(L718_RESET_PIN, GPIO_OUT) < 0)
        return -1;
    if (gpio->set_value(L718_RESET_PIN, GPIO_LOW) < 0)
        return -1;
    msleep(100);
    if (gpio->set_value(L718_RESET_PIN, GPIO_HIGH) < 0)
        return -1;
    if (gpio->set_direction(L718_RESET_PIN, GPIO_IN) < 0)
        return -1;
    return 0;
}
