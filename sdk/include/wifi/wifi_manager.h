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

#ifndef _WIFI_MANAGER_H
#define _WIFI_MANAGER_H

#include <network/network_manager.h>


enum wifi_state {
    UNKNOW_STATE,          //未知状态
    COMPLETED,             //连接完成
    INTERFACE_DISABLED,    //接口被关闭
    DISCONNECTED,          //断开
    ASSOCIATING,           //尝试连接中
    SCANNING,              //扫描
    INACTIVE,              //所有配置项未激活
    WAY_HANDSHAKE,         //认证状态
};

enum func_ret {
    FUNC_SUCCESS    =  0,
    FUNC_FAILED     = -1,
    PARAM_INVALID   = -2,
    SSID_NOEXIST    = -3,
    WPA_CMD_FAILED  = FUNC_FAILED,
    REQUEST_TIMEOUT = -4,
};

enum encryption {
    OPEN = 0,
    WEP  = 1,
    WPA_PSK = 2,
    WPA2_PSK = 4,
    WPA_WPA2_PSK = WPA_PSK|WPA2_PSK,
};

struct ap_info {
    uint8_t bssid[18];
    int32_t signal_level;
    enum encryption encryption;
};

#define MAX_AP_INFO    6 //最大同名AP信息的个数
struct ssid_info {
    char ssid[33];       //AP名字，需要先指定
    uint8_t count;       //当前环境同名AP个数
    struct ap_info ap[MAX_AP_INFO];
};

struct wifi_manager {
    /**
     * Function: wifi_manager_init
     * Description: wifi管理接口初始化
     * Input: 无
     * Return: 0: 成功  小于0: 失败
     * Others: 该函数必须在wifi_manager的其它函数之前被调用
     */
    int32_t (*init)(void);

    /**
     * Function: wifi_manager_deinit
     * Description: wifi管理接口释放
     * Input: 无
     * Return: 无
     * Others: 不再需要管理wifi网络时调用此函数释放
     */
    void (*deinit)(void);

    /**
     * Function: wifi_set_if_state
     * Description: 设置wifi接口的状态, 通常是wlan0
     * Input:
     *      state: IF_UP 或 IF_DOWN
     * Return: 0: 成功  小于0: 失败
     * Others: 此函数用于设置wifi接口 up 或 down， 类似: ifconfig xxx up/down
     */
    int32_t (*set_if_state)(enum if_state);
    /**
     * Function: wifi_get_if_state
     * Description: 设置wifi接口的状态, 通常是wlan0
     * Return: 0: 成功  小于0: 失败
     * Others: 此函数用于获取wifi接口 up 或 down， 类似: ifconfig xxx up/down
     */
    int32_t (*get_if_state)(void);
    /**
     * Function: wifi_get_hwaddr
     * Description: 获取wifi的mac地址
     * Input:
     *      mac: 存放ip地址的缓存指针
     *      mode: 获取模式，有如下模式：
     *              0: 网络字节序的mac地址
     *              1: aa:bb:cc:dd:ee:ff 形式的mac地址
     *              2: aa-bb-cc-dd-ee-ff 形式的mac地址
     *              3: aabbccddeeff 形式的mac地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 mac 指向的缓存区足够大
     */
    int32_t (*get_hwaddr)(uint8_t *mac, int mode);

    /**
     * Function: wifi_get_ipaddr
     * Description: 获取wifi的ip地址
     * Input:
     *      ip: 存放ip地址的缓存指针
     *      mode: 获取模式，有如下模式：
     *              0: 网络字节序的ip地址
     *              1: 点分十进制字符串的ip地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 ip 指向的缓存区足够大
     */
    int32_t (*get_ipaddr)(uint8_t *ip, int mode);
    /**
     * Function: wifi_set_ipaddr
     * Description: 设置wifi的ip地址
     * Input:
     *      ip: 存放ip地址的缓存指针
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 ip 指向的缓存区足够大
     */
    int32_t (*set_ipaddr)(uint8_t *ip);
    /**
     * Function: wifi_get_netmask
     * Description: 获取网络的掩码地址
     * Input:
     *      mask: 存放ip掩码地址的缓存指针
     *      mode: 获取模式，有如下模式：
     *              0: 网络字节序的ip地址
     *              1: 点分十进制字符串的ip地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 mask 指向的缓存区足够大
     */
    int32_t (*get_netmask)(uint8_t *mask, int mode);
    /**
     * Function: wifi_set_netmask
     * Description: 设置网络的掩码地址
     * Input:
     *      mask: 存放ip掩码地址的缓存指针
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 mask 指向的缓存区足够大
     */
    int32_t (*set_netmask)(uint8_t *mask);

    /**
     * Function: wifi_get_bcast
     * Description: 获取wifi的广播地址
     * Input:
     *      brd: 存放广播地址的缓存指针
     *      mode: 获取模式，有如下模式：
     *              0: 网络字节序的ip地址
     *              1: 点分十进制字符串的ip地址
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 brd 指向的缓存区足够大
     */
    int32_t (*get_bcast)(uint8_t *brd, int mode);
   /**
     * Function: wifi_set_bcast
     * Description: 获取wifi的广播地址
     * Input:
     *      brd: 存放广播地址的缓存指针
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证 brd 指向的缓存区足够大
     */
    int32_t (*set_bcast)(uint8_t *brd);

