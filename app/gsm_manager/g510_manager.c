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
#include <g510_manager.h>


#define G510_PWR_EN_PIN             GPIO_PC(20)
#define G510_PWR_EN_LEVEL           GPIO_HIGH
#define G510_STATUE_PIN             GPIO_PC(12)
#define G510_PWR_ON_PIN             GPIO_PC(13)
#define G510_WAKEUP_PIN             GPIO_PC(22)
#define G510_EMERG_RST_PIN          GPIO_PC(23)

#define G510_UART_NAME              "/dev/ttyS0"
#define G510_UART_BAUDRATE          115200
#define G510_UART_DATABITS          8
#define G510_UART_PARITY            UART_PARITY_NONE
#define G510_UART_STOPBITS          1
#define G510_UART_TRANS_TIMEIOUT    500 //ms

#define LOG_TAG    "g510"

static struct gpio_manager *gpio;
static struct uart_manager *uart;


int g510_gpio_init(void)
{
    gpio = get_gpio_manager();
    if (gpio->init() < 0) {
        LOGE("Failed to init gpio\n");
        return -1;
    }

    if (gpio->open(G510_PWR_EN_PIN) < 0)
        goto err1;
    if (gpio->set_direction(G510_PWR_EN_PIN, GPIO_OUT) < 0)
        goto err2;
    if (gpio->set_value(G510_PWR_EN_PIN, !G510_PWR_EN_LEVEL) < 0)
        goto err2;

    if (gpio->open(G510_PWR_ON_PIN) < 0)
        goto err2;
    if (gpio->set_direction(G510_PWR_ON_PIN, GPIO_OUT) < 0)
        goto err3;
    if (gpio->set_value(G510_PWR_ON_PIN, GPIO_HIGH) < 0)
        goto err3;

    if (gpio->open(G510_WAKEUP_PIN) < 0)
        goto err3;
    if (gpio->set_direction(G510_WAKEUP_PIN, GPIO_IN) < 0)
        goto err4;

    if (gpio->open(G510_EMERG_RST_PIN) < 0)
        goto err4;
    if (gpio->set_direction(G510_EMERG_RST_PIN, GPIO_OUT) < 0)
        goto err5;
    if (gpio->set_value(G510_EMERG_RST_PIN, GPIO_HIGH) < 0)
        goto err5;

    if (gpio->open(G510_STATUE_PIN) < 0)
        goto err5;
    if (gpio->set_direction(G510_STATUE_PIN, GPIO_IN) < 0)
        goto err6;

    return 0;

err6:
    gpio->close(G510_STATUE_PIN);
err5:
    gpio->close(G510_EMERG_RST_PIN);
err4:
    gpio->close(G510_WAKEUP_PIN);
err3:
    gpio->close(G510_PWR_ON_PIN);
err2:
    gpio->close(G510_PWR_EN_PIN);
err1:
    gpio->deinit();
    return -1;
}

void g510_gpio_deinit(void)
{
    //gpio->set_value(G510_PWR_EN_PIN, !G510_PWR_EN_LEVEL); // power off

    gpio->close(G510_STATUE_PIN);
    gpio->close(G510_EMERG_RST_PIN);
    gpio->close(G510_WAKEUP_PIN);
    gpio->close(G510_PWR_ON_PIN);
    gpio->close(G510_PWR_EN_PIN);
    gpio->deinit();
}

int g510_uart_init(void)
{
    uart = get_uart_manager();

    if (uart->init(G510_UART_NAME,
                   G510_UART_BAUDRATE,
                   G510_UART_DATABITS,
                   G510_UART_PARITY,
                   G510_UART_STOPBITS) < 0) {
        LOGE("Failed to init %s\n", G510_UART_NAME);
        return -1;
    }

    return 0;
}

void g510_uart_deinit(void)
{
    uart->deinit(G510_UART_NAME);
}

int g510_uart_read(void *buf, uint32_t count)
{
    return uart->read(G510_UART_NAME, buf, count,
                G510_UART_TRANS_TIMEIOUT);
}

int g510_uart_write(void *buf)
{
    return uart->write(G510_UART_NAME, buf, strlen(buf),
                G510_UART_TRANS_TIMEIOUT);
}

int g510_power_on(void)
{
    char rxbuf[32];
    int count = 0;

    if (gpio->set_value(G510_PWR_ON_PIN, GPIO_LOW) < 0)
        goto err_power_on;
    if (gpio->set_value(G510_PWR_EN_PIN, G510_PWR_EN_LEVEL) < 0)
        goto err_power_on;
    sleep(1);

    do {
        bzero(rxbuf, sizeof(rxbuf));
        if (g510_uart_write("AT+IPR=11\r\n") < 0) {
            LOGE("%s: uart failed to write\n", __FUNCTION__);
            goto err_power_on;
        }
        sleep(1);
        if (g510_uart_read(rxbuf, sizeof(rxbuf)) < 0) {
            LOGE("%s: uart failed to read\n", __FUNCTION__);
            goto err_power_on;
        }
        LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);
        if (strstr(rxbuf, "OK"))
            break;
    } while(count++ < 10);

    if (count >= 10 ||
        gpio->set_value(G510_PWR_ON_PIN, GPIO_HIGH) < 0)
        goto err_power_on;

    LOGI("power on OK!!!\n");

    return 0;

err_power_on:
    gpio->set_value(G510_PWR_EN_PIN, !G510_PWR_EN_LEVEL);
    return -1;
}

void g510_power_off(void)
{
    char rxbuf[32];
    int count = 0;

    g510_uart_write("AT+CFUN=0\r\n");
    do {
        bzero(rxbuf, sizeof(rxbuf));
        g510_uart_read(rxbuf, sizeof(rxbuf));
        LOGI("%s: rxstr=%s\n", __FUNCTION__, rxbuf);
        if (strstr(rxbuf, "OK"))
            break;
        sleep(1);
    } while(count++ < 5);

    if (count >= 5)
        sleep(3);
    else
        msleep(500);

    gpio->set_value(G510_PWR_EN_PIN, !G510_PWR_EN_LEVEL);
}

int g510_wakeup(void)
{
    if (gpio->set_direction(G510_WAKEUP_PIN, GPIO_OUT) < 0)
        return -1;
    if (gpio->set_value(G510_WAKEUP_PIN, GPIO_LOW) < 0)
        return -1;
    msleep(1);
    return gpio->set_direction(G510_WAKEUP_PIN, GPIO_IN);
}

int g510_emerg_reset(void)
{
    if (gpio->set_value(G510_EMERG_RST_PIN, GPIO_LOW) < 0)
        return -1;
    sleep(1);
    return 0;
}
