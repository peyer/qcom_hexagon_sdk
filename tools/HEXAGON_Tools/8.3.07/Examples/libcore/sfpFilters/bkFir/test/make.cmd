::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Windows Make.cmd to build the example
::

@echo off

@set Q6VERSION=v60
@set TESTFILE=bkfir_c

@set CXX=hexagon-clang
@set SIM=hexagon-sim

@set CXXFLAGS=-m%Q6VERSION%

@set SIMFLAGS=--timing --m%Q6VERSION% %TESTFILE%.elf
@set SRCS=..\asm_src\flt_bkfir.S ..\src\%TESTFILE%.c test_bkfir.c
@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_stats* @DEL pmu_stats*
@IF EXIST *.o @DEL *.o
@IF "%1"=="clean" GOTO End

::
:: Build the test case
::
:Build
@echo.
@echo Compiling test case ...
@echo %CXX% %CXXFLAGS% %SRCS% -I..\include -o %TESTFILE%.elf
%CXX% %CXXFLAGS% -I..\include %SRCS% -o %TESTFILE%.elf -lhexagon
@IF "%1"=="build" GOTO End

::
:: Simulate the example
::
:Sim
@echo.
@echo Simulating example
%SIM% %SIMFLAGS%

::
:End
::
