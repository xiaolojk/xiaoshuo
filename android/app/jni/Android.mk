LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := main
# /workspace 根目录: game.c 与全部内嵌头文件所在处
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../..
LOCAL_SRC_FILES := ../../../game.c
LOCAL_CFLAGS := -O2 -std=c99
# Android 15+ 的 16KB 页内核设备: LOAD 段必须按 16KB 对齐, 否则 dlopen 直接失败(启动闪退)
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_LDFLAGS := -Wl,-z,max-page-size=16384
endif
LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lOpenSLES -llog -landroid -lm
include $(BUILD_SHARED_LIBRARY)

$(call import-module,SDL2)
