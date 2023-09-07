#!/bin/bash
set -x
set -e

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac
echo ${machine}


if [ ${machine} = "Linux" ]
then    
    CMAKE_ROOT="/usr"
    CMAKE=${CMAKE_ROOT}/bin/cmake
    ANDROID_NDK_ROOT=${ANDROID_ROOT_DIR}
    HEXAGON_TOOLCHAIN_ROOT=${HEXAGON_SDK_ROOT}/tools/HEXAGON_Tools/8.3.04/
    HEXAGON_CMAKE_MAKE_PROGRAM=/usr/bin/make
    HEXAGON_CMAKE_MAKE_FILETYPE="-GUnix Makefiles"
    HEXAGON_CMAKE_ROOT=${HEXAGON_SDK_ROOT}/build/cmake
    _src=${PWD}
    _dir_surfix=""
else

    CMAKE_ROOT="<Path to cmake installation>\\cmake-3.14.4-win64-x64"
    CMAKE="${CMAKE_ROOT}\\bin\\cmake.exe"
    ANDROID_NDK_ROOT=${ANDROID_ROOT_DIR}
    HEXAGON_CMAKE_MAKE_PROGRAM="${HEXAGON_SDK_ROOT}\\tools\\utils\\gow-0.8.0\\bin\\make.exe"
    HEXAGON_CMAKE_MAKE_FILETYPE="-GMinGW Makefiles"
    HEXAGON_TOOLCHAIN_ROOT=${HEXAGON_SDK_ROOT}/tools/HEXAGON_Tools/8.3.04/    
    HEXAGON_CMAKE_ROOT=${HEXAGON_SDK_ROOT}/build/cmake
    _src=$("${HEXAGON_SDK_ROOT}\\tools\\utils\\gow-0.8.0\\bin\\pwd.EXE")
    _dir_surfix=".win"
fi


doClean() {
    rm -rf $1
    rm -rf $2    
}

doAndroid() {
    # Android Build to run on the Device 

    V="android_Release_aarch64"
    _build=${_src}/${V}.cmake${_dir_surfix}
    ANDROID_BUILD_FULLPATH=${_src}/${V}.cmake${_dir_surfix}
    DSP_TYPE=$1

    rm -rf ${_build} && mkdir -p ${_build}
     
    cd ${_src} && \
    ${CMAKE} \
    -H${_src} \
    -B${_build} \
    -DANDROID_NDK=${ANDROID_NDK_ROOT} \
    -G"Unix Makefiles"  \
    -DCMAKE_SYSTEM_NAME="Android"  \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${_build} \
    -DCMAKE_BUILD_TYPE=Debug  \
    -DV=${V}  \
    -DHEXAGON_CMAKE_ROOT=${HEXAGON_CMAKE_ROOT}\
    -DDSP_TYPE=${DSP_TYPE}\
    -DTREE=${1} \
    -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a -DANDROID_STL=none\
    -DANDROID_NATIVE_API_LEVEL=21 \
    -DCMAKE_VERBOSE_MAKEFILE=ON && cd -     
}

doHexagon() {
    # Hexagon Build    
    V=$1
    TARGET=$2
    # Read Hexagon TOOLS and ARCH version
    #read TOOL_VER ARCH_VER <<<${V//[^0-9]/ }   
    declare -i qurt_flag    
    if [ "$3" = "qurt" ];
    then
        qurt_flag=1
    else
        qurt_flag=0
    fi
    HEXAGON_BUILD=${V}.cmake${_dir_surfix}
    HEXAGON_BUILD_FULLPATH=${_src}/${HEXAGON_BUILD}

    _build=${HEXAGON_BUILD_FULLPATH}

    rm -rf ${_build}     

    cd ${_src} && \
        ${CMAKE} \
        -H${_src} \
        -B${_build} \
        -DV=${V} \
        -DBUILD_NAME=${TARGET} \
        -DCMAKE_TOOLCHAIN_FILE=${HEXAGON_CMAKE_ROOT}/Hexagon_Toolchain.cmake -DQURT_OS=${qurt_flag} \
        -DHEXAGON_CMAKE_ROOT=${HEXAGON_CMAKE_ROOT} \
        -DCMAKE_MAKE_PROGRAM=${HEXAGON_CMAKE_MAKE_PROGRAM} \
        "${HEXAGON_CMAKE_MAKE_FILETYPE}" \
        -DHEXAGON_TOOLCHAIN_ROOT=${HEXAGON_TOOLCHAIN_ROOT} \
        -DCMAKE_VERBOSE_MAKEFILE=ON && cd -
}

