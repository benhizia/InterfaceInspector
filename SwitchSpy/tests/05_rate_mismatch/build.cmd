@echo off
REM Build script for Test 05: Rate Mismatch

echo ========================================
echo Building Test 05: Rate Mismatch
echo ========================================
echo.

where g++ >nul 2>&1
if errorlevel 1 (
    echo ERROR: g++ not found in PATH
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
echo To run: run_test.cmd
pause
