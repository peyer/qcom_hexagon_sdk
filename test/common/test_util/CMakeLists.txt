# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.14.3)

project(test_util C CXX ASM)

enable_language(ASM)

set(CMAKE_DEBUG_TARGET_PROPERTIES
    INCLUDE_DIRECTORIES
    COMPILE_DEFINITIONS
    POSITION_INDEPENDENT_CODE
    CONTAINER_SIZE_REQUIRED
    LIB_VERSION
)

include(${HEXAGON_CMAKE_ROOT}/hexagon_fun.cmake)

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    # build library test_util.a
    set(ignoreMe ${DSP_TYPE})    
    add_library(test_util
        STATIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/test_utils.c        
    )
    # This changes the libname to test_util.a from default libtest_util.a
    set_target_properties(test_util PROPERTIES PREFIX "" )
    
    add_library(HAP_debug_printf
        STATIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAP_debug_printf.c        
    )
    # This changes the libname to HAP_debug_printf.a from default libHAP_debug_printf.a
    set_target_properties(HAP_debug_printf PROPERTIES PREFIX "" )   

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

    # Necessary Includes for Base Build
    set(incs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${HEXAGON_SDK_ROOT}/incs/
        ${HEXAGON_SDK_ROOT}/incs/stddef       
    )

    
    # Add necessary includes for the base build
    include_directories(
        ${incs}        
    )
    # Link the dependent libs
    target_link_libraries(test_util HAP_debug_printf)
    # install the built Libs 
    install(TARGETS test_util DESTINATION ./lib EXPORT test_util-targets)
    install(EXPORT test_util-targets DESTINATION ./lib/test_util)    
    install(TARGETS HAP_debug_printf DESTINATION ./lib EXPORT HAP_debug_printf-targets)
    install(EXPORT HAP_debug_printf-targets DESTINATION ./lib/HAP_debug_printf)    
    
else()
    #################################################
    #Hexagon Dynamic Library
    #################################################
    if(${BUILD_NAME} MATCHES "skel")

        add_library(test_util_skel
            SHARED 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/test_utils.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sysmon_vtcm_mgr_client.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sysmon_vtcm_mgr_srv.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sysmon_cachelock_srv.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sysmon_cachelock_client.c
        )
        add_library(test_util
            STATIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/test_utils.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sysmon_vtcm_mgr_client.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sysmon_vtcm_mgr_srv.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sysmon_cachelock_srv.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sysmon_cachelock_client.c
        )
        # This changes the libname to test_util.a from default libtest_util.a
        set_target_properties(test_util PROPERTIES PREFIX "" )
        add_library(HAP_debug_printf
            STATIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/HAP_debug_printf.c        
        )
        # This changes the libname to HAP_debug_printf.a from default libHAP_debug_printf.a
        set_target_properties(HAP_debug_printf PROPERTIES PREFIX "" )

        # Set some Flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")


        set(incs 
            ${CMAKE_CURRENT_SOURCE_DIR}/inc/
            ${HEXAGON_SDK_ROOT}/incs/
            ${HEXAGON_SDK_ROOT}/incs/stddef
        )        
       
        # Include necessary dependent lib directory
        include_directories(
            ${incs}            
        )
        target_link_libraries(test_util_skel test_util HAP_debug_printf)        
        # install the built Libs      
        install(TARGETS test_util DESTINATION ./lib EXPORT test_util-targets)
        install(EXPORT test_util-targets DESTINATION ./lib/test_util)    
        install(TARGETS HAP_debug_printf DESTINATION ./lib EXPORT HAP_debug_printf-targets)
        install(EXPORT HAP_debug_printf-targets DESTINATION ./lib/HAP_debug_printf)            

    elseif(${BUILD_NAME} MATCHES "sim")

    
        #################################################
        # Do nothing
        #################################################        

    endif()
endif()

# vim: set noet fenc=utf-8 ff=unix ft=cmake :

