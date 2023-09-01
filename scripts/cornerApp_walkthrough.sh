#!/bin/bash

if [ -z "$HEXAGON_SDK_ROOT" ]; then
    echo "SDK Environment Not Setup, please run setup_sdk_env.sh located in the SDK root directory"
    exit 1
fi

echo ---- Build calculator example for both Android and Hexagon ----
make -C $HEXAGON_SDK_ROOT/examples/fastcv/cornerApp tree_clean V=android_Release || exit 1
make -C $HEXAGON_SDK_ROOT/examples/fastcv/cornerApp tree V=android_Release || exit 1
make -C $HEXAGON_SDK_ROOT/examples/fastcv/cornerApp tree_clean V=hexagon_Release_dynamic || exit 1
make -C $HEXAGON_SDK_ROOT/examples/fastcv/cornerApp tree V=hexagon_Release_dynamic || exit 1

source $HEXAGON_SDK_ROOT/tools/scripts/testsig.sh

echo ---- Push Android components ----
adb wait-for-device
adb root
adb wait-for-device
adb remount
adb push $HEXAGON_SDK_ROOT/examples/fastcv/cornerApp/android_Release/ship/cornerApp /data/
adb shell chmod 777 /data/cornerApp
adb push $HEXAGON_SDK_ROOT/examples/fastcv/cornerApp/android_Release/ship/libcornerApp.so /system/lib

echo ---- Push Hexagon Components ----
adb shell mkdir -p /system/lib/rfsa/adsp
adb push $HEXAGON_SDK_ROOT/examples/fastcv/cornerApp/hexagon_Release_dynamic/ship/libcornerApp_skel.so /system/lib/rfsa/adsp
adb push $HEXAGON_SDK_ROOT/lib/fastcv/dspCV/hexagon_Release_dynamic/ship/libdspCV_skel.so /system/lib/rfsa/adsp
adb push $HEXAGON_SDK_ROOT/lib/fastcv/fastcv/hexagon_Release_dynamic/libfastcvadsp.so /system/lib/rfsa/adsp
adb push $HEXAGON_SDK_ROOT/lib/common/apps_mem_heap/ship/hexagon_Release_dynamic/libapps_mem_heap.so /system/lib/rfsa/adsp

echo ---- Run cornerApp Example on aDSP ----
adb shell ADSP_LIBRARY_PATH=/system/lib/rfsa/adsp /data/cornerApp

echo Done

