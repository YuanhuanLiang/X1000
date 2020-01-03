 #
 #  Copyright (C) 2017, Zhang YanMing <yanmin.zhang@ingenic.com, jamincheung@126.com>
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

TOPDIR ?= ..

include ./config.mk

LINK_LIBS := -L$(LIBS_DIR) -l$(TARGET_NAME)

define build_example
	$(QUIET_LINK)$(LINK_OBJS) $(LINK_LIBS) -o $(EXAMPLES_OUTDIR)/$(2) $(1) $(LDFLAGS)
endef

define clean_example
	@rm -rf $(1) $(EXAMPLES_OUTDIR)/$(2)
endef

BUILD_EXAMPLES :=
CLEAN_EXAMPLES :=

#
# 74HC595
#
ifeq ($(CONFIG_74HC595_MANAGER), y)
EXAMPLE_74HC595 := test_74hc595
EXAMPLE_74HC595_CLEAN := test_74hc595_clean
EXAMPLE_74HC595_OBJ := 74hc595/main.o
$(EXAMPLE_74HC595): $(EXAMPLE_74HC595_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_74HC595_CLEAN):
	$(call clean_example,$(EXAMPLE_74HC595_OBJ),$(EXAMPLE_74HC595))

BUILD_EXAMPLES += $(EXAMPLE_74HC595)
CLEAN_EXAMPLES += $(EXAMPLE_74HC595_CLEAN)
endif


#
# SL2623
#
ifeq ($(CONFIG_SL2623_MANAGER), y)
EXAMPLE_SL2623 := test_sl2623
EXAMPLE_SL2623_CLEAN := test_sl2623_clean
EXAMPLE_SL2623_OBJ := nfc/main.o nfc/iso14443_4.o nfc/iso14443a.o nfc/iso14443b.o nfc/mifare.o \
                      nfc/Rfid_interface.o nfc/sl2523.o nfc/sl2523-spi.o nfc/sl2523-gpio.o
$(EXAMPLE_SL2623): $(EXAMPLE_SL2623_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_SL2623_CLEAN):
	$(call clean_example,$(EXAMPLE_SL2623_OBJ),$(EXAMPLE_SL2623))

BUILD_EXAMPLES += $(EXAMPLE_SL2623)
CLEAN_EXAMPLES += $(EXAMPLE_SL2623_CLEAN)
endif

#
# TM1620
#
ifeq ($(CONFIG_TM1620_MANAGER), y)
EXAMPLE_TM1620 := test_tm1620
EXAMPLE_TM1620_CLEAN := test_tm1620_clean
EXAMPLE_TM1620_OBJ := tm1620/main.o
$(EXAMPLE_TM1620): $(EXAMPLE_TM1620_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_TM1620_CLEAN):
	$(call clean_example,$(EXAMPLE_TM1620_OBJ),$(EXAMPLE_TM1620))

BUILD_EXAMPLES += $(EXAMPLE_TM1620)
CLEAN_EXAMPLES += $(EXAMPLE_TM1620_CLEAN)
endif

#
# Timer
#
ifeq ($(CONFIG_TIMER_MANAGER), y)
EXAMPLE_TIMER := test_timer
EXAMPLE_TIMER_CLEAN := test_timer_clean
EXAMPLE_TIMER_OBJ := timer/main.o
$(EXAMPLE_TIMER): $(EXAMPLE_TIMER_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_TIMER_CLEAN):
	$(call clean_example,$(EXAMPLE_TIMER_OBJ),$(EXAMPLE_TIMER))

BUILD_EXAMPLES += $(EXAMPLE_TIMER)
CLEAN_EXAMPLES += $(EXAMPLE_TIMER_CLEAN)
endif

#
# Alarm
#
ifeq ($(CONFIG_ALARM_MANAGER), y)
EXAMPLE_ALARM := test_alarm
EXAMPLE_ALARM_CLEAN := test_alarm_clean
EXAMPLE_ALARM_OBJ := alarm/main.o
$(EXAMPLE_ALARM): $(EXAMPLE_ALARM_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_ALARM_CLEAN):
	$(call clean_example,$(EXAMPLE_ALARM_OBJ),$(EXAMPLE_ALARM))

BUILD_EXAMPLES += $(EXAMPLE_ALARM)
CLEAN_EXAMPLES += $(EXAMPLE_ALARM_CLEAN)
endif


#
# Audio alsa
#
ifeq ($(CONFIG_ALSA_AUDIO_PLAYER)_$(CONFIG_ALSA_AUDIO_MIXER)_$(CONFIG_INPUT_MANAGER), y_y_y)
EXAMPLE_ALSA_PLAY := test_wave_play
EXAMPLE_ALSA_PLAY_CLEAN := test_wave_play_clean
EXAMPLE_ALSA_PLAY_OBJ := audio/alsa/test_wave_play.o
$(EXAMPLE_ALSA_PLAY): $(EXAMPLE_ALSA_PLAY_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_ALSA_PLAY_CLEAN):
	$(call clean_example,$(EXAMPLE_ALSA_PLAY_OBJ),$(EXAMPLE_ALSA_PLAY))

