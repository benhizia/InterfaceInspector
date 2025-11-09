@echo off
REM Interactive test launcher for SwitchSpy

:menu
cls
echo ========================================
echo SwitchSpy Test Suite Launcher
echo ========================================
echo.
echo Select a test to run:
echo.
echo   1. TCP Basic (simple relay validation)
echo   2. UDP Relay (connectionless datagram forwarding)
echo   3. Multicast Broadcast (passive spy mode)
echo   4. High Throughput (10K msg/s stress test)
echo   5. Rate Mismatch (buffer overflow test)
echo   6. Complex Nested Structures (tree view validation)
echo.
echo   B. Build all tests
echo   Q. Quit
echo.
set /p choice="Enter your choice: "

if "%choice%"=="1" goto test01
if "%choice%"=="2" goto test02
if "%choice%"=="3" goto test03
if "%choice%"=="4" goto test04
if "%choice%"=="5" goto test05
if "%choice%"=="6" goto test06
if /i "%choice%"=="B" goto build_all
if /i "%choice%"=="Q" exit /b 0

echo Invalid choice!
timeout /t 2 /nobreak >nul
goto menu

:test01
cd 01_tcp_basic
call run_test.cmd
cd ..
goto menu

:test02
cd 02_udp_relay
call run_test.cmd
cd ..
goto menu

:test03
cd 03_multicast
call run_test.cmd
cd ..
goto menu

:test04
cd 04_high_throughput
call run_test.cmd
cd ..
goto menu

:test05
cd 05_rate_mismatch
call run_test.cmd
cd ..
goto menu

:test06
cd 06_complex_structs
call run_test.cmd
cd ..
goto menu

:build_all
call build_all.cmd
pause
goto menu
