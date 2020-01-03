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
#ifndef _74HC595_MANAGER_H
#define _74HC595_MANAGER_H

#define  SN74HC595_DEVICE_NUM    1

/*
 * 定义74HC595设备的id, 跟 SN74HC595_DEVICE_NUM 密切相关, 可修改此枚举变量添加或删除设备id
 */
enum sn74hc595 {
    SN74HC595_DEV0,
    SN74HC595_DEV1,
    SN74HC595_DEV2,
};


struct sn74hc595_manager {
    /**
     * Function: sn74hc595_init
     * Description: 74hc595 初始化
     * Input:
     *      id: 74hc595设备的id, 多个74hc595级联看作是一个设备
     *      out_bits: 74hc595输出数据的长度，单位：byte，例如，
     *              一个8-bit 74hc595, out_bytes为1，两个8-bit 74hc595级联，out_bytes为2
     * Return: 正数或0: 成功写入的字节数  -1: 失败
     * Others: 该函数必须在其他74hc595_manager的其它函数之前被调用
     */
    int32_t (*init)(enum sn74hc595 id);

    /**
     * Function: sn74hc595_deinit
     * Description: 74hc595 释放
     *  Input:
     *      id: 74hc595设备的id, 多个74hc595级联看作是一个设备
     * Return: 无
     * Others: 不再使用设备应该释放
     */
    void (*deinit)(enum sn74hc595 id);

    /**
     * Function: sn74hc595_write
     * Description: 74hc595 写数据
     * Input:
     *      id: 74hc595设备的id, 多个74hc595级联看作是一个设备
     *      data: 写数据的指针
     *      out_bits: 74hc595输出数据的长度，单位：bits，例如，
     *              一个8-bit 74hc595, out_bits为8，两个8-bit 74hc595级联，out_bits为16.
     *              通过74hc595_get_outbits可以从内核驱动中获取设定的值
     * Return: 正数或0: 成功写入的字节数  -1: 失败
     * Others: 该函数向74hc595写入数据data，来控制74hc595的输出，返回实际写入大小
     */
    int32_t (*write)(enum sn74hc595 id, void *data, uint32_t out_bits);

    /**
     * Function: sn74hc595_read
     * Description: 74hc595 读数据
     * Input:
     *      id: 74hc595设备的id, 多个74hc595级联看作是一个设备
     *      data: 写数据的指针
     *      out_bits: 74hc595输出数据的长度，单位：bits，例如，
     *              一个8-bit 74hc595, out_bits为8，两个8-bit 74hc595级联，out_bits为16.
     *              通过74hc595_get_outbits可以从内核驱动中获取设定的值
     * Return: 正数或0: 成功读取的字节数  -1: 失败
     * Others: 该函数读取74hc595正输出的数据，返回实际读取字节数
     */
    int32_t (*read)(enum sn74hc595 id, void *data, uint32_t out_bits);


    /**
     * Function: sn74hc595_clear
     * Description: 清除74hc595 移位寄存器
     * Input:
     *      id: 74hc595设备的id, 两个74hc595级联看作是一个设备
     * Return: 0: 成功  -1: 失败
     * Others: 在74hc595的移位寄存器清零pin有效时，通过拉低该pin来实现, 否则通过让74hc595输出0
     */
    int32_t (*clear)(enum sn74hc595 id);

    /**
     * Function: sn74hc595_get_outbits
     * Description: 从内核驱动获取74hc595设备的输出位大小
     * Input:
     *      id: 74hc595设备的id, 两个74hc595级联看作是一个设备
     *      out_bits: 保存74hc595输出数据的长度变量指针
     * Return: outbits大小: 成功  -1: 失败
     * Others: 该函数在已经确定outbits大小时可以不调用
     */
    uint32_t (*get_outbits)(enum sn74hc595 id, uint32_t *out_bits);
};

/**
 * Function: get_sn74hc595_manager
 * Description: 获取sn74hc595_manager句柄
 * Input:  无
 *      id: 74hc595设备的id, 多个74hc595级联看作是一个设备
 * Output: 无
 * Return: 返回sn74hc595_manager结构体指针
 * Others: 通过该结构体指针访问sn74hc595_manager内部提供的方法
 */
struct sn74hc595_manager *get_sn74hc595_manager(void);

#endif /*_74HC595_MANAGER_H */
