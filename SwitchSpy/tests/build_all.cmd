@echo off
REM Build all SwitchSpy tests

echo ========================================
echo Building All SwitchSpy Tests
echo ========================================
echo.

set TEST_DIRS=01_tcp_basic 02_udp_relay 03_multicast 04_high_throughput 05_rate_mismatch 06_complex_structs

for %%T in (%TEST_DIRS%) do (
    echo.
    echo ----------------------------------------
    echo Building %%T...
    echo ----------------------------------------
    pushd %%T
    call build.cmd
    if errorlevel 1 (
        echo ERROR: Failed to build %%T
        popd
        pause
        exit /b 1
    )
    popd
)

echo.
echo ========================================
echo All tests built successfully!
echo ========================================
echo.
echo You can now run individual tests with:
echo   cd ^<test_folder^>
echo   run_test.cmd
echo.
pause
