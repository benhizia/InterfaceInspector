#!/bin/bash
# Activation script for header_inspect project

echo "
 Activating header_inspect environment..."

# Check if virtual environment exists
if [ ! -d "venv" ]; then
    echo "
 Virtual environment not found. Please run setup first."
    exit 1
fi

# Activate virtual environment
source venv/bin/activate

# Verify dependencies
python -c "import lxml.etree; import jinja2; print('✅ Python dependencies OK')" || exit 1

# Check external tools
if ! command -v doxygen &> /dev/null; then
    echo "
 doxygen not found. Install with: brew install doxygen"
    exit 1
fi

if ! command -v castxml &> /dev/null; then
    echo "
 castxml not found. Install with: brew install castxml"
    exit 1
fi

echo "
 External tools OK (doxygen, castxml)"

# --- Configuration ---
# Dynamically find LLVM, C++, and SDK include paths
LLVM_PREFIX=$(brew --prefix llvm)
LLVM_INCLUDE_PATH="${LLVM_PREFIX}/include"
CPP_INCLUDE_PATH="${LLVM_PREFIX}/include/c++/v1"
SDK_INCLUDE_PATH="$(xcrun --show-sdk-path)/usr/include"


# --- Execution ---
if [ $# -eq 0 ]; then
    echo "
 Environment ready! You can now run: python header_inspector.py <header.hpp>"
    echo ""
    echo "To activate the environment manually:"
    echo "  source venv/bin/activate"
else
    HEADER_FILE=$1
    echo "Running header_inspector.py on ${HEADER_FILE}..."
    python header_inspector.py "${HEADER_FILE}" \
        -I"${LLVM_INCLUDE_PATH}" \
        -I"${SDK_INCLUDE_PATH}" \
        -I"${CPP_INCLUDE_PATH}"
fi
