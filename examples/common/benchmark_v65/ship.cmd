@echo off

mkdir benchmark_sdm845\lib
mkdir benchmark_sdm845\lib64
mkdir benchmark_sdm845\bin
mkdir benchmark_sdm845\dsplib
mkdir benchmark_sdm845\dspsim

copy android_ReleaseG\ship\benchmark benchmark_sdm845\bin
copy android_ReleaseG\ship\libbenchmark.so benchmark_sdm845\lib
copy android_ReleaseG_aarch64\ship\libbenchmark.so benchmark_sdm845\lib64
copy hexagon_ReleaseG_dynamic_toolv81_v65\ship\libbenchmark_skel.so benchmark_sdm845\dsplib
copy hexagon_ReleaseG_toolv81_v65\benchmark_q benchmark_sdm845\dspsim

echo adb wait-for-device                       > benchmark_sdm845\install.cmd
echo adb root                                  >> benchmark_sdm845\install.cmd
echo adb wait-for-device                       >> benchmark_sdm845\install.cmd
echo adb remount                               >> benchmark_sdm845\install.cmd
echo adb push lib /vendor/lib                  >> benchmark_sdm845\install.cmd
echo adb push lib64 /vendor/lib64              >> benchmark_sdm845\install.cmd
echo adb push dsplib /vendor/lib/rfsa/adsp     >> benchmark_sdm845\install.cmd
echo adb push bin /data/local                  >> benchmark_sdm845\install.cmd
echo adb shell chmod 755 /data/local/benchmark >> benchmark_sdm845\install.cmd
