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
@set TESTNAME=particle
@set SIM=hexagon-sim
@set CONVERT=hexagon-llvm-profdata
@set CXX=hexagon-clang++
@set Q6VERSION=v60
@set CXXFLAGS=-O2 -g -std=c++03 -m%Q6VERSION%
@set SIMFLAG=-m%Q6VERSION%
@set INSTR_FLAG=-fprofile-instr-generate
@set PGO=hexagon-llvm-profdata
@set PGOFLAGS=-fprofile-instr-use=code.profdata -mllvm -disable-block-placement=false -mllvm -whole-function-placement=true


@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build_std" GOTO Build_std
@IF "%1"=="sim_std" GOTO Sim_std
@IF "%1"=="build_instr" GOTO Build_instr
@IF "%1"=="sim_instr" GOTO Sim_instr
@IF "%1"=="convert" GOTO Convert

:Clean
@echo.
@echo Cleaning files...
@IF EXIST %TESTNAME%_std @DEL %TESTNAME%_std
@IF EXIST pmu_stat* @DEL pmu_stat*
@IF EXIST default.profraw @DEL default.profraw
@IF EXIST code.profdata @DEL code.profdata
@IF EXIST instr.txt @DEL instr.txt
@IF EXIST %TESTNAME%_* @DEL %TESTNAME%_*


@IF "%1"=="clean" GOTO End

:Build_std
:: build libnative example
@echo.
@echo Building example...
%CXX% %CXXFLAGS% %TESTNAME%.cpp  -o %TESTNAME%_std -lhexagon
@IF "%1"=="build" GOTO End

:Sim_std
:: simulate the original example
@echo Simulating standard example...
%SIM% %SIMFLAG% %TESTNAME%_std
@IF "%1"=="sim_std" GOTO End

:Build_instr
:: build with profile optimization
@echo Building instrumented result...
%CXX% %CXXFLAGS% %INSTR_FLAG% %TESTNAME%.cpp -o %TESTNAME%_instr -lhexagon
@IF "%1"=="build_instr" GOTO End

:Sim_instr
:: simulate the profile optimized build
@echo Simulating instrumented build...
%SIM% %SIMFLAG% %TESTNAME%_instr > instr.txt 2>&1
@IF "%1"=="sim_instr" GOTO End

:Convert
:: Convert to llvm format
@echo Converting instrumented result to llvm format
%CONVERT% merge default.profraw -o code.profdata
@IF "%1"=="convert" GOTO End

:Build_pgo
:: # This is the pgo result
@echo Building Profile Guided Optimization code...
%CXX% %CXXFLAGS% %PGOFLAGS% %TESTNAME%.cpp -o %TESTNAME%_pgo -l hexagon
@IF "%1"=="build_pgo" GOTO End

:Sim_pgo
:: simulate Profile Guided Optimization and check for improvement
@echo Simulating PGO code...
@echo There will be improvement in pcycles to standard simulation
%SIM% %SIMFLAG% %TESTNAME%_pgo

:End
