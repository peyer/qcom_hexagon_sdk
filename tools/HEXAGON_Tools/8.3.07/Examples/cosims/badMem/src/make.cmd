::
:: Make.cmd to build the example
:: Please note that this example assumes you have Microsoft Visual Studio 10.
:: A 64-bit dll will be created as part of this example.
::

:: Get Hexagon tools path
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

for /f "delims=" %%a in ('cd') do @set CURRENT_DIR=%%a

@set TESTFILE_NAME=badMem
@set Q6VERSION=v60
@set CC=hexagon-clang
@set CFLAGS=-m%Q6VERSION% -g
@set SIM=hexagon-sim
@set SIMFLAGS=-m%Q6VERSION% --timing --cosim_file cosimCfg

:: Microsoft compiler to use to build cosim
@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set COSIM_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set COSIM_CC=%VCTOOLS%\bin\amd64\cl.exe
@set COSIM_NAME=..\bin\%TESTFILE_NAME%.dll
@set COSIM_CFLAGS=/MT /O2 /nologo /Zm1000 /EHsc  -DVCPP -DLITTLE_ENDIAN /TP /I. /DCYGPC /I%HEXAGON_TOOLS%\..\..\include\iss\ /I%CURRENT_DIR%\..\src
@set COSIM_LINK=/nologo /link /dll libwrapper.lib /MACHINE:X64 /libpath:%HEXAGON_TOOLS%\..\..\lib\iss\

@echo on
@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim

:: clean
:Clean
@echo.
@echo Cleaning files...
@IF EXIST ..\bin\%TESTFILE_NAME%.dll @DEL ..\bin\%TESTFILE_NAME%.*
@IF EXIST ..\bin\hello @DEL ..\bin\hello
@IF EXIST pmu_stats.txt @DEL pmu_stats.txt
@IF EXIST cosimCfg @DEL cosimCfg
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST ..\build @RMDIR /S /Q ..\build
@IF "%1"=="clean" GOTO End

:: build
:Build
@echo.
@echo Compiling dll

@MKDIR ..\build
@set PATH_TEMP=%PATH%
call %COSIM_INIT%
%COSIM_CC% %COSIM_CFLAGS%  -c %TESTFILE_NAME%.c /Fo%CURRENT_DIR%\..\build\%TESTFILE_NAME%.obj
%COSIM_CC% %CURRENT_DIR%\..\build\%TESTFILE_NAME%.obj %COSIM_LINK% /out:%CURRENT_DIR%\..\bin\%TESTFILE_NAME%.dll
@set PATH=%PATH_TEMP%

%CC% %CFLAGS% %CURRENT_DIR%\..\test\hello.c -o %CURRENT_DIR%\..\bin\hello -lhexagon
@echo %CURRENT_DIR%\..\bin\%TESTFILE_NAME%.dll -w 0x0--0x100000; 0x30000000--0x40000000; > %CURRENT_DIR%\cosimCfg
@IF "%1"=="build" GOTO End

:: run the example
:Sim
@echo.
@echo Running example...
%SIM% %SIMFLAGS% %CURRENT_DIR%/../bin/hello

:End
