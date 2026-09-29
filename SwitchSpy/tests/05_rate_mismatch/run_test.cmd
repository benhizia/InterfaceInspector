@echo off
echo ========================================
echo Test 05: Rate Mismatch (Buffer Test)
echo ========================================
echo.

if not exist sender.exe (
    echo ERROR: Build first with build.cmd
    pause
    exit /b 1
)

echo This test demonstrates buffer overflow handling:
echo   - Sender: 100 packets/second (FAST)
echo   - Receiver: 50 packets/second (SLOW - intentionally 2x slower)
echo   - Expected: Buffer warnings, packet drops
echo.
echo What to observe in SwitchSpy:
echo   - Logs: Buffer pressure warnings (75%%, 90%%)
echo   - Performance: buffer.push turns YELLOW/RED
echo   - Statistics: Dropped packets increases
echo.
pause

echo Starting SLOW receiver (50 pkt/s)...
start "Test 05 - SLOW Receiver" cmd /k receiver.exe

timeout /t 2 /nobreak >nul

echo.
echo ========================================
echo Start SwitchSpy GUI:
echo   cd ..\..\
echo   build\windows-x64-release\Release\switchspy.exe tests\05_rate_mismatch\switchspy.ini
echo.
echo Watch for buffer warnings in Logs panel!
echo ========================================
echo.
pause

echo Starting FAST sender (100 pkt/s)...
start "Test 05 - FAST Sender" cmd /k sender.exe

echo.
echo Rate mismatch test running!
echo This is EXPECTED behavior - buffer should fill and drop old packets.
pause
