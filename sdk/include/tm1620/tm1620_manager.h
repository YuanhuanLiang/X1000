/*
 *  Copyright (C) 2018, Wang Qiuwei <qiuwei.wang@ingenic.com, panddio@163.com>
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

#ifndef _TM1620_MANAGER_H
#define _TM1620_MANAGER_H

struct tm1620_manager {
    /**
     * Function: tm1620_init
     * Description: tm1620 设备初始化
     * Input: 无
     * Return: 0: 成功  小于0: 失败
     * Others: 该函数必须在其他tm1620_manager的其它函数之前被调用
     */
    int32_t (*init)(void);

    /**
     * Function: tm1620_deinit
     * Description: tm1620 设备释放
     * Input: 无
     * Return: 无
     * Others: 不再使用设备应该释放
     */
    void (*deinit)(void);

    /**
     * Function: tm1620_all_display
     * Description: tm1620 控制所有数码管显示指定的数字
     * Input:
     *      display_data: 待显示数字的缓存区，缓存区的必须大于6bytes
     * Return: 0: 成功  小于0: 失败
     * Others: 此函数把display_data的数据传给tm1620，进而控制数码管的显示
     */
    int32_t (*all_display)(uint8_t *display_data);

    /**
     * Function: tm1620_grid_display
     * Description: tm1620 控制某一位数码管显示指定的数字
     * Input:
     *      grid_id: 指定控制哪一位数码管
     *      display_data: 待显示数字
     * Return: 0: 成功  小于0: 失败
     * Others: 此函数把display_data的数据传给tm1620，只刷新某一个数码管的显示
     */
    int32_t (*grid_display)(uint8_t grid_id, uint8_t display_data);

    /**
     * Function: tm1620_close_display
     * Description: tm1620 关闭所有数码管的显示
     * Input: 无
     * Return: 0: 成功  小于0: 失败
     * Others: 此函数只关闭数码管的显示，并不刷新tm1620的显示寄存器
     */
    int32_t (*close_display)(void);

    /**
     * Function: tm1620_open_display
     * Description: tm1620 打开所有数码管的显示
     * Input: 无
     * Return: 0: 成功  小于0: 失败
     * Others: 此函数打开数码管的显示，显示tm1620的显示寄存器的数据
     */
    int32_t (*open_display)(void);
};

/**
 * Function: get_tm1620_manager
 * Description: 获取tm1620_manager句柄
 * Input:  无
 * Output: 无
 * Return: 返回tm1620_manager结构体指针
 * Others: 通过该结构体指针访问tm1620_manager内部提供的方法
 */
struct tm1620_manager *get_tm1620_manager(void);

#endif
