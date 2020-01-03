# copyright (c) 2011 - 2012 espressif system
#
DRIVER_NAME := esp_prealloc

####################### NORMAL OPTION ########################################
ifeq ($(CONFIG_ESP_USE_SDIO),y)
EXTRA_CFLAGS += -DESP_PRE_MEM              # normal mode, support SDIO
endif
ifeq ($(CONFIG_ESP_USE_SPI),y)
EXTRA_CFLAGS += -DESP_PRE_MEM -DESP_SPI    # normal mode, support SPI
endif
##############################################################################

####################### ADVANCE OPTION ###########################
#EXTRA_CFLAGS += -DESP_SLAB                # ex mode, most users unlikely use this mode, if someone use, please add this macro extractly
##############################################################################

obj-$(CONFIG_ESP_PREALLOC) := $(DRIVER_NAME).o
$(DRIVER_NAME)-y += esp_mem.o
$(DRIVER_NAME)-y += esp_slab.o
$(DRIVER_NAME)-y += esp_mem_init.o
