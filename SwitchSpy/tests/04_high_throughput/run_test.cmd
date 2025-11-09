@echo off
echo ========================================
echo Test 04: High Throughput Stress Test
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

echo WARNING: This is a STRESS TEST!
echo   - Sender: 10,000 ticks/second (~0.7 MB/s)
echo   - Tests deferred mode performance
echo   - Watch Performance panel for metrics
echo.
echo System requirements:
echo   - Modern CPU (2GHz+ recommended)
echo   - 4GB+ RAM
echo   - Deferred mode MUST be enabled
echo.
pause

echo Starting receiver...
start "Test 04 - Receiver" cmd /k receiver.exe

timeout /t 2 /nobreak >nul

echo.
echo ========================================
echo Start SwitchSpy GUI (DEFERRED MODE):
echo   cd ..\..\
echo   build\windows-x64-release\Release\switchspy.exe tests\04_high_throughput\switchspy.ini
echo.
echo IMPORTANT: Check Performance panel!
echo   - relay.forward should be GREEN (^<500us)
echo   - Buffer utilization should stabilize
echo ========================================
echo.
pause

echo Starting HIGH THROUGHPUT sender (10K ticks/s)...
start "Test 04 - Sender" cmd /k sender.exe 127.0.0.1 8004 100

echo.
echo STRESS TEST RUNNING!
echo Monitor SwitchSpy Performance panel closely.
pause
