#
#  Copyright (C) 2018, Wang Qiuwei <qiuwei.wang@ingenic.com / panddio@163.com>
#
#  Ingenic ms800 Project
#
#  This program is free software; you can redistribute it and/or modify it
#  under  the terms of the GNU General  Public License as published by the
#  Free Software Foundation;  either version 2 of the License, or (at your
#  option) any later version.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  675 Mass Ave, Cambridge, MA 02139, USA.
#

#
# For norflash
ifeq ($(strip $(TARGET_STORAGE_MEDIUM)),norflash)
SDK_BUILD_CONFIG := ms800_sdk_defconfig

LOADER_BUILD_CONFIG := ms800_nor_config
LOADER_TARGET_FILE := out/x-loader-pad-with-sleep-lib.bin

ifeq ($(strip $(TARGET_DEVICE_VERSION)),v10)
KERNEL_BUILD_CONFIG := ms800_v10_linux_sfcnor_jffs2_defconfig
endif
KERNEL_TARGET_IMAGE := xImage
KERNEL_TARGET_PATH := arch/mips/boot/zcompressed

ROOTFS_TARGET_TYPE := jffs2
ROOTFS_JFFS2_ERASESIZE := 0x8000
ROOTFS_JFFS2_SIZE := 0x500000
endif

#
# For spinand
ifeq ($(strip $(TARGET_STORAGE_MEDIUM)),spinand)
SDK_BUILD_CONFIG := ms800_sdk_defconfig

LOADER_BUILD_CONFIG := ms800_nand_config
LOADER_TARGET_FILE := out/x-loader-pad-with-sleep-lib.bin

ifeq ($(strip $(TARGET_DEVICE_VERSION)),v10)
KERNEL_BUILD_CONFIG := ms800_v10_linux_sfcnand_ubi_defconfig
endif
KERNEL_TARGET_IMAGE := xImage
KERNEL_TARGET_PATH := arch/mips/boot/zcompressed

ROOTFS_TARGET_TYPE := ubi
ROOTFS_UBIFS_LEBSIZE := 0x1f000
ROOTFS_UBIFS_MAXLEBCNT := 2048
ROOTFS_UBIFS_MINIOSIZE := 0x800
endif
