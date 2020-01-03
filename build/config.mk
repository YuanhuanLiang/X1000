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

# MESSAGE Macro -- display a message in bold type
pkgname = $(lastword $(subst /, ,$(1)))
MESSAGE = echo "$(TERM_BOLD)>>> $(call pkgname, $(TARGET_PATH)) $(1)$(TERM_RESET)"
TERM_BOLD  := $(shell tput smso)
TERM_RESET := $(shell tput rmso)

JLEVEL := 8

LOADER_BUILD_PATH := $(TOPDIR)/bootloader/x-loader
KERNEL_BUILD_PATH := $(TOPDIR)/kernel
SDK_BUILD_PATH := $(TOPDIR)/sdk
APP_BUILD_PATH := $(TOPDIR)/app

HOST_TOOLS_PATH := $(TOPDIR)/tools/host
COMMON_SYSTEM_PATH := $(TOPDIR)/device/common/system
SYSTEM_PATCH_PATH := $(TOPDIR)/device/$(TARGET_DEVICE)/$(TARGET_STORAGE_MEDIUM)
SDK_LIBS_OUTPUT_PATH := $(SDK_BUILD_PATH)/out/system/usr/lib

TARGET_FILES_OUTDIR := $(TOPDIR)/out
TARGET_DEVICE_OUTDIR := $(TARGET_FILES_OUTDIR)/$(TARGET_DEVICE)
TARGET_IMAGE_OUTDIR := $(TARGET_DEVICE_OUTDIR)/image
TARGET_SYSTEM_OUTDIR := $(TARGET_DEVICE_OUTDIR)/system
TARGET_SDK_OUTDIR := $(TARGET_DEVICE_OUTDIR)/sdk

TARGET_SDK_SHARED_LIBS := $(SDK_LIBS_OUTPUT_PATH)/*.so*
TARGET_SDK_TESTUNIT := $(SDK_BUILD_PATH)/out/examples/test_*
TARGET_SDK_SYSTEM := $(SDK_BUILD_PATH)/out/system/*

INSTALL_SDKLIB_TO_FSDIR := $(TARGET_SYSTEM_OUTDIR)/usr/lib
INSTALL_SDKLIB_TO_FSDIR := $(TARGET_SYSTEM_OUTDIR)/usr/lib
INSTALL_SDKTESTUNIT_TO_FSDIR := $(TARGET_SYSTEM_OUTDIR)/usr/bin
INSTALL_SDKLIB_TO_APPDIR := $(APP_BUILD_PATH)/lib

TARGET_APP_NAME := ms800_app
TARGET_APP_EXEFILE := $(APP_BUILD_PATH)/out/$(TARGET_APP_NAME)
INSTALL_APP_EXEFILE_TO_FSDIR := $(TARGET_SYSTEM_OUTDIR)/usr/bin
