#!/bin/bash
echo "========================================"
echo "Test 03: Multicast Broadcast"
echo "========================================"

if [ ! -f sender ] || [ ! -f receiver ]; then
    echo "ERROR: Build first with ./build.sh"
    exit 1
fi

echo "Starting receiver in background..."
./receiver > receiver.log 2>&1 &
RECEIVER_PID=$!
sleep 2

echo
echo "========================================"
echo "Start SwitchSpy GUI (SPY MODE):"
echo "  cd ../../"
echo "  build/linux-release/switchspy tests/03_multicast/switchspy.ini"
echo "========================================"
read -p "Press Enter when ready..."

echo "Starting sender (Ctrl+C to stop)..."
trap "kill $RECEIVER_PID 2>/dev/null; exit" INT TERM
./sender

kill $RECEIVER_PID 2>/dev/null
