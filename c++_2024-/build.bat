@echo off
cd /D "%~dp0"

:: usage
:: build.bat [dx11|gl33] [debug|release] [assets|]
:: defaults to dx11 debug no assets
set debug=0
set release=0
set dx11=0
set gl33=0
set assets=0

:: unpack args
for %%a in (%*) do set "%%a=1"
if not "%release%"=="1" set debug=1
if "%debug%"=="1" set release=0
if "%release%"=="1" set debug=0
if not "%gl33%"=="1" set dx11=1
if "%dx11%"=="1" set gl33=0
if "%gl33%"=="1" set dx11=0

:: figure out compile args
set cl_debug=call cl /Od /Ob1
set cl_release=call cl /O2 /DNDEBUG
if "%dx11%"=="1" (
    set compileoutdir=.\app-dx11.dir
    set flags=/D__DX11=1
    set libs=user32.lib
)
if "%gl33%"=="1" (
    set compileoutdir=.\app-gl33.dir
    set flags=/D__GL33=1
    set libs=user32.lib gdi32.lib
)
if "%debug%"=="1" (
    set outdir=.\bin\Debug
    set objectdir=%compileoutdir%\Debug\
    set compile=%cl_debug%
)
if "%release%"=="1" (
    set outdir=.\bin\Release\
    set objectdir=%compileoutdir%\Release\
    set compile=%cl_release%
)

:: create dirs
if not exist %outdir% mkdir %outdir%
if not exist %objectdir% mkdir %objectdir%

:: build
%compile% /Fe%outdir%\app-dx11.exe /Fo%objectdir% src\main.cpp %libs% /D__WIN64=1 %flags%

:: post-build step (/E for recursive, /Y for overwrite without prompting)
if "%assets%"=="1" xcopy .\assets\ %outdir%\assets\ /E /Y
