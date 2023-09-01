::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build and simulate the streamer/src example
::
@set TEST=vdelta_helper

@set CC=hexagon-clang
@set SIM=hexagon-sim
@set Q6VERSION=v60

@set REQ_LDFLAGS=-m%Q6VERSION% -Wl,--section-start=.start=0x01000000
@set C_FLAGS=-Wall -O3 -m%Q6VERSION% -I..\include -c
@echo off
for /f "delims=" %%a in ('where hexagon-sim') do @set RELEASE_DIR=%%a
for /f "delims=" %%a in ('dirname %RELEASE_DIR%') do @set RELEASE_DIR=%%a
@set %RELEASE_DIR%=%RELEASE_DIR%\..
@set INCLUDE_DIR=%RELEASE_DIR%\..\include\

for /f "delims=" %%a in ('cd') do @set SRC=%%a
@set SRC_TOP=%SRC%\..
@set BUILD_DIR=%SRC_TOP%\build
@set SRC_DIR=%SRC_TOP%\src

@set SIMFLAGS=--timing --simulated_returnval --m%Q6VERSION% %TEST%.elf

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST .\%TEST%.elf @DEL /S /Q .\%TEST%.elf
@IF EXIST pmu_statsfile.txt @DEL /S /Q pmu_statsfile.txt
@IF EXIST ..\build @RMDIR /S /Q %BUILD_DIR%
@IF "%1"=="clean" GOTO End

::
:: Build test case
::
@echo.
:Build
@echo Building files...
@MKDIR %BUILD_DIR%
%CC% %C_FLAGS% -o %BUILD_DIR%\vdelta_helper_c.o %SRC_DIR%\vdelta_helper_c.c
%CC% %C_FLAGS% -o %BUILD_DIR%\vdelta_helper_d.o %SRC_DIR%\vdelta_helper_d.c
%CC% %REQ_LDFLAGS% -o %TEST%.elf %BUILD_DIR%\vdelta_helper_c.o %BUILD_DIR%\vdelta_helper_d.o -lhexagon
@IF "%1"=="build" GOTO End

::
:: Simulate test case
::
:Sim
@echo.
@echo Simulating vdelta_helper example
%SIM% %SIMFLAGS%
::
:End
::