BUILD_EXAMPLES += $(EXAMPLE_ALSA_PLAY)
CLEAN_EXAMPLES += $(EXAMPLE_ALSA_PLAY_CLEAN)
endif

ifeq ($(CONFIG_ALSA_AUDIO_RECORDER)_$(CONFIG_ALSA_AUDIO_RECORDER)_$(CONFIG_INPUT_MANAGER), y_y_y)
EXAMPLE_ALSA_RECORD := test_wave_record
EXAMPLE_ALSA_RECORD_CLEAN := test_wave_record_clean
EXAMPLE_ALSA_RECORD_OBJ := audio/alsa/test_wave_record.o
$(EXAMPLE_ALSA_RECORD): $(EXAMPLE_ALSA_RECORD_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_ALSA_RECORD_CLEAN):
	$(call clean_example,$(EXAMPLE_ALSA_RECORD_OBJ),$(EXAMPLE_ALSA_RECORD))

BUILD_EXAMPLES += $(EXAMPLE_ALSA_RECORD)
CLEAN_EXAMPLES += $(EXAMPLE_ALSA_RECORD_CLEAN)
endif

ifeq ($(CONFIG_ALSA_AUDIO_MIXER), y)
EXAMPLE_ALSA_MIXER := test_mixer
EXAMPLE_ALSA_MIXER_CLEAN := test_mixer_clean
EXAMPLE_ALSA_MIXER_OBJ := audio/alsa/test_mixer.o
$(EXAMPLE_ALSA_MIXER): $(EXAMPLE_ALSA_MIXER_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_ALSA_MIXER_CLEAN):
	$(call clean_example,$(EXAMPLE_ALSA_MIXER_OBJ),$(EXAMPLE_ALSA_MIXER))

BUILD_EXAMPLES += $(EXAMPLE_ALSA_MIXER)
CLEAN_EXAMPLES += $(EXAMPLE_ALSA_MIXER_CLEAN)
endif

ifeq ($(CONFIG_ALSA_AUDIO_PLAYER)_$(CONFIG_ALSA_AUDIO_RECORDER)_$(CONFIG_ALSA_AUDIO_MIXER), y_y_y)
EXAMPLE_ALSA_PCM_LOOP := test_pcm_loop
EXAMPLE_ALSA_PCM_LOOP_CLEAN := test_pcm_loop_clean
EXAMPLE_ALSA_PCM_LOOP_OBJ := audio/alsa/test_pcm_loop.o
$(EXAMPLE_ALSA_PCM_LOOP): $(EXAMPLE_ALSA_PCM_LOOP_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_ALSA_PCM_LOOP_CLEAN):
	$(call clean_example,$(EXAMPLE_ALSA_PCM_LOOP_OBJ),$(EXAMPLE_ALSA_PCM_LOOP))

BUILD_EXAMPLES += $(EXAMPLE_ALSA_PCM_LOOP)
CLEAN_EXAMPLES += $(EXAMPLE_ALSA_PCM_LOOP_CLEAN)
endif


#
# Battery
#
ifeq ($(CONFIG_BATTERY_MANAGER), y)
EXAMPLE_BATTERY := test_battery
EXAMPLE_BATTERY_CLEAN := test_battery_clean
EXAMPLE_BATTERY_OBJ := battery/main.o
$(EXAMPLE_BATTERY): $(EXAMPLE_BATTERY_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_BATTERY_CLEAN):
	$(call clean_example,$(EXAMPLE_BATTERY_OBJ),$(EXAMPLE_BATTERY))

BUILD_EXAMPLES += $(EXAMPLE_BATTERY)
CLEAN_EXAMPLES += $(EXAMPLE_BATTERY_CLEAN)
endif


#
# Camera(char device)
#
ifeq ($(CONFIG_CHAR_CAMERA_MANAGER), y)
EXAMPLE_CAMERA_CHAR := test_camera_char
EXAMPLE_CAMERA_CHAR_CLEAN := test_camera_char_clean
EXAMPLE_CAMERA_CHAR_OBJ := camera/main.o                                       \
                           camera/sensor.o
$(EXAMPLE_CAMERA_CHAR): $(EXAMPLE_CAMERA_CHAR_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_CAMERA_CHAR_CLEAN):
	$(call clean_example,$(EXAMPLE_CAMERA_CHAR_OBJ),$(EXAMPLE_CAMERA_CHAR))

BUILD_EXAMPLES += $(EXAMPLE_CAMERA_CHAR)
CLEAN_EXAMPLES += $(EXAMPLE_CAMERA_CHAR_CLEAN)
endif


#
# Camera(V4L)
#
ifeq ($(CONFIG_V4L2_CAMERA_MANAGER), y)
EXAMPLE_CAMERA_V4L := test_camerav4l2
EXAMPLE_CAMERA_V4L_CLEAN := test_camerav4l_clean
EXAMPLE_CAMERA_V4L_OBJ :=  camera_v4l/main.o camera_v4l/uvc-gadget.o
$(EXAMPLE_CAMERA_V4L): $(EXAMPLE_CAMERA_V4L_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_CAMERA_V4L_CLEAN):
	$(call clean_example,$(EXAMPLE_CAMERA_V4L_OBJ),$(EXAMPLE_CAMERA_V4L))

