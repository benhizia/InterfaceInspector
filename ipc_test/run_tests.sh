#!/bin/bash
set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
TEST_NAME="test_simple_struct"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to cleanup on exit
cleanup() {
    print_status "Cleaning up..."
    cd "$BUILD_DIR" 2>/dev/null || true
    ./sender 01_simple_struct.meta.json "$TEST_NAME" 0 2>/dev/null || true
    cd "$SCRIPT_DIR" 2>/dev/null || true
}

# Function to show usage
usage() {
    echo "Usage: $0 [MODE]"
    echo "  MODE can be:"
    echo "    spy     - Run with interactive spy tool (default)"
    echo "    test    - Run unit tests without GUI"
    echo "    build   - Only build the project"
    exit 1
}

# Parse arguments
MODE=${1:-spy}

if [[ "$MODE" != "spy" && "$MODE" != "test" && "$MODE" != "build" ]]; then
    usage
fi

# Setup vendor dependencies
if [ ! -d "$SCRIPT_DIR/vendor/json" ]; then
    print_status "Setting up vendor dependencies..."
    chmod +x "$SCRIPT_DIR/setup_vendor.sh"
    "$SCRIPT_DIR/setup_vendor.sh"
fi

# Build the project
print_status "Building project..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [[ "$MODE" == "build" ]]; then
    print_status "Build complete!"
    exit 0
fi

# Copy metadata
print_status "Copying metadata..."
cp "$SCRIPT_DIR/../MetaDataGen/01_simple_struct.meta.json" .

# Test the shared memory system
print_status "Starting IPC test in $MODE mode..."

if [[ "$MODE" == "spy" ]]; then
    print_status "Starting spy mode with interactive monitoring..."
    
    # Start the sender in background
    print_status "Starting sender..."
    ./sender "01_simple_struct.meta.json" "$TEST_NAME" 2 &
    SENDER_PID=$!
    
    sleep 2
    
    # Start the spy
    print_status "Starting spy tool..."
    ./simple_spy "01_simple_struct.meta.json" "$TEST_NAME"
    
    kill $SENDER_PID 2>/dev/null || true
    
elif [[ "$MODE" == "test" ]]; then
    print_status "Starting unit test mode..."
    
    # Start the sender in background
    print_status "Starting sender..."
    ./sender "01_simple_struct.meta.json" "$TEST_NAME" 1 &
    SENDER_PID=$!
    
    sleep 2
    
    # Run receiver with validation
    print_status "Running receiver validation..."
    if ./receiver "01_simple_struct.meta.json" "$TEST_NAME" validate; then
        print_status "All tests PASSED!"
        exit_code=0
    else
        print_error "Tests FAILED!"
        exit_code=1
    fi
    
    kill $SENDER_PID 2>/dev/null || true
    exit $exit_code
fi