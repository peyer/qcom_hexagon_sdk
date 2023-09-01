@echo off

set FILENAME=%1
IF %1.==. set FILENAME=/data/local/benchmark.csv

adb wait-for-device
adb root
adb wait-for-device
adb remount

adb shell rm %FILENAME%

for %%a in (epsilon bilateral fast9 integrate dilate3x3 dilate5x5 conv3x3 gaussian7x7 sobel3x3) do (
    for /L %%b in (0,1,6) do ( 
        adb shell /vendor/bin/benchmark -o %FILENAME% -f %%a -P %%b -L 10 -l 10 -s
    )
)

echo Saving benchmark results to %FILENAME%
