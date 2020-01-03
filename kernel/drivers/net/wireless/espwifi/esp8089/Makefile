# copyright (c) 2010 - 2013 espressif system
#

DRIVER_NAME := esp8089

#ccflags-y += -DDEBUG -DSIP_DEBUG -DFAST_TX_STATUS -DKERNEL_IV_WAR -DRX_SENDUP_SYNC -DHAS_FW -DTEST_MODE -DINIT_DATA_CONF -DESP_CLASS

obj-$(CONFIG_ESP8089) := $(DRIVER_NAME).o
$(DRIVER_NAME)-y += esp_init.o
$(DRIVER_NAME)-y += esp_debug.o
$(DRIVER_NAME)-$(CONFIG_ESP_USE_SDIO) += sdio_sif_esp.o
$(DRIVER_NAME)-$(CONFIG_ESP_USE_SPI) += spi_sif_esp.o
$(DRIVER_NAME)-y += esp_io.o
$(DRIVER_NAME)-y += esp_file.o
$(DRIVER_NAME)-y += esp_main.o
$(DRIVER_NAME)-y += esp_sip.o
$(DRIVER_NAME)-y += esp_ext.o
$(DRIVER_NAME)-y += esp_ctrl.o
$(DRIVER_NAME)-y += esp_mac80211.o
$(DRIVER_NAME)-y += esp_debug.o
$(DRIVER_NAME)-y += esp_utils.o
$(DRIVER_NAME)-y += esp_pm.o
$(DRIVER_NAME)-y += testmode.o

ccflags-y += -DSIP_DEBUG -DFAST_TX_STATUS -DKERNEL_IV_WAR -DRX_SENDUP_SYNC -DHAS_FW -DTEST_MODE -DESP_CLASS -DLOWER_CLK -DHAS_INIT_DATA #-DDEBUG -DINIT_DATA_CONF -DESP_PREALLOC
