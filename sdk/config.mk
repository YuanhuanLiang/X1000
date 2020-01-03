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

SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
        else if [ -x /bin/bash ]; then echo /bin/bash; \
        else echo sh; fi; fi)

#
# Top directory
#
TOPDIR := $(shell pwd)
export TOPDIR

#
# SDK defconfig
#
SDK_DEFCONFIG := ilock_sdk_defconfig

#
# To make sure we do not include .config for any targets.
#
is-exist-dot-config := $(wildcard $(TOPDIR)/.config)
no-dot-config-targets := defconfig %config clean distclean

ifeq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
ifneq ($(TOPDIR)/.config, $(is-exist-dot-config))
$(warning ***)
$(warning *** Configuration file "$(TOPDIR)/.config" not found !!!)
$(warning *** Using defconfig file "$(TOPDIR)/configs/$(SDK_DEFCONFIG)")
$(warning *** You can select a new configuration file by run "make xxxconfig")
$(warning ***)
$(shell cp $(TOPDIR)/configs/$(SDK_DEFCONFIG) $(TOPDIR)/.config)
endif
include $(TOPDIR)/.config
else
ifeq ($(TOPDIR)/.config, $(is-exist-dot-config))
include $(TOPDIR)/.config
endif
endif

#
# Out & Tools directory
#
OUTDIR := $(TOPDIR)/out
$(shell [ -d $(OUTDIR) ] || mkdir -p $(OUTDIR))
$(if $(OUTDIR),,$(error output directory "$(OUTDIR)" does not exist))

SYSTEM_OUTDIR := $(OUTDIR)/system
$(shell [ -d $(SYSTEM_OUTDIR) ] || mkdir -p $(SYSTEM_OUTDIR))
$(if $(SYSTEM_OUTDIR),,$(error output directory "$(SYSTEM_OUTDIR)" does not exist))
