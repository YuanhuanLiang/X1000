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

TOPDIR := $(shell pwd)
include $(TOPDIR)/device/$(TARGET_DEVICE)/device.mk
include $(TOPDIR)/build/config.mk


#
# Build all
all:$(TARGET_SYSTEM_OUTDIR) \
	build_sdk \
	build_app \
	build_loader \
	build_kernel \
	build_rootfs

PHONY := all

#
# For bootloader
build_loader:$(TARGET_IMAGE_OUTDIR)
	@$(call MESSAGE,"Compiling bootloader...")
	$(MAKE) $(LOADER_BUILD_CONFIG) -C $(LOADER_BUILD_PATH)
	$(MAKE) -C $(LOADER_BUILD_PATH)
	cp -f $(LOADER_BUILD_PATH)/$(LOADER_TARGET_FILE) $(TARGET_IMAGE_OUTDIR)

clean_loader:
	@$(call MESSAGE,"Cleaning bootloader...")
	$(MAKE) distclean -C $(LOADER_BUILD_PATH)

PHONY += build_loader clean_loader

#
# For kernel
build_kernel:$(TARGET_IMAGE_OUTDIR)
	@$(call MESSAGE,"Compiling kernel...")
	$(MAKE) $(KERNEL_BUILD_CONFIG) -C $(KERNEL_BUILD_PATH)
	$(MAKE) $(KERNEL_TARGET_IMAGE) -j$(JLEVEL) -C $(KERNEL_BUILD_PATH)
	cp -f $(KERNEL_BUILD_PATH)/$(KERNEL_TARGET_PATH)/$(KERNEL_TARGET_IMAGE) $(TARGET_IMAGE_OUTDIR)

clean_kernel:
	@$(call MESSAGE,"Cleaning kernel...")
	$(MAKE) distclean -C $(KERNEL_BUILD_PATH)

PHONY += build_kernel clean_kernel

#
# For rootfs
build_rootfs:$(TARGET_SYSTEM_OUTDIR) $(TARGET_IMAGE_OUTDIR) build_rootfs_do_patch build_rootfs_image

build_rootfs_do_patch:
	@$(call MESSAGE, "Building rootfs...")
	$(shell if [ -f $(SYSTEM_PATCH_PATH)/system_patch.sh ];then echo "$(SYSTEM_PATCH_PATH)/system_patch.sh"; fi)
	$(shell if [ -d $(SYSTEM_PATCH_PATH)/system.patch ];then echo "cp -af $(SYSTEM_PATCH_PATH)/system.patch/* $(TARGET_SYSTEM_OUTDIR)"; fi)

build_rootfs_image:
ifeq ($(strip $(ROOTFS_TARGET_TYPE)),jffs2)
	$(HOST_TOOLS_PATH)/mkfs.jffs2 -e $(ROOTFS_JFFS2_ERASESIZE) -p $(ROOTFS_JFFS2_SIZE) -d $(TARGET_SYSTEM_OUTDIR) -o $(TARGET_IMAGE_OUTDIR)/system.jffs2
endif

ifeq ($(strip $(ROOTFS_TARGET_TYPE)),ubi)
	$(HOST_TOOLS_PATH)/mkfs.ubifs -e $(ROOTFS_UBIFS_LEBSIZE) -c $(ROOTFS_UBIFS_MAXLEBCNT) -m $(ROOTFS_UBIFS_MINIOSIZE) -d $(TARGET_SYSTEM_OUTDIR) -o $(TARGET_IMAGE_OUTDIR)/system.ubi
endif

clean_rootfs:
	@$(call MESSAGE, "Delete $(TARGET_SYSTEM_OUTDIR)...")
	rm -rf $(TARGET_SYSTEM_OUTDIR)

PHONY += build_rootfs build_rootfs_do_patch build_rootfs_image clean_rootfs

#
# For SDK
build_sdk:$(TARGET_SDK_OUTDIR) $(TARGET_SYSTEM_OUTDIR)
	@$(call MESSAGE,"Compiling sdk...")
	$(MAKE) $(SDK_BUILD_CONFIG) -C $(SDK_BUILD_PATH)
	$(MAKE) -j$(JLEVEL) -C $(SDK_BUILD_PATH)
	@test -e $(TARGET_SDK_OUTDIR)/lib || mkdir -p $(TARGET_SDK_OUTDIR)/lib
	@test -e $(TARGET_SDK_OUTDIR)/testunit || mkdir -p $(TARGET_SDK_OUTDIR)/testunit
	@test -e $(TARGET_SDK_OUTDIR)/include || ln -s $(SDK_BUILD_PATH)/include $(TARGET_SDK_OUTDIR)/include
	cp -af $(TARGET_SDK_TESTUNIT) $(TARGET_SDK_OUTDIR)/testunit
	cp -af $(TARGET_SDK_SHARED_LIBS) $(TARGET_SDK_OUTDIR)/lib
	cp -af $(TARGET_SDK_SYSTEM) $(TARGET_SYSTEM_OUTDIR)
