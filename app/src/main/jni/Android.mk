LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += \
    $(NDK_PATH)/platforms/android-21/arch-arm/usr/include \

LOCAL_MODULE    := gpio
LOCAL_SRC_FILES := gpio.c \
                   onewire.c \
                   dallas.c \
                   delay.c \
                   stack.c

LOCAL_CFLAGS    += -UNDEBUG -DANDROID

LOCAL_LDLIBS    := -ldl -llog
include $(BUILD_SHARED_LIBRARY)
