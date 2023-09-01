BINARY_FOLDER=${HEXAGON_SDK_ROOT}/tools/debug/hexagon-trace-analyzer/binaries/

echo "*** Push sysmon on device"
adb root
adb remount
adb push ${HEXAGON_SDK_ROOT}/tools/utils/sysmon/sysMonApp /data/sysMonApp
adb shell chmod 777 /data/sysMonApp

echo "*** Write to $BINARY_FOLDER binaries needed for tracing analysis"
mkdir -p $BINARY_FOLDER
adb pull /vendor/dsp/cdsp/fastrpc_shell_3 $BINARY_FOLDER
adb pull /vendor/lib/rfsa/adsp/libdspCV_skel.so $BINARY_FOLDER
adb pull /vendor/firmware_mnt/image/ $BINARY_FOLDER
