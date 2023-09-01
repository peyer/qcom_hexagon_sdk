::
:: Make.cmd to build the streamer/memcpy_test files
::
:: Get Hexagon tools path

@set TEST=memcpy_v60
@set Q6VERSION=v60

@set IMG_BASE=0x40000000
@set TCM_BASE=0xd8000000

::#################################################
@set STREAMER_BASE=0xd0000000
::# WARNING! Changing the streamer base requires ##
::# a corresponding change in the cosim.cfg file.##
::#################################################

@set EXEC=%TEST%
@set ASM_SRC_DIR=..\asm_src
@set INC_DIR=..\include
@set OBJ_DIR=..\build
@set IN_DIR=..\input
@set OUT_DIR=..\output

@set INPUT=%IN_DIR%\bayer_3k_x_2k.in
@set OUTPUT=%OUT_DIR%\bayer.out
@set SIZE=3072 2048

@set C_SRC=test_memcpy
@set ASM_SRC=hvx_copy_line

@set CC=hexagon-clang
@set LD=%CC%
@set AS=%CC%

@set ASM_FLAGS=-m%Q6VERSION%
@set CFLAGS=-Wall -mhvx -mhvx-length=128B -O3 -m%Q6VERSION% -I%INC_DIR% -DTCM_BASE=%TCM_BASE%
@set CFLAGS=%CFLAGS% -DSTREAMER_BASE=%STREAMER_BASE% -c -o
@set LDFLAGS=-m%Q6VERSION% -Wl,--section-start=.start=%IMG_BASE%
@set LDFLAGS=%LDFLAGS%,--defsym=HEAP_SIZE=0x3000,--defsym=STACK_SIZE=0x4000
@set LDFLAGS=%LDFLAGS%,--defsym=L2_CACHE_SIZE=1,--defsym=ISDB_SECURE_FLAG=2
@set LDFLAGS=%LDFLAGS%,--defsym=ISDB_TRUSTED_FLAG=2,--defsym=ISDB_DEBUG_FLAG=2

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build

:: clean
:Clean
@echo.
@echo Cleaning files...
@IF EXIST ..\build @RMDIR /S /Q ..\build
@IF EXIST ..\output @RMDIR /S /Q ..\output
@MKDIR %OUT_DIR%
@MKDIR %OBJ_DIR%
@IF "%1"=="clean" GOTO End

:: build
:Build
@echo.
@echo Compiling test/memcpy_test ...
%CC% %CFLAGS% %OBJ_DIR%\%C_SRC%.o %C_SRC%.c
%CC% %CFLAGS% %OBJ_DIR%\%ASM_SRC%.obj %ASM_SRC_DIR%\%ASM_SRC%.S
%CC% %LDFLAGS% -o %OBJ_DIR%\%TEST% %OBJ_DIR%\%C_SRC%.o %OBJ_DIR%\%ASM_SRC%.obj
@IF "%1"=="build" GOTO End


:End
