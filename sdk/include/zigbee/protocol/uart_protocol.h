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
#ifndef _UART_PROTOCOL_H_
#define _UART_PROTOCOL_H_
#include <types.h>

#define UART_RECV_BUF_LEN        (128*8)
#define UART_PRO_BUF_LEN         128
#define UART_SEND_BUF_LEN        128        //It will double to malloc because of the need to reserve the translation char


/**
 *  应用模块传入串口协议层的资源，由协议模块接口申请和释放内存，应用模块不应该直接操作它
 */
typedef struct _rollbuf{
    uint16_t read;                          //底层串口数据接收环形缓冲区的读写指针
    uint16_t write;
    uint8_t* rbuf;                          //底层串口数据接收环形缓冲区
    uint16_t rbuf_len;

    uint8_t* pbuf;                          //数据解析后完整数据包的缓冲区
    uint16_t pbuf_len;

    uint8_t* sbuf;                          //发送前 数据包封装并转译后完整协议包的缓冲区
    uint16_t sbuf_len;
}pro_src_st;


typedef void (*uart_pro_recv_cb)(uint8_t cmd, uint8_t* const pl, uint16_t len);
typedef int16_t (*uart_pro_send_hd)(uint8_t* buf, uint16_t len);

struct uart_pro_manager{
    /**
     *  @brief   串口协议模块初始化，为src中的buf申请内存
     *
     *  @param   src - 应用模块的资源
     *
     *  @return  0 - 成功
     *           -1  失败
     */
    int16_t (*init)(pro_src_st* src);
    /**
     *  @brief   接收串口原生的数据到环形缓冲区，该接口在串口接收的服务函数中调用
     *
     *  @param   src - 应用模块的资源
     *  @param   buf - 串口接收到的数据
     *  @param   len - 串口数据长度
     *
     *  @return  已接收的数据长度
     */
    int16_t (*receive)(pro_src_st* src, uint8_t* buf, uint16_t len);
    /**
     *  @brief   应用模块数据封装成协议包后发送
     *
     *  @param   src - 应用模块的资源
     *  @param   cmd - 指令
     *  @param   pl  - 载荷
     *  @param   pllen - 载荷数据长度
     *  @param   hd  -  平台串口发送的接口，传入给串口协议模块调用，由应用模块封装后传入
     *
     *  @return  hd return value
     *
     */
    int16_t (*send)(pro_src_st* src, uint8_t cmd, uint8_t* pl, uint16_t pllen, uart_pro_send_hd hd);
    /**
     *  @brief   处理串口接收到的数据，在receive接口后调用，解析完整包后通过回调函数通知
     *           循环逻辑操作，最好不要直接在串口的中断服务函数中调用而是通过消息处理
     *
     *  @param   src - 应用模块的资源
     *  @param   cb - 应用的回调函数
     *
     */
    void (*handle)(pro_src_st* src, uart_pro_recv_cb cb);
    /**
     *  @brief   释放应用资源
     *
     *  @param   src - 应用模块的资源
     *
     */
    void (*deinit)(pro_src_st* src);
};


struct uart_pro_manager* get_uart_pro_manager(void) ;


#endif
