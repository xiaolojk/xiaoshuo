#!/bin/bash
# PixelLakeHeart Android 打包脚本 (ndk-build + aapt2/d8/apksigner, 无需 gradle)
# 用法: android/build_apk.sh   产物: android/PixelLakeHeart.apk
set -e
cd "$(dirname "$0")/.."   # /workspace

SDK=/opt/android-sdk
NDK=$SDK/ndk/26.3.11579264
BT=$SDK/build-tools/34.0.0
PLATFORM=$SDK/platforms/android-34/android.jar
APP=android/app
OUT=android/build

echo "== 1/5 ndk-build (SDL2 + game) =="
rm -rf $APP/libs $APP/obj
$NDK/ndk-build NDK_PROJECT_PATH=$APP APP_BUILD_SCRIPT=$APP/jni/Android.mk \
  NDK_APPLICATION_MK=$APP/jni/Application.mk NDK_MODULE_PATH=$APP/jni -j3

echo "== 2/5 javac (SDL java + GameActivity) =="
rm -rf $OUT/classes && mkdir -p $OUT/classes
find $APP/src/main/java -name '*.java' > $OUT/sources.txt
javac --release 11 -classpath $PLATFORM -d $OUT/classes @$OUT/sources.txt

echo "== 3/5 d8 -> classes.dex =="
jar cf $OUT/classes.jar -C $OUT/classes .
$BT/d8 --release --lib $PLATFORM --min-api 19 $OUT/classes.jar --output $OUT

echo "== 4/5 aapt2 打资源 + manifest =="
rm -rf $OUT/res.zip $OUT/unsigned.apk
$BT/aapt2 compile --dir $APP/src/main/res -o $OUT/res.zip
$BT/aapt2 link -o $OUT/unsigned.apk -I $PLATFORM \
  --manifest $APP/src/main/AndroidManifest.xml \
  --min-sdk-version 19 --target-sdk-version 34 \
  --version-code 1 --version-name 1.0 \
  $OUT/res.zip

echo "== 5/5 塞入 dex/so, 对齐并签名 =="
cd $OUT
zip -q -j unsigned.apk classes.dex   # dex 在 apk 根目录
for abi in arm64-v8a armeabi-v7a; do
  if [ -d ../app/libs/$abi ]; then
    cd ../app/libs/$abi
    zip -q $OUT/unsigned.apk lib/$abi/* 2>/dev/null || \
      { mkdir -p $OUT/lib/$abi && cp *.so $OUT/lib/$abi/ && cd $OUT && zip -q -r unsigned.apk lib; }
    cd $OUT
  fi
done
$BT/zipalign -f 4 unsigned.apk aligned.apk
if [ ! -f debug.keystore ]; then
  keytool -genkeypair -keystore debug.keystore -storepass android -keypass android \
    -alias androiddebugkey -dname "CN=Android Debug,O=Android,C=US" \
    -keyalg RSA -keysize 2048 -validity 10000
fi
$BT/apksigner sign --ks debug.keystore --ks-pass pass:android \
  --min-sdk-version 19 --out PixelLakeHeart.apk aligned.apk
$BT/apksigner verify PixelLakeHeart.apk && echo "OK: $(pwd)/PixelLakeHeart.apk"
