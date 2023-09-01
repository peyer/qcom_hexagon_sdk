@echo off

REM --- This script is not intended to be used with the Hexagon SDK. ---

mkdir deploy\docs
copy /Y docs\* deploy\docs

REM 8.0 tools
for %%a in (Debug Release ReleaseG ) do (
    for %%b in (60 62) do ( 
        for %%f in (hexagon_%%a_toolv80_v%%b hexagon_%%a_dynamic_toolv80_v%%b) do ( 
            pakman get pak?%%f
            make tree V=%%f
            mkdir deploy\%%f
            copy /Y %%f\ship\* deploy\%%f
        )
    )
)

REM 8.1 tools
for %%a in (Debug Release ReleaseG ) do (
    for %%b in (60 62 65) do ( 
        for %%f in (hexagon_%%a_toolv81_v%%b hexagon_%%a_dynamic_toolv81_v%%b) do ( 
            pakman get pak?%%f
            make tree V=%%f
            mkdir deploy\%%f
            copy /Y %%f\ship\* deploy\%%f
        )
    )
)

