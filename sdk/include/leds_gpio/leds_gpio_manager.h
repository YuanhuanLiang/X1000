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
#ifndef _LEDS_GPIO_MANAGER_H_
#define _LEDS_GPIO_MANAGER_H_

#include <types.h>

typedef enum {
    LED_OFF = 0,
    LED_ON,
}led_onoff_m;

typedef enum {
    KB_LED_D1 = 0,
    KB_LED_D2,
    KB_LED_D3,
    KB_LED_D4,
    KB_LED_D5,
    KB_LED_D6,
    KB_LED_D7,
    KB_LED_D8,
    KB_LED_D9,
    KB_LED_D10,
    KB_LED_D11,
    KB_LED_D12,
    KB_LED_MAX,
}keyboard_led_m;

typedef enum {
    BL_LED_D1 = 0,
    BL_LED_MAX,
}backlight_led_m;

typedef enum {
    FP_LED_W = 0,
    FP_LED_B,
    FP_LED_G,
    FP_LED_R,
    FP_LED_MAX,
}indication_led_m;

typedef enum {
    MATRIX_LED_D1 = 0,
    MATRIX_LED_D2,
    MATRIX_LED_D3,
    MATRIX_LED_D4,
    MATRIX_LED_D5,
    MATRIX_LED_D6,
    MATRIX_LED_D7,
    MATRIX_LED_D8,
    MATRIX_LED_D9,
    MATRIX_LED_D10,
    MATRIX_LED_D11,
    MATRIX_LED_D12,
    MATRIX_LED_MAX,
}matrix_led_m;

#define MATRIX_LEDS_COL_NUM             3
#define MATRIX_LEDS_ROW_NUM             4

struct leds_gpio_manager {
    /**
     *  @brief   初始化 keyboard leds
     *
     *  @return  0  - 成功
     *           <0 - 失败
     */
    int (*keyboard_init)(void);
    /**
     *  @brief   keyboard leds 开关
     *
     *  @param   led - 指定LED
     *           onoff - 开关
     *
     *  @return  0  - 成功
     *           <0 - 失败
     *
     */
    int (*keyboard_onoff)(keyboard_led_m led, led_onoff_m onoff);
    /**
     *  @brief   释放 keyboard leds
     *
     *  @return  0  - 成功
     *           <0 - 失败
     */
    int (*keyboard_deinit)(void);

    /**
     *  @brief   初始化 backlight leds
     *
     *  @return  0  - 成功
     *           <0 - 失败
     */
    int (*backlight_init)(void);
    /**
     *  @brief   backlight leds 开关
     *
     *  @param   led - 指定LED
     *           onoff - 开关
     *
     *  @return  0  - 成功
     *           <0 - 失败
     *
     */
    int (*backlight_onoff)(backlight_led_m led, led_onoff_m onoff);
    /**
     *  @brief   释放 backlight leds
     *
     *  @return  0  - 成功
     *           <0 - 失败
     */
    int (*backlight_deinit)(void);

    /**
     *  @brief   初始化 matrix leds
     *
     *  @return  0  - 成功
     *           <0 - 失败
     */
    int (*matrix_init)(void);
    /**
     *  @brief   matrix leds 开关
     *
     *  @param   led - 指定LED
     *           onoff - 开关
     *
     *  @return  0  - 成功
     *           <0 - 失败
     *
     */
    int (*matrix_onoff)(matrix_led_m led, led_onoff_m onoff);
    /**
     *  @brief   释放 matrix leds
     *
     *  @return  0  - 成功
     *           <0 - 失败
     */
    int (*matrix_deinit)(void);
    /**
     *  @brief   初始化 indication leds
     *
     *  @return  0  - 成功
     *           <0 - 失败
     */
    int (*indication_init)(void);
    /**
     *  @brief   indication leds 开关
     *
     *  @param   led - 指定LED
     *           onoff - 开关
     *
     *  @return  0  - 成功
     *           <0 - 失败
     *
     */
    int (*indication_onoff)(indication_led_m led, led_onoff_m onoff);
    /**
     *  @brief   indication leds 闪烁
     *
     *  @param   led - 指定LED
     *           delay_on - 持续打开的时间  单位：ms
     *           delay_off- 持续关闭的时间  单位：ms
     *
     *  @return  0  - 成功
     *           <0 - 失败
     *
     */
    int (*indication_blink)(indication_led_m led, uint32_t delay_on, uint32_t delay_off);
    /**
     *  @brief   释放 indication leds
     *
     *  @return  0  - 成功
     *           <0 - 失败
     */
    int (*indication_deinit)(void);
};

struct leds_gpio_manager* get_leds_gpio_manager(void);



#endif
