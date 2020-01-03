/*************************************************************************
	> Filename: gsm_manager.h
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Tue 03 Jul 2018 05:57:28 PM CST
 ************************************************************************/

#ifndef _GSM_MANAGER_H
#define _GSM_MANAGER_H
#include <g510_manager.h>
#include <l718_manager.h>

#if defined (CONFIG_G510)
#define GSM_MODULE_NAME "G510"
#define gsm_gpio_init() g510_gpio_init()
#define gsm_gpio_deinit() g510_gpio_deinit()
#define gsm_uart_init() g510_uart_init()
#define gsm_uart_deinit() g510_uart_deinit()
#define gsm_uart_read(buf, count) g510_uart_read(buf, count)
#define gsm_uart_write(buf) g510_uart_write(buf)
#define gsm_power_on() g510_power_on()
#define gsm_wakeup() g510_wakeup()
#define gsm_reset() g510_emerg_reset()
#elif defined (CONFIG_L718)
#define GSM_MODULE_NAME "L718"
#define gsm_gpio_init() l718_gpio_init()
#define gsm_gpio_deinit() l718_gpio_deinit()
#define gsm_uart_init() l718_uart_init()
#define gsm_uart_deinit() l718_uart_deinit()
#define gsm_uart_read(buf, count) l718_uart_read(buf, count)
#define gsm_uart_write(buf) l718_uart_write(buf)
#define gsm_power_on() l718_power_on()
#define gsm_wakeup() l718_wakeup()
#define gsm_reset() l718_reset()
#else
#define GSM_MODULE_NAME "UNKNOWN"
int gsm_gpio_init(void) {return -1;}
void gsm_gpio_deinit(void) {}
int gsm_uart_init(void) {return -1;}
void gsm_uart_deinit(void) {}
int gsm_uart_read(void *buf, uint32_t count) {return -1;}
int gsm_uart_write(void *buf) {return -1;}
int gsm_power_on(void) {return -1;}
int gsm_wakeup(void) {return -1;}
int gsm_reset(void) {return -1;}
#endif

#endif /* _GSM_MANAGER_H */
