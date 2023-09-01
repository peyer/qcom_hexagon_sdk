::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd script to build/simulate and/or debug the TCM example
::

:: Compiler to use to build TCM example
@set CC=hexagon-clang
@set Q6VERSION=v60
@set CFLAGS=-O2 -g -G0 -m%Q6VERSION%
@set SIM=hexagon-sim

:: set tcm size to 128k
@set TCM_SIZE=0x20000
@set STACK_SIZE=0x4000
@set HEAP_SIZE=0x10000
@set TCM_BASE_ADDR=0xd8000000

:: Find full path to TCM example
for /f "delims=" %%a in ('cd') do @set SRC=%%a
@set SRC_TOP=%SRC%

@set OBJ_DIR=%SRC_TOP%\bin
@set SRC_DIR=%SRC_TOP%\src
@set ELF=%OBJ_DIR%\main

@set SIMFLAGS=-m%Q6VERSION% --timing

@set LINKFLAGS=-Wl,--section-start,.start=%TCM_BASE_ADDR%,
@set LINKFLAGS=%LINKFLAGS% -Wl,--defsym,TCM_BASE_ADDR=%TCM_BASE_ADDR%,
@set LINKFLAGS=%LINKFLAGS% -Wl,--defsym,L2_CACHE_SIZE=1,
@set LINKFLAGS=%LINKFLAGS% -Wl,--defsym,HEAP_SIZE=%HEAP_SIZE%,
@set LINKFLAGS=%LINKFLAGS% -Wl,--defsym,STACK_SIZE=%STACK_SIZE%

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim
@IF "%1"=="t32" GOTO T32

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_statsfile.txt @DEL pmu_statsfile.txt
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST .\bin @RMDIR /S /Q .\bin
@IF "%1"=="clean" GOTO End

::
:: Build the TCM Example
::
:Build
@echo.
@echo Compiling TCM Example ...
@MKDIR .\bin
"%CC%" %CFLAGS% -c %SRC_DIR%\main.c -o %OBJ_DIR%\main.o
"%CC%" %CFLAGS% -c %SRC_DIR%\tlb.c -o %OBJ_DIR%\tlb.o
"%CC%" %CFLAGS% %OBJ_DIR%\main.o %OBJ_DIR%\tlb.o -o %ELF% %LINKFLAGS%
@IF "%1"=="build" GOTO End

::
:: Simulate the TCM example
::
:Sim
@echo.
%SIM% %SIMFLAGS% %ELF%
GOTO End

::
:: Debug the TCM Example using Trace32 debugger
::
:T32
@set T32SYS=%T32SYS%
%T32SYS%/t32mqdsp6.exe -c cmm/win.cfg -s cmm/hexagon.cmm %ELF% %Q6VERSION%

::
:End
::
