::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Windows Make.cmd to build the example
::

@echo off

@set Q6VERSION=v62

@if "%1"=="Q6VERSION" @set Q6VERSION=%2

@set TESTFILE=mandelbrot

@set CC=hexagon-clang
@set SIM=hexagon-sim

@IF "%Q6VERSION%"=="v62" @set SIMFLAGS=--bypass_idle -mv62d_1536
@IF "%Q6VERSION%"=="v65" @set SIMFLAGS=--bypass_idle -mv65h_1536
@IF "%Q6VERSION%"=="v66" @set SIMFLAGS=--bypass_idle -mv66d_1536
@IF "%Q6VERSION%"=="v67" (
        @set VALID=@hexagon-sim --help|findstr v67
        @IF NOT "%VALID%"=="" (
                @set SIMFLAGS=--bypass_idle -mv67h_3072
                ) ELSE (
                @echo "v67 is invalid architecture for this version of tools, using v66"
                @set Q6VERSION=v66
        )
)
@set CFLAGS=-O2 -g -m%Q6VERSION%
@set CLADEFLAGS=-Wl,-T,clade.lcs,-Map,clade.map

@echo.
@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim
@IF "%1"=="lldb" GOTO LLDB

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_stats* @DEL pmu_stats*
@IF EXIST *.o @DEL *.o
@IF EXIST *.map @DEL *.map
@IF EXIST %TESTFILE% @DEL %TESTFILE%
@IF "%1"=="clean" GOTO End

::
:: Build the test case
::
:Build
@echo.
@echo Compiling test case ...
%CC% %CFLAGS% -c %TESTFILE%.c -o %TESTFILE%.o
%CC% %CFLAGS% %TESTFILE%.o -o %TESTFILE% -lhexagon %CLADEFLAGS%
@IF "%1"=="build" GOTO End

::
:: Simulate the example
::
:Sim
@echo.
@echo Simulating example
%SIM% %SIMFLAGS% %TESTFILE%
GOTO End

::
:: Debug example with lldb
::
:LLDB
@echo.
@echo Debugging with hexagon-lldb
hexagon-lldb %TESTFILE% -- %SIMFLAGS%

::
:End
::
