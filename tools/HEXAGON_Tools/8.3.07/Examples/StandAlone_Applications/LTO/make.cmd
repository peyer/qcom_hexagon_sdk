::
:: Make.cmd to build the libnative example
:: libnative.lib is 64 bit and requires the visual studio 64 bit compiler/linker
::
:: Get Hexagon tools path
@echo .
@echo off
@for /f "delims=" %%a in ('where hexagon-sim') do (
    @set HEXAGON_TOOLS=%%a
    @goto :done
)
:done
@set HEXAGON_TOOLS=%HEXAGON_TOOLS%\..\..
@echo on
@set TESTNAME=
@set SIM=hexagon-sim
@set CC=hexagon-clang
@set Q6VERSION=v60
@set CFLAGS=-O2 -g -m%Q6VERSION% -G0 -Wall
@set LTOFLAG=-flto
@set SIMFLAG=-m%Q6VERSION%
@set INSTR_FLAG=-fprofile-instr-generate

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="main_lto" GOTO Main_lto
@IF "%1"=="main" GOTO Main

:Clean
@echo.
@echo Cleaning files...
@IF EXIST *.bc @DEL *.bc
@IF EXIST pmu_stat* @DEL pmu_stat*
@IF EXIST *.o @DEL *.o
@IF EXIST *lto @DEL *lto
@IF EXIST main @DEL main


@IF "%1"=="clean" GOTO End

:Main_lto
:: build example
@echo.
@echo Building example with LTO
%CC% %CFLAGS% %LTOFLAG% -c file1.c -o file1.bc
%CC% %CFLAGS% %LTOFLAG% -c main.c -o main.bc
%CC% %CFLAGS% %LTOFLAG% file1.bc main.bc %LTOFLAG% -o main_lto
@IF "%1"=="build" GOTO End

:Main
:: build
@echo .
@echo Building example without LTO
%CC% %CFLAGS% -c file1.c -o file1.o
%CC% %CFLAGS% -c main.c -o main.o
%CC% %CFLAGS% file1.o main.o -o main
@IF "%1"=="main" GOTO End

:Read_main
:: show main without LTO
@echo .
@echo Without LTO, all the foo functions are kept
hexagon-readelf -s main | grep -w "foo[0-9]\|main" | grep -v main.clang
@echo .

:read_main_lto
@echo .
@echo With LTO, all the foo functions are removed
hexagon-readelf -s main_lto | grep -w "foo[0-9]\|main"
@echo .

:End
