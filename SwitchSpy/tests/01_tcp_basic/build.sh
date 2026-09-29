#!/bin/bash

# Build script for Test 01: TCP Basic

echo "========================================"
echo "Building Test 01: TCP Basic"
echo "========================================"
echo

# Check for g++
if ! command -v g++ &> /dev/null; then
    echo "ERROR: g++ not found"
    echo "Please install g++ (e.g., sudo apt install g++ or brew install gcc)"
    exit 1
fi

echo "Building sender..."
g++ -std=c++17 -O2 sender.cpp -o sender -pthread

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build sender"
    exit 1
fi

echo "Building receiver..."
g++ -std=c++17 -O2 receiver.cpp -o receiver -pthread

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build receiver"
    exit 1
fi

# Make executables runnable
chmod +x sender receiver

echo
echo "========================================"
echo "Build complete!"
echo "========================================"
echo
echo "Executables created:"
echo "  - sender"
echo "  - receiver"
echo
echo "To run the test:"
echo "  1. Run './generate_metadata.sh' first (if exists)"
echo "  2. Run './run_test.sh'"
echo
