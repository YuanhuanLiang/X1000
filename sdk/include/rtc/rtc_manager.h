/*
 *  Copyright (C) 2016, Xin Shuan <shuan.xin@ingenic.com>
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
#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H
#include <types.h>
#include <linux/rtc.h>

enum {
    JZ_HIBERNATE_WKUP_APD = 0x01,
    JZ_HIBERNATE_WKUP_PPR,
    JZ_HIBERNATE_WKUP_ALM,
    JZ_HIBERNATE_WKUP_PIN,
    JZ_HIBERNATE_WKUP_UNKNOWN = 0xff
};

/*
 * 用于设置RTC时间的结构体:
 * struct rtc_time {
 *   int tm_sec;             秒 – 取值区间为[0,59]
 *   int tm_min;            分 - 取值区间为[0,59]
 *   int tm_hour;           时 - 取值区间为[0,23]
 *   int tm_mday;         一个月中的日期 - 取值区间为[1,31]
 *   int tm_mon;           月份（从一月开始，0代表一月） - 取值区间为[0,11]
 *   int tm_year;           年份，其值等于实际年份减去1900
 *   int tm_wday;          星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推
 *   int tm_yday;           从每年的1月1日开始的天数 – 取值区间为[0,365]，
 *                                   其中0代表1月1日，1代表1月2日，以此类推
 *   int tm_isdst;           夏令时标识符，实行夏令时的时候，tm_isdst为正。
 *                                   不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负。
 * };
 */

struct rtc_manager {
    int (*init)(void);

    int (*deinit)(void);

    /**
     *      Function: read
     *      Description: 读RTC时钟
     *      Input:
     *          time: 读取时间的结构体
     *      注意：time是struct rtc_time结构体指针
     *      Return: -1 --> 失败, 0 --> 成功
     */
    int32_t (*get_rtc)(struct rtc_time *time);

     /**
     *      Function: write
     *      Description: 写RTC时钟
     *      Input:
     *          time: 需要写入的时间
     *      注意：time是struct rtc_time结构体指针
     *      Return: -1 --> 失败, 0 --> 成功
     */
    int32_t (*set_rtc)(const struct rtc_time *time);

    int32_t (*get_hibernate_wakeup_status)(void);

    int32_t (*set_bootup_alarm)(struct rtc_time* time);
};

/**
 *      Function: get_rtc_manager
 *      Description: 获取 rtc_manager 句柄
 *      Input: 无
 *      Return: 返回 struct rtc_manager 结构体指针
 *      Others: 通过该结构体指针访问内部提供的方法
 */
struct rtc_manager *get_rtc_manager(void);


#endif /* RTC_MANAGER_H */
