@echo off
echo ========================================
echo Test 06: Complex Nested Structures
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

echo This test validates nested struct visualization:
echo   - RobotTelemetry contains 5 nested structs:
echo     - Vec3 (position, velocity)
echo     - Quaternion (orientation)
echo     - Color (status_color)
echo     - SensorData (sensors)
echo   - Plus arrays: joint_angles[6], padding[3]
echo.
echo What to test in Tree View:
echo   - Expand nested structs (click arrows)
echo   - Verify all fields visible
echo   - Check array element expansion
echo.
pause

echo Starting receiver...
start "Test 06 - Receiver" cmd /k receiver.exe

timeout /t 2 /nobreak >nul

echo.
echo ========================================
echo Start SwitchSpy GUI:
echo   cd ..\..\
echo   build\windows-x64-release\Release\switchspy.exe tests\06_complex_structs\switchspy.ini
echo.
echo CRITICAL: Test tree view expansion!
echo   - Click arrows to expand nested structs
echo   - Verify Vec3, Quaternion, Color, SensorData expand
echo   - Check joint_angles[6] array expansion
echo ========================================
echo.
pause

echo Starting sender...
start "Test 06 - Sender" cmd /k sender.exe

echo.
echo Complex structs test running!
echo Focus on Tree View panel for nested structure validation.
pause
