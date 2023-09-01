# ===============================================================================
#     Copyright 2018 Qualcomm Technologies, Inc.  All rights reserved.
#     Confidential & Proprietary
# ===============================================================================
cmake_minimum_required(VERSION 3.14.3)

macro (list2string out in)
    #string(REPLACE ";" " " ${out} "${in}")
    #message(STATUS "OUT:${${out}}")
    set(list ${ARGV})
    list(REMOVE_ITEM list ${out})
    foreach(item ${list})
        set(${out} "${${out}} ${item}")
    endforeach()
endmacro(list2string)

# Cross Compiling for Hexagon
set(HEXAGON TRUE)
set(CMAKE_SYSTEM_NAME QURT)
set(CMAKE_SYSTEM_PROCESSOR Hexagon)
set(CMAKE_SYSTEM_VERSION "1") #${HEXAGON_PLATFORM_LEVEL})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

#Get the Binary extension of the Hexagon Toolchain 
if(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    set(HEXAGON_TOOLCHAIN_SUFFIX .exe)
endif()
message(STATUS "CMAKE_HOST_SYSTEM_NAME:${CMAKE_HOST_SYSTEM_NAME}")

# Root of 'Hexagon SDK'
get_filename_component(HEXAGON_SDK_ROOT "$ENV{HEXAGON_SDK_ROOT}" REALPATH)
# Root of 'Hexagon LLVM Toolchain Release'
# Currently setting this from SDK Environment variable
get_filename_component(HEXAGON_TOOLCHAIN "$ENV{DEFAULT_HEXAGON_TOOLS_ROOT}" REALPATH)

# LLVM toolchain versioning, to be set from ENV Variables
set(TOOLCHAIN_VER v83)
set(CMAKE_LIBRARY_ARCHITECTURE v66)
set(HEXAGON_ARCH v66)
set(HEXAGON_LIB_DIR "${HEXAGON_TOOLCHAIN}/Tools/target/hexagon/lib")
set(HEXAGON_ISS_DIR ${HEXAGON_TOOLCHAIN}/Tools/lib/iss)
set(SIM_V_ARCH "${HEXAGON_ARCH}g_1024")
set(RUN_MAIN_HEXAGON "${HEXAGON_SDK_ROOT}/libs/common/run_main_on_hexagon/ship/${V}/run_main_on_hexagon_sim")
#QURT SPECIFIC LIBS and Includes
set(HEXAGON_CMAKE_ROOT ${HEXAGON_SDK_ROOT}/build/cmake)

#include(${HEXAGON_CMAKE_ROOT}/qurt_libs.cmake)
# QURT Related Includes
set(V_ARCH ${HEXAGON_ARCH})
set(_QURT_INSTALL_DIR "${HEXAGON_SDK_ROOT}/libs/common/qurt/ADSP${V_ARCH}MP${V_ARCH_EXTN}") 
if(${V_ARCH} MATCHES v65 OR ${V_ARCH} MATCHES v66)
    set(_QURT_INSTALL_DIR "${HEXAGON_SDK_ROOT}/libs/common/qurt/compute${V_ARCH}${V_ARCH_EXTN}") 
endif() 

if( ${TREE} MATCHES PAKMAN )
    if(${V_ARCH} MATCHES v65 OR ${V_ARCH} MATCHES v66)
        set(_QURT_INSTALL_DIR "${QURT_IMAGE_DIR}/compute${V_ARCH}${V_ARCH_EXTN}") 
    endif() 
endif()
message(STATUS "_QURT_INSTALL_DIR:${_QURT_INSTALL_DIR}")
set(RTOS_DIR ${_QURT_INSTALL_DIR})
set(QCC_DIR "${HEXAGON_QCC_DIR}/${V_ARCH}/G0")
set(TARGET_DIR "${HEXAGON_LIB_DIR}/${V_ARCH}/G0")
include_directories(
    ${_QURT_INSTALL_DIR}/include
    ${_QURT_INSTALL_DIR}/include/qurt
    ${_QURT_INSTALL_DIR}/include/posix
    )
set(QURT_START_LINK_LIBS)
list2string(QURT_START_LINK_LIBS
    "${TARGET_DIR}/init.o"
    "${RTOS_DIR}/lib/crt1.o"
    "${RTOS_DIR}/lib/libqurt.a"
    "${TARGET_DIR}/libc.a"
    "${TARGET_DIR}/libqcc.a"
    "${TARGET_DIR}/libhexagon.a"
    "${RTOS_DIR}/lib/libqurtcfs.a"
    "${RTOS_DIR}/lib/libtimer.a"
    "${RTOS_DIR}/lib/libposix.a"
    )
set(QURT_END_LINK_LIBS
    ${TARGET_DIR}/fini.o
    )
set(LD_FLAG "-Wl,")
set(EXE_LD_FLAGS)
list2string(EXE_LD_FLAGS
    -m${V_ARCH}
    -g -nostdlib
    ${LD_FLAG}--section-start ${LD_FLAG}.interp=0x23000000
    ${LD_FLAG}--dynamic-linker= ${LD_FLAG}--force-dynamic ${LD_FLAG}-E ${LD_FLAG}-z ${LD_FLAG}muldefs ${LD_FLAG}--whole-archive
    "-o <TARGET>"
    ${_LDFLAG}--start-group  
    "<OBJECTS>" 
    ${QURT_START_LINK_LIBS}
    "<LINK_LIBRARIES>" 
    ${QURT_END_LINK_LIBS}
    ${_LDFLAG}--end-group
    )
set(EXE_PURE_LD_FLAGS)
list2string(EXE_PURE_LD_FLAGS
    -m${V_ARCH}
    -g -nostdlib
    --section-start .interp=0x23000000
    --dynamic-linker= --force-dynamic -E -z muldefs --whole-archive
    "-o <TARGET>"
    --start-group  
    ${QURT_START_LINK_LIBS}
    "<OBJECTS>" 
    "<LINK_LIBRARIES>" 
    ${QURT_END_LINK_LIBS}
    --end-group
    )
if(QURT_OS)
    set(HEXAGON_C_LINK_EXECUTABLE_LINK_OPTIONS "${EXE_PURE_LD_FLAGS}" )
    message(STATUS "Hexagon C Executable Linker Line:${HEXAGON_C_LINK_EXECUTABLE_LINK_OPTIONS}")
    set(HEXAGON_CXX_LINK_EXECUTABLE_LINK_OPTIONS "${EXE_PURE_LD_FLAGS}")
    message(STATUS "Hexagon  CXX Executable Linker Line:${HEXAGON_CXX_LINK_EXECUTABLE_LINK_OPTIONS}")
endif()
#END OF QURT INCLUDES


# System include paths
include_directories(SYSTEM ${HEXAGON_SDK_ROOT}/incs)
include_directories(SYSTEM ${HEXAGON_SDK_ROOT}/incs/stddef)

# LLVM toolchain setup
# Compiler paths, options and architecture
set(CMAKE_C_COMPILER ${HEXAGON_TOOLCHAIN}/Tools/bin/hexagon-clang${HEXAGON_TOOLCHAIN_SUFFIX})
set(CMAKE_CXX_COMPILER ${HEXAGON_TOOLCHAIN}/Tools/bin/hexagon-clang++${HEXAGON_TOOLCHAIN_SUFFIX})
set(CMAKE_ASM_COMPILER ${HEXAGON_TOOLCHAIN}/Tools/bin/hexagon-clang++${HEXAGON_TOOLCHAIN_SUFFIX})
set(HEXAGON_LINKER ${HEXAGON_TOOLCHAIN}/Tools/bin/hexagon-link${HEXAGON_TOOLCHAIN_SUFFIX})
set(CMAKE_PREFIX_PATH ${HEXAGON_TOOLCHAIN}/Tools/target/hexagon)
#Hexagon Simulator
set(HEXAGON_SIM    "${HEXAGON_TOOLCHAIN}/Tools/bin/hexagon-sim${HEXAGON_TOOLCHAIN_SUFFIX}")

#Compiler and Linker Options
add_compile_options(-O2)
add_compile_options(-m${HEXAGON_ARCH})
add_compile_options(-Wno-error=undefined-optimized) # Erroneous warning supposedly fixed in Toolchains 8.3 but not. Disabled until further notice.
# HVX settings
add_compile_options("-mhvx" "-mhvx-length=128B")
set(CMAKE_ASM_FLAGS  "${CMAKE_ASM_FLAGS} ${CMAKE_CXX_FLAGS}")
# Linker Options
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -m${HEXAGON_ARCH} ")
if(QURT_OS)
    set(CMAKE_C_LINK_EXECUTABLE "${HEXAGON_LINKER} ${HEXAGON_C_LINK_EXECUTABLE_LINK_OPTIONS}")
    set(CMAKE_CXX_LINK_EXECUTABLE "${HEXAGON_LINKER} ${HEXAGON_CXX_LINK_EXECUTABLE_LINK_OPTIONS}")
endif()


#Helper Function to run on Hexagon SIM
##############################
#
# runHexagonSim (<targetToRunOnSimulator>) 
# 
# This fuction will help create a target to run simulator on the target
# specified from the argument
#
# You can update <HEXAGON_EXEC_SIM_OPTIONS> to customized the options for
# simualtor
#
# You can also update <HEXAGON_EXEC_CMD_OPTIONS> to customized the options
# for executable 
#
##############################

function(runHexagonSim currentTarget)
    message(STATUS "runHexagonSim:currentTarget:${currentTarget}")
    set(q6ssOUT ${CMAKE_CURRENT_BINARY_DIR}/q6ss.cfg)
    if(CMAKE_HOST_WIN32)
        set(q6ssLine1
            ${HEXAGON_ISS_DIR}/qtimer.dll --csr_base=0xFC900000 --irq_p=1 --freq=19200000 --cnttid=1)
        set(q6ssLine2
            ${HEXAGON_ISS_DIR}/l2vic.dll 32 0xab010000)
    else()
        set(q6ssLine1
            "${HEXAGON_ISS_DIR}/qtimer.so --csr_base=0xFC900000 --irq_p=1 --freq=19200000 --cnttid=1")
        set(q6ssLine2
            "${HEXAGON_ISS_DIR}/l2vic.so 32 0xFC910000")
    endif()
    set(osamOUT ${CMAKE_CURRENT_BINARY_DIR}/osam.cfg)
    if(CMAKE_HOST_WIN32)
        set(dll qurt_model.dll)
        set(osamString ${RTOS_DIR}/debugger/Win/${dll})
    else()
        set(osamString ${RTOS_DIR}/debugger/lnx64/qurt_model.so)
    endif()
    set(HEXAGON_EXEC ${currentTarget})
    message(STATUS "QURT FLAG ; ${QURT_OS}")
    if(QURT_OS)
    
        add_custom_target( ${currentTarget}-q6ss
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
            COMMAND @echo ${q6ssLine1} > ${q6ssOUT}
            COMMAND @echo ${q6ssLine2} >> ${q6ssOUT}
            BYPRODUCTS  ${q6ssOUT}
        )
        add_custom_target( ${currentTarget}-osam_cfg 
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
            COMMAND echo ${osamString} > ${osamOUT}
            BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/osam.cfg
        ) 

        set(QEXE_EXEC
            ${HEXAGON_SIM} -m${SIM_V_ARCH} --simulated_returnval
            --usefs ${CMAKE_CURRENT_BINARY_DIR}
            --pmu_statsfile ${CMAKE_CURRENT_BINARY_DIR}/pmu_stats.txt
            ${HEXAGON_EXEC_SIM_OPTIONS}
            --cosim_file ${CMAKE_CURRENT_BINARY_DIR}/q6ss.cfg
            --l2tcm_base 0xd800
            --rtos ${CMAKE_CURRENT_BINARY_DIR}/osam.cfg ${RTOS_DIR}/sdksim_bin/runelf.pbn --# 
            ${RUN_MAIN_HEXAGON} --#
            lib${HEXAGON_EXEC}.so    
            ${HEXAGON_EXEC_CMD_OPTIONS}
        )
        message(STATUS "QEXE_EXEC:${QEXE_EXEC}")

        add_custom_target(${currentTarget}-sim 
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
            COMMAND ${QEXE_EXEC}  
            DEPENDS  ${currentTarget}-osam_cfg ${HEXAGON_EXEC} ${currentTarget}-q6ss
        )    
    endif()
endfunction()


