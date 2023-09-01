# Copyright (c) 2017 QUALCOMM Technologies Incorporated.
# _all Rights Reserved Qualcomm Proprietary

# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)

# Set HEXAGON_SDK_ROOT and Build variant
set(HEXAGON_SDK_ROOT "C:/Qualcomm/Hexagon_SDK/3.4.1")
set(V "android_Debug")

set(incs
    ${HEXAGON_SDK_ROOT}/incs/
    ${HEXAGON_SDK_ROOT}/incs/stddef/
    )
include_directories(${incs})

macro (list2string out in)
	#string(REPLACE ";" " " ${out} "${in}")
	#message(STATUS "OUT:${${out}}")
	set(list ${ARGV})
	list(REMOVE_ITEM list ${out})
	foreach(item ${list})
		set(${out} "${${out}} ${item}")
	endforeach()
endmacro(list2string)

##############################
#
# prepare_hexagon_helper_libraries (<hexagon_targets> <hexagon_incs>  <hexagon_libs>
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
function(prepare_hexagon_helper_libraries hexagonTarget hexagonIncs hexagonLibs)
	add_custom_target(${hexagonTarget}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/
		COMMAND echo "dump allHexTargets" 
		BYPRODUCTS  
		)
	set(allHexLibs)

	# When setting up the dependent tree, we don't want to set up current
	# target depending on all others in the first level tree. Rather we want
	# to set up the them depend on each other sequencially. The reason we
	# want to this is when compiling in mutli-process way, we want to
	# dependent modules to be built one after the other, because otherwise
	# it might be conflicting with complicated pakman dependent tree
	set(lastTarget ${hexagonTarget})

	foreach(libNameUnstripped ${ARGN})
		set(FIND_LIBRARY_HEXAGON_STATIC FALSE)

		# For prebuild library, the name and include file are with
		# different convention

		if(libNameUnstripped MATCHES "cdsprpc")
			set(allHexLibs ${allHexLibs} -L${remote_src}/ship/${V} libcdsprpc.so)
			set(HEXAGON_ALL_INCS ${HEXAGON_ALL_INCS} ${remote_inc})
			continue()
		endif()

		if(${libNameUnstripped} MATCHES "\.a$") #ends with .a
			STRING(REGEX REPLACE "(.*)\.a" "\\1"
				libName
				${libNameUnstripped})
			set(FIND_LIBRARY_HEXAGON_STATIC TRUE)
		elseif() #ends with .so
			STRING( REGEX MATCH "(.*)\.so" 
				libName
				${libNameUnstripped})
		else() #dynamic by default
			set(libName
				${libNameUnstripped})
		endif()
		message(STATUS "INC for libName:${libName}")

		set(libSrc ${${libName}_src})
		if(FIND_LIBRARY_HEXAGON_STATIC)
			add_custom_target( qLibTarget${libName}
				WORKING_DIRECTORY ${libSrc}/
				COMMAND ${HEXAGON_SDK_ROOT}/setup_sdk_env.cmd
				COMMAND make tree_clean V=${V} && make tree V=${V}
				BYPRODUCTS ${libSrc}/${V}/ship/${libName}.a
				)

			message(STATUS
				"BYPRODUCTS:${libSrc}/${V}/ship/${libName}.a")

			add_library( ${libName}-lib
				STATIC 
				IMPORTED )
			set_target_properties( # Specifies the target library.
				${libName}-lib
				PROPERTIES IMPORTED_LOCATION
				${libSrc}/${V}/ship/${libName}.a
				)
		else()
			add_custom_target( qLibTarget${libName} 
				WORKING_DIRECTORY ${libSrc}/
				COMMAND ${HEXAGON_SDK_ROOT}/setup_sdk_env.cmd
				COMMAND make tree_clean V=${V} && make tree V=${V}
				COMMAND echo "" 
				BYPRODUCTS  ${libSrc}/${V}/ship/${libName}.so
				)

			message(STATUS
				"BYPRODUCTS:${libSrc}/${V}/ship/${libName}.so")

			add_library( ${libName}-lib
				SHARED 
				IMPORTED )
				
			set_target_properties( # Specifies the target library.
				${libName}-lib
				PROPERTIES IMPORTED_LOCATION
				${libSrc}/${V}/ship/${libName}.so
				)
		endif()
		
		set(HEXAGON_ALL_INCS ${HEXAGON_ALL_INCS} ${${libName}_inc})
		add_dependencies(${lastTarget} qLibTarget${libName} )
		message(STATUS "${lastTarget} depending on qLibTarget${libName}")
		set(lastTarget qLibTarget${libName} )
		message(STATUS "lastTarget:${lastTarget}" )
		set(allHexLibs ${allHexLibs} ${libName}-lib)
		
	endforeach(libNameUnstripped)

	set(${hexagonTarget} ${hexagonTarget} PARENT_SCOPE)
	set(${hexagonLibs} ${allHexLibs} PARENT_SCOPE)
	set(${hexagonIncs} ${HEXAGON_ALL_INCS} PARENT_SCOPE)
	message(STATUS "hexagonTarget:${hexagonTarget}")
	message(STATUS "hexagonLibs:${${hexagonLibs}}")
	message(STATUS "hexagonIncs:${${hexagonIncs}}")
endfunction()


