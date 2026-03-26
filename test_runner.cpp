#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <iomanip>
#include "CationEngine.h"
#include "Ligand.h"

// Include test functions from the test files
void test_ligand_loading();
void test_CationEngineTest();
void test_SolverTest();
void test_validation_suite();
void test_ReferenceSolverTest();

int main() {
    std::cout << "--- Cation-Engine Test Suite ---" << std::endl;

    try {
        test_ligand_loading();
        test_CationEngineTest();
        test_SolverTest();
        test_validation_suite();
        test_ReferenceSolverTest();  // P1: Real solver tests with known values

        std::cout << "\nAll tests passed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Runtime Error during testing: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}