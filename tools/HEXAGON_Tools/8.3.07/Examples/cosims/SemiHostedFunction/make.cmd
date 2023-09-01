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
@set Q6VERSION=
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
@set Q6VERSION=v60
@set HEXAGON_TOOLS=%HEXAGON_TOOLS%\..\..
@set COSIM_NAME=SHFcosim
@set TESTFILE_NAME=shf

@set INCLUDE_DIR=%HEXAGON_TOOLS%\include\iss
@set HEX_LIB_PATH=%HEXAGON_TOOLS%\lib\iss
@for /f "delims=" %%a in ('chdir') do @set LOCAL_DIR=%%a
@IF NOT EXIST %LOCAL_DIR%\bin mkdir %LOCAL_DIR%\bin
@set BIN_DIR=%LOCAL_DIR%\bin
@set TEST_DIR=%LOCAL_DIR%\Test_program
@set COMMON_DIR=%LOCAL_DIR%\common
@set COSIM_DIR=%LOCAL_DIR%\cosim


:: Microsoft compiler to use to build cosim
@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set COSIM_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set COSIM_CFLAGS=/MT /O2 /nologo /Zm1000 /EHsc  -DVCPP -DLITTLE_ENDIAN /TP /I. /DCYGPC /I%HEXAGON_TOOLS%\include\iss\ /I%COMMON_DIR%
@set COSIM_CC=%VCTOOLS%\bin\amd64\cl.exe
@set COSIM_LINK=/nologo /link /dll libwrapper.lib /MACHINE:X64  /libpath:%HEXAGON_TOOLS%\lib\iss

:: clean
@echo.
@echo Cleaning files...
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST %BIN_DIR%\cosim.cfg @DEL %BIN_DIR%\cosim.cfg
@IF EXIST %BIN_DIR%\%COSIM_NAME%.dll @DEL %BIN_DIR%\%COSIM_NAME%.dll
@IF EXIST %BIN_DIR%\%TESTFILE_NAME%_test @DEL %BIN_DIR%\%TESTFILE_NAME%_test
@IF EXIST %LOCAL_DIR%\stats.txt @DEL %LOCAL_DIR%\stats.txt
@IF EXIST %LOCAL_DIR%\pmu_stats*.txt @DEL %LOCAL_DIR%\pmu_stats*.txt
@IF EXIST %BIN_DIR% RMDIR /s /q %BIN_DIR%
@IF "%1"=="clean" GOTO End

:: build cosim
@echo.
@echo Compiling cosim...
@IF NOT EXIST %BIN_DIR% MKDIR %BIN_DIR%
@set PATH_TEMP=%PATH%
call %COSIM_INIT%
%COSIM_CC% %COSIM_CFLAGS% -c %COSIM_DIR%\%COSIM_NAME%.cpp /I%INCLUDE_DIR% /Fo%BIN_DIR%\%COSIM_NAME%.obj
%COSIM_CC% %BIN_DIR%\%COSIM_NAME%.obj %COSIM_LINK% /out:%BIN_DIR%\%COSIM_NAME%.dll
@set PATH=%PATH_TEMP%

:: delete unneeded files
@IF EXIST %COSIM_NAME%.obj @DEL %COSIM_NAME%.obj
@IF EXIST %BIN_DIR%\%COSIM_NAME%.lib @DEL %BIN_DIR%\%COSIM_NAME%.lib
@IF EXIST %BIN_DIR%\%COSIM_NAME%.exp @DEL %BIN_DIR%\%COSIM_NAME%.exp

:: build test file
@echo.
@echo Compiling test file...
hexagon-clang -m%Q6VERSION% %TEST_DIR%\%TESTFILE_NAME%.c -I.\common -o %BIN_DIR%\%TESTFILE_NAME%_test

:: create cosim configuration file
@echo.
@echo Creating cosim.cfg file...
@echo %BIN_DIR%\%COSIM_NAME%.dll > %BIN_DIR%\cosim.cfg

:: run the test
@echo.
@echo Running the test...
hexagon-sim --cosim_file %BIN_DIR%\cosim.cfg --timing %BIN_DIR%\%TESTFILE_NAME%_test

:End