#get_filename_component(HEXAGON_SDK_ROOT "$ENV{HEXAGON_SDK_ROOT}" REALPATH)

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
function(buildIDL idlFile currentTarget)
	set(QIDLINC "")
	set(defaultIncs
		${HEXAGON_SDK_ROOT}/incs/
		${HEXAGON_SDK_ROOT}/incs/stddef/
		${HEXAGON_SDK_ROOT}/libs/common/rpcmem/inc
		${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/
		)
	foreach(path ${defaultIncs})
		set(QIDLINC ${QIDLINC} "-I${path}")
	endforeach(path)
#	get_target_property(dirs ${currentTarget} INCLUDE_DIRECTORIES)
#	message(STATUS "QIDLINC-dirs:${dirs}")
#	#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#	foreach(path ${dirs})
#		set(QIDLINC ${QIDLINC} "-I${path}")
#	endforeach(path)
	#foreach(path ${${currentIncs}})
	#    set(QIDLINC ${QIDLINC} "-I${path}")
	#endforeach(path)
	message(STATUS "QIDLINC:${QIDLINC}")

	get_filename_component(fileName ${idlFile} NAME_WE)

	set(cmdLineOptions -mdll -o ${CMAKE_CURRENT_BINARY_DIR} ${QIDLINC} ${idlFile})
	message(STATUS "QIDL CMDLine:${cmdLine}")

	add_custom_target( qidlTarget${fileName} 
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
		COMMAND "${proj-qaic_src}/${hostOS}/qaic" ${cmdLineOptions}  
		BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/${fileName}_skel.c
		${CMAKE_CURRENT_BINARY_DIR}/${fileName}_stub.c
		${CMAKE_CURRENT_BINARY_DIR}/${fileName}.h
		)

	message(STATUS
		"BYPRODUCTS:${CMAKE_CURRENT_BINARY_DIR}/${fileName}_skel.c
		${CMAKE_CURRENT_BINARY_DIR}/${fileName}_stub.c
		${CMAKE_CURRENT_BINARY_DIR}/${fileName}.h")


#	add_dependencies(${currentTarget} qidlTarget${fileName})

	set_source_files_properties(
		${CMAKE_CURRENT_BINARY_DIR}/${fileName}_skel.c
		${CMAKE_CURRENT_BINARY_DIR}/${fileName}_stub.c
		${CMAKE_CURRENT_BINARY_DIR}/${fileName}.h
		PROPERTIES
		GENERATED TRUE
		)
endfunction()


####################################
# Core part of the Hexagon SDK Tree data base for library name to library
# source and include file mapping
####################################

if(CMAKE_HOST_WIN32)
	set(hostOS WinNT)
else()
	set(hostOS Ubuntu14)
endif()

if(NOT DEFINED TREE)
	set(TREE SDKTREE)
endif()

message(STATUS "TREE:${TREE}")

if( ${TREE} MATCHES PAKMAN )
	if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
		include(glue.cmake.android)
	else()
		include(glue.cmake)
	endif()
else()
	#set(HEXAGON_SDK_ROOT $ENV{HEXAGON_SDK_ROOT})
	message(STATUS "HEXAGON_SDK_ROOT:${HEXAGON_SDK_ROOT}")
	set(rpcmem_src ${HEXAGON_SDK_ROOT}/libs/common/rpcmem) 
	set(rpcmem_inc ${HEXAGON_SDK_ROOT}/libs/common/rpcmem/inc)
	set(atomic_src ${HEXAGON_SDK_ROOT}/libs/common/atomic) 
	set(atomic_inc ${atomic_src}/${V}/ship)
	set(remote_src ${HEXAGON_SDK_ROOT}/libs/common/remote) 
	set(remote_inc ${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/) 
	set(libdspCV_skel_src ${HEXAGON_SDK_ROOT}/libs/fastcv/dspCV) 
	set(libdspCV_skel_inc
		${HEXAGON_SDK_ROOT}/libs/fastcv/dspCV/inc
		${HEXAGON_SDK_ROOT}/libs/fastcv/dspCV/${V}/ship/
		) 
	set(libdspCV_src ${HEXAGON_SDK_ROOT}/libs/fastcv/dspCV) 
	set(libdspCV_inc 
		${HEXAGON_SDK_ROOT}/libs/fastcv/dspCV/inc
		${HEXAGON_SDK_ROOT}/libs/fastcv/dspCV/${V}/ship/
		) 
	set(libqdebug_src ${HEXAGON_SDK_ROOT}/libs/qdebug) 
	set(libqdebug_inc 
		${HEXAGON_SDK_ROOT}/libs/qdebug/inc
		) 
	set(test_util_src ${HEXAGON_SDK_ROOT}/test/common/test_util) 
	set(test_util_inc) 
	set(proj-qaic_src ${HEXAGON_SDK_ROOT}/tools/qaic/)

	set(HEXAGON_ALL_INCS
		${HEXAGON_SDK_ROOT}/incs/
		${HEXAGON_SDK_ROOT}/incs/stddef/
		${HEXAGON_SDK_ROOT}/libs/common/rpcmem/inc
		${HEXAGON_SDK_ROOT}/libs/common/remote/ship/${V}/
		)
endif() 

# vim: set noet fenc=utf-8 ff=unix ft=cmake :
