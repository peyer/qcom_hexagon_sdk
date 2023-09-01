# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.14.3)

project(benchmark C CXX ASM)

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
    set(ignoreMe ${DSP_TYPE})

    add_library(benchmark
        SHARED 
        ${CMAKE_CURRENT_BINARY_DIR}/benchmark_stub
        )

    add_executable(benchmark_device
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/benchmark 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/dilate5x5_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/dilate3x3_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/integrate_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/epsilon_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/bilateral_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/fast9_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/conv3x3_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/gaussian7x7_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/sobel3x3_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/crash10_ref 
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/fft_ref
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/warp_ref
        ${CMAKE_CURRENT_SOURCE_DIR}/src_app/scatter_gather_ref
        )

    target_compile_definitions(
        benchmark_device
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

    set(incs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
        ${CMAKE_CURRENT_BINARY_DIR}/
        )


    ##############################
    #
    # prepare_libraries_hexagon (<hexagon_targets> <hexagon_incs>  <hexagon_libs> 
    #   libName [libNames...]) 
    # 
    # The first 3 arguments will be the output arguments.  And the
    # following arguments will be the library names.  Without surfix, it is
    # treated as a dynamic library. surfixed with ".a" will be processed as
    # static library. And surffixed with ".so" will be processed as dynamic
    # library.
    #
    # This function will do the following:
    #
    # (1) For all libraries that specified, it will search Hexagon SDK tree
    # to find the corresponding library, and add a target into the
    # <hexagon_targets> list.  The custom_target will specify what to do
    # for that target.  It can be going into that corresponding directory to
    # build the directory or do nothing if it's prebuilt library. Caller of
    # this function can add this target as the dependency of their own
    # target or multiple targets
    #
    # (2) This call will add the "library specific" include directories
    # into <hexagon_incs> list. Caller of the function can add this include
    # path list into their corresponding include list  
    #
    # (3) This library call will also return a "library-target" list
    # <hexagon_libs> so that it can be added into the linker dependency
    # list from target_link_libraries call
    #
    ##############################
    prepare_libraries_hexagon(hexagonTargets_1 hexagonIncs_1 hexagonLibs_1 hexagonSharedLibs_1
        adsprpc
        )

    message(STATUS "hexagonTarget:${hexagonTargets_1}")
    message(STATUS "hexagonLibs:${hexagonLibs_1}")
    message(STATUS "hexagonIncs:${hexagonIncs_1}")


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
    buildIDL( inc/benchmark.idl benchmark)

    # Step 1: Add hexagonIncs as part of your includes
    include_directories(
        ${incs}
        ${hexagonIncs_1}
        )
    # Step 2: Add hexagonTargets as one of your dependencies 
    add_dependencies(benchmark ${hexagonTargets_1})

    # Step 3: Add hexagonLibs as your target link dependency 
    target_link_libraries( benchmark ${hexagonLibs_1} )

    

    ##############################
    #
    # prepare_libraries_hexagon (<hexagon_targets> <hexagon_incs>  <hexagon_libs> 
    #   libName [libNames...]) 
    # 
    # The first 3 arguments will be the output arguments.  And the
    # following arguments will be the library names.  Without surfix, it is
    # treated as a dynamic library. surfixed with ".a" will be processed as
    # static library. And surffixed with ".so" will be processed as dynamic
    # library.
    #
    # This function will do the following:
    #
    # (1) For all libraries that specified, it will search Hexagon SDK tree
    # to find the corresponding library, and add a target into the
    # <hexagon_targets> list.  The custom_target will specify what to do
    # for that target.  It can be going into that corresponding directory to
    # build the directory or do nothing if it's prebuilt library. Caller of
    # this function can add this target as the dependency of their own
    # target or multiple targets
    #
    # (2) This call will add the "library specific" include directories
    # into <hexagon_incs> list. Caller of the function can add this include
    # path list into their corresponding include list  
    #
    # (3) This library call will also return a "library-target" list
    # <hexagon_libs> so that it can be added into the linker dependency
    # list from target_link_libraries call
    #
    ##############################
    prepare_libraries_hexagon(hexagonTargets_2 hexagonIncs_2 hexagonLibs_2 hexagonSharedLibs_2
        rpcmem.a
        )

    # Step 1: Add hexagonIncs as part of your includes
    include_directories(
        ${hexagonIncs_2}
        )
    # Step 2: Add hexagonTargets as one of your dependencies 
    add_dependencies(benchmark_device ${hexagonTargets_2})
    # Step 3: Add hexagonLibs as your target link dependency 
    set (EXTRA_LIBS ${EXTRA_LIBS} ${hexagonLibs_2})
    set (EXTRA_LIBS ${EXTRA_LIBS} benchmark)

    add_dependencies(benchmark_device benchmark)

    #cmake_policy(SET CMP0003 OLD)
    target_link_libraries( benchmark_device ${EXTRA_LIBS} m )


else()
    #################################################
    #Hexagon Dynamic Library
    #################################################
    if(${BUILD_NAME} MATCHES "skel")

        add_library(benchmark_skel
            SHARED 
            ${CMAKE_CURRENT_BINARY_DIR}//benchmark_skel 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/benchmark_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/dilate5x5_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/dilate3x3_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/integrate_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/epsilon_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/bilateral_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/fast9_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/conv3x3a32_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/gaussian7x7_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/warp_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/sobel3x3_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/sobel3x3_C_intrinsics 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/fft_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/scatter_gather_imp
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/crash10_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/dilate5x5_asm.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/dilate3x3_asm.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/integrate_asm.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/epsilon.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/bilateral.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/bilateral_v65.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/fast9_coarse.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/fast9_fine.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/conv3x3a32_asm.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/gaussian7x7_asm.S 
            )

        # Set some Flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")


        set(incs 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/
            ${CMAKE_CURRENT_BINARY_DIR}/
            ${HEXAGON_SDK_ROOT}/incs/HAP/
            #${HEXAGON_TOOLCHAIN_ROOT}/Tools/target/hexagon/include/
            )

        ##############################
        #
        # prepare_libraries_hexagon (<hexagon_targets> <hexagon_incs>  <hexagon_libs> 
        #   libName [libNames...]) 
        # 
        # The first 3 arguments will be the output arguments.  And the
        # following arguments will be the library names.  Without surfix, it is
        # treated as a dynamic library. surfixed with ".a" will be processed as
        # static library. And surffixed with ".so" will be processed as dynamic
        # library.
        #
        # This function will do the following:
        #
        # (1) For all libraries that specified, it will search Hexagon SDK tree
        # to find the corresponding library, and add a target into the
        # <hexagon_targets> list.  The custom_target will specify what to do
        # for that target.  It can be going into that corresponding directory to
        # build the directory or do nothing if it's prebuilt library. Caller of
        # this function can add this target as the dependency of their own
        # target or multiple targets
        #
        # (2) This call will add the "library specific" include directories
        # into <hexagon_incs> list. Caller of the function can add this include
        # path list into their corresponding include list  
        #
        # (3) This library call will also return a "library-target" list
        # <hexagon_libs> so that it can be added into the linker dependency
        # list from target_link_libraries call
        #
        ##############################
        prepare_libraries_hexagon(hexagonTargets hexagonIncs hexagonLibs hexagonSharedLibs
            libdspCV_skel
            libqprintf.a
            )
        message(STATUS "hexagonTarget:${hexagonTargets}")
        message(STATUS "hexagonLibs:${hexagonLibs}")
        message(STATUS "hexagonIncs:${hexagonIncs}")

        # Step 1: Add hexagonIncs as part of your includes
        include_directories(
            ${incs}
            ${hexagonIncs}
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
        buildIDL( "inc/benchmark.idl" benchmark_skel incs)

        # Step 2: Add hexagonTargets as one of your dependencies 
        add_dependencies(benchmark_skel ${hexagonTargets})

        # Step 3: Add hexagonLibs as your target link dependency 
        target_link_libraries( benchmark_skel 
            ${hexagonLibs})


    elseif(${BUILD_NAME} MATCHES "sim")

    
        #################################################
        # Static elf
        #################################################

        set(incs 
            ${CMAKE_CURRENT_SOURCE_DIR}/src/
            ${CMAKE_CURRENT_BINARY_DIR}/
            ${HEXAGON_SDK_ROOT}/incs/HAP/
            #${HEXAGON_TOOLCHAIN_ROOT}/Tools/target/hexagon/include/
            )
        #add_executable(benchmark_q 
        add_library(benchmark_q             
            SHARED
            ${CMAKE_CURRENT_BINARY_DIR}/benchmark_skel 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/benchmark_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/benchmark 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/dilate5x5_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/dilate5x5_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/dilate3x3_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/dilate3x3_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/integrate_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/integrate_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/epsilon_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/epsilon_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/bilateral_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/bilateral_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/warp_ref
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/warp_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/fast9_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/fast9_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/fast9_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/fft_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/fft_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/scatter_gather_ref
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/scatter_gather_imp
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/conv3x3_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/conv3x3a32_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/gaussian7x7_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/gaussian7x7_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/sobel3x3_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/sobel3x3_imp 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/sobel3x3_C_intrinsics 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_app/crash10_ref 
            ${CMAKE_CURRENT_SOURCE_DIR}/src_dsp/crash10_imp 

            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/dilate5x5_asm.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/dilate3x3_asm.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/integrate_asm.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/epsilon.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/bilateral.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/bilateral_v65.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/fast9_coarse.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/fast9_fine.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/conv3x3a32_asm.S 
            ${CMAKE_CURRENT_SOURCE_DIR}/asm_src/gaussian7x7_asm.S 
            )

        # Set some of the flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions ")

        ##############################
        #
        # prepare_libraries_hexagon (<hexagon_targets> <hexagon_incs>  <hexagon_libs> 
        #   libName [libNames...]) 
        # 
        # The first 3 arguments will be the output arguments.  And the
        # following arguments will be the library names.  Without surfix, it is
        # treated as a dynamic library. surfixed with ".a" will be processed as
        # static library. And surffixed with ".so" will be processed as dynamic
        # library.
        #
        # This function will do the following:
        #
        # (1) For all libraries that specified, it will search Hexagon SDK tree
        # to find the corresponding library, and add a target into the
        # <hexagon_targets> list.  The custom_target will specify what to do
        # for that target.  It can be going into that corresponding directory to
        # build the directory or do nothing if it's prebuilt library. Caller of
        # this function can add this target as the dependency of their own
        # target or multiple targets
        #
        # (2) This call will add the "library specific" include directories
        # into <hexagon_incs> list. Caller of the function can add this include
        # path list into their corresponding include list  
        #
        # (3) This library call will also return a "library-target" list
        # <hexagon_libs> so that it can be added into the linker dependency
        # list from target_link_libraries call
        #
        ##############################
        
        prepare_libraries_hexagon(hexagonTargets hexagonIncs hexagonLibs hexagonSharedLibs
            libdspCV_skel
            libqprintf
            rpcmem
            )

        message(STATUS "hexagonTarget:${hexagonTargets}")
        message(STATUS "hexagonLibs:${hexagonLibs}")
        message(STATUS "hexagonIncs:${hexagonIncs}")
        message(STATUS "hexagonSharedLibs:${hexagonSharedLibs}")

        # Step 1: Add hexagonIncs as part of your includes
        include_directories(
            ${incs}
            ${hexagonIncs}
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
        buildIDL( "inc/benchmark.idl" benchmark_q ${hexagonIncs})
        
         # Step 2: Add hexagonTargets as one of your dependencies 
        add_dependencies(benchmark_q ${hexagonTargets})

        # Step 3: Add hexagonLibs as your target link dependency 
        target_link_libraries( benchmark_q ${hexagonLibs} )

        set(HEXAGON_EXEC benchmark_q)
        #set(HEXAGON_EXEC_SIM_OPTIONS 
        #    ${HEXAGON_EXEC_SIM_OPTIONS} --dsp_clock 1000 --ahb:lowaddr
        #    0xc0000000 --ahb:highaddr 0xc0ffffff)

        set(HEXAGON_EXEC_CMD_OPTIONS ${HEXAGON_EXEC_CMD_OPTIONS} -f epsilon -w 256 -h 64)
        file(GLOB MY_PUBLIC_LIBS  ${hexagonLibs} )            
        
        # HAck to copy some of the missing Libs
        # Currently, yet to figure out on finding these missing libs in the dependency list          
        file(COPY ${HEXAGON_LIB_DIR}/${HEXAGON_ARCH}/G0/pic/libc++abi.so.1 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(COPY ${HEXAGON_LIB_DIR}/${HEXAGON_ARCH}/G0/pic/libc++.so.1 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        
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
        runHexagonSim(benchmark_q hexagonSharedLibs)

    endif()
endif()

# vim: set noet fenc=utf-8 ff=unix ft=cmake :
