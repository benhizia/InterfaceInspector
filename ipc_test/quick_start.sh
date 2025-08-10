#!/bin/bash
set -e

# IPC Testing Framework - Quick Start Script
# This script provides the fastest way to get started with the IPC testing framework

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Quick start function
quick_start() {
    print_status "🚀 IPC Testing Framework - Quick Start"
    echo
    
    # Step 1: Setup vendor dependencies
    if [ ! -d "vendor/json" ]; then
        print_status "Setting up vendor dependencies..."
        chmod +x setup_vendor.sh
        ./setup_vendor.sh
    fi
    
    # Step 2: Build project
    print_status "Building project..."
    mkdir -p build
    cd build
    if [ ! -f "Makefile" ]; then
        cmake .. -DCMAKE_BUILD_TYPE=Release
    fi
    make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    # Step 3: Copy metadata
    cp ../01_simple_struct.meta.json . 2>/dev/null || true
    
    print_status "✅ Build complete!"
    echo
    
    # Step 4: Show usage options
    echo "Available executables:"
    echo "  ./sender <metadata.json> <test_name> [delay]"
    echo "  ./receiver <metadata.json> <test_name> [validate]"
    echo "  ./simple_spy <metadata.json> <test_name>"
    echo
    
    echo "Quick test commands:"
    echo "  Terminal 1: ./sender 01_simple_struct.meta.json test1"
    echo "  Terminal 2: ./simple_spy 01_simple_struct.meta.json test1"
    echo "  Terminal 3: ./receiver 01_simple_struct.meta.json test1 validate"
    echo
    
    # Step 5: Run automated test if requested
    if [[ "$1" == "--test" ]]; then
        print_status "Running automated test..."
        echo
        
        # Start sender in background
        ./sender 01_simple_struct.meta.json quick_test 1 &
        SENDER_PID=$!
        
        sleep 1
        
        # Run receiver validation
        if ./receiver 01_simple_struct.meta.json quick_test validate; then
            print_status "✅ All tests PASSED!"
        else
            print_error "❌ Tests FAILED!"
            exit 1
        fi
        
        kill $SENDER_PID 2>/dev/null || true
        
        # Cleanup shared memory
        SharedMemory::remove quick_test 2>/dev/null || true
    fi
}

# Handle command line arguments
case "$1" in
    --test)
        quick_start --test
        ;;
    --spy)
        cd build
        ./simple_spy 01_simple_struct.meta.json test1
        ;;
    --help|-h)
        echo "IPC Testing Framework Quick Start"
        echo
        echo "Usage: $0 [OPTION]"
        echo
        echo "Options:"
        echo "  --test    Run automated test (default)"
        echo "  --spy     Start interactive spy mode"
        echo "  --help    Show this help message"
        echo
        echo "No arguments: Setup and build only"
        ;;
    *)
        quick_start
        ;;
esac