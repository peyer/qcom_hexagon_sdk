# Script enabling tracing
#
# This script needs to be run each time your device has been rebooted in order to enable tracing


echo "Rebooting device and gaining root access"

# The reboot is only needed to modify the tracing configurations such as the trace buffer size

adb reboot
adb wait-for-device root
adb wait-for-device remount

echo "Configuring and enabling tracing"

adb shell "echo 0 > /sys/kernel/debug/tracing/events/enable"
adb shell "echo 0 > /sys/bus/coresight/devices/coresight-stm/enable_source"
adb shell "echo 0x4000000 > /sys/bus/coresight/devices/coresight-tmc-etr/mem_size"
adb shell "echo 1 > /sys/bus/coresight/reset_source_sink"
adb shell "echo 1 > /sys/bus/coresight/devices/coresight-tmc-etr/enable_sink"
adb shell "echo 1 > /sys/bus/coresight/devices/coresight-turing-etm0/enable_source"
adb shell setprop vendor.fastrpc.process.attrs 17

adb shell /data/sysMonApp etmTrace --command etm --q6 CDSP --etmType ca_pc

echo "Tracing enabled"
