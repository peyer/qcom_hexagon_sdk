::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build the example
:: Please note that this example assumes you have Microsoft Visual Studio 14.
:: A 64-bit dll will be created as part of this example.
::

@echo off
@set HEXAGON_TOOLS=
@set INCLUDE=
@set LIB=
@set ERRORLEVEL=
::################################################
:: Make sure that `where` command only picks up ##
:: first path it finds, in case there are more  ##
:: than one version tools path in PATH variable ##
::################################################
@for /f "delims=" %%a in ('where hexagon-sim') do (
    @set HEXAGON_TOOLS=%%a
    @goto :done
)
:done
@set HEXAGON_TOOLS=%HEXAGON_TOOLS%\..

for /f "delims=" %%a in ('cd') do @set CURRENT_DIR=%%a
@set SRC_TOP=%CURRENT_DIR%\..

@set CC=hexagon-clang
@set SIM=hexagon-sim
@set Q6VERSION=v60
@set TESTFILE_NAME=hello
@set CFLAGS=-m%Q6VERSION% -g

@set BIN_DIR=%SRC_TOP%\bin
@set TEST_DIR=%SRC_TOP%\test

@set SIMFLAGS=--timing --bypass_idle --cosim_file newcosimCfg --m%Q6VERSION% %BIN_DIR%\%TESTFILE_NAME%

:: Microsoft compiler to use to build cosim
@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set COSIM_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set COSIM_CC=%VCTOOLS%\bin\amd64\cl.exe
@set COSIM_NAME=FunctionProfiler
@set COSIM_CFLAGS=/MT /O2 /nologo /Zm1000 /EHsc  -DVCPP -DLITTLE_ENDIAN /TP /I. /DCYGPC /I%HEXAGON_TOOLS%\..\include\iss\ /I%SRC_TOP%\src
@set COSIM_LINK=/nologo /link /dll libwrapper.lib /MACHINE:X64 /libpath:%HEXAGON_TOOLS%\..\lib\iss

@set LINKFLAGS=-nologo -dll libwrapper.lib
@set LIBPATH=%HEXAGON_TOOLS%\..\lib\iss\

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
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST newcosimCfg @DEL newcosimCfg
@IF EXIST ..\bin @RMDIR /S /Q ..\bin
@IF EXIST *.dat @DEL *.dat
@IF "%1"=="clean" GOTO End

::
:: Build the bus cosim
::
:Build
@echo.
@echo Compiling cosim ...
@MKDIR ..\bin
@set PATH_TEMP=%PATH%
call %COSIM_INIT%
%COSIM_CC% %COSIM_CFLAGS%  -c %COSIM_NAME%.c /Fo%BIN_DIR%\%COSIM_NAME%.obj
%COSIM_CC% %BIN_DIR%\%COSIM_NAME%.obj %COSIM_LINK% /out:%BIN_DIR%\%COSIM_NAME%.dll
@set PATH=%PATH_TEMP%

::
:: Copy over ..\test\cosimCfg and change .so to .dll
::
::
@echo off

@setlocal
FOR /f "delims=" %%x IN (%TEST_DIR%\cosimCfg) DO (
	@set  concat=%BIN_DIR%\%%x
	@setlocal enabledelayedexpansion
	@set concat=!concat:\ =\!
	@echo !concat:so=dll! >> newcosimCfg
)

::
:: Build test case
::
cd ..\test
@echo %CC% %CFLAGS% -o %BIN_DIR%\%TESTFILE_NAME% %TEST_DIR%\%TESTFILE_NAME%.c
%CC% %CFLAGS% -o %BIN_DIR%\%TESTFILE_NAME% %TEST_DIR%\%TESTFILE_NAME%.c
cd ..\src

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
