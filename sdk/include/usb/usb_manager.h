/*
 *  Copyright (C) 2017, Zhang YanMing <yanmin.zhang@ingenic.com, jamincheung@126.com>
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

#ifndef USB_MANAGER_H
#define USB_MANAGER_H
#include <types.h>

/*系统支持的最大USB设备数目*/
#define USB_DEVICE_MAX_COUNT  1

struct usb_device_manager {
    /**
     * Function: usb_device_init
     * Description: usb设备初始化
     * Input:
     *      devname: usb设备名称
     *          共支持2类usb设备
     *          hid设备: /dev/hidg0
     *          cdc acm设备: /dev/ttyGS0
     *  Output: 无
     *  Return: 0: 成功， -1: 失败
     *  Others: 内部最大可以支持1个usb设备通道[配置宏USB_DEVICE_MAX_COUNT], 每个通道在使用前必须优先调用usb_device_init
     *               若要切换usb设备功能，需要先释放(deinit)当前设备，再初始化(init)新设备
     */
    int32_t (*init)(char* devname);
    /**
     * Function: usb_device_deinit
     * Description: hid释放
     * Input:
     *      devname: usb设备名称
     *          共支持2类usb设备
     *          hid设备: /dev/hidg0
     *          cdc acm设备: /dev/ttyGS0
     * Output: 无
     * Return: 0: 成功， -1: 失败
     * Others: 对应usb_device_init, 在不再使用该设备时调用
     *              若要切换usb设备功能，需要先释放(deinit)当前设备，再初始化(init)新设备
     */
    int32_t (*deinit)(char* devname);

    /**
     * Function: usb_device_switch_func
     * Description: usb功能设备切换
     * Input:
     *      switch_to: 目标切换功能设备名称
     *      switch_from:  当前功能设备名称
     *          举例: 从hid切换到cdc acm
     *                  switch_from应设置为/dev/ttyhidg0
     *                  switch_to应设置为/dev/ttyGS0
     * Output: 无
     * Return: 0: 成功， -1: 失败
     * Others: 也可以调用deinit释放当前设备，再init初始化新设备，完成切换
     *              测试程序为test_usb_switch
     */
    int32_t (*switch_func)(char* switch_to, char* switch_from);
    /**
     * Function: usb_device_get_max_transfer_unit
     * Description: 获取usb最大传输单元
     * Input:
     *      devname: usb设备名称
     *          共支持2类usb设备
     *          hid设备: /dev/hidg0
     *          cdc acm设备: /dev/ttyGS0
     * Output: 无
     * Return: 返回最大传输单元，单位:bytes
     * Others: 读写函数单次读写大小最大值等于最大传输单元大小
     */
    uint32_t (*get_max_transfer_unit)(char* devname);
    /**
     * Function: usb_device_write
     * Description: usb写数据
     * Input:
     *      devname: usb设备名称
     *          共支持2类usb设备
     *          hid设备: /dev/hidg0
     *          cdc acm设备: /dev/ttyGS0
     *      buf: 写数据缓冲区
     *      count: 写数据长度，单位:bytes
     *      timeout_ms: 写超时，单位:ms
     * Output: 无
     * Return: 正数或0: 成功写入的字节数  -1: 失败
     * Others: 该函数在指定超时时间内写入count字节数据，返回实际写入大小
     *         在使用之前要先调用usb_device_init
     */
    int32_t (*write)(char* devname, void* buf, uint32_t count,
        uint32_t timeout_ms);
    /**
     * Function: usb_device_read
     * Description: usb读数据
     * Input:
     *      devname: usb设备名称
     *          共支持2类usb设备
     *          hid设备: /dev/hidg0
     *          cdc acm设备: /dev/ttyGS0
     *      buf: 读数据缓冲区
     *      count: 读数据长度，单位:bytes
     *      timeout_ms: 读超时，单位:ms
     * Output: 无
     * Return: 正数或0: 成功读取的字节数  -1: 失败
     * Others: 该函数在指定超时时间内读取预设count字节数据，返回实际读取大小
     *         在使用之前要先调用usb_device_init
     */
    int32_t (*read)(char* devname, void* buf, uint32_t count,
        uint32_t timeout_ms);
};

struct usb_device_manager*  get_usb_device_manager(void);

#endif
