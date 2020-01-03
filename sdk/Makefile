 #
 #  Copyright (C) 2018 Ingenic Semiconductor Co.,Ltd
 #
 #  Ingenic Linux plarform SDK project
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

ifneq (config.mk, $(wildcard config.mk))
$(error Could not find $(shell pwd)/config.mk !!!)
endif

include config.mk

SOURCE_EXIST  := $(shell if [ -d $(TOPDIR)/source ]; then echo "exist"; else echo "notexist"; fi;)

all: libingenic examples

.PHONY : all clean distclean backup examples examples_clean libingenic libingenic_clean \
		 ilock_sdk_defconfig ms800_sdk_defconfig


define sdk_config
	@echo "#"
	$(shell if [ -f $(TOPDIR)/configs/$1 ]; \
		then \
			cp $(TOPDIR)/configs/$1 $(TOPDIR)/.config; \
			echo "# $1 is written to .config"; \
		else \
			echo "# $(TOPDIR)/configs/$1 does not exist"; fi)
	@echo "#"
endef

#
# SDK config
#
%config:
	$(call sdk_config,$@)

defconfig:
	@cp $(TOPDIR)/configs/$(SDK_DEFCONFIG) $(TOPDIR)/.config
	@echo "#"
	@echo "# $(SDK_DEFCONFIG) is written to .config"
	@echo "#"

ilock_sdk_defconfig:
	$(call sdk_config,$@)

ms800_sdk_defconfig:
	$(call sdk_config,$@)

#
# Put your board's config here
#


#
# Targets
#
libingenic:
ifeq ($(SOURCE_EXIST), exist)
	$(MAKE) -sC $(TOPDIR)/source all
endif
	@test -e $(SYSTEM_OUTDIR)/usr || mkdir -p $(SYSTEM_OUTDIR)/usr
	@test -e $(SYSTEM_OUTDIR)/usr/lib || mkdir -p $(SYSTEM_OUTDIR)/usr/lib
	@test -e $(SYSTEM_OUTDIR)/usr/firmware || mkdir -p $(SYSTEM_OUTDIR)/usr/firmware
	@rm -rf $(SYSTEM_OUTDIR)/usr/lib/*
	@rm -rf $(SYSTEM_OUTDIR)/usr/firmware/*
	@cp -af $(TOPDIR)/lib/$(CONFIG_TARGET_DEVICE)/*.so* $(SYSTEM_OUTDIR)/usr/lib
ifeq ($(CONFIG_EI_FACE_RECOGNIZE), y)
	@cp -af $(TOPDIR)/firmware/ei_face $(SYSTEM_OUTDIR)/usr/firmware/
endif
ifeq ($(CONFIG_FRMAEBUFFER_MANAGER), y)
	@cp -af $(TOPDIR)/firmware/freetype $(SYSTEM_OUTDIR)/usr/firmware/
endif

libingenic_clean:
ifeq ($(SOURCE_EXIST), exist)
	@$(MAKE) -sC $(TOPDIR)/source clean
endif


#
# Examples
#
examples: libingenic
	@rm -rf $(OUTDIR)/examples/*
	@$(MAKE) -sC examples all

examples_clean:
	@$(MAKE) -sC examples clean

#
# For clean
#
clean: examples_clean libingenic_clean
	@rm -rf $(OUTDIR)

distclean: clean
	@rm -rf $(TOPDIR)/.config

backup: distclean
	F=`basename $(TOPDIR)` ; cd .. ; \
	tar --force-local --exclude=.git -Jcvf `date "+$$F-%Y-%m-%d-%T.tar.xz"` $$F
