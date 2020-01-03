LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libtinyalsa
LOCAL_MODULE_TAGS :=optional
LOCAL_SRC_FILES := pcm.c \
		   mixer.c
LOCAL_EXPORT_C_INCLUDE_FILES:=include/tinyalsa/asoundlib.h
LOCAL_CFLAGS := -Wa,-mips32r2 -O2 -G 0 -Wall -fPIC -shared


include $(BUILD_SHARED_LIBRARY)


#===================================================
# tinyplay
# #

include $(CLEAR_VARS)
LOCAL_MODULE := tinyplay
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := tinyplay.c
LOCAL_CFLAGS := -fpic
LOCAL_LDLIBS := -lc -ltinyalsa

LOCAL_MODULE_GEN_BINRARY_FILES := tinyplay

LOCAL_DEPANNER_MODULES := libtinyalsa
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_MODULE := tinycap
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := tinycap.c
LOCAL_CFLAGS := -fpic
LOCAL_MODULE_GEN_BINRARY_FILES := tinyplay
LOCAL_LDLIBS := -lc -ltinyalsa
LOCAL_DEPANNER_MODULES := libtinyalsa
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_MODULE := tinymix
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := tinymix.c
LOCAL_CFLAGS := -fpic
LOCAL_LDLIBS := -lc -ltinyalsa
LOCAL_DEPANNER_MODULES := libtinyalsa
LOCAL_MODULE_GEN_BINRARY_FILES := tinymix
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_MODULE := tinypcminfo
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := tinypcminfo.c
LOCAL_CFLAGS := -fpic
LOCAL_LDLIBS := -lc -ltinyalsa
LOCAL_MODULE_GEN_BINRARY_FILES := tinypcminfo
LOCAL_DEPANNER_MODULES := libtinyalsa
include $(BUILD_EXECUTABLE)




