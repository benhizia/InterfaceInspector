@echo off
REM SwitchSpy Build Script for Windows
REM Requires: Visual Studio 2022, CMake 3.21+

setlocal enabledelayedexpansion

echo ========================================
echo SwitchSpy Build Script
echo ========================================
echo.

REM Parse command line arguments
set ARCH=x64
set CONFIG=debug
set ACTION=configure

if "%1"=="" goto :usage
if "%1"=="-h" goto :usage
if "%1"=="--help" goto :usage
if "%1"=="/?" goto :usage

:parse_args
if "%1"=="" goto :validate_args
if /i "%1"=="x64" set ARCH=x64
if /i "%1"=="x86" set ARCH=x86
if /i "%1"=="32" set ARCH=x86
if /i "%1"=="64" set ARCH=x64
if /i "%1"=="debug" set CONFIG=debug
if /i "%1"=="release" set CONFIG=release
if /i "%1"=="configure" set ACTION=configure
if /i "%1"=="build" set ACTION=build
if /i "%1"=="rebuild" set ACTION=rebuild
if /i "%1"=="clean" set ACTION=clean
if /i "%1"=="test" set ACTION=test
if /i "%1"=="run" set ACTION=run
shift
goto :parse_args

:validate_args
set PRESET=windows-%ARCH%-%CONFIG%

echo Configuration:
echo   Architecture: %ARCH%
echo   Build Type:   %CONFIG%
echo   Preset:       %PRESET%
echo   Action:       %ACTION%
echo.

REM Check for CMake
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake 3.21 or later
    exit /b 1
)

REM Check CMake version
for /f "tokens=3" %%i in ('cmake --version ^| findstr /R "version"') do set CMAKE_VERSION=%%i
echo Using CMake version: %CMAKE_VERSION%
echo.

REM Check for Visual Studio 2022
set VS2022_PATH=
for %%d in (Community Professional Enterprise) do (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\%%d\Common7\IDE\devenv.exe" (
        set VS2022_PATH=C:\Program Files\Microsoft Visual Studio\2022\%%d
        goto :vs_found
    )
)

if "%VS2022_PATH%"=="" (
    echo WARNING: Visual Studio 2022 not detected in standard location
    echo Attempting to continue anyway...
    echo.
) else (
    :vs_found
    echo Found Visual Studio 2022 at: %VS2022_PATH%
    echo.
)

REM Execute requested action
if /i "%ACTION%"=="configure" goto :do_configure
if /i "%ACTION%"=="build" goto :do_build
if /i "%ACTION%"=="rebuild" goto :do_rebuild
if /i "%ACTION%"=="clean" goto :do_clean
if /i "%ACTION%"=="test" goto :do_test
if /i "%ACTION%"=="run" goto :do_run

:do_configure
echo Configuring project with preset: %PRESET%
echo.
cmake --preset %PRESET%
if errorlevel 1 (
    echo.
    echo ERROR: Configuration failed
    exit /b 1
)
echo.
echo Configuration complete!
echo Build directory: build\%PRESET%
echo.
echo To build, run: %0 %ARCH% %CONFIG% build
goto :eof

:do_build
echo Building project with preset: %PRESET%
echo.
cmake --build --preset %PRESET% --config %CONFIG%
if errorlevel 1 (
    echo.
    echo ERROR: Build failed
    exit /b 1
)
echo.
echo Build complete!
echo Executable: build\%PRESET%\%CONFIG%\switchspy.exe
echo.
echo To run, use: %0 %ARCH% %CONFIG% run
goto :eof

:do_rebuild
echo Cleaning and rebuilding project with preset: %PRESET%
echo.
cmake --build --preset %PRESET% --config %CONFIG% --clean-first
if errorlevel 1 (
    echo.
    echo ERROR: Rebuild failed
    exit /b 1
)
echo.
echo Rebuild complete!
goto :eof

:do_clean
echo Cleaning build directory: build\%PRESET%
echo.
if exist "build\%PRESET%" (
    rmdir /s /q "build\%PRESET%"
    echo Clean complete!
) else (
    echo Build directory does not exist.
)
goto :eof

:do_test
echo Running tests with preset: %PRESET%
echo.
ctest --preset %PRESET% --config %CONFIG%
if errorlevel 1 (
    echo.
    echo WARNING: Some tests failed
)
goto :eof

:do_run
set EXE_PATH=build\%PRESET%\%CONFIG%\switchspy.exe
if not exist "%EXE_PATH%" (
    echo ERROR: Executable not found: %EXE_PATH%
    echo Please build first: %0 %ARCH% %CONFIG% build
    exit /b 1
)
echo Running: %EXE_PATH%
echo.
"%EXE_PATH%" config\example.ini
goto :eof

:usage
echo.
echo Usage: %0 [architecture] [configuration] [action]
echo.
echo Arguments:
echo   architecture  : x64 (default), x86, 32, 64
echo   configuration : debug (default), release
echo   action        : configure, build, rebuild, clean, test, run
echo.
echo Examples:
echo   %0 x64 debug configure     - Configure x64 Debug build
echo   %0 x64 debug build         - Build x64 Debug
echo   %0 x64 release build       - Build x64 Release
echo   %0 x86 debug build         - Build x86 (32-bit) Debug
echo   %0 x64 release run         - Build and run x64 Release
echo   %0 x64 debug test          - Run tests for x64 Debug
echo   %0 x64 debug clean         - Clean x64 Debug build
echo.
echo Workflow:
echo   1. Configure:  %0 x64 debug configure
echo   2. Build:      %0 x64 debug build
echo   3. Run:        %0 x64 debug run
echo.
echo Quick build and run:
echo   %0 x64 debug configure ^&^& %0 x64 debug build ^&^& %0 x64 debug run
echo.
exit /b 0
