#!/bin/bash
echo "========================================"
echo "Test 02: UDP Relay - Test Runner"
echo "========================================"

if [ ! -f sender ] || [ ! -f receiver ]; then
    echo "ERROR: Executables not found! Run './build.sh' first"
    exit 1
fi

if [ ! -f shared.meta.json ]; then
    echo "WARNING: Metadata not found"
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then exit 1; fi
fi

echo "Starting receiver in background..."
./receiver > receiver.log 2>&1 &
RECEIVER_PID=$!
echo "Receiver PID: $RECEIVER_PID"
sleep 2

echo
echo "========================================"
echo "Start SwitchSpy GUI in another terminal:"
echo "  cd ../../"
echo "  build/linux-release/switchspy tests/02_udp_relay/switchspy.ini"
echo "========================================"
read -p "Press Enter when SwitchSpy is running..."

echo "Starting sender (Ctrl+C to stop)..."
trap "kill $RECEIVER_PID 2>/dev/null; exit" INT TERM
./sender

kill $RECEIVER_PID 2>/dev/null
echo "Test stopped."
