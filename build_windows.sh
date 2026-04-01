#!/bin/bash

# Build script for Windows using MinGW cross-compiler

echo "Building Windows executable in $(pwd)..."

# Create build directory
mkdir -p build-win
cd build-win

# Run cmake with MinGW toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw.cmake -DCMAKE_BUILD_TYPE=Release ..

# Build the project
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful!"
    cd ..
    cp build-win/bin/cation_engine_main.exe cation_engine.exe
    echo "Created cation_engine.exe"
else
    echo "Build failed!"
    exit 1
fi
