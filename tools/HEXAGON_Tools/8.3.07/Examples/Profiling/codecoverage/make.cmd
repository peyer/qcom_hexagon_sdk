::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build the CodeCoverage example
::

::@echo off
@set CC=hexagon-clang
@set SIM=hexagon-sim
@set Q6VERSION=v60
@set TESTFILE_NAME=mandelbrot
@set CFLAGS=-m%Q6VERSION% -O2 -g
@set SIMFLAGS=--timing -m%Q6VERSION% %TESTFILE_NAME%.elf

for /f "delims=" %%a in ('cd') do @set SRC_TOP=%%a
@set BIN_DIR=%SRC_TOP%\bin
@set SRC_DIR=%SRC_TOP%\Source

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim
@IF "%1"=="merge" GOTO Merge
@IF "%1"=="coverage" GOTO Coverage
@IF "%1"=="dump" GOTO Dump

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_stats* @DEL pmu_stats*
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST .\report @RMDIR /S /Q .\report
@IF EXIST gmon* @DEL gmon*
@IF EXIST *.dump @DEL *.dump
@IF EXIST *.elf @DEL *.elf
@IF EXIST *.o @DEL *.o
@IF "%1"=="clean" GOTO End

::
:: Build the bus cosim
::
:Build
@echo.
@echo Compiling mandelbrot...
%CC% %CFLAGS% %TESTFILE_NAME%.c -o %TESTFILE_NAME%.elf -lhexagon
@IF "%1"=="build" GOTO End

::
:: Simulate the example with profiling output
::
:Sim
@echo.
@echo Simulating example
%SIM% --profile %SIMFLAGS%
@IF "%1"=="sim" GOTO End

::
:: merge gmon files
::
:Merge
@IF EXIST gmon.merged @DEL gmon.merged
hexagon-gmon-merge -o gmon.merged gmon.t_0 gmon.t_1 gmon.t_2
@IF "%1"=="merge" GOTO End

::
:: run the code coverate tool
::
:Coverage
hexagon-coverage -i %TESTFILE_NAME%.elf --html report gmon.merged

::
:End
@Exit /B 0

::
:: run the code coverage tool
::
:Dump
hexagon-gmon-dump gmon.merged > dumpgmon.txt
GOTO End
