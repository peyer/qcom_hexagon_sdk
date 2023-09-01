#!/bin/bash

source ${HEXAGON_SDK_ROOT}/build/cmake/cmake_configure.bash

doTest() {
# Test on the Device
    adb wait-for-device
    adb root && adb remount
    V=hexagon_ReleaseG_dynamic_toolv83_v66
    HEXAGON_BUILD=${V}.cmake    
    app=calculator_device
    ANDROID_BUILD=android_Release_aarch64.cmake
    adb shell mkdir -p /vendor/bin
    adb push ${ANDROID_BUILD}/${app} /vendor/bin
    adb push ${ANDROID_BUILD}/libcalculator.so /vendor/lib64/
    adb shell mkdir -p /vendor/lib/rfsa/adsp/
    adb push ${HEXAGON_BUILD}/libcalculator_skel.so /vendor/lib/rfsa/adsp/
    adb shell "echo 0x1f > /vendor/lib/rfsa/adsp/calculator.farf"
    adb shell chmod 777 /vendor/bin/${app}
    adb shell ADSP_LIBRARY_PATH="/vendor/lib/rfsa/adsp:/system/lib/rfsa/adsp:/dsp;" /vendor/bin/${app} 0 1000
    adb shell ADSP_LIBRARY_PATH="/vendor/lib/rfsa/adsp:/system/lib/rfsa/adsp:/dsp;" /vendor/bin/${app} 1 1000
}

if [ "to$1" = "totest" ];
then
    # Test on Device ( make sure that the test is already built to run on device)
    doTest
elif [ "to$1" = "toandroid" ]; 
then
    # Build the Host code to run on Device
    DSP_TYPE=$2    
    doAndroid ${DSP_TYPE}
    cd ${_src} && \
       ${CMAKE} \
       --build \
       ${_build} \
       --target calculator_device -- \
       && \
       cd -    
elif [ "to$1" = "tohexagon" ]; 
then
    # Build the Hexagon code and generate dynamic lib
    if [ "$2" = "qurt" ];        
    then
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 calculator_skel qurt    
    else
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 calculator_skel
    fi
    cd ${_src} && \
       ${CMAKE} \
       --build \
       ${_build} \
       --target calculator_skel -- \
       && \
       cd -
elif [ "to$1" = "toclean" ]; 
then
    # Clean the builds
    doClean hexagon_ReleaseG_dynamic_toolv83_v66.cmake android_Release_aarch64.cmake
elif [ "to$1" = "todevice" ]; 
then
    # Build to Run on the Device 
    DSP_TYPE="$2"
    doAndroid ${DSP_TYPE}
    cd ${_src} && \
       ${CMAKE} \
       --build \
       ${_build} \
       --target calculator_device -- \
       && \
       cd -  
    if [ "$3" = "qurt" ];        
    then
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 calculator_skel qurt    
    else
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 calculator_skel
    fi
    cd ${_src} && \
       ${CMAKE} \
       --build \
       ${_build} \
       --target calculator_skel -- \
       && \
       cd -
else
    # Run on the Simulator ( default option)
    if [ "$1" = "qurt" ];        
    then
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 calculator_q-sim qurt    
    else
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 calculator_q-sim
    fi
    cd ${_src} && \
       ${CMAKE} \
       --build \
       ${_build} \
       --target calculator_q-sim -- \
       && \
       cd -
    
fi