BUILD_EXAMPLES += $(EXAMPLE_CAMERA_V4L)
CLEAN_EXAMPLES += $(EXAMPLE_CAMERA_V4L_CLEAN)
endif

#
# JPG_API
#
ifeq ($(CONFIG_JPG), y)
EXAMPLE_JPG_API := test_jpg_api
EXAMPLE_JPG_API_CLEAN := test_jpg_clean
EXAMPLE_JPG_API_OBJ :=jpg_api/test-jpeg.o jpg_api/convert_soft_C.o \
						jpg_api/convert_soft_mxu.o jpg_api/jpeg_encode2.o \
						jpg_api/jpeg_encode2.o jpg_api/jz_mem.o jpg_api/vpu_common.o
$(EXAMPLE_JPG_API): $(EXAMPLE_JPG_API_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_JPG_API_CLEAN):
	$(call clean_example,$(EXAMPLE_JPG_API_OBJ),$(EXAMPLE_JPG_API))

BUILD_EXAMPLES += $(EXAMPLE_JPG_API)
CLEAN_EXAMPLES += $(EXAMPLE_JPG_API_CLEAN)
endif

#
# Cypress(PSOC4)
#
ifeq ($(CONFIG_CYPRESS_MANAGER), y)
EXAMPLE_CYPRESS := test_cypress
EXAMPLE_CYPRESS_CLEAN := test_cypress_clean
EXAMPLE_CYPRESS_OBJ :=  cypress/main.o
$(EXAMPLE_CYPRESS): $(EXAMPLE_CYPRESS_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_CYPRESS_CLEAN):
	$(call clean_example,$(EXAMPLE_CYPRESS_OBJ),$(EXAMPLE_CYPRESS))

BUILD_EXAMPLES += $(EXAMPLE_CYPRESS)
CLEAN_EXAMPLES += $(EXAMPLE_CYPRESS_CLEAN)
endif


#
# EFUSE
#
ifeq ($(CONFIG_EFUSE_MANAGER), y)
EXAMPLE_EFUSE := test_efuse
EXAMPLE_EFUSE_CLEAN := test_efuse_clean
EXAMPLE_EFUSE_OBJ :=  efuse/main.o
$(EXAMPLE_EFUSE): $(EXAMPLE_EFUSE_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_EFUSE_CLEAN):
	$(call clean_example,$(EXAMPLE_EFUSE_OBJ),$(EXAMPLE_EFUSE))

BUILD_EXAMPLES += $(EXAMPLE_EFUSE)
CLEAN_EXAMPLES += $(EXAMPLE_EFUSE_CLEAN)
endif


#
# Jiufeng face detect
#
ifeq ($(CONFIG_JIUFENG_FACE_RECOGNIZE)_$(CONFIG_LIB_FACE_JIUFENG)_$(CONFIG_GRAPHICS_DRAWER)_$(CONFIG_FRMAEBUFFER_MANAGER)_$(CONFIG_ALARM_MANAGER)_$(CONFIG_V4L2_CAMERA_MANAGER), y_y_y_y_y_y)
EXAMPLE_FACE_JIUFENG := test_face_jiufeng
EXAMPLE_FACE_JIUFENG_CLEAN := test_face_jiufeng_clean
EXAMPLE_FACE_JIUFENG_OBJ :=  face/jiufeng/test_jiufeng.o
$(EXAMPLE_FACE_JIUFENG): $(EXAMPLE_FACE_JIUFENG_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FACE_JIUFENG_CLEAN):
	$(call clean_example,$(EXAMPLE_FACE_JIUFENG_OBJ),$(EXAMPLE_FACE_JIUFENG))

BUILD_EXAMPLES += $(EXAMPLE_FACE_JIUFENG)
CLEAN_EXAMPLES += $(EXAMPLE_FACE_JIUFENG_CLEAN)
endif

#
# EI face detect
#
ifeq ($(CONFIG_EI_FACE_RECOGNIZE)_$(CONFIG_LIB_FACE_EI)_$(CONFIG_GRAPHICS_DRAWER)_$(CONFIG_FRMAEBUFFER_MANAGER)_$(CONFIG_ALARM_MANAGER)_$(CONFIG_V4L2_CAMERA_MANAGER), y_y_y_y_y_y)
EXAMPLE_FACE_EI := test_face_ei
EXAMPLE_FACE_EI_CLEAN := test_face_ei_clean
EXAMPLE_FACE_EI_OBJ :=  face/ei/test_ei.o
$(EXAMPLE_FACE_EI): $(EXAMPLE_FACE_EI_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FACE_EI_CLEAN):
	$(call clean_example,$(EXAMPLE_FACE_EI_OBJ),$(EXAMPLE_FACE_EI))

BUILD_EXAMPLES += $(EXAMPLE_FACE_EI)
CLEAN_EXAMPLES += $(EXAMPLE_FACE_EI_CLEAN)
endif


