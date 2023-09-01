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
@set SYSTEM_CC=
@set PATH_TEMP=

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
@set SRC_TOP=%CURRENT_DIR%

@set CC=hexagon-clang
@set SIM=hexagon-sim
@set ARCH=62
@set Q6VERSION=v%ARCH%
@set SYS_NAME=hexagon-system
@set TESTFILE_NAME=hello
@set CFLAGS=-m%Q6VERSION% -g
@set CFLAGS=-m%Q6VERSION% -g
@if not defined PORT (
	@set PORT=9912
)

@set INCLUDE_DIR1=%HEXAGON_TOOLS%\include\iss
@set INCLUDE_DIR2="C:\Program Files (x86)\Windows Kits\10\Include\10.0.10240.0\ucrt"
@set INCLUDE_DIR3="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include"
@set LIB="C:\Program Files (x86)\Windows Kits\10\Lib\10.0.10240.0\um\x64";"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.10240.0\ucrt\x64";"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib\amd64";
@set BIN_DIR=%SRC_TOP%\bin
@set TEST_DIR=%SRC_TOP%\src
@set SIMFLAGS=-m%Q6VERSION% %BIN_DIR%\%TESTFILE_NAME% --timing

:: Microsoft compiler to use to build cosim
@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set SYSTEM_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set SYSTEM_CC=%VCTOOLS%\bin\amd64\cl.exe
@set SYSTEM_CFLAGS=/MT /Zi /O2 /nologo /Zm1000 /EHsc  /FC -DVCPP -DLITTLE_ENDIAN /TP /I. /I%INCLUDE_DIR1%
@set SYSTEM_LINK=/nologo /link libwrapper.lib /MACHINE:X64 /libpath:%HEXAGON_TOOLS%\lib\iss

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim
@IF "%1"=="lldb" GOTO lldb
@IF "%1"=="t32" GOTO T32

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_stats* @DEL pmu_stats*
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST %SYS_NAME%.obj @DEL %SYS_NAME%.obj
@IF EXIST *.pdb @DEL *.pdb
@IF EXIST %BIN_DIR% @DEL /Q %BIN_DIR%\*.*
@IF EXIST %BIN_DIR% @RMDIR /Q %BIN_DIR%
@IF "%1"=="clean" GOTO End

::
:: Build the system simulator
::
:Build
@echo.
@echo Compiling system simulator ...
@MKDIR %BIN_DIR%
@set PATH_TEMP=%PATH%
echo %SYSTEM_INIT%
call %SYSTEM_INIT%

@echo %SYSTEM_CC% %SYSTEM_CFLAGS%  %TEST_DIR%\%SYS_NAME%.cpp /I%INCLUDE_DIR1% /I%INCLUDE_DIR2% /I%INCLUDE_DIR3% %SYSTEM_LINK% /out:%BIN_DIR%\%SYS_NAME%.exe
%SYSTEM_CC% %SYSTEM_CFLAGS%  %TEST_DIR%\%SYS_NAME%.cpp /I%INCLUDE_DIR1% /I%INCLUDE_DIR2% /I%INCLUDE_DIR3% %SYSTEM_LINK% /out:%BIN_DIR%\%SYS_NAME%.exe
@set PATH=%PATH_TEMP%
%CC% %CFLAGS% -o %BIN_DIR%\%TESTFILE_NAME%.elf %TEST_DIR%\%TESTFILE_NAME%.c -lhexagon

@IF "%1"=="build" GOTO End

::
:: Simulate the example
::
:Sim
@echo.
@echo Running example with Port=0 so it will run to completion
@echo start /b %BIN_DIR%\%SYS_NAME%.exe -m%Q6VERSION% -G 0 %BIN_DIR%\%TESTFILE_NAME%.elf
start /b %BIN_DIR%\%SYS_NAME%.exe -m%Q6VERSION% -G 0 %BIN_DIR%\%TESTFILE_NAME%.elf
GOTO End

::
:: Invoke lldb to step through hello.elf
:: When in hexagon-lldb to connect: gdb-remote 9912
::
:lldb
@echo Running example so it interfaces to hexagon-lldb via gdb-remote port
start cmd /C %BIN_DIR%\%SYS_NAME%.exe -m%Q6VERSION% -G %PORT% %BIN_DIR%\%TESTFILE_NAME%.elf
start cmd /C hexagon-lldb -s src/lldb-setup.txt
start cmd /C devenv
call :printinfo
GOTO End


::
:: Invoke T32 to step through hello.elf
::
:T32
@echo Running example so it interfaces to Lauterbach trace32 via gdb-remote port
start cmd /C %BIN_DIR%\%SYS_NAME%.exe -m%Q6VERSION% -G %PORT% %BIN_DIR%\%TESTFILE_NAME%.elf
start cmd /C %T32SYS%\bin\windows64\t32mqdsp6.exe -c cmm/win.cfg -s cmm/hex.cmm %PORT% %ARCH% %BIN_DIR%\%TESTFILE_NAME%.elf win_layout
start cmd /C devenv
call :printinfo
GOTO End

::
:: Print info on how to get visual studio to attach to process
::
:printinfo
SETLOCAL
@echo.
@echo *****************************************************
@echo To debug your system simulation using Visual Studio
@echo Click on Debug->Attach to Process or (Ctrl+Alt+P)
@echo and find hexagon-system.exe process and Attach.
@echo After libraries are loaded, you can stop process by
@echo clicking on || or (Ctrl+Alt+Break).
@echo *****************************************************
@echo.
ENDLOCAL

::
:End
