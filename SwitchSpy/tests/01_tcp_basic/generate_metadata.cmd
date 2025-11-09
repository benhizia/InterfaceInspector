@echo off
REM Generate metadata for Test 01 - TCP Basic
REM This script uses InterfaceInspector to extract struct information

echo ========================================
echo Generating Metadata for Test 01
echo ========================================
echo.

REM Check if Python is available
where python >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found in PATH
    echo Please install Python 3.x
    exit /b 1
)

REM Navigate to InterfaceInspector root
cd ..\..\..\

echo Current directory: %CD%
echo.

REM Check for header_inspector.py
if not exist "header_inspector.py" (
    echo ERROR: header_inspector.py not found
    echo Please run this script from the correct location
    exit /b 1
)

REM Run InterfaceInspector on shared.hpp
echo Running InterfaceInspector on shared.hpp...
echo.

python header_inspector.py SwitchSpy\tests\01_tcp_basic\shared.hpp

if errorlevel 1 (
    echo.
    echo ERROR: Metadata generation failed
    exit /b 1
)

echo.
echo ========================================
echo Metadata Generation Complete!
echo ========================================
echo.

REM Check if metadata was generated
if exist "MetaDataGen\shared.meta.json" (
    echo Generated file: MetaDataGen\shared.meta.json
    echo.

    REM Copy to test directory for convenience
    copy MetaDataGen\shared.meta.json SwitchSpy\tests\01_tcp_basic\ >nul
    echo Copied to: SwitchSpy\tests\01_tcp_basic\shared.meta.json
    echo.

    REM Show a preview
    echo Preview:
    type SwitchSpy\tests\01_tcp_basic\shared.meta.json
) else (
    echo WARNING: Metadata file not found in expected location
)

echo.
echo Next steps:
echo   1. Configure SwitchSpy with: switchspy.ini
echo   2. Start SwitchSpy: switchspy switchspy.ini
echo   3. Run sender: sender.exe
echo   4. Run receiver: receiver.exe
echo.
pause