#
# Framebuffer
#
ifeq ($(CONFIG_FRMAEBUFFER_MANAGER), y)
EXAMPLE_FB := test_fb
EXAMPLE_FB_CLEAN := test_fb_clean
EXAMPLE_FB_OBJ :=  fb/main.o
$(EXAMPLE_FB): $(EXAMPLE_FB_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FB_CLEAN):
	$(call clean_example,$(EXAMPLE_FB_OBJ),$(EXAMPLE_FB))

BUILD_EXAMPLES += $(EXAMPLE_FB)
CLEAN_EXAMPLES += $(EXAMPLE_FB_CLEAN)
endif

#
# Freetype
#
ifeq ($(CONFIG_FRMAEBUFFER_MANAGER), y)
EXAMPLE_FREETYPE := test_freetype
EXAMPLE_FREETYPE_CLEAN := test_freetype_clean
EXAMPLE_FREETYPE_OBJ :=  freetype/main.o
$(EXAMPLE_FREETYPE): $(EXAMPLE_FREETYPE_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FREETYPE_CLEAN):
	$(call clean_example,$(EXAMPLE_FREETYPE_OBJ),$(EXAMPLE_FREETYPE))

BUILD_EXAMPLES += $(EXAMPLE_FREETYPE)
CLEAN_EXAMPLES += $(EXAMPLE_FREETYPE_CLEAN)
endif


#
# Fingerprint
#
ifeq ($(CONFIG_LIB_FINGERPRINT_MA)_$(CONFIG_POWER_MANAGER), y_y)
EXAMPLE_FINGER_MA := test_ma_fp
EXAMPLE_FINGER_MA_CLEAN := test_ma_fp_clean
EXAMPLE_FINGER_MA_OBJ :=  fingerprint/microarray/test_ma.o
$(EXAMPLE_FINGER_MA): $(EXAMPLE_FINGER_MA_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FINGER_MA_CLEAN):
	$(call clean_example,$(EXAMPLE_FINGER_MA_OBJ),$(EXAMPLE_FINGER_MA))

BUILD_EXAMPLES += $(EXAMPLE_FINGER_MA)
CLEAN_EXAMPLES += $(EXAMPLE_FINGER_MA_CLEAN)
endif

ifeq ($(CONFIG_LIB_FINGERPRINT_GD)_$(CONFIG_POWER_MANAGER), y_y)
EXAMPLE_FINGER_GF := test_gf_fp
EXAMPLE_FINGER_GF_CLEAN := test_gf_fp_clean
EXAMPLE_FINGER_GF_OBJ :=  fingerprint/goodix/test_gf.o
$(EXAMPLE_FINGER_GF): $(EXAMPLE_FINGER_GF_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FINGER_GF_CLEAN):
	$(call clean_example,$(EXAMPLE_FINGER_GF_OBJ),$(EXAMPLE_FINGER_GF))

BUILD_EXAMPLES += $(EXAMPLE_FINGER_GF)
CLEAN_EXAMPLES += $(EXAMPLE_FINGER_GF_CLEAN)
endif

ifeq ($(CONFIG_LIB_FINGERPRINT_FPC)_$(CONFIG_POWER_MANAGER), y_y)
EXAMPLE_FINGER_FPC := test_fpc_fp
EXAMPLE_FINGER_FPC_CLEAN := test_fpc_fp_clean
EXAMPLE_FINGER_FPC_OBJ :=  fingerprint/fpc/test_fpc.o
$(EXAMPLE_FINGER_FPC): $(EXAMPLE_FINGER_FPC_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FINGER_FPC_CLEAN):
	$(call clean_example,$(EXAMPLE_FINGER_FPC_OBJ),$(EXAMPLE_FINGER_FPC))

BUILD_EXAMPLES += $(EXAMPLE_FINGER_FPC)
CLEAN_EXAMPLES += $(EXAMPLE_FINGER_FPC_CLEAN)
endif

ifeq ($(CONFIG_LIB_FINGERPRINT_BYD)_$(CONFIG_POWER_MANAGER), y_y)
EXAMPLE_FINGER_BYD := test_byd
EXAMPLE_FINGER_BYD_CLEAN := test_byd_clean
EXAMPLE_FINGER_BYD_OBJ :=  fingerprint/byd/test_byd.o
$(EXAMPLE_FINGER_BYD): $(EXAMPLE_FINGER_BYD_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FINGER_BYD_CLEAN):
	$(call clean_example,$(EXAMPLE_FINGER_BYD_OBJ),$(EXAMPLE_FINGER_BYD))

BUILD_EXAMPLES += $(EXAMPLE_FINGER_BYD)
CLEAN_EXAMPLES += $(EXAMPLE_FINGER_BYD_CLEAN)
endif

