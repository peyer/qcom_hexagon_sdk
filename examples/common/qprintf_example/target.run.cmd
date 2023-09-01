rem Script automating the process of running on target (v62 compatible)

make tree VERBOSE=1   V=android_Release || goto :error
make tree VERBOSE=1   V=hexagon_ReleaseG_dynamic_toolv82_v65 || goto :error

adb wait-for-device
adb root
adb wait-for-device
adb remount

adb push hexagon_ReleaseG_dynamic_toolv82_v65\ship\libqprintf_example_skel.so /vendor/lib/rfsa/adsp/
adb push android_Release\ship\qprintf_example /vendor/bin


adb shell chmod 777 /vendor/bin/qprintf_example ; cd /vendor/bin; ./qprintf_example 

:error