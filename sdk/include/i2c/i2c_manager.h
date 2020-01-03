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
#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H
#include <types.h>

#define I2C_BUS_MAX            3
/*
 * 读写I2C设备所发送的地址的长度, 以BIT为单位, 有8BIT或16BIT, 应该根据实际使用的器件修改宏值
 */
#define I2C_DEV_ADDR_LENGTH    8
/*
 * 对设备的这个地址进行读操作, 以检测I2C总线上有没有 chip_addr这个从设备, 可根据实际修改宏值
 */
#define I2C_CHECK_READ_ADDR    0x00
/*
 * 对设备一次读写操作后, 在进行下次读写操作时的延时时间,单位:us, 值不能太小, 否则导致读写出错
 */
#define I2C_ACCESS_DELAY_US    5000

/*
 * 定义芯片所支持的所有I2C总线的id, 不能修改任何一个enum i2c成员的值, 否则导致不可预想的错误
 */
enum i2c {
    I2C0,
    I2C1,
    I2C2,
};

/*
 * 应为每个I2C从设备定义一个struct i2c_unit 结构体变量, 每个成员说明如下:
 *        id: 表示从设备所挂载的I2C总戏
 * chip_addr: 为从设备的7位地址
 *
 */
struct i2c_unit {
    enum i2c id;
    int32_t chip_addr;
};

struct i2c_manager {
    /**
     *    Function: init
     * Description: I2C初始化
     *       Input:
     *            i2c: 每个I2C设备对应 struct i2c_unit 结构体指针, 必须先初始化结构体的成员
     *                 其中: id 的值应大于0, 小于I2C_DEVICE_MAX; chip_addr: 为设备的7位地址
     *      Others: 必须优先调用 init函数, 可以被多次调用, 用于初始化不同的I2C设备
     *      Return: 0 --> 成功, 非0 --> 失败
     */
    int32_t (*init)(struct i2c_unit *i2c);

    /**
     *    Function: deinit
     * Description: I2C 设备释放
     *       Input:
     *            i2c: 每个I2C设备对应 struct i2c_unit 结构体指针, 必须先初始化结构体的成员
     *                 其中: id 的值应大于0, 小于I2C_DEVICE_MAX; chip_addr: 为设备的7位地址
     *      Others: 对应于init函数, 不再使用某个I2C设备时, 调用此函数释放, 释放后不能再进行读写操作
     *      Return: 无
     */
    void (*deinit)(struct i2c_unit *i2c);

    /**
     *    Function: read
     * Description: 从I2C设备读取数据
     *       Input:
     *            i2c: 每个I2C设备对应 struct i2c_unit 结构体指针, 必须先初始化结构体的成员
     *                 其中: id 的值应大于0, 小于I2C_DEVICE_MAX; chip_addr: 为设备的7位地址
     *            buf: 存储读取数据的缓存区指针, 不能是空指针
     *           addr: 指定从I2C设备的哪个地址开始读取数据
     *          count: 读取的字节数
     *      Others: 无
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*read)(struct i2c_unit *i2c, uint8_t *buf, int32_t addr, int32_t count);

    /**
     *    Function: write
     * Description: 往I2C设备写入数据
     *       Input:
     *            i2c: 每个I2C设备对应 struct i2c_unit 结构体指针, 必须先初始化结构体的成员
     *                 其中: id 的值应大于0, 小于I2C_DEVICE_MAX; chip_addr: 为设备的7位地址
     *            buf: 存储待写入数据的缓存区指针, 不能是空指针
     *           addr: 指定从I2C设备的哪个地址开始写入数据
     *          count: 写入的字节数
     *      Others: 无
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*write)(struct i2c_unit *i2c, uint8_t *buf, int32_t addr, int32_t count);
};


/**
 *    Function: get_i2c_manager
 * Description: 获取 i2c_manager 句柄
 *       Input: 无
 *      Return: 返回 struct i2c_manager 结构体指针
 *      Others: 通过该结构体指针访问内部提供的方法
 */
struct i2c_manager *get_i2c_manager(void);

#endif /* I2C_MANAGER_H */
