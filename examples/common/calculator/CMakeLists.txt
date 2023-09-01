# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.14.3)

project(calculator C CXX ASM)

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

    # Build libcalculator.so
    add_library(calculator
        SHARED 
        ${CMAKE_CURRENT_BINARY_DIR}/calculator_stub
    )
    # This changes the libname to libcalculator_stub.so from default libcalculator.so
    #set_target_properties(calculator PROPERTIES SUFFIX "_stub.so")
    
    # Build the APPs Binary calculator Device 
    add_executable(calculator_device
        ${CMAKE_CURRENT_SOURCE_DIR}/src/calculator_main
        ${CMAKE_CURRENT_SOURCE_DIR}/src/calculator_test
    )
    
    target_compile_definitions(
        calculator_device
        PUBLIC VERIFY_PRINT_ERROR
    )

    target_link_options(calculator_device PUBLIC -llog)

    message(STATUS "ANDROID_NDK_REVISION:${ANDROID_NDK_REVISION}")
    message(STATUS "ANDROID_NDK_RELEASE:${ANDROID_NDK_RELEASE}")

    #This is to work around the NDK r19c C++ issues
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/")
    endif()
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${ANDROID_NDK}/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/")
    endif()

    # Base Include dirs for the Project
    set(incs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_BINARY_DIR}/
		${HEXAGON_SDK_ROOT}/incs/
        ${HEXAGON_SDK_ROOT}/incs/stddef
        #${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/        
    )
    
    ##############################
    #
    # buildIDL (<idlFile> <currentTaget>) 
    # 
    # This function will set up a custom_target to build <idlFile> using qaic
    # IDL compiler. For foo.idl, it wll generate foo.h, foo_stub.c and
    # foo_skel.c into ${CMAKE_CURRENT_BINARY_DIR} diretory.  
    #
    # This function will also add the custom_target created as the dependency
    # of <currentTarget>
    #
    ##############################
    buildIDL( inc/calculator.idl calculator)

    # Add necessary includes for the dependent libs (dsprpc)
    include_directories(
        ${incs}        
    )
    
    # slect the required RPC Build
    #default is CDSP
    SET(dsprpc "cdsprpc")        
    if(${DSP_TYPE} MATCHES "mdsp" )
        #  MDSP
        SET(dsprpc "mdsprpc")    
    elseif(${DSP_TYPE} MATCHES "adsp")
        #  ADSP
        SET(dsprpc "adsprpc")        
    elseif(${DSP_TYPE} MATCHES "sdsp")
        #  SDSP
        SET(dsprpc "sdsprpc")        
    else()
        #  Default CDSP
        SET(dsprpc "cdsprpc")
    endif()
    add_library(${dsprpc} SHARED IMPORTED) # or STATIC instead of SHARED
    set_target_properties(${dsprpc} PROPERTIES
        IMPORTED_LOCATION "${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/lib${dsprpc}.so"
        INTERFACE_INCLUDE_DIRECTORIES "${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/"
    )
    
    # Build the required Libs from sources as an External Project
	ExternalProject_Add(rpcmem
        #PREFIX ${HEXAGON_SDK_ROOT}/libs/common/rpcmem
        SOURCE_DIR ${HEXAGON_SDK_ROOT}/libs/common/rpcmem
        STAMP_DIR ${HEXAGON_SDK_ROOT}/libs/common/rpcmem/${V}.cmake/
        #BUILD_IN_SOURCE 1
        BINARY_DIR ${HEXAGON_SDK_ROOT}/libs/common/rpcmem/${V}.cmake/
        INSTALL_DIR  ${CMAKE_CURRENT_BINARY_DIR}
        BUILD_COMMAND make all
        CMAKE_ARGS -DV=${V} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR} -DHEXAGON_CMAKE_ROOT=${HEXAGON_CMAKE_ROOT} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON -DANDROID_NDK=${ANDROID_NDK} -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_STL=none -DANDROID_NATIVE_API_LEVEL=21        
    )
    
    include_directories(${HEXAGON_SDK_ROOT}/libs/common/rpcmem/inc)
    # Mark the Dependecies
    add_dependencies(calculator ${dsprpc})
    add_dependencies(calculator_device calculator rpcmem)
    # Link the libs and the binary        
    target_link_libraries( calculator  ${dsprpc} )
    target_link_libraries( calculator_device calculator  ${CMAKE_CURRENT_BINARY_DIR}/lib/rpcmem.a)


