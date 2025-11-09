@echo off
echo ========================================
echo Test 02: UDP Relay - Test Runner
echo ========================================
echo.

if not exist sender.exe (
    echo ERROR: sender.exe not found! Run 'build.cmd' first
    pause
    exit /b 1
)

if not exist receiver.exe (
    echo ERROR: receiver.exe not found! Run 'build.cmd' first
    pause
    exit /b 1
)

if not exist shared.meta.json (
    echo WARNING: Metadata not found. Generating...
    call generate_metadata.cmd
)

echo This test opens 3 windows:
echo   1. Receiver (UDP port 9002)
echo   2. SwitchSpy GUI (relay UDP 8002 -^> 9002)
echo   3. Sender (sends to UDP 8002)
echo.
pause

echo Starting receiver...
start "Test 02 - Receiver" cmd /k receiver.exe

timeout /t 2 /nobreak >nul

echo.
echo ========================================
echo Start SwitchSpy GUI:
echo   cd ..\..\
echo   build\windows-x64-release\Release\switchspy.exe tests\02_udp_relay\switchspy.ini
echo ========================================
echo.
pause

echo Starting sender...
start "Test 02 - Sender" cmd /k sender.exe

echo.
echo Test running! See README.md for GUI validation steps.
pause
