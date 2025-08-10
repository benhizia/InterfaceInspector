#!/bin/bash
# Setup script for Git submodules

echo "Setting up Git submodules..."

# Initialize submodules
git submodule init

# Update submodules
git submodule update

# Build the project
echo "Building with submodules..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

echo "Setup complete!"
echo ""
echo "To clone this repository with submodules:"
echo "  git clone --recurse-submodules [repository-url]"
echo "  OR:"
echo "  git clone [repository-url]"
echo "  cd ipc_test"
echo "  git submodule init"
echo "  git submodule update"