else()
    #################################################
    #Hexagon Dynamic Library
    #################################################
    if(${BUILD_NAME} MATCHES "skel")

        # Build libcalculator_stub.so
        add_library(calculator_skel
            SHARED 
            ${CMAKE_CURRENT_BINARY_DIR}/calculator_skel 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/calculator_imp 
            
        )
        # Set some Flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

        set(incs 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/
            ${CMAKE_CURRENT_BINARY_DIR}/
            ${HEXAGON_SDK_ROOT}/incs/HAP/
			${HEXAGON_SDK_ROOT}/incs/
            ${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/
            #${HEXAGON_TOOLCHAIN_ROOT}/Tools/target/hexagon/include/
            )

        
        # Include necessary dependent lib directory
        include_directories(
            ${incs}          
        )

        ##############################
        #
        # buildIDL (<idlFile> <currentTaget>) 
        # 
        # This function will set up a custom_target to build <idlFile> using qaic
        # IDL compiler. For foo.idl, it wll generate foo.h, foo_stub.c and
        # foo_skel.c into ${CMAKE_CURRENT_BINARY_DIR} diretory.  
        #
        # This function will also add the custom_target created as the dependency
        # of <currentTarget>
        #
        ##############################
        buildIDL( "inc/calculator.idl" calculator_skel incs)

        # Add the dependent libs
        #add_dependencies(calculator_skel ${hexagonTargets})

        # Link the dependent libs
        target_link_libraries(calculator_skel)


    elseif(${BUILD_NAME} MATCHES "sim")

    
        #################################################
        # Static elf
        #################################################

        set(incs 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/
            ${CMAKE_CURRENT_BINARY_DIR}/
            ${HEXAGON_SDK_ROOT}/incs/HAP/
            ${HEXAGON_SDK_ROOT}/incs/stddef/
            ${HEXAGON_SDK_ROOT}/incs/
            ${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/
            #${HEXAGON_TOOLCHAIN_ROOT}/Tools/target/hexagon/include/
            )
        #add_executable(calculator_q 
        add_executable(calculator_q-sim                        
            ${CMAKE_CURRENT_SOURCE_DIR}/src/calculator_test_main 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/calculator_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/calculator_test             
            )

        # Set some of the flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions ")
        
        # Include necessary dependent lib directory
        include_directories(
            ${incs}          
        )
        
        # Import the Required RTLD Lib    
        add_library(rtld STATIC IMPORTED) # or STATIC instead of SHARED
        set_target_properties(rtld PROPERTIES
            IMPORTED_LOCATION "${HEXAGON_SDK_ROOT}/libs/common/rtld/ship/${V}/rtld.a"
            INTERFACE_INCLUDE_DIRECTORIES "${HEXAGON_SDK_ROOT}/libs/common/rtld/ship/${V}/"
        )
        
        ##############################
        #
        # buildIDL (<idlFile> <currentTaget>) 
        # 
        # This function will set up a custom_target to build <idlFile> using qaic
        # IDL compiler. For foo.idl, it wll generate foo.h, foo_stub.c and
        # foo_skel.c into ${CMAKE_CURRENT_BINARY_DIR} diretory.  
        #
        # This function will also add the custom_target created as the dependency
        # of <currentTarget>
        #
        ##############################
        buildIDL( "inc/calculator.idl" calculator_q-sim incs)
        
        # Build the required Libs from sources as an External Project
						
        ExternalProject_Add(test_util
            #PREFIX ${HEXAGON_SDK_ROOT}/test/common/test_util
            SOURCE_DIR ${HEXAGON_SDK_ROOT}/test/common/test_util
            STAMP_DIR ${HEXAGON_SDK_ROOT}/test/common/test_util/${V}.cmake/
            #BUILD_IN_SOURCE 1
            BINARY_DIR ${HEXAGON_SDK_ROOT}/test/common/test_util/${V}.cmake/
            INSTALL_DIR  ${CMAKE_CURRENT_BINARY_DIR}
            BUILD_COMMAND make all
            CMAKE_ARGS -DV=${V} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR} -DHEXAGON_CMAKE_ROOT=${HEXAGON_CMAKE_ROOT} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DBUILD_NAME=test_util_skel
        )    
        include_directories(${HEXAGON_SDK_ROOT}/test/common/test_util/src)
        
        ExternalProject_Add(rpcmem
            #PREFIX ${HEXAGON_SDK_ROOT}/libs/common/rpcmem
            SOURCE_DIR ${HEXAGON_SDK_ROOT}/libs/common/rpcmem
            STAMP_DIR ${HEXAGON_SDK_ROOT}/libs/common/rpcmem/${V}.cmake/
            #BUILD_IN_SOURCE 1
            BINARY_DIR ${HEXAGON_SDK_ROOT}/libs/common/rpcmem/${V}.cmake/
            INSTALL_DIR  ${CMAKE_CURRENT_BINARY_DIR}
            BUILD_COMMAND make all
			CMAKE_ARGS -DV=${V} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR} -DHEXAGON_CMAKE_ROOT=${HEXAGON_CMAKE_ROOT} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DBUILD_NAME=rpcmem_skel            
        )    
        include_directories(${HEXAGON_SDK_ROOT}/libs/common/rpcmem/inc)
        
        ExternalProject_Add(atomic
            #PREFIX ${HEXAGON_SDK_ROOT}/libs/common/atomic
            SOURCE_DIR ${HEXAGON_SDK_ROOT}/libs/common/atomic
            STAMP_DIR ${HEXAGON_SDK_ROOT}/libs/common/atomic/${V}.cmake/
            #BUILD_IN_SOURCE 1
            BINARY_DIR ${HEXAGON_SDK_ROOT}/libs/common/atomic/${V}.cmake/
            INSTALL_DIR  ${CMAKE_CURRENT_BINARY_DIR}
            BUILD_COMMAND make all
			CMAKE_ARGS -DV=${V} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR} -DHEXAGON_CMAKE_ROOT=${HEXAGON_CMAKE_ROOT} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} -DBUILD_NAME=atomic_skel
        )    
        include_directories(${HEXAGON_SDK_ROOT}/libs/common/atomic/inc)
        
        # Add the Target Dependencies     
        add_dependencies(calculator_q-sim rpcmem rtld test_util atomic)
        # Link the dependent libs
        target_link_libraries(calculator_q-sim ${CMAKE_CURRENT_BINARY_DIR}/lib/rpcmem.a rtld ${CMAKE_CURRENT_BINARY_DIR}/lib/test_util.a ${CMAKE_CURRENT_BINARY_DIR}/lib/atomic.a)        
                  
        message(STATUS "running on simulator")
        set(HEXAGON_EXEC calculator_q-sim)
        set(QEXE_EXEC 
		    ${HEXAGON_SIM} ${HEXAGON_EXEC_SIM_OPTIONS} -m${SIM_V_ARCH} --simulated_returnval
			--usefs ${CMAKE_CURRENT_BINARY_DIR}/ 
			--pmu_statsfile ${CMAKE_CURRENT_BINARY_DIR}/pmu_stats.txt 
			calculator_q-sim ${HEXAGON_EXEC_CMD_OPTIONS}-- )
        message(STATUS "QEXE_EXEC:${QEXE_EXEC}")
        set(UNIT_TEST calculator_q-sim)
        add_custom_command(
            TARGET ${UNIT_TEST}
            COMMENT "running on simulator"
            POST_BUILD 
            COMMAND ${QEXE_EXEC}
        )     

    endif()
endif()

# vim: set noet fenc=utf-8 ff=unix ft=cmake :
