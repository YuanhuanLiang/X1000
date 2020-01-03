#ifndef __SFC_NOR_TABLE_H
#define __SFC_NOR_TABLE_H

#include <mach/spinor.h>

struct spi_nor_info spi_nor_info_table[] = {
	[0] = {
		.name = "GD25Q128C",
		.id = 0xc84018,

		.read_standard.cmd = 0x03,
		.read_standard.dummy_byte = 0,
		.read_standard.addr_nbyte = 3,
		.read_standard.transfer_mode = 0,

		.read_quad.cmd = 0x6b,
		.read_quad.dummy_byte = 8,
		.read_quad.addr_nbyte = 3,
		.read_quad.transfer_mode = 5,

		.write_standard.cmd = 0x02,
		.write_standard.dummy_byte = 0,
		.write_standard.addr_nbyte = 3,
		.write_standard.transfer_mode = 0,

		.write_quad.cmd = 0x32,
		.write_quad.dummy_byte = 0,
		.write_quad.addr_nbyte = 3,
		.write_quad.transfer_mode = 5,

		.sector_erase.cmd = 0x52,
		.sector_erase.dummy_byte = 0,
		.sector_erase.addr_nbyte = 3,
		.sector_erase.transfer_mode = 0,

		.wr_en.cmd = 0x06,
		.wr_en.dummy_byte = 0,
		.wr_en.addr_nbyte = 0,
		.wr_en.transfer_mode = 0,

		.en4byte.cmd = 0xb7,
		.en4byte.dummy_byte = 0,
		.en4byte.addr_nbyte = 0,
		.en4byte.transfer_mode = 0,

		.quad_set.cmd = 0x31,
		.quad_set.bit_shift = 1,
		.quad_set.val = 1,
		.quad_set.mask = 1,
		.quad_set.len = 1,
		.quad_set.dummy = 0,

		.quad_get.cmd = 0x35,
		.quad_get.bit_shift = 1,
		.quad_get.val = 1,
		.quad_get.mask = 1,
		.quad_get.len = 1,
		.quad_get.dummy = 0,

		.busy.cmd = 0x05,
		.busy.bit_shift = 0,
		.busy.val = 0,
		.busy.mask = 1,
		.busy.len = 1,
		.busy.dummy = 0,

		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 50,

		.chip_size = 16777216,
		.page_size = 256,
		.erase_size = 32768,

		.quad_ops_mode = 1,
		.addr_ops_mode = 0,

	},
	[1] = {
		.name = "GD25Q256C",
		.id = 0xc84019,

		.read_standard.cmd = 0x03,
		.read_standard.dummy_byte = 0,
		.read_standard.addr_nbyte = 4,
		.read_standard.transfer_mode = 0,

		.read_quad.cmd = 0x6b,
		.read_quad.dummy_byte = 8,
		.read_quad.addr_nbyte = 4,
		.read_quad.transfer_mode = 5,

		.write_standard.cmd = 0x02,
		.write_standard.dummy_byte = 0,
		.write_standard.addr_nbyte = 4,
		.write_standard.transfer_mode = 0,

		.write_quad.cmd = 0x32,
		.write_quad.dummy_byte = 0,
		.write_quad.addr_nbyte = 4,
		.write_quad.transfer_mode = 5,

		.sector_erase.cmd = 0x52,
		.sector_erase.dummy_byte = 0,
		.sector_erase.addr_nbyte = 4,
		.sector_erase.transfer_mode = 0,

		.wr_en.cmd = 0x06,
		.wr_en.dummy_byte = 0,
		.wr_en.addr_nbyte = 0,
		.wr_en.transfer_mode = 0,

		.en4byte.cmd = 0xb7,
		.en4byte.dummy_byte = 0,
		.en4byte.addr_nbyte = 0,
		.en4byte.transfer_mode = 0,

		.quad_set.cmd = 0x31,
		.quad_set.bit_shift = 1,
		.quad_set.val = 1,
		.quad_set.mask = 1,
		.quad_set.len = 1,
		.quad_set.dummy = 0,

		.quad_get.cmd = 0x35,
		.quad_get.bit_shift = 1,
		.quad_get.val = 1,
		.quad_get.mask = 1,
		.quad_get.len = 1,
		.quad_get.dummy = 0,

		.busy.cmd = 0x05,
		.busy.bit_shift = 0,
		.busy.val = 0,
		.busy.mask = 1,
		.busy.len = 1,
		.busy.dummy = 0,

		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 50,

		.chip_size = 33554432,
		.page_size = 256,
		.erase_size = 32768,

		.quad_ops_mode = 1,
		.addr_ops_mode = 0,
	},
	[2] = {
		.name = "WIN25Q128JVSQ",
		.id = 0xef4018,

		.read_standard.cmd = 0x03,
		.read_standard.dummy_byte = 0,
		.read_standard.addr_nbyte = 3,
		.read_standard.transfer_mode = 0,

		.read_quad.cmd = 0x6b,
		.read_quad.dummy_byte = 8,
		.read_quad.addr_nbyte = 3,
		.read_quad.transfer_mode = 5,

		.write_standard.cmd = 0x02,
		.write_standard.dummy_byte = 0,
		.write_standard.addr_nbyte = 3,
		.write_standard.transfer_mode = 0,

		.write_quad.cmd = 0x32,
		.write_quad.dummy_byte = 0,
		.write_quad.addr_nbyte = 3,
		.write_quad.transfer_mode = 5,

		.sector_erase.cmd = 0x52,
		.sector_erase.dummy_byte = 0,
		.sector_erase.addr_nbyte = 3,
		.sector_erase.transfer_mode = 0,

		.wr_en.cmd = 0x06,
		.wr_en.dummy_byte = 0,
		.wr_en.addr_nbyte = 0,
		.wr_en.transfer_mode = 0,

		.en4byte.cmd = 0xb7,
		.en4byte.dummy_byte = 0,
		.en4byte.addr_nbyte = 0,
		.en4byte.transfer_mode = 0,

		.quad_set.cmd = 0x31,
		.quad_set.bit_shift = 1,
		.quad_set.val = 1,
		.quad_set.mask = 1,
		.quad_set.len = 1,
		.quad_set.dummy = 0,

		.quad_get.cmd = 0x35,
		.quad_get.bit_shift = 1,
		.quad_get.val = 1,
		.quad_get.mask = 1,
		.quad_get.len = 1,
		.quad_get.dummy = 0,

		.busy.cmd = 0x05,
		.busy.bit_shift = 0,
		.busy.val = 0,
		.busy.mask = 1,
		.busy.len = 1,
		.busy.dummy = 0,

		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 50,

		.chip_size = 16777216,
		.page_size = 256,
		.erase_size = 32768,

		.quad_ops_mode = 1,
		.addr_ops_mode = 0,
	},
	[3] = {
		.name = "WIN25Q256JVEQ",
		.id = 0xef4019,

		.read_standard.cmd = 0x013,
		.read_standard.dummy_byte = 0,
		.read_standard.addr_nbyte = 4,
		.read_standard.transfer_mode = 0,

		.read_quad.cmd = 0x6b,
		.read_quad.dummy_byte = 8,
		.read_quad.addr_nbyte = 4,
		.read_quad.transfer_mode = 5,

		.write_standard.cmd = 0x12,
		.write_standard.dummy_byte = 0,
		.write_standard.addr_nbyte = 4,
		.write_standard.transfer_mode = 0,

		.write_quad.cmd = 0x32,
		.write_quad.dummy_byte = 0,
		.write_quad.addr_nbyte = 4,
		.write_quad.transfer_mode = 5,

		.sector_erase.cmd = 0x52,
		.sector_erase.dummy_byte = 0,
		.sector_erase.addr_nbyte = 4,
		.sector_erase.transfer_mode = 0,

		.wr_en.cmd = 0x06,
		.wr_en.dummy_byte = 0,
		.wr_en.addr_nbyte = 0,
		.wr_en.transfer_mode = 0,

		.en4byte.cmd = 0xb7,
		.en4byte.dummy_byte = 0,
		.en4byte.addr_nbyte = 0,
		.en4byte.transfer_mode = 0,

		.quad_set.cmd = 0x31,
		.quad_set.bit_shift = 1,
		.quad_set.val = 1,
		.quad_set.mask = 1,
		.quad_set.len = 1,
		.quad_set.dummy = 0,

		.quad_get.cmd = 0x35,
		.quad_get.bit_shift = 1,
		.quad_get.val = 1,
		.quad_get.mask = 1,
		.quad_get.len = 1,
		.quad_get.dummy = 0,

		.busy.cmd = 0x05,
		.busy.bit_shift = 0,
		.busy.val = 0,
		.busy.mask = 1,
		.busy.len = 1,
		.busy.dummy = 0,

		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 50,

		.chip_size = 33554432,
		.page_size = 256,
		.erase_size = 32768,

		.quad_ops_mode = 1,
		.addr_ops_mode = 0,
	},
	[4] = {
		.name = "MX25L12845G",
		.id = 0xc22018,

		.read_standard.cmd = 0x03,
		.read_standard.dummy_byte = 0,
		.read_standard.addr_nbyte = 3,
		.read_standard.transfer_mode = 0,

		.read_quad.cmd = 0x6b,
		.read_quad.dummy_byte = 8,
		.read_quad.addr_nbyte = 3,
		.read_quad.transfer_mode = 5,

		.write_standard.cmd = 0x02,
		.write_standard.dummy_byte = 0,
		.write_standard.addr_nbyte = 3,
		.write_standard.transfer_mode = 0,

		.write_quad.cmd = 0x32,
		.write_quad.dummy_byte = 0,
		.write_quad.addr_nbyte = 3,
		.write_quad.transfer_mode = 5,

		.sector_erase.cmd = 0x52,
		.sector_erase.dummy_byte = 0,
		.sector_erase.addr_nbyte = 3,
		.sector_erase.transfer_mode = 0,

		.wr_en.cmd = 0x06,
		.wr_en.dummy_byte = 0,
		.wr_en.addr_nbyte = 0,
		.wr_en.transfer_mode = 0,

		.en4byte.cmd = 0xb7,
		.en4byte.dummy_byte = 0,
		.en4byte.addr_nbyte = 0,
		.en4byte.transfer_mode = 0,

		.quad_set.cmd = 0x01,
		.quad_set.bit_shift = 6,
		.quad_set.val = 1,
		.quad_set.mask = 1,
		.quad_set.len = 1,
		.quad_set.dummy = 0,

		.quad_get.cmd = 0x05,
		.quad_get.bit_shift = 6,
		.quad_get.val = 1,
		.quad_get.mask = 1,
		.quad_get.len = 1,
		.quad_get.dummy = 0,

		.busy.cmd = 0x05,
		.busy.bit_shift = 0,
		.busy.val = 0,
		.busy.mask = 1,
		.busy.len = 1,
		.busy.dummy = 0,

		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 50,

		.chip_size = 16777216,
		.page_size = 256,
		.erase_size = 32768,

		.quad_ops_mode = 1,
		.addr_ops_mode = 0,
	},
	[5] = {
		.name = "MX25L25645G",
		.id = 0xc22019,

		.read_standard.cmd = 0x03,
		.read_standard.dummy_byte = 0,
		.read_standard.addr_nbyte = 4,
		.read_standard.transfer_mode = 0,

		.read_quad.cmd = 0x6b,
		.read_quad.dummy_byte = 8,
		.read_quad.addr_nbyte = 4,
		.read_quad.transfer_mode = 5,

		.write_standard.cmd = 0x02,
		.write_standard.dummy_byte = 0,
		.write_standard.addr_nbyte = 4,
		.write_standard.transfer_mode = 0,

		.write_quad.cmd = 0x32,
		.write_quad.dummy_byte = 0,
		.write_quad.addr_nbyte = 4,
		.write_quad.transfer_mode = 5,

		.sector_erase.cmd = 0x52,
		.sector_erase.dummy_byte = 0,
		.sector_erase.addr_nbyte = 4,
		.sector_erase.transfer_mode = 0,

		.wr_en.cmd = 0x06,
		.wr_en.dummy_byte = 0,
		.wr_en.addr_nbyte = 0,
		.wr_en.transfer_mode = 0,

		.en4byte.cmd = 0xb7,
		.en4byte.dummy_byte = 0,
		.en4byte.addr_nbyte = 0,
		.en4byte.transfer_mode = 0,

		.quad_set.cmd = 0x01,
		.quad_set.bit_shift = 6,
		.quad_set.val = 1,
		.quad_set.mask = 1,
		.quad_set.len = 1,
		.quad_set.dummy = 0,

		.quad_get.cmd = 0x05,
		.quad_get.bit_shift = 6,
		.quad_get.val = 1,
		.quad_get.mask = 1,
		.quad_get.len = 1,
		.quad_get.dummy = 0,

		.busy.cmd = 0x05,
		.busy.bit_shift = 0,
		.busy.val = 0,
		.busy.mask = 1,
		.busy.len = 1,
		.busy.dummy = 0,

		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 50,

		.chip_size = 33554432,
		.page_size = 256,
		.erase_size = 32768,

		.quad_ops_mode = 1,
		.addr_ops_mode = 0,
	},
	[6] = {
		.name = "MICRON-N25-256MB",
		.id = 0x20ba19,

		.read_standard.cmd = 0x03,
		.read_standard.dummy_byte = 0,
		.read_standard.addr_nbyte = 4,
		.read_standard.transfer_mode = 0,

		.read_quad.cmd = 0x6b,
		.read_quad.dummy_byte = 8,
		.read_quad.addr_nbyte = 4,
		.read_quad.transfer_mode = 5,

		.write_standard.cmd = 0x02,
		.write_standard.dummy_byte = 0,
		.write_standard.addr_nbyte = 4,
		.write_standard.transfer_mode = 0,

		.write_quad.cmd = 0x32,
		.write_quad.dummy_byte = 0,
		.write_quad.addr_nbyte = 4,
		.write_quad.transfer_mode = 5,

		.sector_erase.cmd = 0xd8,
		.sector_erase.dummy_byte = 0,
		.sector_erase.addr_nbyte = 4,
		.sector_erase.transfer_mode = 0,

		.wr_en.cmd = 0x06,
		.wr_en.dummy_byte = 0,
		.wr_en.addr_nbyte = 0,
		.wr_en.transfer_mode = 0,

		.en4byte.cmd = 0xb7,
		.en4byte.dummy_byte = 0,
		.en4byte.addr_nbyte = 0,
		.en4byte.transfer_mode = 0,


		.busy.cmd = 0x05,
		.busy.bit_shift = 0,
		.busy.val = 0,
		.busy.mask = 1,
		.busy.len = 1,
		.busy.dummy = 0,

		.tCHSH = 5,
		.tSLCH = 5,
		.tSHSL_RD = 20,
		.tSHSL_WR = 50,

		.chip_size = 33554432,
		.page_size = 256,
		.erase_size = 65536,

		.quad_ops_mode = 0,
		.addr_ops_mode = 1,
	},

};
#endif
