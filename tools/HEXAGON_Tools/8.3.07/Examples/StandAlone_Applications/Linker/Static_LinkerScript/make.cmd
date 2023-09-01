::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build the example
::

::################################################
:: Make sure that `where` command only picks up ##
:: first path it finds, in case there are more  ##
:: than one version tools path in PATH variable ##
::################################################

@echo off
@for /f "delims=" %%a in ('where hexagon-sim') do (
    @set RELEASE_DIR=%%a
    @goto :done
)

:done
::################################################
:: Truncate hexagon-sim.exe from string         ##
::################################################
@for %%F in (%RELEASE_DIR%) do @set dirname=%%~dpF
@set RELEASE_DIR=%dirname%..

:: Get Current working directory
for /f "delims=" %%a in ('cd') do @set SRC_TOP=%%a

@set Q6VERSION=v60

@set CFLAGS=-m%Q6VERSION% -O2 -G0 -ffunction-sections -fdata-sections
@set CC=hexagon-clang %CFLAGS%
@set SIM=hexagon-sim

@set MAIN=smain
@set LIB1=mylib1
@set INCLUDE_DIR=%RELEASE_DIR%\include\iss
@set BIN_DIR=%SRC_TOP%\bin
@set SRC_DIR=%SRC_TOP%\src
@set COMMON_DIR=%SRC_TOP%\..\common

@set SIMFLAGS=--timing --m%Q6VERSION% %BIN_DIR%\%MAIN%.elf

:: Set Linkflag for no script and test if one is requested in either arg1 or arg2
@set LINKFLAGS=-Wl,--section-start,.start=0xdf0000,--gc-sections
@IF "%1"=="script" set LINKFLAGS=-Wl,--section-start,.start=0xdf0000,-T%SRC_DIR%/slink.script,-Map,%BIN_DIR%/link.map,--gc-sections
@IF "%2"=="script" set LINKFLAGS=-Wl,--section-start,.start=0xdf0000,-T%SRC_DIR%/slink.script,-Map,%BIN_DIR%/link.map,--gc-sections

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="read" GOTO Read
@IF "%1"=="sim" GOTO Sim

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_stats* @DEL pmu_stats*
@IF EXIST *.o @DEL *.o
@IF EXIST *.map @DEL *.map
@IF EXIST %BIN_DIR% @RMDIR /S /Q %BIN_DIR%
@IF "%1"=="clean" GOTO End

::
:: Build the test case
::
:Build
@echo.
@echo Compiling test case ...
@IF NOT EXIST %BIN_DIR% @MKDIR %BIN_DIR%
	%CC% %CFLAGS% %SRC_DIR%\%MAIN%.c %COMMON_DIR%\%LIB1%.c -o %BIN_DIR%\%MAIN%.elf %LINKFLAGS%
@IF "%1"=="build" GOTO End

::
:: Demonstrate map of example
::
:Read
@echo.
hexagon-readelf -e %BIN_DIR%\%MAIN%.elf
@IF "%1"=="read" GOTO End

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