/*
 *  Copyright (C) 2016, Zhang YanMing <yanmin.zhang@ingenic.com, jamincheung@126.com>
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

#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H
#include <types.h>
/*
 * AES128
 */
#define AES_KEY_128BIT      (0x00000000)

/*
 * AES192
 */
#define AES_KEY_192BIT      (0x00000001)

/*
 * AES256
 */
#define AES_KEY_256BIT      (0x00000002)

#define AES_MODE_NONE       (0x00000000)

/*
 * AES ECB模式
 */
#define AES_MODE_ECB        (0x00000001)

/*
 * AES CBC模式
 */
#define AES_MODE_CBC        (0x00000002)

struct aes_key {
    /*
     * AES key
     */
    uint8_t *key;

    /*
     * AES key长度
     */
    uint32_t keylen;

    /*
     * AES key模式
     * AES128/AES192/AES256
     */
    uint32_t bitmode;

    /*
     * AES工作模式
     * ECB/CBC
     */
    uint32_t aesmode;

    /*
     * AES工作模式为CBC时IV
     */
    uint8_t *iv;

    /*
     * AES工作模式为CBC时IV的长度
     */
    uint32_t ivlen;
};

struct aes_data {
    /*
     * AES 输入数据
     */
    uint8_t *input;

    /*
     * AES 输入数据长度
     */
    uint32_t input_len;

    /*
     * AES 输出数据
     */
    uint8_t *output;

    /*
     * 加密: 0
     * 解密: 1
     */
    int32_t encrypt;
};

struct security_manager {
    /**
     * Function: security_init
     * Description: security模块初始化
     * Input:无
     * Output: 无
     * Return: 0: 成功， -1: 失败
     * Others: 无
     */
    int32_t (*init)(void);

    /**
     * Function: security_deinit
     * Description: security模块释放
     * Input:无
     * Output: 无
     * Return: 无
     * Others: 无
     */
    void (*deinit)(void);

    /**
     * Function: simple_aes_load_key
     * Description: 加载AES key
     * Input:
     *      aes_key: AES key
     * Output: 无
     * Return: 0: 成功  -1: 失败
     * Others: 无
     */
    int32_t (*simple_aes_load_key)(struct aes_key* aes_key);

    /**
     * Function: simple_aes_crypt
     * Description: AES加/解密
     * Input:
     *      aes_data: AES data
     * Output: 无
     * Return: 0: 成功  -1: 失败
     * Others: 无
     */
    int32_t (*simple_aes_crypt)(struct aes_data* aes_data);
};

/**
 * Function: get_security_manager
 * Description: 获取security_manager句柄
 * Input:  无
 * Output: 无
 * Return: 返回security_manager结构体指针
 * Others: 通过该结构体指针访问security_manager内部提供的方法
 */
struct security_manager* get_security_manager(void);

#endif /* SECURITY_MANAGER_H */
