/*
 *  Copyright (C) 2016, Wang Qiuwei <qiuwei.wang@ingenic.com, panddio@163.com>
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
#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H
#include <types.h>
/*
 * sensor每设置一个寄存器之后的延时时间，单位是us, 可以根据实际要求修改此宏值
 */
#define SENSOR_SET_REG_DELAY_US  50
/*
 * sensor寄存器地址的长度, 以BIT为单位, 有8BIT或16BIT, 应该根据实际使用的sensor修改此宏值
 */
#define SENSOR_ADDR_LENGTH  8

#if (SENSOR_ADDR_LENGTH == 8)
/* 一下三个宏的值不能随便修改, 否则导致不可预知的错误 */
#define ADDR_END    0xff
#define VAL_END     0xff
#define ENDMARKER   {0xff, 0xff}

/*
 * 配置sensor寄存器时, 需传入struct regval_list结构体指针, 以指定配置的寄存器和配置的值
 * regaddr: 寄存器的地址
 *  regval: 对应寄存器的值
 */
struct camera_regval_list {
    uint8_t regaddr;
    uint8_t regval;
};
#elif (SENSOR_ADDR_LENGTH == 16)
/* 一下三个宏的值不能随便修改, 否则导致不可预知的错误 */
#define ADDR_END    0xffff
#define VAL_END     0xff
#define ENDMARKER   {0xffff, 0xff}

/*
 * 配置sensor寄存器时, 需传入struct regval_list结构体指针, 以指定配置的寄存器和配置的值
 * regaddr: 寄存器的地址
 *  regval: 对应寄存器的值
 */
struct camera_regval_list {
    uint16_t regaddr;
    uint8_t regval;
};
#endif

/*
 * 用于设置控制器捕捉图像的分辨率和像素深度, 每个成员说明如下:
 *   width: 图像水平方向的分辨率
 *  height: 图像垂直方向的分辨率
 *     bpp: 图像的像素深度
 *    size: 图像的大小, 字节为单位, size = width * height * bpp / 2
 */
struct camera_img_param {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;    /* bits per pixel: 8/16/32 */
    uint32_t size;
};

/*
 * 用于设置camera 控制器时序的结构, 每个成员说明如下:
 *            mclk_freq: mclk 的频率
 *    pclk_active_level: pclk 的有效电平, 为1是高电平有效, 为0则是低电平有效
 *   hsync_active_level: hsync 的有效电平, 为1是高电平有效, 为0则是低电平有效
 *   vsync_active_level: vsync 的有效电平, 为1是高电平有效, 为0则是低电平有效
 */
struct camera_timing_param {
    uint32_t mclk_freq;
    uint8_t pclk_active_level;
    uint8_t hsync_active_level;
    uint8_t vsync_active_level;
};

struct camera_manager {
    /**
     *    Function: camera_init
     * Description: 摄像头初始化
     *       Input: 无
     *      Others: 必须优先调用 camera_init
     *      Return: 0 --> 成功, 非0 --> 失败
     */
    int32_t (*camera_init)(void);

    /**
     *    Function: camera_deinit
     * Description: 摄像头释放
     *       Input: 无
     *      Others: 对应 camera_init, 不再使用camera时调用
     *      Return: 无
     */
    void (*camera_deinit)(void);

    /**
     *    Function: camera_read
     * Description: 读取摄像头采集数据,保存在 yuvbuf 指向的缓存区中
     *       Input:
     *          yuvbuf: 图像缓存区指针, 缓存区必须大于或等于读取的大小
     *            size: 读取数据大小,字节为单位, 一般设为 image_size
     *      Others: 在此函数中会断言yuvbuf是否等于NULL, 如果为NULL, 将推出程序
     *      Return: 返回实际读取到的字节数, 如果返回-1 --> 失败
     */
    int32_t (*camera_read)(uint8_t *yuvbuf, uint32_t size);

    /**
     *    Function: set_img_param
     * Description: 设置控制器捕捉图像的分辨率和像素深度
     *       Input:
     *           img: struct img_param_t 结构指针, 指定图像的分辨率和像素深度
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*set_img_param)(struct camera_img_param *img);

    /**
     *    Function: set_timing_param
     * Description: 设置控制器时序, 包括mclk 频率、pclk有效电平、hsync有效电平、vsync有效电平
     *       Input:
     *          timing: struct timing_param_t 结构指针, 指定 mclk频率、pclk有效电平、hsync有效电平、vsync有效电平
     *                  在camera_init函数中分别初始化为:24000000、0、1、1, 为0是高电平有效, 为1则是低电平有效
     *      Others: 如果在camera_init函数内默认设置已经符合要求,则不需要调用
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*set_timing_param)(struct camera_timing_param *timing);

    /**
     *    Function: sensor_setup_addr
     * Description: 设置摄像头sensor的I2C地址, 应该调用此函数设置sensor的I2C地址
     *       Input:
     *              chip_addr: 摄像头sensor的I2C地址, 不包括读写控制位
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*sensor_setup_addr)(int32_t chip_addr);

    /**
     *    Function: sensor_setup_regs
     * Description: 设置摄像头sensor的多个寄存器,用于初始化sensor
     *       Input:
     *          vals: struct regval_list 结构指针, 通常传入struct regval_list结构数组
     *      Others: 在开始读取图像数据时, 应该调用此函数初始化sensor寄存器
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*sensor_setup_regs)(const struct camera_regval_list *vals);

    /**
     *    Function: sensor_write_reg
     * Description: 设置摄像头sensor的单个寄存器
     *       Input:
     *          regaddr: 摄像头sensor的寄存器地址
     *           regval: 摄像头sensor寄存器的值
     *      Return: 0 --> 成功, -1 --> 失败
     */
    int32_t (*sensor_write_reg)(uint32_t regaddr, uint8_t regval);

    /**
     *    Function: sensor_read_reg
     * Description: 读取摄像头sensor某个寄存器的知
     *       Input:
     *          regaddr: 摄像头sensor的寄存器地址
     *      Return: -1 --> 失败, 其他 --> 寄存器的值
     */
    uint8_t (*sensor_read_reg)(uint32_t regaddr);
};

/**
 *    Function: get_camera_manager
 * Description: 获取 camera_mananger 句柄
 *       Input: 无
 *      Return: 返回 camera_manager 结构体指针
 *      Others: 通过该结构体指针访问内部提供的方法
 */
struct camera_manager *get_camera_manager(void);

#endif /* CAMERA_MANAGER_H */
