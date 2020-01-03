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
#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H
#include <types.h>

struct watchdog_manager {
    /**
     *    Function: init
     * Description: 看门狗初始化
     *       Input:
     *          timeout: 看门狗超时的时间, 以秒为单位, 其值必须大于零
     *      Others: 必须优先调用init函数初始化看门狗和设置timeout, 可被多次调用
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*init)(uint32_t timeout);

    /**
     *    Function: deinit
     * Description: 看门狗释放
     *       Input: 无
     *      Others: 对应init函数, 不再使用看门狗时调用, 该函数将关闭看门狗, 释放设备
     *      Return: 无
     */
    void (*deinit)(void);

    /**
     *    Function: reset
     * Description: 看门狗喂狗
     *       Input: 无
     *      Others: 使能看门狗后, 在timeout时间内不调用此函数, 系统将复位
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*reset)(void);

    /**
     *    Function: enable
     * Description: 看门狗使能
     *       Input: 无
     *      Others: 在init函数初始化或disable函数关闭看门狗之后, 调用此函数启动看门狗
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*enable)(void);

    /**
     *    Function: disable
     * Description: 看门狗关闭
     *       Input: 无
     *      Others: 对应enable函数, 区别deinit函数的地方在于, 调用此函数之后, 能通过enable函数重新启动
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*disable)(void);
};

/**
 *    Function: get_watchdog_manager
 * Description: 获取 watchdog_manager 句柄
 *       Input: 无
 *      Return: 返回 struct watchdog_manager 结构体指针
 *      Others: 通过该结构体指针访问内部提供的方法
 */
struct watchdog_manager *get_watchdog_manager(void);

#endif /* WATCHDOG_MANAGER_H */