#
# QRcode
#
ifeq ($(CONFIG_LIB_QRCODE_TUTENG)_$(CONFIG_FRMAEBUFFER_MANAGER)_$(CONFIG_V4L2_CAMERA_MANAGER), y_y_y)
EXAMPLE_QRCODE_TUTENG := test_tuteng
EXAMPLE_QRCODE_TUTENG_CLEAN := test_tuteng_clean
EXAMPLE_QRCODE_TUTENG_OBJ :=  qrcode/test_tuteng.o
$(EXAMPLE_QRCODE_TUTENG): $(EXAMPLE_QRCODE_TUTENG_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_QRCODE_TUTENG_CLEAN):
	$(call clean_example,$(EXAMPLE_QRCODE_TUTENG_OBJ),$(EXAMPLE_QRCODE_TUTENG))

BUILD_EXAMPLES += $(EXAMPLE_QRCODE_TUTENG)
CLEAN_EXAMPLES += $(EXAMPLE_QRCODE_TUTENG_CLEAN)
endif

#
# MTD flash
#
ifeq ($(CONFIG_MTD_FLASH_MANAGER), y)
EXAMPLE_MTD_FLASH := test_flash
EXAMPLE_MTD_FLASH_CLEAN := test_flash_clean
EXAMPLE_MTD_FLASH_OBJ :=  flash/main.o
$(EXAMPLE_MTD_FLASH): $(EXAMPLE_MTD_FLASH_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_MTD_FLASH_CLEAN):
	$(call clean_example,$(EXAMPLE_MTD_FLASH_OBJ),$(EXAMPLE_MTD_FLASH))

BUILD_EXAMPLES += $(EXAMPLE_MTD_FLASH)
CLEAN_EXAMPLES += $(EXAMPLE_MTD_FLASH_CLEAN)
endif


#
# GPIO
#
ifeq ($(CONFIG_GPIO_MANAGER), y)
EXAMPLE_GPIO := test_gpio
EXAMPLE_GPIO_CLEAN := test_gpio_clean
EXAMPLE_GPIO_OBJ :=  gpio/main.o
$(EXAMPLE_GPIO): $(EXAMPLE_GPIO_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_GPIO_CLEAN):
	$(call clean_example,$(EXAMPLE_GPIO_OBJ),$(EXAMPLE_GPIO))

BUILD_EXAMPLES += $(EXAMPLE_GPIO)
CLEAN_EXAMPLES += $(EXAMPLE_GPIO_CLEAN)
endif


#
# Graphics
#
ifeq ($(CONFIG_GRAPHICS_DRAWER), y)
EXAMPLE_GRAPHICS := test_graphics
EXAMPLE_GRAPHICS_CLEAN := test_graphics_clean
EXAMPLE_GRAPHICS_OBJ :=  graphics/main.o
$(EXAMPLE_GRAPHICS): $(EXAMPLE_GRAPHICS_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_GRAPHICS_CLEAN):
	$(call clean_example,$(EXAMPLE_GRAPHICS_OBJ),$(EXAMPLE_GRAPHICS))

BUILD_EXAMPLES += $(EXAMPLE_GRAPHICS)
CLEAN_EXAMPLES += $(EXAMPLE_GRAPHICS_CLEAN)
endif


#
# I2C
#
ifeq ($(CONFIG_I2C_MANAGER), y)
EXAMPLE_I2C := test_i2c
EXAMPLE_I2C_CLEAN := test_i2c_clean
EXAMPLE_I2C_OBJ :=  i2c/main.o
$(EXAMPLE_I2C): $(EXAMPLE_I2C_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_I2C_CLEAN):
	$(call clean_example,$(EXAMPLE_I2C_OBJ),$(EXAMPLE_I2C))

BUILD_EXAMPLES += $(EXAMPLE_I2C)
CLEAN_EXAMPLES += $(EXAMPLE_I2C_CLEAN)
endif


#
# Input
#
ifeq ($(CONFIG_INPUT_MANAGER), y)
EXAMPLE_INPUT := test_input
EXAMPLE_INPUT_CLEAN := test_input_clean
EXAMPLE_INPUT_OBJ :=  input/main.o
$(EXAMPLE_INPUT): $(EXAMPLE_INPUT_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_INPUT_CLEAN):
	$(call clean_example,$(EXAMPLE_INPUT_OBJ),$(EXAMPLE_INPUT))

BUILD_EXAMPLES += $(EXAMPLE_INPUT)
CLEAN_EXAMPLES += $(EXAMPLE_INPUT_CLEAN)
endif


#
# Network
#
ifeq ($(CONFIG_NETWORK_MANAGER), y)
EXAMPLE_NETWORK := test_network
EXAMPLE_NETWORK_CLEAN := test_network_clean
EXAMPLE_NETWORK_OBJ :=  network/main.o
$(EXAMPLE_NETWORK): $(EXAMPLE_NETWORK_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_NETWORK_CLEAN):
	$(call clean_example,$(EXAMPLE_NETWORK_OBJ),$(EXAMPLE_NETWORK))

BUILD_EXAMPLES += $(EXAMPLE_NETWORK)
CLEAN_EXAMPLES += $(EXAMPLE_NETWORK_CLEAN)
endif


