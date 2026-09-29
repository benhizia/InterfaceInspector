@echo off
REM Run script for Test 01: TCP Basic

echo ========================================
echo Test 01: TCP Basic - Test Runner
echo ========================================
echo.

REM Check if executables exist
if not exist sender.exe (
    echo ERROR: sender.exe not found!
    echo Please run 'build.cmd' first
    pause
    exit /b 1
)

if not exist receiver.exe (
    echo ERROR: receiver.exe not found!
    echo Please run 'build.cmd' first
    pause
    exit /b 1
)

REM Check if metadata exists
if not exist shared.meta.json (
    echo WARNING: Metadata file not found!
    echo.
    echo Generating metadata now...
    call generate_metadata.cmd
    echo.
)

echo ========================================
echo Test Execution Instructions
echo ========================================
echo.
echo This script will open 3 windows:
echo   1. Receiver (listens on port 9001)
echo   2. Instructions for starting SwitchSpy GUI
echo   3. Sender (sends to port 8001)
echo.
echo Press any key to start...
pause >nul

echo.
echo Starting receiver in new window...
start "Test 01 - Receiver" cmd /k receiver.exe

echo Waiting 2 seconds for receiver to start...
timeout /t 2 /nobreak >nul

echo.
echo ========================================
echo IMPORTANT: Start SwitchSpy GUI Now!
echo ========================================
echo.
echo In a new terminal, navigate to SwitchSpy root and run:
echo.
echo   cd ..\..\
echo   build\windows-x64-release\Release\switchspy.exe tests\01_tcp_basic\switchspy.ini
echo.
echo Or use your preferred method to launch SwitchSpy with the config:
echo   tests\01_tcp_basic\switchspy.ini
echo.
echo Once SwitchSpy GUI is running, press any key to start sender...
pause >nul

echo.
echo Starting sender in new window...
start "Test 01 - Sender" cmd /k sender.exe

echo.
echo ========================================
echo Test Running!
echo ========================================
echo.
echo Three windows should now be open:
echo   1. Receiver - showing received messages
echo   2. SwitchSpy GUI - showing tree view and statistics
echo   3. Sender - sending messages every 500ms
echo.
echo Monitor the SwitchSpy GUI following instructions in README.md
echo.
echo Press any key to exit this script (test will continue running)...
pause >nul
