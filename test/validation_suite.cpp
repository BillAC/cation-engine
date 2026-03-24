#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <iomanip>
#include "Solver.h" // Assumes Claude named the header Solver.h
#include "Ligand.h"

// Helper for floating point comparison
bool is_near(double specialized, double expected, double tolerance = 0.02) {
    double diff = std::abs(specialized - expected);
    return (diff / expected) <= tolerance;
}

void test_physiological_calcium_buffer() {
    std::cout << "Test 1: Physiological EGTA/Ca Buffer (pH 7.2)... ";
    
    CationSystem system;
    system.setSolutionParams(7.20, 25.0, 100.0); // pH, Temp, Ionic Strength
    
    // 10mM EGTA, 5mM Total Ca
    system.addLigand("EGTA", 10.0);
    system.setCationTotal("Ca2+", 5.0);
    
    system.solve();
    
    double freeCa = system.getFreeConcentration("Ca2+"); // result in mM
    double expectedFreeCa = 0.000155; // ~155 nM
    
    if (is_near(freeCa, expectedFreeCa)) {
        std::cout << "PASSED (Result: " << freeCa * 1e6 << " nM)" << std::endl;
    } else {
        std::cout << "FAILED (Result: " << freeCa * 1e6 << " nM, Expected: ~155 nM)" << std::endl;
    }
}

void test_multiligand_competition() {
    std::cout << "Test 2: ATP/EDTA Competition (pH 7.0)... ";
    
    CationSystem system;
    system.setSolutionParams(7.0, 25.0, 150.0);
    
    system.addLigand("ATP", 5.0);
    system.addLigand("EDTA", 1.0);
    
    system.setCationTotal("Mg2+", 5.0);
    system.setCationTotal("Ca2+", 0.5);
    
    system.solve();
    
    // In this scenario, EDTA (high affinity) should grab almost all Ca2+
    double freeCa = system.getFreeConcentration("Ca2+");
    
    // Expected: extremely low free Ca (pCa > 10)
    if (freeCa < 1e-7) { 
        std::cout << "PASSED (Ca effectively sequestered)" << std::endl;
    } else {
        std::cout << "FAILED (Free Ca too high: " << freeCa << " mM)" << std::endl;
    }
}

void test_solver_reversibility() {
    std::cout << "Test 3: Two-Way Reversibility (Total from Free)... ";
    
    CationSystem forward;
    forward.setSolutionParams(7.4, 25.0, 150.0);
    forward.addLigand("Citrate", 10.0);
    
    // Goal: Target 1.0 mM Free Mg
    double targetFreeMg = 1.0;
    
    // Use the "Reverse" mode of your solver
    double requiredTotalMg = forward.calculateRequiredTotal("Mg2+", targetFreeMg);
    
    // Now verify by plugging that Total back into a Forward solve
    CationSystem verification;
    verification.setSolutionParams(7.4, 25.0, 150.0);
    verification.addLigand("Citrate", 10.0);
    verification.setCationTotal("Mg2+", requiredTotalMg);
    verification.solve();
    
    double resultFreeMg = verification.getFreeConcentration("Mg2+");
    
    if (is_near(resultFreeMg, targetFreeMg, 0.001)) {
        std::cout << "PASSED (Total: " << requiredTotalMg << " mM -> Free: " << resultFreeMg << " mM)" << std::endl;
    } else {
        std::cout << "FAILED (Reversibility error. Result: " << resultFreeMg << ")" << std::endl;
    }
}

int main() {
    std::cout << "--- Cation-Engine Validation Suite ---" << std::endl;
    try {
        test_physiological_calcium_buffer();
        test_multiligand_competition();
        test_solver_reversibility();
    } catch (const std::exception& e) {
        std::cerr << "Runtime Error during validation: " << e.what() << std::endl;
    }
    return 0;
}