APP_ABI := arm64-v8a armeabi-v7a
APP_PLATFORM := android-19
# 16KB 页对齐: Android 15+ 16KB 内核设备上, 4KB 对齐的 .so 会在 dlopen 时失败(启动闪退)
# APP_LDFLAGS 对全部模块生效(含 SDL2); arm64 上的 max-page-size=16384 在 4KB 设备上也兼容
APP_LDFLAGS := -Wl,-z,max-page-size=16384
