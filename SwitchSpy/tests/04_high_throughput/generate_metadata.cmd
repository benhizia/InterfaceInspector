@echo off
REM Generate metadata for Test 04: High Throughput

echo ========================================
echo Generating Metadata for Test 04
echo ========================================
echo.

REM Navigate to InterfaceInspector root (3 levels up)
pushd ..\..\..\

echo Running header_inspector.py on shared.hpp...
python header_inspector.py SwitchSpy\tests\04_high_throughput\shared.hpp

if errorlevel 1 (
    echo.
    echo ERROR: Metadata generation failed!
    popd
    exit /b 1
)

echo.
echo Copying metadata file to test directory...
copy MetaDataGen\shared.meta.json SwitchSpy\tests\04_high_throughput\

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
type shared.meta.json | findstr /i "MarketTick symbol_id bid_price ask_price"
echo ---
echo.
echo Ready to use with SwitchSpy!
