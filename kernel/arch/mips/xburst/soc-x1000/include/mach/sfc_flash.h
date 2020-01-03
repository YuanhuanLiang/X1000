#ifndef __SFC_FLASH_H
#define __SFC_FLASH_H
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/mtd/mtd.h>
#include <mach/sfc.h>

struct sfc_flash {

	struct sfc *sfc;
	void *flash_info;
	struct device* dev;
	struct mtd_info mtd;
	struct mutex	lock;

};

struct flash_power_ctrl {
    int power_pin;
    int power_en_level;
    int power_on_delay_ms;
};

struct jz_sfc_info {

	uint8_t num_partition;		//flash partiton len
	uint8_t use_board_info;		//use board flag
	void *flash_param;		//flash param
	void *flash_partition;		//flash partiton
	void *other_args;		//other args
#ifdef CONFIG_JZ_SFC_FLASH_POWER_CTRL
    struct flash_power_ctrl *flash_power_ctrl;
#endif
};

#endif
