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
#ifndef VIBRATOR_MANAGER_H
#define VIBRATOR_MANAGER_H
#include <types.h>

/*
 * motor 状态
 *           MOTOR_COAST    滑行
 *           MOTOR_FORWARD    正转
 *           MOTOR_REVERSE    反转
 *           MOTOR_BRAKE    刹车
 *           MOTOR_FAULT    故障
 */
typedef enum _motor_status {
    MOTOR_COAST = 0x0,
    MOTOR_FORWARD = 0x01,
    MOTOR_REVERSE = 0x02,
    MOTOR_BRAKE = 0x03,
    MOTOR_FAULT = 0xff,
}motor_status;

typedef void (*motor_event_callback_t)(uint32_t motor_id, motor_status status);

struct vibrator_manager {
    /**
     * Function: manager_init
     * Description: 初始化库资源
     *  Return: 0: 成功， -1: 失败
     */
    int32_t (*init)(void);

    /**
     * Function: manager_deinit
     * Description: 释放库使用的资源
     */
    void (*deinit)(void);

    /**
       * Function: motor_open
       * Description: motor初始化
       * Input:
       *   motor_id: 需要操作的motor编号
       *  注意：操作motor前必须先motor_open，默认状态：MOTOR_COAST
       *  Return: 0: 成功， -1: 失败
       */
      int32_t (*open)(uint32_t motor_id);

      /**
       * Function: motor_close
       * Description: motor释放
       * Input:
       *   motor_id: 需要操作的motor编号
       *  不需要用到某个motor时，可以调用clsoe来释放资源
       */
      void (*close)(uint32_t motor_id);

      /**
       * Function: set_function
       * Description: 设置motor工作模式
       * Input:
       *   motor_id:  需要操作的motor编号
       *   function: 设置功能
       *        参数：MOTOR_COAST        滑行
       *                    MOTOR_FORWARD  正转
       *                    MOTOR_REVERSE      反转
       *                    MOTOR_BRAKE         刹车
       *  Return: 0: 成功， -1: 失败
       */
      int32_t (*set_function)(uint32_t motor_id, motor_status function);

      /**
       * Function: get_status
       * Description: 获取motor状态
       * Input:
       *   motor_id:  需要操作的motor编号
       *  Return: 0: MOTOR_COAST        滑行
       *                1：MOTOR_FORWARD  正转
       *                2：MOTOR_REVERSE      反转
       *                3：MOTOR_BRAKE         刹车
       *                0xff：MOTOR_FAULT    故障
       *                -1: 失败
       */
      int32_t (*get_status)(uint32_t motor_id);

      /**
       * Function: set_speed
       * Description: 设置motor速度
       * Input:
       *   motor_id:  需要操作的motor编号
       *   speed: 速度 0-10
       *  Return: 0: 成功， -1: 失败
       */
      int32_t (*set_speed)(uint32_t motor_id, uint32_t speed);

      /**
       * Function: get_speed
       * Description: 获取motor速度
       * Input:
       *   motor_id:  需要操作的motor编号
       *  Return: 0 - 10: 速度值
       *                -1: 失败
       */
      int32_t (*get_speed)(uint32_t motor_id);

      /**
       * Function: set_cycle
       * Description: 设置motor PWM周期
       * Input:
       *   motor_id:  需要操作的motor编号
       *   cycle: 周期  10-1000
       *              单位： ms
       *  Return: 0: 成功， -1: 失败
       */
      int32_t (*set_cycle)(uint32_t motor_id, uint32_t cycle);

      /**
       * Function: get_cycle
       * Description: 获取motor PWM周期
       * Input:
       *   motor_id:  需要操作的motor编号
       *  Return: 10 - 1000: 周期值
       *                -1: 失败
       */
      int32_t (*get_cycle)(uint32_t motor_id);

      /**
       * Function: register_event_callback
       * Description: 注册 motor故障回调函数
       * Input:
       *   motor_id:  需要操作的motor编号
       *   callback：回调函数
       *  Return: 0: 成功， -1: 失败
       */
      int32_t (*register_event_callback)(uint32_t motor_id, motor_event_callback_t callback);

      /**
       * Function: unregister_event_callback
       * Description: 注销 motor故障回调函数
       * Input:
       *   motor_id:  需要操作的motor编号
       *   callback：回调函数
       *  Return: 0: 成功， -1: 失败
       */
      int32_t (*unregister_event_callback)(uint32_t motor_id, motor_event_callback_t callback);

      /**
       * Function: get_netlink_handler
       * Description: 获取 motor的netlink的核心结构体
       *  Return: NULL: 失败， 成功返回netlink_handler结构指针
       */
      struct netlink_handler* (*get_netlink_handler)(void);
};

/**
 * Function: get_vibrator_manager
 * Description: 获取vibrator_manager句柄
 * Input:  无
 * Output: 无
 * Return: 返回vibrator_manager结构体指针
 * Others: 通过该结构体指针访问vibrator_manager内部提供的方法
 */
struct vibrator_manager* get_vibrator_manager(void);

#endif
