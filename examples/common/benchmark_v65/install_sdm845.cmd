adb wait-for-device
adb root
adb wait-for-device
adb remount
adb push android_Release\ship\benchmark /data/local
adb shell chmod 755 /data/local/benchmark
adb push android_Release\ship\libbenchmark.so /vendor/lib
adb push android_Release_aarch64\ship\libbenchmark.so /vendor/lib64
adb push hexagon_Release_dynamic_toolv81_v65\ship\. /vendor/lib/rfsa/adsp
