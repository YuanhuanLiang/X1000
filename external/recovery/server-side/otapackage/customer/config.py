# default setting for partition.py, using to generate partition.conf
# exclusively ####
search_prefix = 're_'
# nor flash configuation
re_nor_tag = 'nor'
re_nor_device_size = '16MB'
re_nor_partition = (
    ("uboot", "0x0", "0x40000", "mtdblock0"),
    ("kernel", "0x40000", "0x300000", "mtdblock1"),
    ("rootfs", "0x360000", "0xca0000", "mtdblock2"),
)
# nand flash configuation
re_nand_tag = 'nand'
re_nand_device_size = '128MB'
re_nand_partition = (
                    ("uboot", "0x0", "0x100000", "mtdblock0"),
                    ("kernel", "0x100000", "0x800000", "mtdblock1"),
                    ("rootfs", "0x980000", "0x2800000", "mtdblock2"),
                    ("data", "0x3180000", "0x4e80000", "mtdblock3"),
)
# mmc configuation
re_mmc_tag = 'mmc'
re_mmc_device_size = '512MB'
re_mmc_partition = ()

local = locals()
