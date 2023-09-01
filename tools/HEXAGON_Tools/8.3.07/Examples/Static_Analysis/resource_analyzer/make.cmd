::################################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd script to build/simulate and/or analyze the example
::

:: Compiler to use to build the example
@set CC=hexagon-clang
@set Q6VERSION=v60
@set CFLAGS=-O2 -g -m%Q6VERSION%
@set SIM=hexagon-sim
@set PROGRAM=mandelbrot

:: Find full path to the example
for /f "delims=" %%a in ('cd') do @set SRC=%%a
@set SRC_TOP=%SRC%

@set OBJ_DIR=%SRC_TOP%\bin
@set SRC_DIR=%SRC_TOP%\src
@set ELF=%OBJ_DIR%\%PROGRAM%.elf

@set SIMFLAGS=-m%Q6VERSION% --timing

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="analyze" GOTO Analyze

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_statsfile.txt @DEL pmu_statsfile.txt
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST .\bin @RMDIR /S /Q .\bin
@IF EXIST .\RA @RMDIR /S /Q .\RA
@IF "%1"=="clean" GOTO End

::
:: Build the Example
::
:Build
@echo.
@echo Compiling the Example ...
@MKDIR .\bin .\RA
"%CC%" %CFLAGS% %SRC_DIR%\%PROGRAM%.c -o %ELF%
@IF "%1"=="build" GOTO End

::
:: Simulate the example
::
:Analyze
@echo.
hexagon-analyzer-backend --dsp %Q6VERSION% -o RA --elffile %ELF%
%SIM% %SIMFLAGS% %ELF%
GOTO End

::
:End
::
