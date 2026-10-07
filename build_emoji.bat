:: ============================================================
:: AVIS Neon Core - Win64 Emoji Pager Build Script
:: Filename: build_emoji.bat
:: Purpose: Compile multi-module project using MSVC
:: ============================================================

@echo off
setlocal
cd /d "%~dp0"

:: Initialize MSVC environment
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo ❌ ERROR: MSVC 2022 x64 build environment not found.
    pause
    exit /b 1
)

if exist CVBGOD-EMOJI.exe del /f /q CVBGOD-EMOJI.exe

del /f /q *.obj 2>nul
del /f /q *.ilk 2>nul
del /f /q *.pdb 2>nul
del /f /q *.exp 2>nul
del /f /q *.lib 2>nul
del /f /q page.html 2>nul

echo.
echo Compiling...
echo.

:: ------------------------------------------------------------
:: Build
:: ------------------------------------------------------------

:: Compile the resource file containing executable metadata
rc version.rc

:: Compile C sources and link them together with version.res and required Windows libraries
cl /nologo /W4 /wd4201 /O2 main.c browser_host.c emoji_core.c version.res user32.lib gdi32.lib ole32.lib oleaut32.lib comctl32.lib advapi32.lib /Fe:CVBGOD-EMOJI.exe

if errorlevel 1 goto BUILD_FAILED

:: ------------------------------------------------------------
:: Verify output
:: ------------------------------------------------------------

if not exist CVBGOD-EMOJI.exe goto BUILD_FAILED

echo.
echo ============================================================
echo BUILD SUCCESSFUL
echo ============================================================
echo.

start "" CVBGOD-EMOJI.exe
goto END

:BUILD_FAILED

echo.
echo ============================================================
echo BUILD FAILED
echo ============================================================
echo.
echo Review compiler errors above.
echo.

:END

pause
endlocal