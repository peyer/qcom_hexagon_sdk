::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on 1/12/2018
::################################################################
::
:: Make.cmd to build the example
:: Please note that this example requires Microsoft Visual Studio 15.
::

@set PLUGIN_NAME=ReplaceBlock

@echo off
@set HEXAGON_TOOLS=
@set INCLUDE=
@set LIB=
@set PLUGIN_CC=

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

@set CC=hexagon-clang
@set SIM=hexagon-sim
@set Q6VERSION=v60
@set CFLAGS=-m%Q6VERSION% -O2 -G0 -fno-inline -ffunction-sections -fdata-sections
@set OBJS=(functions,main)

@set INCLUDE_DIR=%HEXAGON_TOOLS%\include\QCLD\PluginAPI
@set LIB_DIR=%HEXAGON_TOOLS%\lib
@set TARGET_DIR=.\target

:: Microsoft MSVC compiler is used to build linker plugin DLL
@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set MSVC_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set PLUGIN_CC=%VCTOOLS%\bin\amd64\cl.exe
@set PLUGIN_CFLAGS=/MT /O2 /nologo /Zm1000 /EHsc  -DVCPP -DLITTLE_ENDIAN /TP /I. /DCYGPC /I"%INCLUDE_DIR%" /I"%CURRENT_DIR%"
@set PLUGIN_LINK=/nologo /link /dll LW.lib /MACHINE:X64 /libpath:"%LIB_DIR%"

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="link" GOTO Link

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST *.dll @DEL *.dll
@IF EXIST *.exp @DEL *.exp
@IF EXIST *.obj @DEL *.obj
@IF EXIST *.lib @DEL *.lib
@IF EXIST %TARGET_DIR%\*.o @DEL %TARGET_DIR%\*.o
@IF EXIST %TARGET_DIR%\main.elf @DEL %TARGET_DIR%\main.elf
@IF EXIST %TARGET_DIR%\*.map @DEL %TARGET_DIR%\*.map
@IF "%1"=="clean" GOTO End

::
:: Build the linker plugin DLL
::
:Build
@echo.
@echo Configure MSVC compiler ...
@set PATH_TEMP=%PATH%
call %MSVC_INIT%
@echo.
@echo Compiling the Linker Plugin ...
@echo %PLUGIN_CC% %PLUGIN_CFLAGS%  -c %PLUGIN_NAME%.cpp /Fo%PLUGIN_NAME%.obj
%PLUGIN_CC% %PLUGIN_CFLAGS%  -c %PLUGIN_NAME%.cpp /Fo%PLUGIN_NAME%.obj
@echo %PLUGIN_CC% %PLUGIN_NAME%.obj %PLUGIN_LINK% /out:%PLUGIN_NAME%.dll
%PLUGIN_CC% %PLUGIN_NAME%.obj %PLUGIN_LINK% /out:%PLUGIN_NAME%.dll
@set PATH=%PATH_TEMP%
@IF "%1"=="build" GOTO End

:Link
@echo.
@echo Build the Hexagon files ...
::
for %%g in %OBJS% do (
@echo %CC% %CFLAGS% -o %TARGET_DIR%\%%g.o -c %TARGET_DIR%\%%g.c
%CC% %CFLAGS% -o %TARGET_DIR%\%%g.o -c %TARGET_DIR%\%%g.c
)
@echo.
@echo Build the executable and invoke the Plugin
@echo %CC% %CFLAGS% %TARGET_DIR%\*.o -o %TARGET_DIR%\main.elf -Wl,-T,%TARGET_DIR%\linker.script,--trace=plugin,-Map,%TARGET_DIR%\%PLUGIN_NAME%.map
%CC% %CFLAGS% %TARGET_DIR%\*.o -o %TARGET_DIR%\main.elf -Wl,-T,%TARGET_DIR%\linker.script,--trace=plugin,-Map,%TARGET_DIR%\%PLUGIN_NAME%.map
@echo.
@echo hexagon-readelf -S %TARGET_DIR%\main.elf ^| findstr "compress marker" 
@echo.
hexagon-readelf -S %TARGET_DIR%\main.elf | findstr "compress marker"
@echo.
@echo hexagon-llvm-objdump -s -j .compress %TARGET_DIR%\main.elf
hexagon-llvm-objdump -s -j .compress %TARGET_DIR%\main.elf

@echo.
@echo Done
::
:End

::