    /**
     * Function: wifi_get_defaut_ssid_name
     * Description: 获取/etc/wpa_supplicant.conf中设定的最后一次wifi网络名
     * Input:
     *      ssid: 保存获取ssid名的缓存区
     * Return: network id: 成功  小于0: 失败
     * Others: 注意保证参数ssid指向的空间足够大
     */
    int32_t (*get_defaut_ssid_name)(char *ssid);

    /**
     * Function: wifi_get_ssid_info
     * Description: 获取当前环境指定的ssid信息（信号强度、加密方式、bssid）
     * Input:
     *      info: 保存获取的信息，info->ssid 必须先指定
     * Return: >0: 获取同名的ssid个数  小于0: 失败
     * Others: 如果返回的info->count > 0, 说明至少获取到一个设备的信息
     */
    int32_t (*get_ssid_info)(struct ssid_info *info);

    /**
     * Function: wifi_check_ssid_is_exist
     * Description: 检测当前环境是否存在指定的wifi网络名
     * Input:
     *      ssid: 指定的ssid名
     * Return: 0: 成功  小于0: 失败
     * Others: 无
     */
    int32_t (*check_ssid_is_exist)(const char *ssid);

    /**
     * Function: wifi_connect_to_defaut_ssid
     * Description: 连接到/etc/wpa_supplicant.conf设定的最后一次wifi网络
     * Input: 无
     * Return: 0: 成功  小于0: 失败
     * Others: 默认连接/etc/wpa_supplicant.conf的最后一次网络, 请保证这个网络正常,
     * 如果这个网络不可用, 请调用wifi_connect_to_new_ssid()函数连接到新的网络
     */
    int32_t (*connect_to_defaut_ssid)(void);

    /**
     * Function: wifi_dhcpc_request_ip
     * Description: 运行 dhcpc 动态获取 ip 和 dns 地址
     * Input:
     *      ifname: 接口名，如: wlan0 或 eth0 等
     * Return: 0: 成功  小于0: 失败
     * Others: 注意保证所连接的网络正常
     */
    int32_t (*dhcpc_request_ip)(void);

    /**
     * Function: wifi_dhcpc_request_ip_cancel
     * Description: 在 dhcpc 还没获取到 ip 前，取消获取
     * Input:
     * Return: 无
     * Others: 在dhcpc长时间没能获取到ip或在系统准备进休眠前调用此函数取消dhcpc进程
     */
    void (*dhcpc_request_ip_cancel)(void);

    /**
     * Function: wifi_dhcpc_release_ip
     * Description: 运行 dhcpc 释放 ip, 这样 dhcpd 才能把这个 ip 分配给其他设备
     * Input:
     * Return: 无
     * Others: 在设备进入深度休眠时可以调用此函数释放ip
     */
    void (*dhcpc_release_ip)(void);

    /**
     * Function: wifi_connect_to_new_ssid
     * Description: 连接到指定的ssid, 并把ssid和passwd保存到/etc/wpa_supplicant.conf
     * Input:
     *      ssid: wifi的名称
     *      passwd: wifi的密码
     * Return: 0: 成功  小于0: 失败
     * Others: 连接新的wifi网络成功后, 此wifi的名称和密码被保存到/etc/wpa_supplicant.conf
     */
    int32_t (*connect_to_new_ssid)(const char *ssid, const char *passwd);

    /**
     * Function: wifi_disconnect_and_suspend
     * Description: 断开wifi连接并关掉wifi接口(类似ifconfig wlan0 down)
     * Input: 无
     * Return: 0: 成功  小于0: 失败
     * Others: 这个函数在系统进入休眠时应该调用
     */
    int32_t (*disconnect_and_suspend)(void);

    /**
     * Function: wifi_resume_and_reconnect
     * Description: 打开wifi接口(类似ifconfig wlan0 up), 并重新连接wifi
     * Input: 无
     * Return: 0: 成功  小于0: 失败
     * Others: 这个函数应在系统被唤醒后调用
     */
    int32_t (*resume_and_reconnect)(void);

    /**
     * Function: wifi_get_connect_state
     * Description: 获取wifi连接的状态
     * Input: 无
     * Return: 0: 成功  小于0: 失败
     * Others: 此函数用于获取当前wifi的连接状态
     */
    enum wifi_state (*get_connect_state)(void);
};

/**
 * Function: get_wifi_manager
 * Description: 获取wifi_manager句柄
 * Input:  无
 * Output: 无
 * Return: 返回wifi_manager结构体指针
 * Others: 通过该结构体指针访问wifi_manager内部提供的方法
 */
struct wifi_manager *get_wifi_manager(void);

#endif /* _WIFI_MANAGER_H */
