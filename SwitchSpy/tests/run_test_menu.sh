#!/bin/bash

while true; do
    clear
    echo "========================================"
    echo "SwitchSpy Test Suite Launcher"
    echo "========================================"
    echo
    echo "Select a test to run:"
    echo
    echo "  1. TCP Basic (simple relay validation)"
    echo "  2. UDP Relay (connectionless datagram forwarding)"
    echo "  3. Multicast Broadcast (passive spy mode)"
    echo "  4. High Throughput (10K msg/s stress test)"
    echo "  5. Rate Mismatch (buffer overflow test)"
    echo "  6. Complex Nested Structures (tree view validation)"
    echo
    echo "  b. Build all tests"
    echo "  q. Quit"
    echo
    read -p "Enter your choice: " choice

    case $choice in
        1)
            cd 01_tcp_basic
            ./run_test.sh
            cd ..
            ;;
        2)
            cd 02_udp_relay
            ./run_test.sh
            cd ..
            ;;
        3)
            cd 03_multicast
            ./run_test.sh
            cd ..
            ;;
        4)
            cd 04_high_throughput
            ./run_test.sh
            cd ..
            ;;
        5)
            cd 05_rate_mismatch
            ./run_test.sh
            cd ..
            ;;
        6)
            cd 06_complex_structs
            ./run_test.sh
            cd ..
            ;;
        b|B)
            ./build_all.sh
            read -p "Press Enter to continue..."
            ;;
        q|Q)
            exit 0
            ;;
        *)
            echo "Invalid choice!"
            sleep 1
            ;;
    esac
done