#
# Wifi
#
ifeq ($(CONFIG_WIFI_MANAGER), y)
EXAMPLE_WIFI := test_wifi
EXAMPLE_WIFI_CLEAN := test_wifi_clean
EXAMPLE_WIFI_OBJ :=  wifi/main.o
$(EXAMPLE_WIFI): $(EXAMPLE_WIFI_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_WIFI_CLEAN):
	$(call clean_example,$(EXAMPLE_WIFI_OBJ),$(EXAMPLE_WIFI))

BUILD_EXAMPLES += $(EXAMPLE_WIFI)
CLEAN_EXAMPLES += $(EXAMPLE_WIFI_CLEAN)
endif


#
# Mount
#
ifeq ($(CONFIG_MOUNT_MANAGER), y)
EXAMPLE_MOUNT := test_mount
EXAMPLE_MOUNT_CLEAN := test_mount_clean
EXAMPLE_MOUNT_OBJ :=  mount/main.o
$(EXAMPLE_MOUNT): $(EXAMPLE_MOUNT_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_MOUNT_CLEAN):
	$(call clean_example,$(EXAMPLE_MOUNT_OBJ),$(EXAMPLE_MOUNT))

BUILD_EXAMPLES += $(EXAMPLE_MOUNT)
CLEAN_EXAMPLES += $(EXAMPLE_MOUNT_CLEAN)
endif


#
# Power
#
ifeq ($(CONFIG_POWER_MANAGER), y)
EXAMPLE_POWER := test_power
EXAMPLE_POWER_CLEAN := test_power_clean
EXAMPLE_POWER_OBJ :=  power/main.o
$(EXAMPLE_POWER): $(EXAMPLE_POWER_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_POWER_CLEAN):
	$(call clean_example,$(EXAMPLE_POWER_OBJ),$(EXAMPLE_POWER))

BUILD_EXAMPLES += $(EXAMPLE_POWER)
CLEAN_EXAMPLES += $(EXAMPLE_POWER_CLEAN)
endif


#
# PWM
#
ifeq ($(CONFIG_PWM_MANAGER), y)
EXAMPLE_PWM := test_pwm
EXAMPLE_PWM_CLEAN := test_pwm_clean
EXAMPLE_PWM_OBJ :=  pwm/main.o
$(EXAMPLE_PWM): $(EXAMPLE_PWM_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_PWM_CLEAN):
	$(call clean_example,$(EXAMPLE_PWM_OBJ),$(EXAMPLE_PWM))

BUILD_EXAMPLES += $(EXAMPLE_PWM)
CLEAN_EXAMPLES += $(EXAMPLE_PWM_CLEAN)
endif


#
# RTC
#
ifeq ($(CONFIG_RTC_MANAGER), y)
EXAMPLE_RTC := test_rtc
EXAMPLE_RTC_CLEAN := test_rtc_clean
EXAMPLE_RTC_OBJ :=  rtc/main.o
$(EXAMPLE_RTC): $(EXAMPLE_RTC_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_RTC_CLEAN):
	$(call clean_example,$(EXAMPLE_RTC_OBJ),$(EXAMPLE_RTC))

BUILD_EXAMPLES += $(EXAMPLE_RTC)
CLEAN_EXAMPLES += $(EXAMPLE_RTC_CLEAN)
endif


#
# Security
#
ifeq ($(CONFIG_SECURITY_MANAGER), y)
EXAMPLE_SECURITY := test_security
EXAMPLE_SECURITY_CLEAN := test_security_clean
EXAMPLE_SECURITY_OBJ :=  security/main.o
$(EXAMPLE_SECURITY): $(EXAMPLE_SECURITY_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_SECURITY_CLEAN):
	$(call clean_example,$(EXAMPLE_SECURITY_OBJ),$(EXAMPLE_SECURITY))

BUILD_EXAMPLES += $(EXAMPLE_SECURITY)
CLEAN_EXAMPLES += $(EXAMPLE_SECURITY_CLEAN)
endif


#
# SPI
#
ifeq ($(CONFIG_SPI_MANAGER), y)
EXAMPLE_SPI := test_spi
EXAMPLE_SPI_CLEAN := test_spi_clean
EXAMPLE_SPI_OBJ :=  spi/main.o
$(EXAMPLE_SPI): $(EXAMPLE_SPI_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_SPI_CLEAN):
	$(call clean_example,$(EXAMPLE_SPI_OBJ),$(EXAMPLE_SPI))

BUILD_EXAMPLES += $(EXAMPLE_SPI)
CLEAN_EXAMPLES += $(EXAMPLE_SPI_CLEAN)
endif


#
# UART
#
ifeq ($(CONFIG_UART_MANAGER), y)
EXAMPLE_UART := test_uart
EXAMPLE_UART_CLEAN := test_uart_clean
EXAMPLE_UART_OBJ :=  uart/main.o
$(EXAMPLE_UART): $(EXAMPLE_UART_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_UART_CLEAN):
	$(call clean_example,$(EXAMPLE_UART_OBJ),$(EXAMPLE_UART))

BUILD_EXAMPLES += $(EXAMPLE_UART)
CLEAN_EXAMPLES += $(EXAMPLE_UART_CLEAN)
endif


