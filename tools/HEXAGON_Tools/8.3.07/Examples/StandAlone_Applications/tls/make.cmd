::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Windows Make.cmd to build the example
::

@echo off

@set Q6VERSION=v60
@set TESTFILE=tls

@set CC=hexagon-clang
@set SIM=hexagon-sim

@set CCFLAGS=-m%Q6VERSION% -g

@set SIMFLAGS=--timing --m%Q6VERSION% %TESTFILE%.elf

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
@IF EXIST *.elf @DEL *.elf
@IF EXIST *.o @DEL *.o
@IF "%1"=="clean" GOTO End

::
:: Build the test case
::
:Build
@echo.
@echo Building example ...
%CC% %CCFLAGS% %TESTFILE%.c -o %TESTFILE%.elf
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
