 #
 #  Copyright (C) 2018, Qiuwei Wang <qiuwei.wang@ingenic.com, panddio@163.com>
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
TOPDIR ?= ..

#
# To make sure we do not include .config for any targets.
#
is-exist-dot-config := $(wildcard $(TOPDIR)/.config)
no-dot-config-targets := clean distclean


ifeq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
ifneq ($(TOPDIR)/.config, $(is-exist-dot-config))
$(error *** Configuration file "$(TOPDIR)/.config" not found !!!)
endif
include $(TOPDIR)/.config
else
ifeq ($(TOPDIR)/.config, $(is-exist-dot-config))
include $(TOPDIR)/.config
endif
endif

#
# Define target name
#
TARGET_NAME := ingenic


#
# Out & Tools directory
#
OUTDIR := $(TOPDIR)/out

LIBS_DIR := $(TOPDIR)/lib/$(CONFIG_TARGET_DEVICE)
$(if $(LIBS_DIR),,$(error librarys output directory "$(LIBS_DIR)" does not exist))

EXAMPLES_OUTDIR := $(OUTDIR)/examples
$(shell [ -d $(EXAMPLES_OUTDIR) ] || mkdir -p $(EXAMPLES_OUTDIR))
$(if $(EXAMPLES_OUTDIR),,$(error output directory "$(EXAMPLES_OUTDIR)" does not exist))

#
# Cross compiler
#
CROSS_COMPILE ?= mips-linux-gnu-
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip

#
# Compiler & Linker options
#
ARFLAGS := rcv
INCLUDES := -I$(TOPDIR)/include                                                \
            -I$(TOPDIR)/include/lib                                            \
            -I$(TOPDIR)/include/lib/zlib                                       \
            -I$(TOPDIR)/include/lib/zip/minizip

CFLAGS := -O2 -g -std=gnu11 $(INCLUDES) -fPIC -D_GNU_SOURCE -mhard-float       \
          -funwind-tables
CHECKFLAGS := -Wall -Wuninitialized -Wundef
LDSHFLAGS := -shared -Wl,-Bsymbolic
LDFLAGS := -rdynamic

#
# Library link - Static
#
LIBS_STATIC_FLAGS += -Wl,-Bstatic

ifeq ($(CONFIG_LIB_FINGERPRINT_GD), y)
LIBS_STATIC_FLAGS += -lgoodix_fingerprint
endif
#
# Put your static library here
#



#
# Library link - Dynamic
#
LIBS_DYNAMIC_FLAGS += -Wl,-Bdynamic -pthread -lm -lrt -ldl -lstdc++

ifeq ($(CONFIG_LIB_ALSA), y)
LIBS_DYNAMIC_FLAGS += -lasound
endif

ifeq ($(CONFIG_LIB_OPENSSL), y)
LIBS_DYNAMIC_FLAGS += -lcrypto -lssl
endif

ifeq ($(CONFIG_FRMAEBUFFER_MANAGER), y)
LIBS_DYNAMIC_FLAGS += -lfreetype
endif

ifeq ($(CONFIG_LIB_FINGERPRINT_MA), y)
LIBS_DYNAMIC_FLAGS += -lmafp-mips
endif

ifeq ($(CONFIG_LIB_FACE_JIUFENG), y)
LIBS_DYNAMIC_FLAGS += -lNmIrFaceSdk
endif

ifeq ($(CONFIG_LIB_FACE_EI), y)
LIBS_DYNAMIC_FLAGS += -lwfFR
endif

ifeq ($(CONFIG_LIB_FINGERPRINT_FPC), y)
LIBS_DYNAMIC_FLAGS += -lfps_360_linux
endif

ifeq ($(CONFIG_LIB_FINGERPRINT_BYD), y)
LIBS_DYNAMIC_FLAGS += -lbyd_fingerprint
endif

ifeq ($(CONFIG_LIB_FFMPEG), y)
LIBS_DYNAMIC_FLAGS += -lavutil -lswscale
endif

ifeq ($(CONFIG_LIB_HW_JPEG), y)
LIBS_DYNAMIC_FLAGS += -ljpeg-hw
endif

ifeq ($(CONFIG_LIB_QRCODE_TUTENG), y)
LIBS_DYNAMIC_FLAGS += -lttdecode
endif

#
# Put your dynamic library here
#

override LDFLAGS := $(LDFLAGS) $(LIBS_STATIC_FLAGS) $(LIBS_DYNAMIC_FLAGS)

ifeq ($(CONFIG_LOCAL_DEBUG), y)
CFLAGS += -DLOCAL_DEBUG -DDEBUG
endif

ifeq ($(CONFIG_DEBUG_TIMESTAMP), y)
CFLAGS += -DDEBUG_TIMESTAMP
endif

ifeq ($(CONFIG_DEBUG_DHCP), y)
CFLAGS += -DDEBUG_DHCP
endif

override CFLAGS := $(CHECKFLAGS) $(CFLAGS)

#
# Quiet compile
#
COMPILE_SRC := $(CC) $(CFLAGS) -c
LINK_OBJS   := $(CC) $(CFLAGS)

ifndef V
QUIET_AR        = @echo -e "  AR\t$@";
QUIET_CC        = @echo -e "  CC\t$@";
QUIET_LINK      = @echo -e "  LINK\t$@";

QUITE_TEST_BUILD = @echo -e "  BUILD\t"
QUITE_TEST_CLEAN = @echo -e "  CLEAN\t"
endif

%.o:%.c
	$(QUIET_CC) $(COMPILE_SRC) $< -o $@
