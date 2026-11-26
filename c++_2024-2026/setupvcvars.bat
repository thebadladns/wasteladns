@echo off
setlocal enabledelayedexpansion

REM Try to find a Visual Studio installation path
set path_candidate=C:\Program Files\Microsoft Visual Studio\
for /F "delims=" %%d in ('dir /b /ad /o-d "%path_candidate%"') do (
    echo %%d | findstr /r "^[1-9][0-9]*" >nul
    if !errorlevel! equ 0 (
        for /f "delims=" %%f in ('dir /b /ad /o-d "%path_candidate%%%d"') do (
            echo Found most recently modified Visual Studio directory: "%path_candidate%%%d\%%f"
            REM end local variable scope early, so the changes in vcvars64 make it to the caller
            endlocal
            call "%path_candidate%%%d\%%f\VC\Auxiliary\Build\vcvars64.bat"
            goto eof REM break loop
        )
    )
)

REM if we haven't found a Visual Studio installation path yet, try in Program Files (x86)
set path_candidate=C:\Program Files (x86)\Microsoft Visual Studio\
for /F "delims=" %%d in ('dir /b /ad /o-d "%path_candidate%"') do (
    echo %%d | findstr /r "^[1-9][0-9]*" >nul
    if !errorlevel! equ 0 (
        for /f "delims=" %%f in ('dir /b /ad /o-d "%path_candidate%%%d"') do (
            echo Found most recently modified Visual Studio directory: "%path_candidate%%%d\%%f"
            REM end local variable scope early, so the changes in vcvars64 make it to the caller
            endlocal
            call "%path_candidate%%%d\%%f\VC\Auxiliary\Build\vcvars64.bat"
            goto eof REM break loop
        )
    )
)
:eof REM note that the script's end will also end the local environment
