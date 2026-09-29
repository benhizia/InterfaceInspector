@echo off
REM Generate metadata for Test 06: Complex Structs

echo ========================================
echo Generating Metadata for Test 06
echo ========================================
echo.

REM Navigate to InterfaceInspector root (3 levels up)
pushd ..\..\..\

echo Running header_inspector.py on shared.hpp...
python header_inspector.py SwitchSpy\tests\06_complex_structs\shared.hpp

if errorlevel 1 (
    echo.
    echo ERROR: Metadata generation failed!
    popd
    exit /b 1
)

echo.
echo Copying metadata file to test directory...
copy MetaDataGen\shared.meta.json SwitchSpy\tests\06_complex_structs\

if errorlevel 1 (
    echo ERROR: Failed to copy metadata file
    popd
    exit /b 1
)

popd

echo.
echo ========================================
echo Metadata generation complete!
echo ========================================
echo.
echo Generated file: shared.meta.json
echo.

REM Show a preview of the metadata
echo Preview of metadata (nested structs):
echo ---
type shared.meta.json | findstr /i "RobotTelemetry Vec3 Quaternion Color SensorData"
echo ---
echo.
echo This metadata includes NESTED STRUCTURES!
echo SwitchSpy tree view should show hierarchical expansion.
echo.
echo Ready to use with SwitchSpy!
