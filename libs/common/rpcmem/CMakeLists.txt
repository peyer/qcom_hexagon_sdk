# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.14.3)

project(rpcmem C CXX ASM)

enable_language(ASM)

set(CMAKE_DEBUG_TARGET_PROPERTIES
    INCLUDE_DIRECTORIES
    COMPILE_DEFINITIONS
    POSITION_INDEPENDENT_CODE
    CONTAINER_SIZE_REQUIRED
    LIB_VERSION
)

include(${HEXAGON_CMAKE_ROOT}/hexagon_fun.cmake)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    # build library rpcmem.a
    set(ignoreMe ${DSP_TYPE})
    add_library(rpcmem
        STATIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcmem_android.c
    )
    # This changes the libname to rpcmem.a from default librpcmem.a
    set_target_properties(rpcmem PROPERTIES PREFIX "" )
    
    # Build the APPS binary
    add_executable(rpcmem_device
        ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcmem_android_q.c        
        )
    
    target_compile_definitions(
        rpcmem_device
        PUBLIC VERIFY_PRINT_ERROR
    )    

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE -llog")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIE -llog")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}  -fPIE -pie -llog")

    message(STATUS "ANDROID_NDK_REVISION:${ANDROID_NDK_REVISION}")
    message(STATUS "ANDROID_NDK_RELEASE:${ANDROID_NDK_RELEASE}")

    #This is to work around the NDK r19c C++ issues
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/")
    endif()
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${ANDROID_NDK}/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/")
    endif()

    ExternalProject_Add(test_util
        #PREFIX ${HEXAGON_SDK_ROOT}//test/common/test_util
        SOURCE_DIR ${HEXAGON_SDK_ROOT}/test/common/test_util
        STAMP_DIR ${HEXAGON_SDK_ROOT}/test/common/test_util/${V}.cmake/
        #BUILD_IN_SOURCE 1
        BINARY_DIR ${HEXAGON_SDK_ROOT}/test/common/test_util/${V}.cmake/
        INSTALL_DIR  ${CMAKE_CURRENT_BINARY_DIR}
        BUILD_COMMAND make all
        CMAKE_ARGS -DV=${V} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR} -DHEXAGON_CMAKE_ROOT=${HEXAGON_CMAKE_ROOT} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON -DANDROID_NDK=${ANDROID_NDK} -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_STL=none -DANDROID_NATIVE_API_LEVEL=21        
     )    
     include_directories(${HEXAGON_SDK_ROOT}/test/common/test_util/src)

    # Necessary Includes for Base Build
    set(incs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${HEXAGON_SDK_ROOT}/incs/qlist
        ${HEXAGON_SDK_ROOT}/incs/stddef
        ${HEXAGON_SDK_ROOT}/incs/
        ${HEXAGON_SDK_ROOT}/incs/a1std
        #${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/        
    )

    
    # Add necessary includes for the base build
    include_directories(
        ${incs}        
    )
    add_dependencies(rpcmem_device test_util)
    # Link the dependent libs
    target_link_libraries(rpcmem_device ${CMAKE_CURRENT_BINARY_DIR}/lib/test_util.a  rpcmem)
    # install the built Libs 
    install(TARGETS rpcmem DESTINATION ./lib EXPORT rpcmem-targets)
    install(EXPORT rpcmem-targets DESTINATION ./lib/rpcmem)    
    
else()
    #################################################
    #Hexagon Dynamic Library
    #################################################
    if(${BUILD_NAME} MATCHES "skel")

        add_library(rpcmem_skel
            SHARED 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcmem_stub            
            )
        add_library(rpcmem
            STATIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/rpcmem_stub            
            )

        set_target_properties(rpcmem PROPERTIES PREFIX "" )
        # Set some Flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")


        set(incs 
            ${CMAKE_CURRENT_SOURCE_DIR}/inc/
            ${HEXAGON_SDK_ROOT}/incs/stddef
            )        
       
        # Include necessary dependent lib directory
        include_directories(
            ${incs}            
            )
        target_link_libraries(rpcmem_skel rpcmem)
        #target_link_libraries(rpcmem)
        #configure_file(${CMAKE_CURRENT_BUILD_DIR}/librpcmem_skel.so ${CMAKE_CURRENT_BUILD_DIR}/rpcmem.so COPYONLY)
        # install the built Libs 
        install(TARGETS rpcmem DESTINATION ./lib EXPORT rpcmem-targets)
        install(EXPORT rpcmem-targets DESTINATION ./lib/rpcmem)    
        

    elseif(${BUILD_NAME} MATCHES "sim")

    
        #################################################
        # Do nothing
        #################################################        

    endif()
endif()

# vim: set noet fenc=utf-8 ff=unix ft=cmake :

