/*
 * include/linux/cypress_psoc4.h
 *
 * Cypress PSoC4 4000S family driver.
 * This driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef __CYPRESS_PSOC4_H
#define __CYPRESS_PSOC4_H


/**
 * I2C message type define
 */
#define MSG_PACKET_MAX_LEN              (128)    /* i2c packet length */
#define MSG_PACKET_SOP                  (0x00)   /* start of packet marker */
#define MSG_PACKET_EOP                  (0xff)   /* end of packet marker */
#define MSG_TYPE_MASTER_G_PACKET        (0xfe)   /* master gets message packet from slave */
#define MSG_TYPE_MASTER_S_SUSPEND       (0xfd)   /* master orders slave into suspend */
#define MSG_TYPE_MASTER_S_RESUME        (0xfc)   /* master resumes slave from suspend */
#define MSG_TYPE_SLAVE_S_KEYCODE        (0xfb)   /* slave sends keycode to master */
#define MSG_TYPE_SLAVE_S_PM_STATE       (0xfa)   /* slave sends pm state to master */
#define MSG_TYPE_MASTER_G_PM_STATE      (0xf9)   /* master gets slave pm state */
#define MSG_TYPE_SLAVE_S_CARD_UID       (0xf8)   /* slave sends card uid to master */
#define MSG_TYPE_MASTER_G_CARD_UID      (0xf7)   /* master gets card uid from slave */
#define MSG_TYPE_SLAVE_S_SECTOR_DATA    (0xf6)   /* slave sends card sector data to master */
#define MSG_TYPE_MASTER_G_SECTOR_DATA   (0xf5)   /* master gets card sector data from slave */
#define MSG_TYPE_MASTER_S_ACK           (0xf4)   /* master sends ACK to slave */
#define MSG_TYPE_SLAVE_S_ACK            (0xf3)   /* slave sends ACK to master */


struct pm_t {
    unsigned char type;
    unsigned char state;
    unsigned char eof;
};

struct ack_t {
    unsigned char type;
    unsigned char eof;
};

struct key_t {
    unsigned char type;
    unsigned char code;
    unsigned char value;
    unsigned char eof;
};

struct card_t {
    unsigned char type;
    unsigned char cardtype;
    unsigned char uid[10];
    unsigned char sector;
    unsigned char keytype;
    unsigned char m1keyA[6];
    unsigned char m1keyB[6];
    unsigned char data[64];
    unsigned char eof;
};

union cypress_psoc4_i2c_msg {
    unsigned char buf[MSG_PACKET_MAX_LEN];
    struct pm_t pm;
    struct ack_t ack;
    struct key_t key;
    struct card_t card;
};

struct cypress_psoc4_platform_data {
#ifdef CONFIG_CYPRESS_PSOC4_SYSFS
    char *name;
#endif

    unsigned int cpu_int_pin; /* IRQ signal from MCU to CPU */
    unsigned int mcu_int_pin; /* IRQ signal from CPU to MCU */
    unsigned int mcu_rst_pin; /* RST signal from CPU to MCU */

#define RST_LOW_EN       0
#define RST_HIGH_EN      1
    unsigned char mcu_rst_level;
    unsigned char mcu_int_level;

    unsigned char *keyscode;
    unsigned char keyscode_num;
};

#endif
