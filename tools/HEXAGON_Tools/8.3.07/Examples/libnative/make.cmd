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
@set TESTFILE_NAME=intrinsics
@set MYLIB=%HEXAGON_TOOLS%\libnative\lib\libnative.lib
@set MYINCLUDE=%HEXAGON_TOOLS%\libnative\include

@set VCTOOLS="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
@set MSVC_INIT=%VCTOOLS%\vcvarsall.bat x86_amd64
@set MSVC_CC=%VCTOOLS%\bin\amd64\cl.exe

::
:: Important to add the path to the libnative.dll to the PATH environment var
::
@set PATH=%MYLIB%\..;%PATH%

@echo Libnative environment is now ready to execute make.cm

:Bypass_setup

@IF "%1"=="clean" GOTO Clean
@IF "%1"=="build" GOTO Build
@IF "%1"=="sim" GOTO Sim
@IF  "%1"=="list" GOTO List

:Clean
@echo.
@echo Cleaning files...
@IF EXIST intrinsics.exe @DEL intrinsics.exe
@IF EXIST intrinsics.obj @DEL intrinsics.obj
@IF EXIST libnative-calls* @DEL libnative-calls*
@IF "%1"=="clean" GOTO End

:Build
:: build libnative example
@echo.
@echo Compiling libnative example...
@set PATH_TEMP=%PATH%
call %MSVC_INIT%
%MSVC_CC% /MD /I%MYINCLUDE% %TESTFILE_NAME%.c  /link  %MYLIB% /out:%TESTFILE_NAME%.exe
@set PATH=%PATH_TEMP%

@IF "%1"=="build" GOTO End

:Sim
:: run the example
@echo.

@echo Running libnative example...
%TESTFILE_NAME%.exe
@GOTO End

:List
@call %CC%\..\dumpbin.exe /ALL /OUT:libnative-calls.txt %HEXAGON_TOOLS%\libnative\lib\libnative.lib

:End
