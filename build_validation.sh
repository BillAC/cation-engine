#!/bin/bash

# Build script for cation-engine tests

echo "Building cation-engine validation suite..."

# Compile the validation suite
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude -o cation_engine_validation test/validation_suite_fixed.cpp src/CationEngine.o src/LigandData.o src/Solver.o

if [ $? -eq 0 ]; then
    echo "✓ Validation suite compiled successfully"

    # Run the validation suite
    echo "Running validation suite..."
    ./cation_engine_validation

    echo "✓ Validation suite completed"
else
    echo "✗ Compilation failed"
    exit 1
fi

echo "Build and test completed successfully!"