#
# USB device
#
ifeq ($(CONFIG_USB_DEVICE_MANAGER), y)
EXAMPLE_USB_HID := test_hid
EXAMPLE_USB_HID_CLEAN := test_hid_clean
EXAMPLE_USB_HID_OBJ :=  usb/test_hid.o
$(EXAMPLE_USB_HID): $(EXAMPLE_USB_HID_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_USB_HID_CLEAN):
	$(call clean_example,$(EXAMPLE_USB_HID_OBJ),$(EXAMPLE_USB_HID))

EXAMPLE_USB_CDC := test_cdc_acm
EXAMPLE_USB_CDC_CLEAN := test_cdc_clean
EXAMPLE_USB_CDC_OBJ :=  usb/test_cdc_acm.o
$(EXAMPLE_USB_CDC): $(EXAMPLE_USB_CDC_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_USB_CDC_CLEAN):
	$(call clean_example,$(EXAMPLE_USB_CDC_OBJ),$(EXAMPLE_USB_CDC))

EXAMPLE_USB_SWITCH := test_usb_switch
EXAMPLE_USB_SWITCH_CLEAN := test_usb_switch_clean
EXAMPLE_USB_SWITCH_OBJ :=  usb/test_switch.o
$(EXAMPLE_USB_SWITCH): $(EXAMPLE_USB_SWITCH_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_USB_SWITCH_CLEAN):
	$(call clean_example,$(EXAMPLE_USB_SWITCH_OBJ),$(EXAMPLE_USB_SWITCH))

BUILD_EXAMPLES += $(EXAMPLE_USB_HID) $(EXAMPLE_USB_CDC) $(EXAMPLE_USB_SWITCH)
CLEAN_EXAMPLES += $(EXAMPLE_USB_HID_CLEAN) $(EXAMPLE_USB_CDC_CLEAN) $(EXAMPLE_USB_SWITCH_CLEAN)
endif


#
# Vibrator
#
ifeq ($(CONFIG_VIBRATOR_MANAGER), y)
EXAMPLE_VIBRATOR := test_vibrator
EXAMPLE_VIBRATOR_CLEAN := test_vibrator_clean
EXAMPLE_VIBRATOR_OBJ :=  vibrator/main.o
$(EXAMPLE_VIBRATOR): $(EXAMPLE_VIBRATOR_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_VIBRATOR_CLEAN):
	$(call clean_example,$(EXAMPLE_VIBRATOR_OBJ),$(EXAMPLE_VIBRATOR))

BUILD_EXAMPLES += $(EXAMPLE_VIBRATOR)
CLEAN_EXAMPLES += $(EXAMPLE_VIBRATOR_CLEAN)
endif


#
# Watchdog
#
ifeq ($(CONFIG_WATCHDOG_MANAGER), y)
EXAMPLE_WATCHDOG := test_watchdog
EXAMPLE_WATCHDOG_CLEAN := test_watchdog_clean
EXAMPLE_WATCHDOG_OBJ :=  watchdog/main.o
$(EXAMPLE_WATCHDOG): $(EXAMPLE_WATCHDOG_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_WATCHDOG_CLEAN):
	$(call clean_example,$(EXAMPLE_WATCHDOG_OBJ),$(EXAMPLE_WATCHDOG))

BUILD_EXAMPLES += $(EXAMPLE_WATCHDOG)
CLEAN_EXAMPLES += $(EXAMPLE_WATCHDOG_CLEAN)
endif


#
# Zigbee
#
ifeq ($(CONFIG_ZIGBEE_MANAGER), y)
EXAMPLE_ZIGBEE := test_zigbee
EXAMPLE_ZIGBEE_CLEAN := test_zigbee_clean
EXAMPLE_ZIGBEE_OBJ :=  zigbee/main.o
$(EXAMPLE_ZIGBEE): $(EXAMPLE_ZIGBEE_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_ZIGBEE_CLEAN):
	$(call clean_example,$(EXAMPLE_ZIGBEE_OBJ),$(EXAMPLE_ZIGBEE))

BUILD_EXAMPLES += $(EXAMPLE_ZIGBEE)
CLEAN_EXAMPLES += $(EXAMPLE_ZIGBEE_CLEAN)
endif


#
# Thread
#
EXAMPLE_THREAD := test_thread
EXAMPLE_THREAD_CLEAN := test_thread_clean
EXAMPLE_THREAD_OBJ :=  thread/main.o
$(EXAMPLE_THREAD): $(EXAMPLE_THREAD_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_THREAD_CLEAN):
	$(call clean_example,$(EXAMPLE_THREAD_OBJ),$(EXAMPLE_THREAD))

BUILD_EXAMPLES += $(EXAMPLE_THREAD)
CLEAN_EXAMPLES += $(EXAMPLE_THREAD_CLEAN)

