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
#ifndef EFUSE_MANAGER_H
#define EFUSE_MANAGER_H
#include <types.h>
/*
 * 一下几个宏是定义EFUSE各个段的长度，以芯片手册为准，这里以字节为单位
 */
#define CHIP_ID_LENGTH          16
#define RANDOM_ID_LENGTH        16
#define USER_ID_LENGTH          32
#define PROTECT_ID_LENGTH       4

/*
 * 定义EFUSE各个段的id, 不能修改任何一个enum efuse_segment 成员的值
 */
enum efuse_segment {
    CHIP_ID,
    RANDOM_ID,
    USER_ID,
    PROTECT_ID,
    SEGMENT_END,
};

struct efuse_manager {
    /**
     *      Function: read
     *      Description: 读EFUSE指定的段
     *      Input:
     *          seg_id: 读取EFUSE段的id
     *             buf: 存储读取数据的缓存区指针
     *          length: 读取的长度，以字节为单位
     *      Return: -1 --> 失败, 0 --> 成功
     */
    int32_t (*read)(enum efuse_segment seg_id, uint32_t *buf, uint32_t length);

     /**
     *      Function: write
     *      Description: 写数据到指定的EFUSE段
     *      Input:
     *          seg_id: 写EFUSE目标段的id
     *             buf: 存储待写入数据的缓存区指针
     *          length: 写入的长度，以字节为单位
     *      Return: -1 --> 失败, 0 --> 成功
     */
    int32_t (*write)(enum efuse_segment seg_id, uint32_t *buf, uint32_t length);
};

/**
 *      Function: get_efuse_manager
 *      Description: 获取 efuse_manager 句柄
 *      Input: 无
 *      Return: 返回 struct efuse_manager 结构体指针
 *      Others: 通过该结构体指针访问内部提供的方法
 */
struct efuse_manager *get_efuse_manager(void);

#endif /* EFUSE_MANAGER_H */
