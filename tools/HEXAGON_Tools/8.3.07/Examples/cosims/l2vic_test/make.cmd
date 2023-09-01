::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build the example
:: Please note that this example requires Microsoft Visual Studio 15.
::

::@echo off
@set HEXAGON_TOOLS=
@set INCLUDE=
@set LIB=
@set COSIM_CC=

::################################################
:: Make sure that `where` command only picks up ##
:: first path it finds, in case there are more  ##
:: than one version tools path in PATH variable ##
::################################################
@echo off
@for /f "delims=" %%a in ('where hexagon-sim') do (
    @set HEXAGON_TOOLS=%%a
    @goto :done
)
:done

::################################################
:: Default setup for 7.x and 8.0.x
::################################################
@set Q6VERSION=v60
@set INTERRUPT=31
@set HEXAGON_TOOLS=%HEXAGON_TOOLS%\..\..
::################################################
:: Get version hi and mid values, ex. 8.0.08
:: Assumption is folder path is
:: C:\Qualcomm\HEXAGON_Tools\version
::################################################
@set VERSION_HI=%HEXAGON_TOOLS:~26,1%
@set VERSION_MID=%HEXAGON_TOOLS:~28,1%
::################################################
:: if version is 8.1 or higher, set for v65
::################################################
@echo off
if %VERSION_HI% EQU 8 (
	if %VERSION_MID% NEQ 0 (
		@set Q6VERSION=v65
		@set INTERRUPT=2
	)
	if %VERSION_MID% GTR 1 (
		@set Q6VERSION=v66
		@set INTERRUPT=2))

for /f "delims=" %%a in ('cd') do @set SRC_TOP=%%a

@set CC=hexagon-clang
@set SIM=hexagon-sim
@set COSIM_NAME=l2vic_test_cosim
@set TESTFILE_NAME=mandelbrot
@set CFLAGS=-m%Q6VERSION%

@set INCLUDE_DIR=%HEXAGON_TOOLS%\include\iss
@set BIN_DIR=%SRC_TOP%\bin
@set SRC_DIR=%SRC_TOP%\Source

@set SIMFLAGS=--timing --cosim_file ./Source/cosims_win.txt --m%Q6VERSION% %BIN_DIR%\%TESTFILE_NAME%.elf

:: Compiler to use to build cosim
@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set COSIM_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set COSIM_CC=%VCTOOLS%\bin\amd64\cl.exe
@set COSIM_CFLAGS=/MT /O2 /nologo /Zm1000 /EHsc  -DVCPP -DLITTLE_ENDIAN /TP /I. /DCYGPC /I%HEXAGON_TOOLS%\include\iss\ /I%SRC_DIR%
@set COSIM_LINK=/nologo /link /dll libwrapper.lib /MACHINE:X64  /libpath:%HEXAGON_TOOLS%\lib\iss

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
@IF EXIST .\bin @RMDIR /S /Q .\bin
@IF "%1"=="clean" GOTO End

::
:: Build the bus cosim
::
:Build
@echo.
@echo Compiling cosim ...
@MKDIR .\bin
@set PATH_TEMP=%PATH%
call %COSIM_INIT%
%COSIM_CC% %COSIM_CFLAGS% -c %SRC_DIR%\%COSIM_NAME%.cpp /I%INCLUDE_DIR% /Fo%BIN_DIR%\%COSIM_NAME%.obj
%COSIM_CC% %BIN_DIR%\%COSIM_NAME%.obj %COSIM_LINK% /out:%BIN_DIR%\%COSIM_NAME%.dll
@set PATH=%PATH_TEMP%

@IF EXIST %BIN_DIR%\%COSIM_NAME%.obj @DEL %BIN_DIR%\%COSIM_NAME%.obj
%CC% %CFLAGS% %SRC_DIR%\%TESTFILE_NAME%.c -o %BIN_DIR%\%TESTFILE_NAME%.elf -lhexagon -DJUST_WAIT

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
