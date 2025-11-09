@echo off
REM Generate metadata for Test 03: Multicast Broadcast

echo ========================================
echo Generating Metadata for Test 03
echo ========================================
echo.

REM Navigate to InterfaceInspector root (3 levels up)
pushd ..\..\..\

echo Running header_inspector.py on shared.hpp...
python header_inspector.py SwitchSpy\tests\03_multicast\shared.hpp

if errorlevel 1 (
    echo.
    echo ERROR: Metadata generation failed!
    popd
    exit /b 1
)

echo.
echo Copying metadata file to test directory...
copy MetaDataGen\shared.meta.json SwitchSpy\tests\03_multicast\

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
echo Preview of metadata:
echo ---
type shared.meta.json | findstr /i "TelemetryBroadcast source_id latitude longitude"
echo ---
echo.
echo Ready to use with SwitchSpy!
