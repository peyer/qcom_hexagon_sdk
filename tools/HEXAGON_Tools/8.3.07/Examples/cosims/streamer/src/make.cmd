::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build and simulate the streamer/src example
::

@set SIM=hexagon-sim
@set Q6VERSION=v60
@set COSIMNAME=Streamer
@set TEST=memcpy_v60
@set TCM_BASE=0xd8000000
::################################################
@set STREAMER_BASE=0xd0000000
:: WARNING! Changing the streamer base requires ##
:: a corresponding change in the cosim.cfg file.##
::################################################
@set INPUT=bayer_3k_x_2k.in
@set OUTPUT=bayer.out
@set SIZE=3072 2048
::################################################
:: Make sure that `where` command only picks up ##
:: first path it finds, in case there are more  ##
:: than one version tools path in PATH variable ##
::################################################
@echo .
@for /f "delims=" %%a in ('where hexagon-sim') do (
    @set HEXAGON_TOOLS=%%a
    @goto :done
)

:done
::################################################
:: Truncate hexagon-sim.exe from string         ##
::################################################

@for %%F in (%HEXAGON_TOOLS%) do @set dirname=%%~dpF
@set HEXAGON_TOOLS=%dirname%
@set INCLUDE_DIR=%HEXAGON_TOOLS%\..\include\iss

::################################################
:: NOTE: MAKE_ARGS variable is only used during ##
:: new tools release testing and is null for the##
:: compiling the streamer example.              ##
::################################################
@set ARCH=
@set DLLIB=
@set O_EXT=obj
@set O_OUTPUT=-Fo

for /f "delims=" %%a in ('cd') do @set SRC=%%a
@set SRC_TOP=%SRC%\..
@set BUILDDIR=%SRC_TOP%\build\WIN

@set BINDIR=%SRC_TOP%\bin\WIN
@set TESTDIR=%SRC_TOP%\test\memcpy_test
@set INPUT_DIR=%SRC_TOP%\input
@set OUTPUT_DIR=%SRC_TOP%\output

@set SIMFLAGS=--simulated_returnval --cosim_file cosim.cfg -m%Q6VERSION%
@set SIMFLAGS=%SIMFLAGS% --timing --tcm:lowaddr %TCM_BASE% %BINDIR%\%TEST%
@set SIMFLAGS=%SIMFLAGS% -- %SIZE% %INPUT_DIR%\%INPUT% %OUTPUT_DIR%\%OUTPUT%

:: Microsoft compiler to use to build cosim
@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set COSIM_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set COSIM_CC=%VCTOOLS%\bin\amd64\cl.exe
@set COSIM_CFLAGS=/nologo /Zm1000 /EHsc /TP /I. /I%INCLUDE_DIR%
@set COSIM_LINK=/nologo /link /dll libwrapper.lib /MACHINE:X64  /libpath:%HEXAGON_TOOLS%\..\lib\iss

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_statsfile.txt @DEL pmu_statsfile.txt
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST cosim.cfg @DEL cosim.cfg
@IF EXIST ..\Bin @RMDIR /S /Q ..\Bin
@IF EXIST ..\build @RMDIR /S /Q ..\build
@IF EXIST ..\input @RMDIR /S /Q ..\input
@IF EXIST ..\output @RMDIR /S /Q ..\output
cd %TESTDIR%
call make.cmd clean
cd %SRC%
@IF "%1"=="clean" GOTO End

::
:: Build the cosim and test case
::
:Build
@echo.
@echo Compiling streamer cosim ...
@MKDIR %BINDIR%
@MKDIR %BUILDDIR%
@set PATH_TEMP=%PATH%
call %COSIM_INIT%
%COSIM_CC% %COSIM_CFLAGS% -c %COSIMNAME%.c -Fo%BUILDDIR%\%COSIMNAME%.obj
%COSIM_CC% %BUILDDIR%\%COSIMNAME%.obj %COSIM_LINK% -out:%BINDIR%\%COSIMNAME%.dll
@set PATH=%PATH_TEMP%

@echo Compiling test\memcpy_test ...
cd %TESTDIR%
call make.cmd build
cd %SRC%

@echo Copying input file and cosim.cfg file ...
copy %TESTDIR%\..\build\%TEST% %BINDIR%\%TEST%
mkdir %INPUT_DIR%
mkdir %OUTPUT_DIR%
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%TESTDIR%\..\input\bayer_3k_x_2k.zip', '%INPUT_DIR%'); }"

@echo off
@for /f "tokens=*" %%i in (%TESTDIR%\cosim.cfg) do (
	@set string=%%i
	@setlocal enabledelayedexpansion
	@echo %BINDIR%\!string:so=dll! >> cosim.cfg
)
@IF "%1"=="build" GOTO End

::
:: Simulate the example
::
:Sim
@echo.
@echo Simulating Streamer example, this may take up to 2 minutes ...
%SIM% %SIMFLAGS%
::
:End
::
