#!/bin/bash
set -euo pipefail

# Path variables
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT="$ROOT/cation_engine.exe"
BUILD_LOG="$ROOT/build_windows.log"

echo "Building Windows executable in $ROOT..."
echo "Command log: $BUILD_LOG"

# Optional clean
if [ "$#" -gt 0 ] && [ "$1" = "clean" ]; then
  echo "Cleaning previous build output..."
  rm -f "$OUT" "$BUILD_LOG"
  echo "Clean done."
  exit 0
fi

cd "$ROOT"

x86_64-w64-mingw32-g++ -I./include src/*.cpp main.cpp -o "$OUT" \
  -static-libgcc -static-libstdc++ -static -lpthread \
  -luser32 -lgdi32 -lcomctl32 -lcomdlg32 \
  2>&1 | tee "$BUILD_LOG"

echo "Build finished."
if [ -f "$OUT" ]; then
  echo "Created $OUT"
else
  echo "Build failed: executable not found"
  exit 1
fi
