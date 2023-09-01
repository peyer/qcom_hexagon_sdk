rem Script automating the process of running on target (v62 compatible)

make tree VERBOSE=1   V=android_ReleaseG || goto :error
make tree VERBOSE=1   V=hexagon_ReleaseG_dynamic_toolv81_v65 || goto :error

adb wait-for-device
adb root
adb wait-for-device
adb remount

adb push hexagon_ReleaseG_dynamic_toolv81_v65\ship\libqfxp_sample_skel.so /vendor/lib/rfsa/adsp/
adb push android_ReleaseG\ship\qfxp_sample /vendor/bin


adb shell chmod 777 /vendor/bin/qfxp_sample ; cd /vendor/bin; ./qfxp_sample 

:error