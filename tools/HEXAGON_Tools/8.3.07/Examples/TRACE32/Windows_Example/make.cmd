REM *********************************
REM Example to illustrate T32
REM
REM Setting Environment variables
REM Make sure you have T32SYS and PATH
REM set to point to t32mqdsp6.exe
REM Also make sure T32TMP is set to
REM a valid folder path.
REM *********************************

set T32TMP=C:\TEMP

REM Cleaning up the local folder
rm -f mandelbrot stats.txt

REM Building the example program
hexagon-clang -O2 -mv60 -g mandelbrot.c -o mandelbrot

REM Launch the T32 example
%T32SYS%\bin\windows64\t32mqdsp6.exe -c win.cfg -s hexagon.cmm mandelbrot v60
