adb wait-for-device
adb root
adb wait-for-device
adb remount
make tree_clean V=android_Release
make tree V=android_Release CDSP_FLAG=1
make tree_clean V=android_Release_aarch64
make tree V=android_Release_aarch64 CDSP_FLAG=1
make tree_clean V=hexagon_Release_dynamic_toolv81_v60
make tree V=hexagon_Release_dynamic_toolv81_v60
adb push android_Release\ship\benchmark /data/local
adb shell chmod 755 /data/local/benchmark
adb push android_Release\ship\libbenchmark.so /vendor/lib
adb push android_Release_aarch64\ship\libbenchmark.so /vendor/lib64
adb push hexagon_Release_dynamic_toolv81_v60\ship /vendor/lib/rfsa/adsp
