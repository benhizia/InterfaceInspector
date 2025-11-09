@echo off
REM Quick build script for SwitchSpy
REM This script performs all steps: configure, build, and optionally run

setlocal

echo ========================================
echo SwitchSpy Quick Build
echo ========================================
echo.

REM Default to x64 Release if no arguments
set ARCH=x64
set CONFIG=release

if not "%1"=="" set ARCH=%1
if not "%2"=="" set CONFIG=%2

echo Building %ARCH% %CONFIG%...
echo.

REM Configure
echo [1/3] Configuring...
call build.cmd %ARCH% %CONFIG% configure
if errorlevel 1 goto :error

REM Build
echo.
echo [2/3] Building...
call build.cmd %ARCH% %CONFIG% build
if errorlevel 1 goto :error

REM Success
echo.
echo [3/3] Success!
echo.
echo ========================================
echo Build complete!
echo ========================================
echo.
echo To run: build.cmd %ARCH% %CONFIG% run
echo Or run directly: build\windows-%ARCH%-%CONFIG%\%CONFIG%\switchspy.exe
echo.

REM Ask if user wants to run
set /p RUN="Run SwitchSpy now? (y/n): "
if /i "%RUN%"=="y" (
    echo.
    call build.cmd %ARCH% %CONFIG% run
)

goto :eof

:error
echo.
echo ========================================
echo ERROR: Build failed!
echo ========================================
exit /b 1
