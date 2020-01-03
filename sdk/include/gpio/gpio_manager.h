/*
 *  Copyright (C) 2016, Xin ShuAn <shuan.xin@ingenic.com>
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

#ifndef GPIO_MANAGER_H
#define GPIO_MANAGER_H
#include <types.h>
#include <lib/gpio/libgpio.h>
/**
 * GPIO编号
 */
#define GPIO_PA(n)  (0*32 + n)
#define GPIO_PB(n)  (1*32 + n)
#define GPIO_PC(n)  (2*32 + n)
#define GPIO_PD(n)  (3*32 + n)

/**
 * GPIO的中断回调函数
 */
typedef void (*gpio_irq_func)(int);

struct gpio_manager {
    /**
     * Function: manager_init
     * Description: 初始化库资源
     *  注意：必须初始化过一次后才能使用成员功能
     *  Return: 0: 成功， -1: 失败
     */
    int32_t (*init)(void);

    /**
     * Function: manager_deinit
     * Description: 释放库使用的资源
     *  在需不要使用GPIO的情况下，可以释放资源。
     *  注意：在使用GPIO中断时，不能释放库资源
     */
    void (*deinit)(void);

    /**
     * Function: gpio_open
     * Description: GPIO初始化
     * Input:
     *   gpio: 需要操作的GPIO编号
     *        例如: GPIO_PA(n) GPIO_PB(n) GPIO_PC(n) GPIO_PD(n)
     *  注意：操作GPIO前必须先open_gpio
     *  Return: 0: 成功， -1: 失败
     */
    int32_t (*open)(uint32_t gpio);

    /**
     * Function: gpio_close
     * Description: GPIO释放
     * Input:
     *   gpio: 需要操作的GPIO编号
     *        例如: GPIO_PA(n) GPIO_PB(n) GPIO_PC(n) GPIO_PD(n)
     *  不需要用到某个GPIO时，可以调用clsoe来释放资源
     */
    void (*close)(uint32_t gpio);

    /**
     * Function: set_direction
     * Description: 设置GPIO输入输出模式
     * Input:
     *   gpio: 需要操作的GPIO编号
     *        例如: GPIO_PA(n) GPIO_PB(n) GPIO_PC(n) GPIO_PD(n)
     *   dir: 设置功能 输入或输出
     *        参数：GPIO_IN or GPIO_OUT
     *   注意：输出模式下IO默认电平为低
     *  Return: 0: 成功， -1: 失败
     */
    int32_t (*set_direction)(uint32_t gpio, gpio_direction dir);

    /**
     * Function: get_direction
     * Description: 获取GPIO输入输出模式
     * Input:
     *   gpio: 需要操作的GPIO编号
     *        例如: GPIO_PA(n) GPIO_PB(n) GPIO_PC(n) GPIO_PD(n)
     *   dir: 获取功能状态 输入或输出
     *        参数：GPIO_IN or GPIO_OUT
     *  注意：dir参数是gpio_direction指针
     *  Return: 0: 成功， -1: 失败
     */
    int32_t (*get_direction)(uint32_t gpio, gpio_direction *dir);

    /**
     * Function: set_value
     * Description: 设置GPIO电平
     * Input:
     *   gpio: 需要操作的GPIO编号
     *        例如: GPIO_PA(n) GPIO_PB(n) GPIO_PC(n) GPIO_PD(n)
     *   value: 设置电平 低电平或高电平
     *        参数：GPIO_LOW or GPIO_HIGH
     *  注意：输入模式下禁止设置电平
     *  Return: 0: 成功， -1: 失败
     */
    int32_t (*set_value)(uint32_t gpio, gpio_value value);

    /**
     * Function: get_value
     * Description: 获取GPIO电平
     * Input:
     *   gpio: 需要操作的GPIO编号
     *        例如: GPIO_PA(n) GPIO_PB(n) GPIO_PC(n) GPIO_PD(n)
     *   value: 获取电平 低电平或高电平
     *        参数：GPIO_LOW or GPIO_HIGH
     *  注意：value参数是gpio_value指针
     *  Return: 0: 成功， -1: 失败
     */
    int32_t (*get_value)(uint32_t gpio, gpio_value *value);

    /**
     * Function: set_irq_func
     * Description: 设置GPIO中断回调函数
     * Input:
     *   func: GPIO中断回调函数
     *       参数：无返回值 和 参数（GPIO的编号）
     *                   typedef void (*irq_work_func)(int);
     *  注意：所有GPIO对于一个中断函数，回调函数参数为触发中断的GPIO编号
     */
    void (*set_irq_func)(gpio_irq_func func);

    /**
     * Function: enable_irq
     * Description: 使能GPIO中断
     * Input:
     *   gpio: 需要操作的GPIO编号
     *        例如: GPIO_PA(n) GPIO_PB(n) GPIO_PC(n) GPIO_PD(n)
     *   mode: 设置中断出发的方式
     *        参数：
     *                       GPIO_RISING,          上升沿触发
     *                       GPIO_FALLING,        下降沿触发
     *                       GPIO_BOTH,             双边沿沿触发
     *  注意：是能前必须设置中断回调函数set_irq_func
     *              GPIO引脚必须为输入模式
     *  Return: 0: 成功， -1: 失败
     */
    int32_t (*enable_irq)(uint32_t gpio, gpio_irq_mode mode);

    /**
     * Function: disable_irq
     * Description: 关闭GPIO中断
     * Input:
     *   gpio: 需要操作的GPIO编号
     *        例如: GPIO_PA(n) GPIO_PB(n) GPIO_PC(n) GPIO_PD(n)
     */
    void (*disable_irq)(uint32_t gpio);
};

/**
 * Function: get_gpio_manager
 * Description: 获取gpio_mananger句柄
 * Input:  无
 * Return: 返回gpio_manager结构体指针
 * Others: 通过该结构体指针访问内部提供的方法
 */
struct gpio_manager* get_gpio_manager(void);

#endif
