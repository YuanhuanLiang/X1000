/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Wed 28 Dec 2016 09:47:27 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <utils/log.h>
#include <utils/assert.h>
#include <spi/spi_manager.h>

/**
 * 注意：init函数的mode参数有一下几种类型
 * SPI_MODE_0 (0|0)                //SCLK空闲时为低电平，串行同步时钟的前沿（上升）数据被采样
 * SPI_MODE_1 (0|SPI_CPHA)         //SCLK空闲时为低电平，串行同步时钟的后沿（下降）数据被采样
 * SPI_MODE_2 (SPI_CPOL|0)         //SCLK空闲时为高电平，串行同步时钟的前沿（下降）数据被采样
 * SPI_MODE_3 (SPI_CPOL|SPI_CPHA)  //SCLK空闲时为高电平，串行同步时钟的后沿（上升）数据被采样
 * SPI_CS_HIGH   0x04              //片选为高
 * SPI_LSB_FIRST 0x08              //低位数据先传输
 * SPI_3WIRE     0x10              //三线式，输入输出数据线为一条线 (这里不支持!!）
 * SPI_LOOP      0x20              //回环模式
 * SPI_NO_CS     0x40              //没有片选信号
 * SPI_READY     0x80              //
 *
 * 以上几个宏在 linux/spi/spidev.h 中定义，用法：
 * mode = SPI_MODE_0 | SPI_CS_HIGH | SPI_LSB_FIRST
 */

/*
 * Macros
 */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define LOG_TAG  "test_spi"

/*
 * Functions
 */
int main(int argc, char *argv[])
{
    uint32_t i, cnt;
    uint32_t speed;
    uint8_t mode;
    uint8_t bits;
    uint8_t tx[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD,
    };
    uint8_t rx[ARRAY_SIZE(tx)] = {0};
    struct spi_manager *spi;

    if (argc > 1)
        cnt = atoi(argv[1]);
    else
        cnt = 20;

    /* 获取操作SPI的句柄 */
    spi = get_spi_manager();

    /* 设置SPI的工作模式 */
    mode = SPI_MODE_1 | SPI_CS_HIGH | SPI_LSB_FIRST;

    /* 设置SPI读写每字的位数 */
    bits = 8;

    /* 设置SPI读写的最大速率(Hz) */
    speed = 1000000;

    /* 初始化SPI */
    spi->init(SPI_DEV0, mode, bits, speed);

    while(cnt--) {
        /* SPI 发送数据， 同时接收数据 */
        spi->transfer(SPI_DEV0, tx, ARRAY_SIZE(tx), rx, ARRAY_SIZE(tx));

        /* 打印接收的数据 */
        for (i = 0; i < ARRAY_SIZE(tx); i++) {
            if (!(i % 6))
                puts("");
            printf("%.2X ", rx[i]);
        }
        puts("");
        usleep(100*1000);
    }

    /* 释放设备 */
    spi->deinit(SPI_DEV0);
    return 0;
}
