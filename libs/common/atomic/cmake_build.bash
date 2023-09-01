#!/bin/bash
source ${HEXAGON_SDK_ROOT}/build/cmake/cmake_configure.bash

doTest() {
# Test on the Device
    adb wait-for-device
    adb root && adb remount
    V=hexagon_ReleaseG_dynamic_toolv83_v66
    HEXAGON_BUILD=${V}.cmake    
    app=atomic_device
    ANDROID_BUILD=android_Release_aarch64.cmake
    adb push ${ANDROID_BUILD}/${app} /vendor/bin
    adb push ${ANDROID_BUILD}/libatomic.so /vendor/lib64/
    adb push ${HEXAGON_BUILD}/libatomic_skel.so /vendor/lib/rfsa/adsp/
    adb push ${HEXAGON_BUILD}/libdspCV_skel.so /vendor/lib/rfsa/adsp/
    adb shell chmod 777 /vendor/bin/${app}
    adb shell ADSP_LIBRARY_PATH="/vendor/lib/rfsa/adsp:/system/lib/rfsa/adsp:/dsp;" /vendor/bin/${app}
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
        --target atomic_device -- \
        && \
        cd -
elif [ "to$1" = "tohexagon" ]; 
then
    # Build the Hexagon code and generate dynamic lib
    if [ "$2" = "qurt" ];        
    then
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 atomic_skel qurt    
    else
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 atomic_skel
    fi    
    cd ${_src} && \
       ${CMAKE} \
       --build \
       ${_build} \
       --target atomic_skel -- \
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
        --target atomic_device -- \
        && \
        cd -
    if [ "$3" = "qurt" ];        
    then
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 atomic_skel qurt    
    else
        doHexagon hexagon_ReleaseG_dynamic_toolv83_v66 atomic_skel
    fi    
    cd ${_src} && \
       ${CMAKE} \
       --build \
       ${_build} \
       --target atomic_skel -- \
       && \
       cd -
else
    # Run on the Simulator ( default option)
    echo "NO simulator target, do nothing" 
fi

