/*
 *  Copyright (C) 2017, Monk Su<rongjin.su@ingenic.com, MonkSu@outlook.com>
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
#ifndef _CAMERA_V4L2_MANAGER_H_
#define _CAMERA_V4L2_MANAGER_H_
#include <linux/videodev2.h>
#include <stdbool.h>


typedef enum {
    V4L2_METHOD_READ,
    V4L2_METHOD_MMAP,
    V4L2_METHOD_USERPTR,
} camera_v4l2_io_m;

typedef enum {
    V4L2_V_FLIP = 0,
    V4L2_H_FLIP
} camera_v4l2_flip;


typedef struct {
    unsigned char awb_r_gain;
    unsigned char awb_g_gain;
    unsigned char awb_b_gain;
}camera_v4l2_white_balance;

typedef void (*frame_receive)(uint8_t* buf, uint32_t pixelformat, uint32_t width, uint32_t height, uint32_t seq);

struct capt_param_t {
    uint32_t width;         // Resolution width(x)
    uint32_t height;        // Resolution height(y)
    uint32_t bpp;           // Bits Per Pixels
    uint32_t nbuf;          // buf queue
    frame_receive fr_cb;
    camera_v4l2_io_m io;            // method of get image data
};

/*

 you can get it form  <linux/videodev2.h>

enum  v4l2_exposure_auto_type {
    V4L2_EXPOSURE_AUTO              = 0,
    V4L2_EXPOSURE_MANUAL            = 1,
    V4L2_EXPOSURE_SHUTTER_PRIORITY  = 2,
    V4L2_EXPOSURE_APERTURE_PRIORITY = 3
};

enum v4l2_auto_n_preset_white_balance {
    V4L2_WHITE_BALANCE_MANUAL        = 0,
    V4L2_WHITE_BALANCE_AUTO          = 1,
    V4L2_WHITE_BALANCE_INCANDESCENT  = 2,
    V4L2_WHITE_BALANCE_FLUORESCENT   = 3,
    V4L2_WHITE_BALANCE_FLUORESCENT_H = 4,
    V4L2_WHITE_BALANCE_HORIZON       = 5,
    V4L2_WHITE_BALANCE_DAYLIGHT      = 6,
    V4L2_WHITE_BALANCE_FLASH         = 7,
    V4L2_WHITE_BALANCE_CLOUDY        = 8,
    V4L2_WHITE_BALANCE_SHADE         = 9,
};

*/

struct camera_v4l2_manager {
    /**
     *  @brief   初始化camera devide
     *
     *  @param   capt_p - 传入获取图像的参数
     *
     */
    int (*init)(struct capt_param_t *capt_p);
    /**
     *  @brief   开启图像获取 ，目前只支持LCD预览，内部开启线程循环
     *
     */
    int (*start)(void);
    /**
     *  @brief   关闭图像获取 ，关闭线程
     *
     */
    int (*stop)(void);
    /**
     * @fn int (*set_flip)(camera_v4l2_flip flip);
     *
     * @brief  设置图像垂直翻转或水平镜像
     *
     * @param flip 水平或垂直
     * @param enable 垂直翻转或水平镜像使能和失能
     *
     *
     * @retval 0 成功
     * @retval <0 失败
     *
     */
    int (*set_flip)(camera_v4l2_flip flip, bool enable);
     /**
     * @fn int (*set_exposure_mode)(camera_v4l2_exposure_mode mode);
     *
     * @brief  设置camera曝光模式
     *
     * @param mode 指定曝光模式
     *
     *
     * @retval 0 成功
     * @retval <0 失败
     *
     */
    int (*set_exposure_mode)(enum v4l2_exposure_auto_type mode);
    /**
     * @fn int (*set_exposure)(uint16_t exposure);
     *
     * @brief  设置camera曝光值
     *
     * @param exposure 曝光值
     *
     *
     * @retval 0 成功
     * @retval <0 失败
     *
     * @note  设置曝光值时, 曝光模式必须先设置为手动曝光(EXPOSURE_MANUAL)
     *
     */
    int (*set_exposure)(uint16_t exposure);
    /**
     * @fn int (*white_balance_mode)(camera_v4l2_white_balance_mode mode);
     *
     * @brief  设置白平衡模式
     *
     * @param mode 白平衡模式
     *
     *
     * @retval 0 成功
     * @retval <0 失败
     *
     * @note  可以设置特定的 scenes
     *
     */
    int (*set_white_balance_mode)(enum v4l2_auto_n_preset_white_balance mode);
    /**
     * @fn int (*set_white_balance)(camera_v4l2_white_balance white_balance);
     *
     * @brief  手动设置白平衡 R\G\B 通道的值
     *
     * @param white_balance 白平衡 R\G\B通道的值
     *
     *
     * @retval 0 成功
     * @retval <0 失败
     *
     * @note  设置白平衡的值时, 白平衡模式必须先设置为手动白平衡(WHITE_BALANCE_MANUAL)
     *
     */
    int (*set_white_balance)(camera_v4l2_white_balance white_balance);
    /**
     *  @brief   释放模块
     *
     */
    int (*deinit)(void);
    /**
     * @fn int (*set_function)(struct v4l2_control ctrl);
     *
     * @brief  手动设置摄像头参数
     *
     * @param struct v4l2_control {
     __u32		     id;
     __s32		     value;
     };
     *
     * @retval 0 成功
     * @retval <0 失败
     *
     *
     */
    int (*set_function)(struct v4l2_control ctrl);
};

struct camera_v4l2_manager* get_camera_v4l2_manager(void);

#endif
