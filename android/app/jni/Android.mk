LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := main
# /workspace 根目录: game.c 与全部内嵌头文件所在处
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../..
LOCAL_SRC_FILES := ../../../game.c
LOCAL_CFLAGS := -O2 -std=c99
LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid -lm
include $(BUILD_SHARED_LIBRARY)

$(call import-module,SDL2)
