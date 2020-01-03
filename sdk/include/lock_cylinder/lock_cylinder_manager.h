/*
 *  Copyright (C) 2017, Wang Qiuwei <qiuwei.wang@ingenic.com, panddio@163.com>
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
#ifndef LOCK_CYLINDER_MANAGER_H
#define LOCK_CYLINDER_MANAGER_H
#include <types.h>

typedef void (*deal_key_detected_handler)(bool is_key_inserted, void *arg);

struct lock_cylinder_manager {
    /**
     * Function: lock_cylinder_init
     * Description: lock cylinder 设备初始化
     * Input:
     *      handler: 检测到钥匙插入或拔出的回调函数
     *      handler_arg: 回调函数的参数
     * Return: 正数或0: 成功写入的字节数  小于0: 失败
     * Others: 该函数必须在其他lock_cylinder_manager的其它函数之前被调用
     */
    int32_t (*init)(deal_key_detected_handler handler, void *handler_arg);

    /**
     * Function: lock_cylinder_deinit
     * Description: lock cylinder 设备释放
     * Input: 无
     * Return: 无
     * Others: 不再使用设备应该释放
     */
    void (*deinit)(void);

    /**
     * Function: lock_cylinder_get_keystatue
     * Description: 获取钥匙的拔插的状态
     * Input: 无
     * Return:
     *      小于0: 获取状态失败
     *      true: 钥匙插入状态
     *      false: 钥匙拔出状态
     * Others: 此函数应该在回调函数调用，来实时获取钥匙的拔插状态，注意检测此函数的返回值
     */
    int8_t (*get_keystatue)(void);

    /**
     * Function: lock_cylinder_get_romid
     * Description: 获取钥匙的唯一id
     * Input:
     *      romid: 保存读取到 ROM ID 的缓存指针
     * Return:
     *      小于0: 获取状态失败
     *      等于0: 获取成功
     * Others: 此函数应该在钥匙插入状态下调用
     */
    int32_t (*get_romid)(uint8_t *romid);

    /**
     * Function: lock_cylinder_register_key
     * Description: 注册钥匙，把密钥写入钥匙的加密芯片
     * Input: 无
     * Return:
     *      小于0: 注册失败
     *      等于0: 注册成功
     * Others: 此函数应该在钥匙插入状态下调用，
     *         注册成功后，保存ROM ID可以用来判断钥匙是否已经注册
     */
    int32_t (*register_key)(void);

    /**
     * Function: lock_cylinder_authenticate_key
     * Description:验证已经注册过的钥匙
     * Input: 无
     * Return:
     *      小于0: 注册失败
     *      等于0: 注册成功
     * Others: 此函数应该在钥匙插入状态下并判断钥匙已经注册后调用
     */
    int32_t (*authenticate_key)(void);

    /**
     * Function: lock_cylinder_power_ctrl
     * Description:控制锁心磁管的电源
     * Input:
     *      pwr_en: 锁心磁管上电或掉电，
     *              上电: 钥匙不能转动锁心
     *              掉电: 钥匙能够转动锁心
     * Return:
     *      小于0: 注册失败
     *      等于0: 注册成功
     * Others: 此函数应该在钥匙插入状态下并验证成功后调用，
     *         如果钥匙验证不成功调用此函数，是无法控制锁芯掉电
     */
    int32_t (*power_ctrl)(bool pwr_en);
};

/**
 *    Function: get_lock_cylinder_manager
 * Description: 获取 lock_cylinder_manager 句柄
 *       Input: 无
 *      Return: 返回 struct lock_cylinder_manager 结构体指针
 *      Others: 通过该结构体指针访问内部提供的方法
 */
struct lock_cylinder_manager *get_lock_cylinder_manager(void);
#endif /* LOCK_CYLINDER_MANAGER_H */
