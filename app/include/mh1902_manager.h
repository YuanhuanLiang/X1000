/*************************************************************************
	> Filename: mh1902_manager.h
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: 2018年05月20日 星期日 11时41分37秒
 ************************************************************************/

#ifndef _MH1902_MANAGER_H
#define _MH1902_MANAGER_H

#define MCU_CS_PIN      GPIO_PA(25)
#define MCU_INT_CPU     GPIO_PB(22)
#define MCU_BUSY_OUT    GPIO_PC(6)
#define CPU_RST_MCU     GPIO_PC(7)
#define CPU_INT_MCU     GPIO_PC(9)

int mh1902_spi_read(unsigned char *rxbuf, unsigned int count);
int mh1902_spi_write(unsigned char *txbuf, unsigned int count);
int mh1902_spi_transfer(unsigned char *txbuf, unsigned char *rxbuf, unsigned int len);
int mh1902_spi_init(uint8_t mode, uint8_t bits, uint32_t speed);
void mh1902_spi_deinit(void);
int mh1902_gpio_init(void);
void mh1902_gpio_deinit(void);

#endif
