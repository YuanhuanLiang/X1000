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
#ifndef _ZIGBEE_MANAGER_H_
#define _ZIGBEE_MANAGER_H_
#include <types.h>

#define SEND_STA_IDLE                   1
#define SEND_STA_SUCCESS                0
#define SEND_STA_FAIL                   -1

#define UART_CMD_CTRL                   0x00
#define UART_CMD_CFG                    0x01

#define UART_CMD_DATA_REPORT            0x40
#define UART_CMD_STATUS_REPORT          0X41
#define UART_CMD_INFO_RSP               0x42

#define GET_ZIGBEE_INFO(zb_info, info)                                                  \
        do{                                                                             \
            zb_info.role          = info[0];                                            \
            zb_info.panid         = (uint16_t)(((info[1] << 8)&0xFF00) | info[2]);      \
            zb_info.short_addr    = (uint16_t)(((info[3] << 8)&0xFF00) | info[4]);      \
            memcpy(zb_info.channel, &info[5], sizeof(zb_info.channel));                 \
            memcpy(zb_info.key, &info[9], sizeof(zb_info.key));                         \
            zb_info.join_aging    = info[25];                                           \
            zb_info.group_id      = (uint16_t)(((info[26] << 8)&0xFF00) | info[27]);    \
            zb_info.send_type     = info[28];                                           \
            zb_info.send_addr     = (uint16_t)(((info[29] << 8)&0xFF00) | info[30]);    \
            zb_info.send_group_id = (uint16_t)(((info[31] << 8)&0xFF00) | info[32]);    \
            zb_info.poll_rate     = (uint16_t)(((info[33] << 8)&0xFF00) | info[34]);    \
            zb_info.tx_power      = info[35];                                           \
        }while(0)

#define GET_NWK_STATUS(id, status, pl)              \
        do{                                         \
            id = ((pl[0]<<8)&0xFF00)|pl[1];         \
            status = pl[2];                         \
        }while(0)

struct zb_info_t{
    uint8_t send_type;
    uint8_t role;
    uint8_t join_aging;
    uint8_t key[16];
    uint8_t channel[4];
    uint8_t tx_power;
    uint16_t panid;
    uint16_t group_id;
    uint16_t send_addr;
    uint16_t send_group_id;
    uint16_t short_addr;
    uint16_t poll_rate;
};

struct zb_uart_cfg_t{
    char* devname;
    uint32_t baudrate;
    uint8_t date_bits;
    uint8_t parity;
    uint8_t stop_bits;
};


enum {
    DEV_STATUS_NONE = 0,
    DEV_STATUS_POWER_ON,
    DEV_STATUS_NWK_CREATED,
    DEV_STATUS_NWK_JOIN,
    DEV_STATUS_NWK_LEAVE
};

/**
 *  @brief   处理解析到完整数据包后的回调函数，由用户编写并在调用init（）时传入
 *
 *  @param   cmd - 接收到的命令字
 *           pl  - 接收到的载荷数据
 *           len - 接收到的数据长度
 *
 *  @note    该接口回调时为子线程上下文，操作全局变量需注意线程安全问题
 */
typedef void (*uart_zigbee_recv_cb)(uint8_t cmd, uint8_t* const pl, uint16_t len);


struct uart_zigbee_manager{
    /**
     *	@brief  初始化zigbee功能模块，内部会初始化串口和串口协议模块，使用前必须先初始化
     *
     *  @param   处理解析到完整数据包后的回调函数，由用户编写并传入
     *
     *  @return   0 - 成功
     *           -1 - 失败
     */
    int (*init)(uart_zigbee_recv_cb recv_cb, struct zb_uart_cfg_t uart_cfg);
    /**
     *  @brief  硬件复位
     *
     *
     *  @return  0  发送成功
     *           -1 - 操作失败
     */
    int (*reset)(void);
    /**
     *  @brief   控制数据透明传输，阻塞发送，关心返回值
     *
     *  @param   pl - 控制数据的载荷部分
     *  @param   len - 载荷数据长度
     *
     *  @return   0  发送成功
     *           －1 失败
     */
    int (*ctrl)(uint8_t* pl, uint16_t len);
    /**
     *  @brief   获取设备当前配置信息，成功后设备将上报配置信息，由接收回调接收，阻塞发送，关心返回值
     *
     *
     *  @return   0 发送成功
     *           －1  失败
     */
    int (*get_info)(void);
    /**
     *  @brief   zigbee设备参数配置　均为阻塞发送，关心返回值
     *  @detail:
     *          factory : 恢复出厂配置
     *          reboot: 软件重启设备
     *          set_role: 设置zigbee设备的角色：00 协调器 01 路由器 02 终端节点
     *          set_panid: 设置zigbee设备的pan id 指定个域网ID进行网络创建（协调器）或加入（节点） 0x0001~0xFFFF (0xFFFF: 自动随机)
     *          set_channel:设置zigbee设备工作的信道 0x0B~0x1A
     *          set_key: aes加密的密钥，16 bytes, len 固定 16
     *          set_join_aging: 协调器和路由器状态下，允许设备加入网络的时限. 0x00~0xFF， 0为不可加入，0xFF为永久可加入
     *          set_cast_type: 数据发送类型，00 广播、01 点播、02 组播  并指定16bit的发送目的地址，addr -- 广播：0xFFFF 组播：group id
     *          set_group_id: 设置设备加入本地的组，用于接收相对应的组播数据，同时只加入一个组
     *          set_poll_rate: 设置睡眠唤醒请求数据周期，配置功耗的关键参数，只对终端有效，协调器和路由器不睡眠
     *          set_tx_power: 设置zigbee模块发射功率
     *
     *  @return  0 - 成功
     *          <0 - 失败
     *  @Note:
     *          配置后zigbee设备软重启并使配置生效: factory \ reboot \ set_role \ set_panid \ set_channel \set_key \ set_poll_rate\set_tx_power
     *          配置后zigbee设备限时生效不会重启 : set_cast_type \ set_group_id \ set_join_aging
     *          需要重启的配置需要等待设备重启后方能响应操作
     */
    int (*factory)(void);
    int (*reboot)(void);
    int (*set_role)(uint8_t role);
    int (*set_panid)(uint16_t panid);
    int (*set_channel)(uint8_t channel);
    int (*set_key)(uint8_t* key, uint8_t keylen);
    int (*set_join_aging)(uint8_t aging);
    int (*set_cast_type)(uint8_t type, uint16_t addr);
    int (*set_group_id)(uint16_t id);
    int (*set_poll_rate)(uint16_t rate);
    int (*set_tx_power)(int8_t power);

    /**
     *  @brief   释放模块
     */
    int (*deinit)(void);
};


struct uart_zigbee_manager* get_zigbee_manager(void) ;

#endif
