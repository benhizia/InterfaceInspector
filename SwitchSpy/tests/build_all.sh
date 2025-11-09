#!/bin/bash

echo "========================================"
echo "Building All SwitchSpy Tests"
echo "========================================"
echo

TEST_DIRS="01_tcp_basic 02_udp_relay 03_multicast 04_high_throughput 05_rate_mismatch 06_complex_structs"

for test_dir in $TEST_DIRS; do
    echo
    echo "----------------------------------------"
    echo "Building $test_dir..."
    echo "----------------------------------------"
    cd "$test_dir"
    ./build.sh
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to build $test_dir"
        exit 1
    fi
    cd ..
done

echo
echo "========================================"
echo "All tests built successfully!"
echo "========================================"
echo
echo "Run individual tests with:"
echo "  cd <test_folder>"
echo "  ./run_test.sh"
echo
