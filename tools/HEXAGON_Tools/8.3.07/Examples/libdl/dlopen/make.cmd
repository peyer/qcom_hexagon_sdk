::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build the libdl/dlopen example
::
:: Get Hexagon tools path
::

@set PROGRAM=dlopen
@set SIM=hexagon-sim
@set Q6VERSION=v60
@set CC=hexagon-clang -m%Q6VERSION% %CFLAGS%
@set LD=hexagon-link

@set CFLAGS=-g -O0 -G0
@set SIMFLAGS=--simulated_returnval --cosim_file cosim.cfg -m%Q6VERSION%
::################################################
:: Make sure that `where` command only picks up ##
:: first path it finds, in case there are more  ##
:: than one version tools path in PATH variable ##
::################################################
@echo off
@for /f "delims=" %%a in ('where hexagon-sim') do (
    @set RELEASE_DIR=%%a
    @goto :done
)

:done
::################################################
:: Truncate hexagon-sim.exe from string         ##
::################################################
@for %%F in (%RELEASE_DIR%) do @set dirname=%%~dpF
@set RELEASE_DIR=%dirname%..

::
:: Librarys from the toolchain
::
@set CRT0_STANDALONE=%RELEASE_DIR%\target\hexagon\lib\%Q6VERSION%\G0\crt0_standalone.o
@set CRT0=%RELEASE_DIR%\target\hexagon\lib\%Q6VERSION%\G0\crt0.o
@set INIT=%RELEASE_DIR%\target\hexagon\lib\%Q6VERSION%\G0\init.o
@set LIB_STANDALONE=%RELEASE_DIR%\target\hexagon\lib\%Q6VERSION%\G0\libstandalone.a
@set LIB_C=%RELEASE_DIR%\target\hexagon\lib\%Q6VERSION%\G0\libc.a
@set LIB_GCC=%RELEASE_DIR%\target\hexagon\lib\%Q6VERSION%\G0\libgcc.a
@set FINI=%RELEASE_DIR%\target\hexagon\lib\%Q6VERSION%\G0\fini.o
@set LIBDL=%RELEASE_DIR%\target\hexagon\lib\%Q6VERSION%\G0\libdl.a
@echo on
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
	@IF EXIST liba.so @DEL liba.so
	@IF EXIST libb.so @DEL libb.so
	@IF EXIST %PROGRAM%.o @DEL %PROGRAM%.o
	@IF EXIST %PROGRAM% @DEL %PROGRAM%
@IF "%1"=="clean" GOTO End

::
:: Build the cosim and test case
::
:Build
@echo.
@echo Compiling dlopen ...

	%CC% -fPIC -shared -nostartfiles liba.c -o liba.so
	%CC% -fPIC -shared -nostartfiles libb.c -o libb.so
	%CC% -c %PROGRAM%.c
	%LD% -o %PROGRAM% %CRT0_STANDALONE% %CRT0% %INIT% %PROGRAM%.o %LIBDL% ^
	--start-group  %LIB_STANDALONE% --whole-archive %LIB_C% ^
	--no-whole-archive %LIB_GCC% --end-group %FINI% ^
	--dynamic-linker= -E --force-dynamic

@IF "%1"=="build" GOTO End

::
:: Simulate the example
::
:Sim
@echo.
@echo Simulating dlopen example ...
@echo.
	%SIM% %PROGRAM%
::
:End
::
