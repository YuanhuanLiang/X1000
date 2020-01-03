/*
 *  Copyright (C) 2016, Zhang YanMing <yanmin.zhang@ingenic.com, jamincheung@126.com>
 *
 *  Ingenic QRcode SDK Project
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
#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H
#include <types.h>

struct flash_manager {
    /**
     * Function: flash_init
     * Description: flash初始化
     * Input: 无
     * Output: 无
     * Return: 0:成功 -1: 失败
     * Others: 在flash的读/写/擦除操作之前，首先执行初始化
     */
    int32_t (*init)(void);
    /**
     * Function: flash_deinit
     * Description: flash释放
     * Input: 无
     * Output: 无
     * Return: 0:成功 -1: 失败
     * Others: 与flash_init相对应
     */
    int32_t (*deinit)(void);
    /**
     * Function: flash_get_erase_unit
     * Description: 获取flash擦除单元， 单位: bytes
     * Input: 无
     * Output: 无
     * Return: >0: 成功返回擦除单元大小  0: 失败
     * Others: 无
     */
    int32_t (*get_erase_unit)(void);
    /**
     * Function: flash_erase
     * Description: flash擦除
     * Input:
     *      offset: flash片内偏移物理地址
     *      length: 擦除大小，单位: byte
     *          注意:该大小必须是擦除单元大小的整数倍
     * Output: 无
     * Return: 0:成功  -1:失败
     * Others: 无
     */
    int64_t (*erase)(int64_t offset,  int64_t length);
    /**
     * Function: flash_read
     * Description: flash读取
     * Input:
     *      offset: flash片内偏移物理地址
     *      buf: 读取缓冲区
     *      length: 读取大小，单位: byte
     * Output: 无
     * Return: >=0: 返回成功读取的字节数  -1:失败
     * Others: 无
     */
    int64_t (*read)(int64_t offset,  void* buf, int64_t length);
    /**
     * Function: flash_write
     * Description: flash写入
     * Input:
     *      offset: flash片内偏移物理地址
     *      buf: 写入缓冲区
     *      length: 写入大小，单位: byte
     * Output: 无
     * Return: >=0: 返回成功写入的字节数  -1:失败
     * Others: 无
     */
    int64_t (*write)(int64_t offset,  void* buf, int64_t length);
};
/**
 * Function: get_flash_manager
 * Description: 获取flash_mananger句柄
 * Input:  无
 * Output: 无
 * Return: 返回flash_manager结构体指针
 * Others: 通过该结构体指针访问flash_manager内部提供的方法
 */
struct flash_manager* get_flash_manager(void);

#endif
