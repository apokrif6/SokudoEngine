#!/bin/bash

set -e

SHADER_DIR="$(cd "$(dirname "$0")/.." && pwd)/shaders"

echo "Compiling shaders in $SHADER_DIR"

if ! command -v glslc &> /dev/null; then
    echo "ERROR: glslc not found. Install Vulkan SDK or add glslc to PATH."
    exit 1
fi

for shader in "$SHADER_DIR"/*.vert "$SHADER_DIR"/*.frag; do
    if [ -f "$shader" ]; then
        output="$shader.spv"
        echo "  $shader -> $output"
        glslc "$shader" -o "$output"
    fi
done

echo "Shader compilation finished"

read -r -p "Press Enter to continue..."