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
#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H
#include <types.h>

struct power_manager {
    /**
     *    Function: power_off
     * Description: 关机
     *       Input: 无
     *      Return: -1 --> 失败, 成功不需要处理返回值
     */
    int32_t (*power_off)(void);

    /**
     *    Function: reboot
     * Description: 系统复位
     *       Input: 无
     *      Return: -1 --> 失败, 成功不需要处理返回值
     */
    int32_t (*reboot)(void);

    /**
     *    Function: sleep
     * Description: 进入休眠
     *       Input: 无
     *      Return: -1 --> 失败, 0 --> 成功
     */
    int32_t (*sleep)(void);
};

/**
 *    Function: get_power_manager
 * Description: 获取 power_manager 句柄
 *       Input: 无
 *      Return: 返回 struct power_manager 结构体指针
 *      Others: 通过该结构体指针访问内部提供的方法
 */
struct power_manager *get_power_manager(void);


#endif /* POWER_MANAGER_H */
