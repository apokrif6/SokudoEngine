#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

SOURCE_DIR="$SCRIPT_DIR/../source"

if [ ! -d "$SOURCE_DIR" ]; then
  echo "Error: Source directory '$SOURCE_DIR' does not exist."
  exit 1
fi

find "$SOURCE_DIR" \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i {} +

echo "Clang-format applied to all .cpp and .h files in $SOURCE_DIR."

read -r -p "Press Enter to continue..."