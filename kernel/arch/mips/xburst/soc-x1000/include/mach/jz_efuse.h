#ifndef __JZ_EFUSE_H__
#define __JZ_EFUSE_H__

struct jz_efuse_platform_data {
	int gpio_vddq_en_n;	/* supply 2.5V to VDDQ */
};

enum segment_id {
	CHIP_ID,
	RANDOM_ID,
	USER_ID,
	TRIM3_ID,
	PROTECT_ID,
};

int jz_efuse_read(uint32_t seg_id, uint32_t data_length, uint32_t *buf);

#endif
