#!/bin/bash
# Setup script for SwitchSpy

echo "SwitchSpy Setup"
echo "==============="

# Check for dependencies
echo "Checking dependencies..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found. Please install CMake."
    exit 1
fi
echo "✓ CMake found"

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "❌ C++ compiler not found. Please install g++ or clang++."
    exit 1
fi
echo "✓ C++ compiler found"

# Initialize git submodules
echo ""
echo "Initializing git submodules..."
cd ..
git submodule update --init --recursive

# Create vendor directory if needed
cd SwitchSpy
mkdir -p vendor

# Check if nlohmann/json exists
if [ ! -d "vendor/json" ]; then
    echo "Setting up nlohmann/json..."
    git clone --depth 1 https://github.com/nlohmann/json.git vendor/json
fi

# Check if ImGui exists
if [ ! -d "vendor/imgui" ]; then
    echo "Setting up ImGui..."
    git clone --depth 1 https://github.com/ocornut/imgui.git vendor/imgui
fi

# Check if inih exists
if [ ! -d "vendor/inih" ]; then
    echo "Setting up inih..."
    git clone --depth 1 https://github.com/benhoyt/inih.git vendor/inih
fi

echo ""
echo "✅ Setup complete!"
echo ""
echo "To build SwitchSpy:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  cmake --build ."
echo ""
echo "To run:"
echo "  ./build/switchspy config/example.ini"
