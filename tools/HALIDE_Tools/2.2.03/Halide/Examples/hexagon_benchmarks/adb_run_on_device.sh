#!/bin/bash

# This script will build and run this app on an Android device via
# adb. You must set the HL_TARGET environment variable prior to
# running this script, e.g.
#  HL_TARGET=arm-64-android adb_run_on_device.sh

DEVICE_PATH=/data/local/tmp/hexagon_benchmarks
BIN=bin

APP_TARGET=arm-64-android


# Build the app.
make bin/${APP_TARGET}/process

if [ $? != 0 ]; then
    exit 1
fi

if [ "z$RUNDROID" = "z" ]; then
    export PREF=echo
    $PREF To run this example on an Android device, do:
else
    num_device_lines=`adb devices -l | sed '/^$/d' | wc -l`
    if [ $num_device_lines -gt 2 ]; then
        if [ "z$ANDROID_SERIAL" == "z" ];  then
            echo "Error: RUNDROID set to 1, but found more than one device. Set ANDROID_SERIAL"
            exit 1
        else
            echo "Running tests on Device:$ANDROID_SERIAL"
            set +x
        fi
    fi
fi

$PREF adb shell mkdir -p ${DEVICE_PATH}
$PREF adb push ${BIN}/${APP_TARGET}/process ${DEVICE_PATH}
$PREF adb shell chmod +x ${DEVICE_PATH}/process
$PREF adb shell  "cd ${DEVICE_PATH}; ./process"

exit $?
