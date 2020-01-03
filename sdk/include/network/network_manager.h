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
#ifndef _NETWORK_MANAGER_H
#define _NETWORK_MANAGER_H

/**
 * 定义ioctl的操作对象，此结构不要修改
 */
enum info_t {
    IP_ADDR,
    NET_MASK,
    BRD_ADDR,
    MAC_ADDR,
    IF_STATE,
};

/**
 * 定义接口的状态，此结构不要修改
 */
enum if_state {
    IF_DOWN,
    IF_UP,
    IF_RUNNING,  //Interface up and the cable is connected
};


struct network_manager {
    /**
     * Function: network_if_init
     * Description: 网络接口初始化
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     * Return: 0: 成功  小于0: 失败
     * Others: 该函数必须在network_manager的其它函数之前被调用
     */
    int32_t (*init)(const char *ifname);

    /**
     * Function: network_if_deinit
     * Description: 网络接口释放
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     * Return: 无
     * Others: 不再操作网络接口时调用此函数释放
     */
    void (*deinit)(const char *ifname);

    /**
     * Function: get_if_state
     * Description: 获取网络接口状态
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     * Return: 小于0: 失败，
     *         IF_UP: 接口启动
     *         IF_DOWN: 接口掉电
     *         IF_RUNNING: 接口上电并且网线插入
     * Others: 无
     */
    int32_t (*get_if_state)(const char *ifname);

    /**
     * Function: set_if_state
     * Description: 设置网络接口状态
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     *      state: IF_UP 或 IF_DOWN
     * Return: 0: 成功  小于0: 失败
     * Others: 此函数用于设置网络接口 up 或 down， 类似: ifconfig xxx up/down
     */
    int32_t (*set_if_state)(const char *ifname, enum if_state state);

    /**
     * Function: get_if_ipaddr
     * Description: 获取网络接口的ip地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     *      ip: 存放ip地址的缓存指针
     *      mode: 获取模式，有如下模式：
     *              0: 网络字节序的ip地址
     *              1: 点分十进制字符串的ip地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 ip 指向的缓存区足够大
     */
    int32_t (*get_if_ipaddr)(const char *ifname, uint8_t *ip, int mode);

    /**
     * Function: set_if_ipaddr
     * Description: 设置网络接口的ip地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     *      ip: 点分十进制字符串的ip地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 ip 指定的地址正确
     */
    int32_t (*set_if_ipaddr)(const char *ifname, char *ip);

    /**
     * Function: get_if_netmask
     * Description: 获取网络接口的ip掩码地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     *      mask: 存放ip掩码地址的缓存指针
     *      mode: 获取模式，有如下模式：
     *              0: 网络字节序的mask地址
     *              1: 点分十进制字符串的mask地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 mask 指向的缓存区足够大
     */
    int32_t (*get_if_netmask)(const char *ifname, uint8_t *mask, int mode);

    /**
     * Function: set_if_netmask
     * Description: 设置网络接口的ip掩码地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     *      mask: 点分十进制字符串的mask地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 mask 指定的mask地址正确
     */
    int32_t (*set_if_netmask)(const char *ifname, char *mask);

    /**
     * Function: get_if_bcast
     * Description: 获取网络接口的广播地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     *      brd: 存放广播地址的缓存指针
     *      mode: 获取模式，有如下模式：
     *              0: 网络字节序的bcast地址
     *              1: 点分十进制字符串的bcast地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 bcast 指向的缓存区足够大
     */
    int32_t (*get_if_bcast)(const char *ifname, uint8_t *bcast, int mode);

    /**
     * Function: set_if_bcast
     * Description: 设置网络接口的广播地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     *      brd: 点分十进制字符串的bcast地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 bcast 指定的广播地址正确
     */
    int32_t (*set_if_bcast)(const char *ifname, char *bcast);

    /**
     * Function: get_if_hwaddr
     * Description: 获取网络接口的mac地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     *      mac: 存放ip地址的缓存指针
     *      mode: 获取模式，有如下模式：
     *              0: 网络字节序的mac地址
     *              1: aa:bb:cc:dd:ee:ff 形式的mac地址
     *              2: aa-bb-cc-dd-ee-ff 形式的mac地址
     *              3: aabbccddeeff 形式的mac地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 mac 指向的缓存区足够大
     */
    int32_t (*get_if_hwaddr)(const char *ifname, uint8_t *mac, int mode);

    /**
     * Function: dhcpc_request_ip
     * Description: 运行 dhcpc 动态获取 ip 和 dns 地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证所连接的网络正常
     */
    int32_t (*dhcpc_request_ip)(const char *ifname);

    /**
     * Function: dhcpc_request_ip_cancel
     * Description: 在 dhcpc 还没获取到 ip 前，取消获取
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     * Return: 无
     * Others: 在dhcpc长时间没能获取到ip或在系统准备进休眠前调用此函数取消dhcpc进程
     */
    void (*dhcpc_request_ip_cancel)(const char *ifname);

    /**
     * Function: dhcpc_release_ip
     * Description: 运行 dhcpc 释放 ip, 这样 dhcpd 才能把这个 ip 分配给其他设备
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     * Return: 无
     * Others: 在设备进入深度休眠时可以调用此函数释放ip
     */
    void (*dhcpc_release_ip)(const char *ifname);
};

/**
 * Function: get_network_manager
 * Description: 获取network_manager句柄
 * Input:  无
 * Output: 无
 * Return: 返回network_manager结构体指针
 * Others: 通过该结构体指针访问network_manager内部提供的方法
 */
struct network_manager *get_network_manager(void);

#endif
