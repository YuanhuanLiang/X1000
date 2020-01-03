/**
 ****************************************************************
 * @file mifare.h
 *
 * @brief 
 *
 * @author 
 *
 * 
 ****************************************************************
 */ 
#ifndef MIFARE_H
#define MIFARE_H
#include "sl2523.h"


#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_RESET            0xE0               //复位


//UltraLight
#define PICC_WRITE_ULTRALIGHT 0xA2                //超轻卡写块

                                  
char pcd_auth_state(u8 auth_mode,u8 block,u8 *psnr, u8 *pkey);      
char pcd_read(u8 addr,u8 *preaddata);                       
char pcd_write(u8 addr,u8 *pwritedata);
char pcd_write_ultralight(u8 addr,u8 *pwritedata);

#endif