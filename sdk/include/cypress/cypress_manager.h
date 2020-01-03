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
#ifndef _CYPRESS_MANAGER_H
#define _CYPRESS_MANAGER_H

/**
 * ioctl commands
 */
#define CYPRESS_IOC_MAGIC   'Y'
#define CYPRESS_DRV_IOC_R_CARD_UID       _IOR(CYPRESS_IOC_MAGIC, 0, int)
#define CYPRESS_DRV_IOC_R_CARD_SECTOR    _IOR(CYPRESS_IOC_MAGIC, 1, int)
#define CYPRESS_DRV_IOC_SEND_ACK         _IOW(CYPRESS_IOC_MAGIC, 2, int)
#define CYPRESS_DRV_IOC_RESET_MCU        _IOW(CYPRESS_IOC_MAGIC, 3, int)
#define CYPRESS_DRV_IOC_ENABLE_IRQ       _IOW(CYPRESS_IOC_MAGIC, 4, int)
#define CYPRESS_DRV_IOC_DISABLE_IRQ      _IOW(CYPRESS_IOC_MAGIC, 5, int)
#define CYPRESS_DRV_IOC_WAIT_R_CARD      _IOR(CYPRESS_IOC_MAGIC, 6, int)

/**
 * input->name
 */
#define CYPRESS_INPUT_DEV_NAME   "cypress_psoc4"

/**
 * M1卡的密码类型，此宏值不要修改
 */
#define M1_AUTH_KEYA    0x60
#define M1_AUTH_KEYB    0x61

/**
 * 定义卡的类型，此结构不要修改
 */
enum card_type {
    CARD_TYPE_A = 0,
    CARD_TYPE_B = 1,
    CARD_MIFARE1 = 2,
};


/**
 * 卡操作的数据包结构体，此结果不要修改
 */
struct card_t {
    uint8_t type;
    uint8_t cardtype;          //IC卡类型: TYPE_A、TYPE_B、MIFARE1
    uint8_t uid[10];           //IC卡UID寄存器
    uint8_t sector;            //data对应的M1卡扇区号
    uint8_t keytype;           //M1卡秘密类型: A/B
    uint8_t m1keyA[6];         //M1卡密码A
    uint8_t m1keyB[6];         //M1卡密码B
    uint8_t data[64];          //读M1卡数据缓存区, 大小为M1卡的一个扇区
    uint8_t eof;               //数据包结束标志
};


/**
 * 处理读卡上报事件的回调函数指针类型
 */
typedef void (*deal_card_report_handler)(int dev_fd);


struct cypress_manager {
    /**
     * Function: cypress_init
     * Description: cypress 设备初始化
     * Input:
     *      card_handler: 处理读卡上报事件和回调函数
     * Return: 0: 成功  小于0: 失败
     * Others: 该函数必须在其他cypress_manager的其它函数之前被调用
     */
    int32_t (*init)(deal_card_report_handler card_handler);

    /**
     * Function: cypress_deinit
     * Description: cypress 设备释放
     * Input:
     * Return: 无
     * Others: 不再使用设备应该释放
     */
    void (*deinit)(void);

     /**
     * Function: cypress_mcu_reset
     * Description: 复位 cypress 设备
     * Input:
     * Return: 无
     * Others: 无
     */
    void (*mcu_reset)(void);
};

/**
 * Function: get_cypress_manager
 * Description: 获取cypress_manager句柄
 * Input:  无
 * Output: 无
 * Return: 返回cypress_manager结构体指针
 * Others: 通过该结构体指针访问cypress_manager内部提供的方法
 */
struct cypress_manager *get_cypress_manager(void);

#endif /* _CYPRESS_MANAGER_H */
