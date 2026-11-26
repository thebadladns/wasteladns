@echo off

REM switch to file's directory
cd /D "%~dp0"

where cl >nul 2>nul
if %errorlevel% neq 0 echo Setting up Visual Studio compiler & call setupvcvars.bat

REM defer setlocal till after we've setup the Visual Studio variables
setlocal

REM track start time
set t0=%time: =0%

REM Before running this bat file, you need to call either vcvars64.bat or vcvarsall.bat.
REM These are usually located in Visual Studio's installation files: C:\Program Files (x86)\Microsoft Visual Studio\{Year}\{Edition}\VC\Auxiliary\Build\

REM usage
REM build.bat [dx11|gl33|] [debug|release|] [assets|]
REM defaults to dx11 debug no assets (unless the assets folder doesn't exist)
set debug=0
set release=0
set dx11=0
set gl33=0
set assets=0

REM unpack args
for %%a in (%*) do set "%%a=1"
if not "%release%"=="1" set debug=1
if "%debug%"=="1" set release=0
if "%release%"=="1" set debug=0
if not "%gl33%"=="1" set dx11=1
if "%dx11%"=="1" set gl33=0
if "%gl33%"=="1" set dx11=0

REM figure out compile args
REM /Od=no optimization, /Ob1=respect __inline, /Zi=generate PDB
set cl_debug=call cl /Od /Ob1 /Zi
REM /O2=speed optimization, /DNDEBUG=define non-debug macro to match CMake
set cl_release=call cl /O2 /DNDEBUG
if "%dx11%"=="1" (
    set outname=app-dx11
    set compileoutdir=.\app-dx11.dir
    set flags=/D__DX11=1 
    set libs=user32.lib
)
if "%gl33%"=="1" (
    set outname=app-gl33
    set compileoutdir=.\app-gl33.dir
    set flags=/D__GL33=1
    set libs=user32.lib gdi32.lib
)
if "%debug%"=="1" (
    set outdir=.\bin\Debug
    set compile=%cl_debug%
    set linkflags=/link /incremental:no /PDB:%outdir%\%outname%.pdb
)
if "%release%"=="1" (
    set outdir=.\bin\Release\
    set compile=%cl_release%
    set linkflags=/link /incremental:no
)

REM create dirs
if not exist %outdir% mkdir %outdir%

REM build
%compile% /nologo /Fe%outdir%\%outname%.exe src\main.cpp %libs% /D__WIN64=1 %flags% %linkflags%

REM post-build step (/E for recursive, /Y for overwrite without prompting)
if not exist %outdir%\assets\ set assets=1
if "%assets%"=="1" xcopy .\assets\ %outdir%\assets\ /E /Y

REM track end time
set t=%time: =0%

REM output total compilation time (/a for arithmetic statements, not strings)
set /a h=1%t0:~0,2%-100
set /a m=1%t0:~3,2%-100
set /a s=1%t0:~6,2%-100
set /a c=1%t0:~9,2%-100
set /a starttime = %h% * 360000 + %m% * 6000 + 100 * %s% + %c%
set /a h=1%t:~0,2%-100
set /a m=1%t:~3,2%-100
set /a s=1%t:~6,2%-100
set /a c=1%t:~9,2%-100
set /a endtime = %h% * 360000 + %m% * 6000 + 100 * %s% + %c%
set /a runtime = %endtime% - %starttime%
echo Compilation time: %runtime%0 ms
