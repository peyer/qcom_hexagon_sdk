#!/bin/bash -v

QHL_ADSP_LIB_PATH=/vendor/lib/rfsa/adsp  # Note: Any path under /vendor is acceptable such as /vendor/mytest
QHL_ANDROID_LIB_PATH=/vendor/lib
QHL_ANDROID_EXE_PATH=/vendor/bin 

QHL_EXAMPLE_NAME=qhl_example

# Build QHL test example and its dependencies

make  

# Run example on simulator

ANDROID_LIB_EXT=dll
QURT_PATH_VARIANT=Win
if [[ "$OSTYPE" == "linux"* ]]; then
ANDROID_LIB_EXT=so
QURT_PATH_VARIANT=lnx64
fi

echo "${HEXAGON_SDK_ROOT}/libs/common/qurt/computev66/debugger/${QURT_PATH_VARIANT}/qurt_model.$ANDROID_LIB_EXT" > osam.cfg
echo "${DEFAULT_HEXAGON_TOOLS_ROOT}/Tools/lib/iss/qtimer.$ANDROID_LIB_EXT --csr_base=0xFC900000 --irq_p=1 --freq=19200000 --cnttid=1" > q6ss.cfg
echo "${DEFAULT_HEXAGON_TOOLS_ROOT}/Tools/lib/iss/l2vic.$ANDROID_LIB_EXT 32 0xFC910000" >> q6ss.cfg

${DEFAULT_HEXAGON_TOOLS_ROOT}/Tools/bin/hexagon-sim -mv66g_1024 --simulated_returnval   --cosim_file q6ss.cfg --l2tcm_base 0xd800 --rtos osam.cfg ${HEXAGON_SDK_ROOT}/libs/common/qurt//computev66/sdksim_bin/runelf.pbn -- ${HEXAGON_SDK_ROOT}/libs/common/run_main_on_hexagon/ship/hexagon_Release_dynamic_toolv83_v66/run_main_on_hexagon_sim  -- qhl_example.so 1 foo 2.0 bar


## push test-signature file 
python ${HEXAGON_SDK_ROOT}/scripts/testsig.py

# Run example on device

## Push required files to device

adb wait-for-device root
adb wait-for-device remount
adb wait-for-device shell "mkdir -p ${QHL_ADSP_LIB_PATH}"
adb wait-for-device push ${HEXAGON_SDK_ROOT}/libs/common/run_main_on_hexagon/ship/android_Release/run_main_on_hexagon /${QHL_ANDROID_EXE_PATH}
adb wait-for-device push ${HEXAGON_SDK_ROOT}/libs/common/run_main_on_hexagon/ship/hexagon_Release_dynamic_toolv83_v66/librun_main_on_hexagon_skel.so /${QHL_ADSP_LIB_PATH}
adb wait-for-device push ${QHL_EXAMPLE_NAME}.so /${QHL_ADSP_LIB_PATH}

## Direct DSP log messages to logcat

adb wait-for-device root
adb wait-for-device remount
adb wait-for-device shell "echo 0x1f > ${QHL_ADSP_LIB_PATH}/run_main_on_hexagon.farf"


## Run main present in .so using run_main_on_hexagon utility executable
adb wait-for-device shell "chmod 774 ${QHL_ANDROID_EXE_PATH}/run_main_on_hexagon"
adb wait-for-device shell "export LD_LIBRARY_PATH=${QHL_ANDROID_LIB_PATH} ADSP_LIBRARY_PATH=\"/vendor/lib/rfsa/dsp/testsig;
${QHL_ADSP_LIB_PATH};\" ; cd ${QHL_ANDROID_EXE_PATH} ; ./run_main_on_hexagon 0 ${QHL_ADSP_LIB_PATH}/${QHL_EXAMPLE_NAME}.so 1 foo 2.0 bar"


## Note: If you would like to see the messages sent by the DSP, run 'adb wait-for-device logcat -s adsprpc' from another shell
