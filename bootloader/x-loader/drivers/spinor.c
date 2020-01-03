/*
 *  Copyright (C) 2016 Ingenic Semiconductor Co.,Ltd
 *
 *  X1000 series bootloader for u-boot/rtos/linux
 *
 *  Zhang YanMing <yanming.zhang@ingenic.com, jamincheung@126.com>
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

#include <common.h>

struct spi_mode_peer spi_mode_local[] = {
    [SPI_MODE_STANDARD] = {TRAN_SPI_STANDARD, CMD_READ},
    [SPI_MODE_QUAD] = {TRAN_SPI_QUAD, CMD_QUAD_READ},
};

void jz_sfc_reset_address_mode(void) {
    uint32_t buf = 0;
    struct jz_sfc sfc;

    SFC_SEND_COMMAND(&sfc, CMD_WREN, 0, 0, 0, 0, 0, 1);
    SFC_SEND_COMMAND(&sfc, CMD_EX4B, 0, 0, 0, 0, 0, 1);
    SFC_SEND_COMMAND(&sfc, CMD_RDSR, 1, 0, 0, 0, 1, 0);

    sfc_read_data(&buf, 1);
    while(buf & CMD_SR_WIP) {
        SFC_SEND_COMMAND(&sfc, CMD_RDSR, 1, 0, 0, 0, 1, 0);
        sfc_read_data(&buf, 1);
    }
}

void spinor_set_quad_mode(void) {
    uint32_t buf;
    uint32_t tmp;
    int i = 10;
    struct jz_sfc sfc;

    SFC_SEND_COMMAND(&sfc, CMD_WREN, 0, 0, 0, 0, 0, 1);
    SFC_SEND_COMMAND(&sfc, CMD_WRSR_1, 1, 0, 0, 0, 1, 1);
    tmp = 0x02;
    sfc_write_data(&tmp, 1);

    SFC_SEND_COMMAND(&sfc, CMD_RDSR, 1, 0, 0, 0, 1, 0);
    sfc_read_data(&tmp, 1);
    while(tmp & CMD_SR_WIP) {
        SFC_SEND_COMMAND(&sfc, CMD_RDSR, 1, 0, 0, 0, 1, 0);
        sfc_read_data(&tmp, 1);
    }

    SFC_SEND_COMMAND(&sfc, CMD_RDSR_1, 1, 0, 0, 0, 1, 0);
    sfc_read_data(&buf, 1);
    while(!(buf & 0x2) && ((i--) > 0)) {
        SFC_SEND_COMMAND(&sfc, CMD_RDSR_1, 1, 0, 0, 0, 1, 0);
        sfc_read_data(&buf, 1);
    }
}

int spinor_read(uint32_t offset, uint32_t len, uint32_t data) {
    int addr_len = 3;
    struct jz_sfc sfc;
#ifndef CONFIG_SPI_STANDARD
    int dummy_byte = 8;
    SFC_SEND_COMMAND(&sfc, SPI_MODE_QUAD, len, offset, addr_len, dummy_byte, 1, 0);
#else
    SFC_SEND_COMMAND(&sfc, SPI_MODE_STANDARD, len, offset, addr_len, 0, 1, 0);
#endif
    sfc_read_data((void *) data, len);
    return 0;
}

int spinor_write(uint32_t offset, uint32_t len, uint32_t* data) {
    int addr_len = 3;
    struct jz_sfc sfc;
    uint32_t tmp;
#ifndef CONFIG_SPI_STANDARD
    SFC_SEND_COMMAND_QUAD(&sfc, CMD_WREN, 0, 0, 0, 0, 0, 0);
    SFC_SEND_COMMAND_QUAD(&sfc, CMD_QPP, len, offset, addr_len, 0, 1, 1);
#else
    SFC_SEND_COMMAND(&sfc, CMD_WREN, 0, 0, 0, 0, 0, 0);
    SFC_SEND_COMMAND(&sfc, CMD_PP, len, offset, addr_len, 0, 1, 1);
#endif
    sfc_write_data((uint32_t*) data, len);
    SFC_SEND_COMMAND(&sfc, CMD_RDSR, 1, 0, 0, 0, 1, 0);
    sfc_read_data(&tmp, 1);
    while(tmp & CMD_SR_WIP) {
        SFC_SEND_COMMAND(&sfc, CMD_RDSR, 1, 0, 0, 0, 1, 0);
        sfc_read_data(&tmp, 1);
    }
    return 0;
}

int spinor_erase(uint32_t offset, uint32_t len) {
    int addr_len = 3;
    struct jz_sfc sfc;
    int command = 0;
    uint32_t tmp;
    switch(len) {
    case 0x1000:
        command = CMD_ERASE_4K;
        break;
    case 0x8000:
        command = CMD_ERASE_32K;
        break;
    case 0x10000:
        command = CMD_ERASE_64K;
        break;
    }
    SFC_SEND_COMMAND(&sfc, CMD_WREN, 0, 0, 0, 0, 0, 0);
    SFC_SEND_COMMAND(&sfc, command, 0, offset, addr_len, 0, 0, 1);
    SFC_SEND_COMMAND(&sfc, CMD_RDSR, 1, 0, 0, 0, 1, 0);
    sfc_read_data(&tmp, 1);
    while(tmp & CMD_SR_WIP) {
        SFC_SEND_COMMAND(&sfc, CMD_RDSR, 1, 0, 0, 0, 1, 0);
        sfc_read_data(&tmp, 1);
    }
    return 0;
}

int spinor_init(void) {
    jz_sfc_reset_address_mode();

#ifndef CONFIG_SPI_STANDARD
    spinor_set_quad_mode();
#endif

    return 0;
}

#ifdef CONFIG_BEIJING_OTA
#define NV_AREA_START (288 * 1024)
static void nv_map_area(unsigned int *base_addr)
{
	unsigned int buf[3][2];
	unsigned int tmp_buf[4];
	unsigned int nv_num = 0, nv_count = 0;
	unsigned int addr, i;

	for (i = 0; i < 3; i++) {
		addr = NV_AREA_START + i * 32 * 1024;
		spinor_read(addr, 4, (void *)buf[i]);
		if (buf[i][0] == 0x5a5a5a5a) {
			spinor_read(addr + 1 *1024, 16, (void *)tmp_buf);
			addr += 32 * 1024 - 8;
			spinor_read(addr, 8, (void *)buf[i]);
			if (buf[i][1] == 0xa5a5a5a5) {
				if (nv_count < buf[i][0]) {
					nv_count = buf[i][0];
					nv_num = i;
				}
			}
		}
	}

	*base_addr = NV_AREA_START + nv_num * 32 * 1024 + 1024;
}

int ota_load(unsigned int *argv, unsigned int dst_addr)
{
	unsigned int nv_buf[4];
	unsigned int count = 16;
	unsigned int src_addr, updata_flag;

	nv_map_area((unsigned int *)&src_addr);
	spinor_read(src_addr, count, (void *)nv_buf);
	updata_flag = nv_buf[3];

	if ((updata_flag & 0x3) != 0x3)
		return spinor_read(CONFIG_KERNEL_OFFSET, CONFIG_KERNEL_LENGTH, (void *)dst_addr);
	else {
		*argv = (unsigned int)CONFIG_OTA_KERNEL_ARGS;
		return spinor_read(CONFIG_OTA_STEP2_KERNEL_OFFSET, CONFIG_KERNEL_LENGTH, (void *)dst_addr);
	}
}
#endif

int is_recovery_update_failed(void) {
    unsigned int update_flag = 0;
    uint32_t offset = RECOVERY_UPDATE_FLAG_OFFSET;
    uint32_t length = RECOVERY_UPDATE_FLAG_SIZE;
    if (spinor_read(offset, length, (uint32_t)&update_flag) != 0) {
        return -1;
    }
    if (update_flag == RECOVERY_UPDATE_FLAG_UPDATING)
        return 1;
    return 0;
}
