/*************************************************************************
	> Filename: g510_manager.h
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Mon 21 May 2018 11:34:10 AM CST
 ************************************************************************/

#ifndef _G510_MANAGER_H
#define _G510_MANAGER_H

#undef msleep
#define msleep(ms)    usleep(ms*1000)

int g510_gpio_init(void);
void g510_gpio_deinit(void);
int g510_uart_init(void);
void g510_uart_deinit(void);
int g510_uart_read(void *buf, uint32_t count);
int g510_uart_write(void *buf);
int g510_power_on(void);
void g510_power_off(void);
int g510_wakeup(void);
int g510_emerg_reset(void);

#endif
