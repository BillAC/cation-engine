#!/bin/bash

# Build script for CationEngine

echo "Building CationEngine..."

# Create build directory
mkdir -p src
cd src

# Run cmake
cmake ..

# Build the project
make

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Run './src/cation_engine_main' to test the engine"
    echo "Run './src/cation_engine_test' to run tests"
else
    echo "Build failed!"
    exit 1
fi