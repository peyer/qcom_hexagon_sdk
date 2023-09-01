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
@set CFLAGS=-m%Q6VERSION% -O2 -Wl,--force-dynamic -Wl,-E -ldl -G0
@set CC=hexagon-clang
@set SIM=hexagon-sim
@set MAIN=dmain
@set LIB=mylib1
@set BIN_DIR=%SRC_TOP%\bin
@set SRC_DIR=%SRC_TOP%\src
@set COMMON_DIR=%SRC_TOP%\..\common
@set ELF=%BIN_DIR%\%MAIN%.elf
@set SHAREDLIB=%BIN_DIR%/%LIB%.so
@set LINKFLAGS=-fPIC -shared -nostartfiles
@set SCRIPT=-Wl,-T,./src/dlink.script
@set SIMFLAGS=--timing --m%Q6VERSION% %ELF%

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="script" GOTO Script
@IF "%1"=="shared" GOTO Shared
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
:: Default - Build the test case with no linker script
::
:Build
@echo.
@echo Compiling test case ...
@IF NOT EXIST %BIN_DIR% @MKDIR %BIN_DIR%
%CC% %SRC_DIR%/%MAIN%.c -o %ELF% %CFLAGS%
%CC% %COMMON_DIR%/%LIB%.c -o %SHAREDLIB% %CFLAGS% %LINKFLAGS%
@IF "%1"=="build" GOTO End
GOTO Read

::
:: Default - Build the test case with linker script fir shared lib
::
:Script
@echo.
@echo Compiling test case ...
@IF NOT EXIST %BIN_DIR% @MKDIR %BIN_DIR%
%CC% %SRC_DIR%/%MAIN%.c -o %ELF% %CFLAGS% %(SCRIPT)%
%CC% %COMMON_DIR%/%LIB%.c -o %SHAREDLIB% %CFLAGS% %LINKFLAGS%
@IF "%1"=="build" GOTO End
GOTO Read

::
:: Build the test case with linker script for main and shared lib
::
:Shared
@echo.
@echo Compiling test case ...
@IF NOT EXIST %BIN_DIR% @MKDIR %BIN_DIR%
%CC% %SRC_DIR%/%MAIN%.c -o %ELF% %CFLAGS% %SCRIPT%
%CC% %COMMON_DIR%/%LIB%.c -o %SHAREDLIB% %CFLAGS% %LINKFLAGS% %SHARED%
@IF "%1"=="build" GOTO End


::
:: Demonstrate map of example
::
:Read
@echo.
hexagon-readelf -e %ELF%
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
