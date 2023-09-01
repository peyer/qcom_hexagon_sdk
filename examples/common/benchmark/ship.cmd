@echo off

mkdir benchmark_8998\lib
mkdir benchmark_8998\lib64
mkdir benchmark_8998\bin
mkdir benchmark_8998\dsplib
mkdir benchmark_8998\dspsim

copy android_Release\ship\benchmark benchmark_8998\bin
copy android_Release\ship\libbenchmark.so benchmark_8998\lib
copy android_Release_aarch64\ship\libbenchmark.so benchmark_8998\lib64
copy hexagon_Release_dynamic_toolv81_v62\ship\libbenchmark_skel.so benchmark_8998\dsplib
copy hexagon_Release_toolv81_v62\benchmark_q benchmark_8998\dspsim

echo adb wait-for-device                       > benchmark_8998\install.cmd
echo adb root                                  >> benchmark_8998\install.cmd
echo adb wait-for-device                       >> benchmark_8998\install.cmd
echo adb remount                               >> benchmark_8998\install.cmd
echo adb push lib /vendor/lib                  >> benchmark_8998\install.cmd
echo adb push lib64 /vendor/lib64              >> benchmark_8998\install.cmd
echo adb push dsplib /vendor/lib/rfsa/adsp     >> benchmark_8998\install.cmd
echo adb push bin /data/local                  >> benchmark_8998\install.cmd
echo adb shell chmod 755 /data/local/benchmark >> benchmark_8998\install.cmd

#copy benchmark_8996\install.cmd benchmark_8998\.
