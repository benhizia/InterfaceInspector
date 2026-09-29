#!/bin/bash
echo "========================================"
echo "Test 05: Rate Mismatch (Buffer Test)"
echo "========================================"
echo
echo "Sender: 100 pkt/s | Receiver: 50 pkt/s"
echo "Expected: Buffer warnings, packet drops"
echo

if [ ! -f sender ] || [ ! -f receiver ]; then
    echo "ERROR: Build first"
    exit 1
fi

echo "Starting SLOW receiver..."
./receiver > receiver.log 2>&1 &
RECEIVER_PID=$!
sleep 2

echo
echo "========================================"
echo "Start SwitchSpy GUI:"
echo "  cd ../../"
echo "  build/linux-release/switchspy tests/05_rate_mismatch/switchspy.ini"
echo.
echo "Watch Logs panel for buffer warnings!"
echo "========================================"
read -p "Press Enter when ready..."

echo "Starting FAST sender (Ctrl+C to stop)..."
trap "kill $RECEIVER_PID 2>/dev/null; exit" INT TERM
./sender

kill $RECEIVER_PID 2>/dev/null