#
# Ring buffer
#
ifeq ($(CONFIG_RING_BUFFER), y)
EXAMPLE_RING_BUFFER := test_ring_buffer
EXAMPLE_RING_BUFFER_CLEAN := test_ring_buffer_clean
EXAMPLE_RING_BUFFER_OBJ :=  ring_buffer/main.o
$(EXAMPLE_RING_BUFFER): $(EXAMPLE_RING_BUFFER_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_RING_BUFFER_CLEAN):
	$(call clean_example,$(EXAMPLE_RING_BUFFER_OBJ),$(EXAMPLE_RING_BUFFER))

BUILD_EXAMPLES += $(EXAMPLE_RING_BUFFER)
CLEAN_EXAMPLES += $(EXAMPLE_RING_BUFFER_CLEAN)
endif

#
# leds gpio
#
ifeq ($(CONFIG_LEDS_GPIO_MANAGER), y)
EXAMPLE_LEDS_GPIO := test_leds_gpio
EXAMPLE_LEDS_GPIO_CLEAN := test_leds_gpio_clean
EXAMPLE_LEDS_GPIO_OBJ :=  leds_gpio/main.o
$(EXAMPLE_LEDS_GPIO): $(EXAMPLE_LEDS_GPIO_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_LEDS_GPIO_CLEAN):
	$(call clean_example,$(EXAMPLE_LEDS_GPIO_OBJ),$(EXAMPLE_LEDS_GPIO))

BUILD_EXAMPLES += $(EXAMPLE_LEDS_GPIO)
CLEAN_EXAMPLES += $(EXAMPLE_LEDS_GPIO_CLEAN)
endif

#
# sgm 42507
#
ifeq ($(CONFIG_SGM42507_MANAGER), y)
EXAMPLE_SGM42507 := test_sgm42507
EXAMPLE_SGM42507_CLEAN := test_sgm42507_clean
EXAMPLE_SGM42507_OBJ :=  sgm42507/main.o
$(EXAMPLE_SGM42507): $(EXAMPLE_SGM42507_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_SGM42507_CLEAN):
	$(call clean_example,$(EXAMPLE_SGM42507_OBJ),$(EXAMPLE_SGM42507))

BUILD_EXAMPLES += $(EXAMPLE_SGM42507)
CLEAN_EXAMPLES += $(EXAMPLE_SGM42507_CLEAN)
endif

#
# fxl lock body
#
ifeq ($(CONFIG_FXL_LOCK_BODY_MANAGER), y)
EXAMPLE_FXL_LOCK_BODY := test_fxl_lock_body
EXAMPLE_FXL_LOCK_BODY_CLEAN := test_fxl_lock_body_clean
EXAMPLE_FXL_LOCK_BODY_OBJ :=  lock_body/fxl_lock_body/main.o
$(EXAMPLE_FXL_LOCK_BODY): $(EXAMPLE_FXL_LOCK_BODY_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_FXL_LOCK_BODY_CLEAN):
	$(call clean_example,$(EXAMPLE_FXL_LOCK_BODY_OBJ),$(EXAMPLE_FXL_LOCK_BODY))

BUILD_EXAMPLES += $(EXAMPLE_FXL_LOCK_BODY)
CLEAN_EXAMPLES += $(EXAMPLE_FXL_LOCK_BODY_CLEAN)
endif

#
# mili lock body
#
ifeq ($(CONFIG_MILI_LOCK_BODY_MANAGER), y)
EXAMPLE_MILI_LOCK_BODY := test_mili_lock_body
EXAMPLE_MILI_LOCK_BODY_CLEAN := test_mili_lock_body_clean
EXAMPLE_MILI_LOCK_BODY_OBJ :=  lock_body/mili_lock_body/main.o
$(EXAMPLE_MILI_LOCK_BODY): $(EXAMPLE_MILI_LOCK_BODY_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_MILI_LOCK_BODY_CLEAN):
	$(call clean_example,$(EXAMPLE_MILI_LOCK_BODY_OBJ),$(EXAMPLE_MILI_LOCK_BODY))

BUILD_EXAMPLES += $(EXAMPLE_MILI_LOCK_BODY)
CLEAN_EXAMPLES += $(EXAMPLE_MILI_LOCK_BODY_CLEAN)
endif

#
# Lock cylinder
#
ifeq ($(CONFIG_LOCK_CYLINDER_MANAGER), y)
EXAMPLE_LOCK_CYLINDER := test_lock_cylinder
EXAMPLE_LOCK_CYLINDER_CLEAN := test_lock_cylinder_clean
EXAMPLE_LOCK_CYLINDER_OBJ :=  lock_cylinder/main.o
$(EXAMPLE_LOCK_CYLINDER): $(EXAMPLE_LOCK_CYLINDER_OBJ)
	$(call build_example,$^,$@)
$(EXAMPLE_LOCK_CYLINDER_CLEAN):
	$(call clean_example,$(EXAMPLE_LOCK_CYLINDER_OBJ),$(EXAMPLE_LOCK_CYLINDER))

BUILD_EXAMPLES += $(EXAMPLE_LOCK_CYLINDER)
CLEAN_EXAMPLES += $(EXAMPLE_LOCK_CYLINDER_CLEAN)
endif


#
# For build
#
all: $(BUILD_EXAMPLES)

#
# For clean
#
clean: $(CLEAN_EXAMPLES)
