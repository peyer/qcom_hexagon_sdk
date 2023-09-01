::###############################################################
:: Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
:: All Rights Reserved.
:: Modified by QUALCOMM INCORPORATED on \$Date\$
::################################################################
::
:: Make.cmd to build the example
::

::@echo off
@set HEXAGON_TOOLS=

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
:: if version is 8.1 or higher, set for v66
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

@set T32_CFG=cmm/t32.cfg hexagon-mcd64.dll
@set T32_EXE=bin/windows64/t32mqdsp6.exe
@set Q6_TIMER=qtimer.dll
@set Q6_L2VIC=l2vic.dll

@set CC=hexagon-clang
@set SIM=hexagon-sim
@set Q6_CFG=q6ss.cfg
@set TIMER_FREQ=19200000
@set L2_INT1=36
@set L2_INT2=58
@set L2_INT3=80
@set L2_INT4=112
@set CSR_BASE1=0xfab00000
@set CSR_BASE2=0xfab40000

@set TESTFILE_NAME=2qtimer_test
@set CFLAGS0=-m%Q6VERSION% -g -DQTMR_FREQ=%TIMER_FREQ%
@set CFLAGS1=-DL2_INT1=%L2_INT1% -DL2_INT2=%L2_INT2% -DL2_INT3=%L2_INT3%
@set CFLAGS2=-DL2_INT4=%L2_INT4% -DINTERRUPT=%INTERRUPT% -DQ6VERSION=%Q6VERSION:~1,2%
@set CFLAGS=%CFLAGS0% %CFLAGS1% %CFLAGS2%

@set SIMFLAGS1=-m%Q6VERSION% --bypass_idle --timing --pmu_statsfile mystats.txt
@set SIMFLAGS2=--cosim_file %Q6_CFG% --l2tcm_base 0xef5a %TESTFILE_NAME%.elf
@set SIMFLAGS=%SIMFLAGS1% %SIMFLAGS2%

@set T32_CFG=cmm/t32.cfg hexagon-mcd64.dll
@set T32_EXE=bin/windows64/t32mqdsp6.exe

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim
@IF "%1"=="t32" GOTO T32

::
:: Clean up
::
:Clean
@echo.
@echo Cleaning files...
@IF EXIST pmu_stats* @DEL pmu_stats*
@IF EXIST stats.txt @DEL stats.txt
@IF EXIST q6ss.cfg @DEL q6ss.cfg
@IF EXIST %TESTFILE_NAME%.elf @DEL %TESTFILE_NAME%.elf
@IF "%1"=="clean" GOTO End

::
:: Build the bus cosim
::
:Build
@echo.
@echo Building example...
@echo %Q6_TIMER% --csr_base=%CSR_BASE1% --irq_p=%L2_INT1%,%L2_INT2% --freq=19200000 --cnttid=0x11 > %Q6_CFG%
@echo %Q6_TIMER% --csr_base=%CSR_BASE2% --irq_p=%L2_INT3%,%L2_INT4% --freq=19200000 --cnttid=0x11 >> %Q6_CFG%
@echo %Q6_L2VIC% 4 0xfab10000 >> %Q6_CFG%
@echo l2vic.dll 4 0xfab10000 >> %Q6_CFG%
@echo %CC% %CFLAGS% %TESTFILE_NAME%.c -o %TESTFILE_NAME%.elf -lhexagon
%CC% %CFLAGS% %TESTFILE_NAME%.c -o %TESTFILE_NAME%.elf -lhexagon
@IF "%1"=="build" GOTO End

::
:: Simulate the example
::
:Sim
@echo.
@echo Simulating example
@echo %SIM% %SIMFLAGS%
%SIM% %SIMFLAGS%
GOTO End

::
:: Debug using Trace32
::
:T32
@set T32SYS=%T32SYS%
@echo %T32SYS%/%T32_EXE% -c %T32_CFG% -s cmm/hexagon.cmm %TESTFILE_NAME%.elf %Q6VERSION% %Q6_CFG%
%T32SYS%/%T32_EXE% -c %T32_CFG% -s cmm/hexagon.cmm %TESTFILE_NAME%.elf %Q6VERSION% %Q6_CFG%

::
:: El Fin
:End
::
