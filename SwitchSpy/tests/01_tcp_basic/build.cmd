@echo off
REM Build script for Test 01: TCP Basic

echo ========================================
echo Building Test 01: TCP Basic
echo ========================================
echo.

REM Check for g++
where g++ >nul 2>&1
if errorlevel 1 (
    echo ERROR: g++ not found in PATH
    echo Please install MinGW-w64 or MSYS2
    pause
    exit /b 1
)

echo Building sender...
g++ -std=c++17 -O2 sender.cpp -o sender.exe -lws2_32

if errorlevel 1 (
    echo ERROR: Failed to build sender
    pause
    exit /b 1
)

echo Building receiver...
g++ -std=c++17 -O2 receiver.cpp -o receiver.exe -lws2_32

if errorlevel 1 (
    echo ERROR: Failed to build receiver
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build complete!
echo ========================================
echo.
echo Executables created:
echo   - sender.exe
echo   - receiver.exe
echo.
echo To run the test:
echo   1. Run 'generate_metadata.cmd' first
echo   2. Run 'run_test.cmd'
echo.
pause
