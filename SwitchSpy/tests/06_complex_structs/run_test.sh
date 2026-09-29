#!/bin/bash
echo "========================================"
echo "Test 06: Complex Nested Structures"
echo "========================================"
echo
echo "Robot telemetry with nested structs:"
echo "  - Vec3, Quaternion, Color, SensorData"
echo "  - Arrays: joint_angles[6]"
echo

if [ ! -f sender ] || [ ! -f receiver ]; then
    echo "ERROR: Build first"
    exit 1
fi

echo "Starting receiver..."
./receiver > receiver.log 2>&1 &
RECEIVER_PID=$!
sleep 2

echo
echo "========================================"
echo "Start SwitchSpy GUI:"
echo "  cd ../../"
echo "  build/linux-release/switchspy tests/06_complex_structs/switchspy.ini"
echo
echo "CRITICAL: Test tree view expansion!"
echo "  - Expand nested structs in tree view"
echo "  - Verify all nested fields visible"
echo "========================================"
read -p "Press Enter when ready..."

echo "Starting sender (Ctrl+C to stop)..."
trap "kill $RECEIVER_PID 2>/dev/null; exit" INT TERM
./sender

kill $RECEIVER_PID 2>/dev/null
