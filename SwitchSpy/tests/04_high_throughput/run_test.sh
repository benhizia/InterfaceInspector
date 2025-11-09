#!/bin/bash
echo "========================================"
echo "Test 04: High Throughput Stress Test"
echo "========================================"
echo
echo "WARNING: 10,000 messages/second!"
echo "Press Ctrl+C to stop"
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
echo "Start SwitchSpy (DEFERRED MODE REQUIRED):"
echo "  cd ../../"
echo "  build/linux-release/switchspy tests/04_high_throughput/switchspy.ini"
echo "========================================"
read -p "Press Enter when ready..."

echo "Starting HIGH THROUGHPUT sender..."
trap "kill $RECEIVER_PID 2>/dev/null; exit" INT TERM
./sender 127.0.0.1 8004 100

kill $RECEIVER_PID 2>/dev/null
