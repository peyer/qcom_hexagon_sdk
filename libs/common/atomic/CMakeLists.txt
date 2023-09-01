# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.14.3)

project(atomic C CXX ASM)

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
    # build library atomic.a
    set(ignoreMe ${DSP_TYPE})    
    add_library(atomic
        STATIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/atomic.c        
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cae.c
    )
    # This changes the libname to atomic.a from default libatomic.a
    set_target_properties(atomic PROPERTIES PREFIX "" )
    # Build the APPS binary
    add_executable(atomic_device
        ${CMAKE_CURRENT_SOURCE_DIR}/src/atomic_q.c        
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

    # Necessary Includes for Base Build
    set(incs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        #${HEXAGON_SDK_ROOT}/incs/
        ${HEXAGON_SDK_ROOT}/incs/stddef       
    )

    
    # Add necessary includes for the base build
    include_directories(
        ${incs}        
    )
    # Link the dependent libs
    target_link_libraries(atomic_device atomic)
    # install the built Libs 
    install(TARGETS atomic DESTINATION ./lib EXPORT atomic-targets)
    install(EXPORT atomic-targets DESTINATION ./lib/atomic)        
    
else()
    #################################################
    #Hexagon Dynamic Library
    #################################################
    if(${BUILD_NAME} MATCHES "skel")

        add_library(atomic
            STATIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/atomic.c        
            ${CMAKE_CURRENT_SOURCE_DIR}/src/cae.c
        )
        # This changes the libname to atomic.a from default libatomic.a
        set_target_properties(atomic PROPERTIES PREFIX "" )
        # Build the APPS binary
        add_executable(atomic_skel
            ${CMAKE_CURRENT_SOURCE_DIR}/src/atomic_q.c        
        )
        set_target_properties(atomic_skel PROPERTIES OUTPUT_NAME "atomic_device" )
    
        # Set some Flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

        set(incs 
            ${CMAKE_CURRENT_SOURCE_DIR}/inc/ 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/            
        )        
       
        # Include necessary dependent lib directory
        include_directories(
            ${incs}            
        )
        # Link the dependent libs
        target_link_libraries(atomic_skel atomic)
        # install the built Libs 
        install(TARGETS atomic DESTINATION ./lib EXPORT atomic-targets)
        install(EXPORT atomic-targets DESTINATION ./lib/atomic)            

    elseif(${BUILD_NAME} MATCHES "sim")

    
        #################################################
        # Do nothing
        #################################################        

    endif()
endif()

# vim: set noet fenc=utf-8 ff=unix ft=cmake :

