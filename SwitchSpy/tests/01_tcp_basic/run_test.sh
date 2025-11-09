#!/bin/bash

# Run script for Test 01: TCP Basic

echo "========================================"
echo "Test 01: TCP Basic - Test Runner"
echo "========================================"
echo

# Check if executables exist
if [ ! -f sender ]; then
    echo "ERROR: sender not found!"
    echo "Please run './build.sh' first"
    exit 1
fi

if [ ! -f receiver ]; then
    echo "ERROR: receiver not found!"
    echo "Please run './build.sh' first"
    exit 1
fi

# Check if metadata exists
if [ ! -f shared.meta.json ]; then
    echo "WARNING: Metadata file not found!"
    echo
    echo "Please generate metadata by running:"
    echo "  cd ../../../"
    echo "  python header_inspector.py SwitchSpy/tests/01_tcp_basic/shared.hpp"
    echo "  cp MetaDataGen/shared.meta.json SwitchSpy/tests/01_tcp_basic/"
    echo
    read -p "Press Enter to continue anyway, or Ctrl+C to abort..."
fi

echo "========================================"
echo "Test Execution Instructions"
echo "========================================"
echo
echo "This script will launch the test in 3 steps:"
echo "  1. Start receiver (background)"
echo "  2. Instructions for SwitchSpy GUI"
echo "  3. Start sender (foreground)"
echo
read -p "Press Enter to start..."

echo
echo "Starting receiver in background..."
./receiver > receiver.log 2>&1 &
RECEIVER_PID=$!
echo "Receiver started with PID: $RECEIVER_PID"

sleep 2

echo
echo "========================================"
echo "IMPORTANT: Start SwitchSpy GUI Now!"
echo "========================================"
echo
echo "In a new terminal, navigate to SwitchSpy root and run:"
echo
echo "  cd ../../"
echo "  build/linux-release/switchspy tests/01_tcp_basic/switchspy.ini"
echo
echo "Or use your preferred method to launch SwitchSpy with:"
echo "  tests/01_tcp_basic/switchspy.ini"
echo
read -p "Once SwitchSpy GUI is running, press Enter to start sender..."

echo
echo "Starting sender..."
echo "Press Ctrl+C to stop the test"
echo "========================================"
echo

# Trap Ctrl+C to clean up receiver
trap "echo; echo 'Stopping receiver...'; kill $RECEIVER_PID 2>/dev/null; exit" INT TERM

./sender

# Clean up
kill $RECEIVER_PID 2>/dev/null
echo
echo "Test stopped. Check receiver.log for receiver output."
