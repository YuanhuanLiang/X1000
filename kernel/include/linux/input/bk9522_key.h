#ifndef __LINUX_BK9522_KEY_H__
#define __LINUX_BK9522_KEY_H__

#define BK9522_NAME                     "bk9522"
#define BK9522_I2C_ADDR                 0x22

#define BK9522_SWITCH_NAME              "wl_mic"

#define BASE_USER_ID                    0x900
#define BASE_FREQ_START                 760000
#define BASE_FREQ_END                   769000

#define IOC_BK9522_MAGIC                'K'
#define BK9522_PAIR                     _IO(IOC_BK9522_MAGIC, 80)
#define BK9522_START                    _IO(IOC_BK9522_MAGIC, 81)
#define BK9522_STOP                     _IO(IOC_BK9522_MAGIC, 82)

#define BK9522_REG_DIV_FREQ             0x03
#define BK9522_REG_FUNCTION0            0x04
#define BK9522_REG_LDO             0x07
#define BK9522_REG_PLL             0x08
#define BK9522_REG_ID                   0x10
#define BK9522_REG_TEMP_DRIFT           0x15
#define BK9522_REG_FIFO                 0x16
#define BK9522_REG_DATA                 0x19
#define BK9522_REG_RESET                 0x1A
#define BK9522_REG_EQ_CONTROL           0x1C
#define BK9522_REG_RECEIVE_CONTROL      0x32
#define BK9522_REG_CHANNEL_FREQ         0x1B
#define BK9522_REG_MATCH_ID             0x34

#define CHECK_ID_COUNT                  10

#define WORK_DELAY_TIME                 20
/* more than the DISCONNECT_COUNT_MAX will change user bank*/
#define DISCONNECT_COUNT_MAX                30
/* more than the FIFO_DATA_THRESHOLD will restart receive*/
#define FIFO_DATA_THRESHOLD             30

/* PAIR_TIME unit: second  */
#define PAIR_TIME                       2

#define STATE_TsdIdle                   0
#define STATE_TsdStart                  1
#define STATE_TsdDataLo                 2
#define STATE_TsdDataHi                 3
#define STATE_TsdChkLo                  4
#define STATE_TsdChkHi                  5

#define LEAD_TsdIdle                    0xFF
#define LEAD_TsdStart                   0x00
#define LEAD_TsdDataLo                  0x60
#define LEAD_TsdDataHi                  0x90
#define LEAD_TsdChkLo                   0x50
#define LEAD_TsdChkHi                   0xa0

struct bk9522_keys {
        unsigned char data;  /* bk9522 data */
        unsigned int code;  /* input event code (KEY_*) */
};

/* The platform data for the bk9522 key driver */
struct bk9522_platform_data {
    int sda_gpio;
    int sck_gpio;
    int irq;
    unsigned long irqflags;
    int power;
    int power_level_en;
    struct bk9522_keys *keys;
    unsigned int key_num;
};

#endif
