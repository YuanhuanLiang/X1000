/*************************************************************************
	> Filename: l718_manager.h
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Mon 02 Jul 2018 05:18:30 PM CST
 ************************************************************************/

#ifndef _L718_MANAGER_H
#define _L718_MANAGER_H

#undef msleep
#define msleep(ms)    usleep(ms*1000)

int l718_gpio_init(void);
void l718_gpio_deinit(void);
int l718_uart_init(void);
void l718_uart_deinit(void);
int l718_uart_read(void *buf, uint32_t count);
int l718_uart_write(void *buf);
int l718_power_on(void);
void l718_power_off(void);
int l718_wakeup(void);
int l718_reset(void);

#endif
