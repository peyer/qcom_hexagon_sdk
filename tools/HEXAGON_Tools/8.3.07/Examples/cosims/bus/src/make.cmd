::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build the example
:: Please note that this example requires Microsoft Visual Studio 15.
::

@echo off
@set HEXAGON_TOOLS=
@set INCLUDE=
@set LIB=
@set COSIM_CC=

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
@set HEXAGON_TOOLS=%HEXAGON_TOOLS%\..\..

for /f "delims=" %%a in ('cd') do @set CURRENT_DIR=%%a
@set SRC_TOP=%CURRENT_DIR%\..

@set CC=hexagon-clang
@set SIM=hexagon-sim
@set Q6VERSION=v60
@set COSIM_NAME=Bus
@set TESTFILE_NAME=hello
@set CFLAGS=-m%Q6VERSION%

@set INCLUDE_DIR=%HEXAGON_TOOLS%\include\iss
@set BIN_DIR=%SRC_TOP%\Bin
@set TEST_DIR=%SRC_TOP%\test
@set SIMFLAGS=--timing --cosim_file newcosimCfg --m%Q6VERSION% %BIN_DIR%\%TESTFILE_NAME%

:: Microsoft compiler to use to build cosim
@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set COSIM_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set COSIM_CC=%VCTOOLS%\bin\amd64\cl.exe
@set COSIM_CFLAGS=/MT /O2 /nologo /Zm1000 /EHsc  -DVCPP -DLITTLE_ENDIAN /TP /I. /DCYGPC /I%HEXAGON_TOOLS%\include\iss\ /I%CURRENT_DIR%\..\src
@set COSIM_LINK=/nologo /link /dll libwrapper.lib /MACHINE:X64 /libpath:%HEXAGON_TOOLS%\lib\iss

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
@IF EXIST %BIN_DIR% @RMDIR /S /Q %BIN_DIR%
@IF "%1"=="clean" GOTO End

::
:: Build the bus cosim
::
:Build
@echo.
@echo Compiling cosim ...
@MKDIR %BIN_DIR%
@set PATH_TEMP=%PATH%
call %COSIM_INIT%
%COSIM_CC% %COSIM_CFLAGS%  -c %COSIM_NAME%.c /Fo%BIN_DIR%\%COSIM_NAME%.obj
%COSIM_CC% %BIN_DIR%\%COSIM_NAME%.obj %COSIM_LINK% /out:%BIN_DIR%\%COSIM_NAME%.dll
@set PATH=%PATH_TEMP%

::
:: Copy over ..\test\cosimCfg and change .so to .dll
::
::@echo off

@setlocal
(SET var=)
FOR /f "delims=" %%x IN (%TEST_DIR%\cosimCfg) DO (
	CALL SET var=%%var%% %%x
)
@set  concat=%BIN_DIR%\%var%
@setlocal enabledelayedexpansion
@set concat=!concat:\ =\!
@echo !concat:so=dll! >> newcosimCfg

::
:: Build test case
::
cd ..\test
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
