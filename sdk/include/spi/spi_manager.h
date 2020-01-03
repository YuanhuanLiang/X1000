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
#ifndef SPI_MANAGER_H
#define SPI_MANAGER_H
#include <types.h>

#define SPI_DEVICE_MAX      2

/*
 * 下次读写操作之前的延时，根据需要修改，transfer函数中用到该宏
 */
#define SPI_MSG_DELAY_US    1000

/*
 * SPI每次传输数据最大长度, 以字节为单位, 此宏值不能被修改
 */
#define SPI_MSG_LENGTH_MAX  4096

/*
 * 定义所支持的SPI设备id, 用于区分每一个SPI设备
 * 不能修改任何一个enum spi成员的值, 否则导致不可预知的错误
 */
enum spi {
    SPI_DEV0,
    SPI_DEV1,
};

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

struct spi_manager {
    /**
     *    Function: init
     * Description: SPI设备初始化
     *       Input:
     *              id: SPI设备id, 其值必须小于SPI_DEVICE_MAX
     *            mode: SPI设备工作模式
     *            bits: SPI读写一个word的位数, 其值有: 8/16/32, 通常为 8
     *           speed: SPI读写最大速率
     *      Others: 在使用每个SPI设备之前，必须优先调用init函数进行初始化
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*init)(enum spi id, uint8_t mode, uint8_t bits, uint32_t speed);

    /**
     *    Function: deinit
     * Description: SPI设备释放
     *       Input:
     *              id: SPI设备id, 其值必须小于SPI_DEVICE_MAX
     *      Others: 对应于init函数, 不再使用某个SPI设备时, 应该调用此函数释放
     *      Return: 无
     */
    void (*deinit)(enum spi id);

    /**
     *    Function: read
     * Description: SPI读设备
     *       Input:
     *              id: SPI设备id, 其值必须小于SPI_DEVICE_MAX
     *           rxbuf: 存储读取数据的缓存区指针, 不能是空指针
     *          length: 读取的字节数
     *      Others: 无
     *      Return: -1 --> 失败, 成功返回实际读取到字节数
     */
    int32_t (*read)(enum spi id, uint8_t *rxbuf, uint32_t length);

    /**
     *    Function: write
     * Description: SPI写设备
     *       Input:
     *              id: SPI设备id, 其值必须小于SPI_DEVICE_MAX
     *           txbuf: 存储待写入数据的缓存区指针, 不能是空指针
     *          length: 读入的字节数
     *      Others: 无
     *      Return: -1 --> 失败, 成功返回实际写入字节数
     */
    int32_t (*write)(enum spi id, uint8_t *txbuf, uint32_t length);

    /**
     *    Function: transfer
     * Description: SPI写设备, 同时读设备
     *       Input:
     *              id: SPI设备id, 其值必须小于SPI_DEVICE_MAX
     *           txbuf: 存储待写入数据的缓存区指针, 不能是空指针
     *           rxbuf: 存储读取数据的缓存区指针, 不能是空指针
     *           txlen: 写的字节数
     *           rxlen: 读的字节数
     *      Others: 无
     *      Return: -1 --> 失败, 0 --> 成功
     */
    int32_t (*transfer)(enum spi id, const uint8_t *txbuf, size_t txlen, uint8_t *rxbuf, size_t rxlen);
};

/**
 *    Function: get_spi_manager
 * Description: 获取 spi_mananger 句柄
 *       Input: 无
 *      Return: 返回 spi_manager 结构体指针
 *      Others: 通过该结构体指针访问内部提供的方法
 */
struct spi_manager *get_spi_manager(void);

#endif /* SPI_MANAGER_H */
