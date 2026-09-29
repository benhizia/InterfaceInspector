@echo off
echo ========================================
echo Test 03: Multicast Broadcast
echo ========================================
echo.

if not exist sender.exe (
    echo ERROR: Build first with build.cmd
    pause
    exit /b 1
)

if not exist shared.meta.json (
    echo Generating metadata...
    call generate_metadata.cmd
)

echo This test demonstrates multicast spy mode:
echo   1. Receiver joins multicast group 239.255.0.1:8003
echo   2. SwitchSpy ALSO joins same group (passive spy)
echo   3. Sender broadcasts to the group
echo   Both receiver and SwitchSpy receive ALL broadcasts!
echo.
pause

echo Starting receiver...
start "Test 03 - Receiver" cmd /k receiver.exe

timeout /t 2 /nobreak >nul

echo.
echo ========================================
echo Start SwitchSpy GUI (SPY MODE):
echo   cd ..\..\
echo   build\windows-x64-release\Release\switchspy.exe tests\03_multicast\switchspy.ini
echo ========================================
echo.
pause

echo Starting sender (broadcasts to multicast group)...
start "Test 03 - Sender" cmd /k sender.exe

echo.
echo Test running! Both receiver AND SwitchSpy should receive broadcasts.
pause