#	cp -af $(TARGET_SDK_SHARED_LIBS) $(INSTALL_SDKLIB_TO_APPDIR)
#	cp -af $(TARGET_SDK_TESTUNIT) $(INSTALL_SDKTESTUNIT_TO_FSDIR)

clean_sdk:
	@$(call MESSAGE, "Cleaning sdk...")
	$(MAKE) distclean -C $(SDK_BUILD_PATH)

PHONY += build_sdk clean_sdk

#
# For app
app:build_app

build_app:build_sdk
	@$(call MESSAGE,"Compiling app...")
	$(MAKE) -C $(APP_BUILD_PATH)
	test -e $(TARGET_SDK_OUTDIR)/app || mkdir -p $(TARGET_SDK_OUTDIR)/app
	cp -f $(TARGET_APP_EXEFILE) $(TARGET_SDK_OUTDIR)/app
	cp -f $(TARGET_APP_EXEFILE) $(INSTALL_APP_EXEFILE_TO_FSDIR)

clean_app:
	@$(call MESSAGE, "Cleaning app...")
	$(MAKE) distclean -C $(APP_BUILD_PATH)

PHONY += app build_app clean_app

#
# For output
$(TARGET_FILES_OUTDIR):
	test -e $(TARGET_FILES_OUTDIR) || mkdir -p $(TARGET_FILES_OUTDIR)

$(TARGET_DEVICE_OUTDIR):
	test -e $(TARGET_DEVICE_OUTDIR) || mkdir -p $(TARGET_DEVICE_OUTDIR)

$(TARGET_IMAGE_OUTDIR):
	test -e $(TARGET_IMAGE_OUTDIR) || mkdir -p $(TARGET_IMAGE_OUTDIR)

$(TARGET_SDK_OUTDIR):
	test -e $(TARGET_SDK_OUTDIR) || mkdir -p $(TARGET_SDK_OUTDIR)

$(TARGET_SYSTEM_OUTDIR):$(TARGET_DEVICE_OUTDIR)
	test -e $(TARGET_SYSTEM_OUTDIR) || cp -af $(COMMON_SYSTEM_PATH) $(TARGET_DEVICE_OUTDIR)

clean_output:
	@$(call MESSAGE, "Delete $(TARGET_FILES_OUTDIR)...")
	rm -rf $(TARGET_FILES_OUTDIR)

PHONY += $(TARGET_FILES_OUTDIR) $(TARGET_DEVICE_OUTDIR) $(TARGET_IMAGE_OUTDIR) $(TARGET_SDK_OUTDIR) $(TARGET_SYSTEM_OUTDIR) clean_output

#
# For get help
help:
	@echo "Welcome to use Ingenic ms800 platform..."
	@echo "-----------------------------------------------------------------"
	@echo "Compile cmd:"
	@echo "   make or make all         - will compiling all"
	@echo "   make build_xxxx          - will only compiling xxxx"
	@echo "   make clean               - will clean all except TOPDIR/out"
	@echo "   make distclean           - will clean all include TOPDIR/out"
	@echo ""
	@echo "Envsetup info:"
	@echo "   TARGET_DEVICE       = $(TARGET_DEVICE)"
	@echo "   ROOTFS_TARGET_TYPE  = $(ROOTFS_TARGET_TYPE)"
	@echo "   LOADER_BUILD_CONFIG = $(LOADER_BUILD_CONFIG)"
	@echo "   KERNEL_BUILD_CONFIG = $(KERNEL_BUILD_CONFIG)"
	@echo "-----------------------------------------------------------------"

PHONY += help

#
# For clean
clean:clean_sdk \
	  clean_app \
	  clean_loader \
	  clean_kernel \
	  clean_rootfs

distclean:clean_sdk \
          clean_app \
          clean_loader \
          clean_kernel \
          clean_output

PHONY += clean distclean

#
# Declare the .PHONY
.PHONY:$(PHONY)
