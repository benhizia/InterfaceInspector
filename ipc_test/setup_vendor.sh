#!/bin/bash
set -e

VENDOR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/vendor"
mkdir -p "$VENDOR_DIR"

echo "Setting up vendor dependencies..."

# Clone nlohmann/json
if [ ! -d "$VENDOR_DIR/json" ]; then
    echo "Cloning nlohmann/json..."
    git clone --depth 1 https://github.com/nlohmann/json.git "$VENDOR_DIR/json"
fi

# Clone ImGui (optional, for advanced GUI)
if [ ! -d "$VENDOR_DIR/imgui" ]; then
    echo "Cloning ImGui..."
    git clone --depth 1 https://github.com/ocornut/imgui.git "$VENDOR_DIR/imgui"
fi

echo "Vendor setup complete!"