/*
 * include/linux/zinitix_ts.h
 *
 * ZINITIX capacitive touchscreen driver.
 *
 * Copyright 2018, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef _ZINITIX_TS_H
#define _ZINITIX_TS_H


#define TPD_RES_MAX_X               720
#define TPD_RES_MAX_Y               1280


#ifdef TPD_HIGH_SPEED_DMA
#define TOUCH_POINT_MODE            0
#else
#define TOUCH_POINT_MODE            1
#endif

/* Upgrade Method*/
#define TOUCH_ONESHOT_UPGRADE       0


#define CHIP_OFF_DELAY              2  /*ms*/
#define CHIP_ON_DELAY               5  /*ms*/
#define FIRMWARE_ON_DELAY           10 /*ms*/

#define ZINITIX_INIT_RETRY_CNT      3

#define ZINITIX_SWRESET_CMD             0x0000
#define ZINITIX_WAKEUP_CMD              0x0001
#define ZINITIX_IDLE_CMD                0x0004
#define ZINITIX_SLEEP_CMD               0x0005
#define ZINITIX_CLEAR_INT_STATUS_CMD    0x0003
#define ZINITIX_CALIBRATE_CMD           0x0006
#define ZINITIX_SAVE_STATUS_CMD         0x0007
#define ZINITIX_SAVE_CALIBRATION_CMD    0x0008
#define ZINITIX_RECALL_FACTORY_CMD      0x000f
#define ZINITIX_SENSITIVITY             0x0020
#define ZINITIX_I2C_CHECKSUM_WCNT       0x016a
#define ZINITIX_I2C_CHECKSUM_RESULT     0x016c
#define ZINITIX_DEBUG_REG               0x0115  //0~7
#define ZINITIX_TOUCH_MODE              0x0010
#define ZINITIX_CHIP_REVISION           0x0011
#define ZINITIX_FIRMWARE_VERSION        0x0012
#define ZINITIX_MINOR_FW_VERSION        0x0121
#define ZINITIX_DATA_VERSION_REG        0x0013
#define ZINITIX_HW_ID                   0x0014
#define ZINITIX_SUPPORTED_FINGER_NUM    0x0015
#define ZINITIX_EEPROM_INFO             0x0018
#define ZINITIX_INITIAL_TOUCH_MODE      0x0019
#define ZINITIX_TOTAL_NUMBER_OF_X       0x0060
#define ZINITIX_TOTAL_NUMBER_OF_Y       0x0061
#define ZINITIX_DELAY_RAW_FOR_HOST      0x007f
#define ZINITIX_BUTTON_SUPPORTED_NUM    0x00B0
#define ZINITIX_BUTTON_SENSITIVITY      0x00B2
#define ZINITIX_X_RESOLUTION            0x00C0
#define ZINITIX_Y_RESOLUTION            0x00C1
#define ZINITIX_POINT_STATUS_REG        0x0080
#define ZINITIX_ICON_STATUS_REG         0x00AA
#define ZINITIX_AFE_FREQUENCY           0x0100
#define ZINITIX_DND_N_COUNT             0x0122
#define ZINITIX_RAWDATA_REG             0x0200
#define ZINITIX_EEPROM_INFO_REG         0x0018
#define ZINITIX_INT_ENABLE_FLAG         0x00f0
#define ZINITIX_PERIODICAL_INTERRUPT_INTERVAL   0x00f1
#define ZINITIX_CHECKSUM_RESULT         0x012c
#define ZINITIX_INIT_FLASH              0x01d0
#define ZINITIX_WRITE_FLASH             0x01d1
#define ZINITIX_READ_FLASH              0x01d2

#define BIT_DOWN             1
#define BIT_MOVE             2
#define BIT_UP               3
#define BIT_PALM             4
#define BIT_PALM_REJECT      5
#define BIT_WAKEUP           6
#define RESERVED_1           7
#define BIT_WEIGHT_CHANGE    8
#define BIT_PT_NO_CHANGE     9
#define BIT_REJECT           10
#define BIT_PT_EXIST         11
#define RESERVED_2           12
#define BIT_MUST_ZERO        13
#define BIT_DEBUG            14
#define BIT_ICON_EVENT       15

#define SUB_BIT_EXIST        0
#define SUB_BIT_DOWN         1
#define SUB_BIT_MOVE         2
#define SUB_BIT_UP           3
#define SUB_BIT_UPDATE       4
#define SUB_BIT_WAIT         5

#define zinitix_bit_set(val, n)     ((val) &= ~(1<<(n)), (val) |= (1<<(n)))
#define zinitix_bit_clr(val, n)     ((val) &= ~(1<<(n)))
#define zinitix_bit_test(val, n)    ((val) & (1<<(n)))
#define zinitix_swap_v(a, b, t)     ((t) = (a), (a) = (b), (b) = (t))
#define zinitix_swap_16(s)          (((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)))


struct zinitix_ts_platform_data {
    unsigned short x_max;
    unsigned short y_max;

    unsigned int ts_rst_pin;
    unsigned int ts_int_pin;
    unsigned int ts_pwr_pin;
    unsigned char pwr_en_level;
};

#endif /* _ZINITIX_TS_H */
