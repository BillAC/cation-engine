#include <iostream>

// Forward declarations of test functions
void test_CationEngineTest();
void test_SolverTest();
void test_validation_suite();
void test_ReferenceSolverTest();

int main() {
    std::cout << "--- Cation-Engine Test Suite ---" << std::endl;
    
    try {
        test_CationEngineTest();
        test_SolverTest();
        test_validation_suite();
        test_ReferenceSolverTest();
        
        std::cout << "\nAll tests passed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\nTest failed with unknown exception." << std::endl;
        return 1;
    }
    
    return 0;
}
