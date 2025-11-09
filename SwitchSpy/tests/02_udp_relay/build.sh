#!/bin/bash
echo "========================================"
echo "Building Test 02: UDP Relay"
echo "========================================"
echo

if ! command -v g++ &> /dev/null; then
    echo "ERROR: g++ not found"
    exit 1
fi

echo "Building sender..."
g++ -std=c++17 -O2 sender.cpp -o sender -pthread
if [ $? -ne 0 ]; then exit 1; fi

echo "Building receiver..."
g++ -std=c++17 -O2 receiver.cpp -o receiver -pthread
if [ $? -ne 0 ]; then exit 1; fi

chmod +x sender receiver

echo
echo "========================================"
echo "Build complete!"
echo "========================================"
echo
echo "To run: ./run_test.sh"
