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

CURDIR := $(shell pwd)
TOPDIR := $(CURDIR)/..
TARGET_DEVICE ?= ms800
include $(TOPDIR)/build/config.mk


#
# Cross compiler
CROSS_COMPILE ?= mips-linux-gnu-
CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip


OBJ_DIR := $(CURDIR)/target
OUT_DIR := $(CURDIR)/out
SUB_DIR :=


CFLAGS  := -EL -Os -g -std=gnu11 -mhard-float

INCFLAGS := -I$(CURDIR)/include                                            \
            -I$(SDK_BUILD_PATH)/include                                    \
            -I$(SDK_BUILD_PATH)/include/lib

LDFLAFS  := -L$(CURDIR)/lib -L$(SDK_LIBS_OUTPUT_PATH)                      \
            -Wl,-Bstatic                                                   \
            -Wl,-Bdynamic                                                  \
            -lpthread -lrt -lm                                             \
            -lingenic                                                      \
            -lavutil                                                       \
            -lswscale                                                      \
            -lfreetype                                                     \
            -lttdecode

APP_SRCS := $(wildcard *.c)                                                \
            $(wildcard mh1902_manager/*.c)                                 \
            $(wildcard file_manager/*.c)


ifeq ($(strip $(TARGET_DEVICE)),ms800)
CFLAGS += -DCONFIG_G510
APP_SRCS += $(wildcard gsm_manager/g510_manager.c)
endif
ifeq ($(strip $(TARGET_DEVICE)),sq22c)
CFLAGS += -DCONFIG_L718
APP_SRCS += $(wildcard gsm_manager/l718_manager.c)
endif

APP_OBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir ${APP_SRCS}))


export TOPDIR CROSS_COMPILE OBJ_DIR CFLAGS INCFLAGS LDFLAFS

.PHONY : all Tips default

all: default

default: prepare_clean $(OUT_DIR) $(OBJ_DIR) $(SUB_DIR) $(TARGET_APP_NAME) Tips

Tips: $(TARGET_APP_NAME)
	@echo -e '\n Target: "$(OUT_DIR)/$(TARGET_APP_NAME)" is ready\n'

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(SUB_DIR):ECHO
	$(MAKE) -C $@
ECHO:
	@echo $(SUB_DIR) > /dev/null

$(OBJ_DIR)/%.o:%.c
	$(CC) -c $(CFLAGS) $(INCFLAGS) $< -o $@
$(OBJ_DIR)/%.o:gsm_manager/%.c
	$(CC) -c $(CFLAGS) $(INCFLAGS) $< -o $@
$(OBJ_DIR)/%.o:mh1902_manager/%.c
	$(CC) -c $(CFLAGS) $(INCFLAGS) $< -o $@
$(OBJ_DIR)/%.o:file_manager/%.c
	$(CC) -c $(CFLAGS) $(INCFLAGS) $< -o $@

$(TARGET_APP_NAME):$(APP_OBJS)
	$(CC) $(OBJ_DIR)/*.o $(CFLAGS) $(LDFLAFS) -o $(OUT_DIR)/$@
	$(STRIP) $(OUT_DIR)/$@
	$(shell if [ -e $(INSTALL_APP_EXEFILE_TO_FSDIR) ];then echo cp -af $(OUT_DIR)/$(TARGET_APP_NAME) $(INSTALL_APP_EXEFILE_TO_FSDIR); fi)

#
# For clean
.PHONY:prepare_clean clean distclean

prepare_clean:
	@rm -rf $(OBJ_DIR)/main.o

clean:
	@rm -rf $(OUT_DIR)
	@rm -rf $(OBJ_DIR)

distclean:clean
