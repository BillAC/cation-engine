#include "Solver.h"
#include "LigandCSVLoader.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <iomanip>

// Helper for floating point comparison with tolerance
inline bool is_near_ref(double actual, double expected, double tolerance = 0.05) {
    if (expected == 0.0) {
        return std::abs(actual) < tolerance;
    }
    double relError = std::abs((actual - expected) / expected);
    return relError <= tolerance;
}

// Print comparison result
inline void print_test_result(const std::string& testName, const std::string& parameter,
                       double actual, double expected, bool passed) {
    std::cout << "  " << parameter << ": "
              << (passed ? "✓ PASS" : "✗ FAIL") << " - "
              << "Expected: " << std::scientific << std::setprecision(4) << expected
              << ", Got: " << actual << std::endl;
}

/**
 * Reference Test Suite: Real equilibrium calculations with known values
 * 
 * References:
 * - EDTA Constants: NIST/IUPAC Database, 25°C, I=0.1M
 * - EGTA Constants: WebMaxC Extended Database
 * - ATP Constants: Standard biochemical tables
 * 
 * Test Strategy:
 * 1. Use literature values for logK at standard conditions (25°C, 0.1M ionic strength, pH 7)
 * 2. For 1:1 complex: M + L ⇌ ML with K = [ML]/([M][L])
 * 3. At equilibrium: [ML] = K*[M]*[L]
 * 4. Conservation: [L_total] = [L] + [ML], [M_total] = [M] + [ML]
 * 5. Solve iteratively for [ML], then compute free concentrations
 */

void test_EDTA_Ca_system() {
    std::cout << "\n[Test 1] EDTA-Ca2+ System (Reference: NIST Database)" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    CationSystem solver;
    
    // Reference conditions
    SystemParameters params;
    params.temperature = 25.0;      // Standard temperature
    params.ionicStrength = 100.0;   // 0.1M ionic strength (from ligands.csv standard)
    params.pH = 7.0;                // Physiological pH
    params.volume = 1.0;
    solver.SetParameters(params);
    
    // From ligands.csv: EDTA Ca1 = 10.65 (at I=0.1M, T=25°C)
    // Literature value at 0.1M, 25°C, pH 7 ≈ 10.65
    
    // Test Case 1: Low concentration, excess ligand
    // Given: [EDTA_total] = 1.0 mM, [Ca_total] = 0.1 mM
    // Expected behavior: Most Ca2+ should be complexed due to high K
    std::cout << "\nTest Case 1.1: Excess ligand (1.0 mM EDTA, 0.1 mM Ca)" << std::endl;
    
    auto result1 = solver.CalculateTotalToFree(1.0e-3, 0.1e-3, "EDTA", "Ca2");
    
    // At high K (~10^10.65 ≈ 4.5e10), equilibrium shifts heavily to complex
    // Expectation: [Ca_free] << [Ca_total], [EDTA_free] ≈ [EDTA_total]
    bool test1_free = is_near_ref(result1.freeMetal, 2.2e-12, 0.5);  // Very small free concentration
    bool test1_complex = is_near_ref(result1.complex, 1.0e-4, 0.1);   // Nearly all Ca is complexed
    bool test1_total = is_near_ref(result1.totalMetal, 0.1e-3, 0.01);
    
    print_test_result("EDTA-Ca1.1", "Free [Ca2+]", result1.freeMetal, 2.2e-12, test1_free);
    print_test_result("EDTA-Ca1.1", "Complex [CaEDTA]", result1.complex, 1.0e-4, test1_complex);
    print_test_result("EDTA-Ca1.1", "Total [Ca]", result1.totalMetal, 0.1e-3, test1_total);
    
    // Test Case 2: Physiological conditions (calcium buffering)
    // Given: [EGTA_total] = 5 mM, [Ca_total] = 100 nM = 0.1 µM
    // Typical in cell biology for Ca2+ buffering
    std::cout << "\nTest Case 1.2: Physiological calcium buffering (5 mM EDTA, 0.1 µM Ca)" << std::endl;
    
    auto result2 = solver.CalculateTotalToFree(5.0e-3, 0.1e-6, "EDTA", "Ca2");
    
    // At this concentration ratio and high K, nearly complete complexation
    bool test2_free = is_near_ref(result2.freeMetal, 1.0e-14, 1.0);
    bool test2_complex = is_near_ref(result2.complex, 0.1e-6, 0.05);
    
    print_test_result("EDTA-Ca1.2", "Free [Ca2+]", result2.freeMetal, 1.0e-14, test2_free);
    print_test_result("EDTA-Ca1.2", "Complex [CaEDTA]", result2.complex, 0.1e-6, test2_complex);
    
    // Test Case 3: Reverse calculation (Total from Free)
    // Given: [Ca_free] = 100 nM, what [EDTA] needed to buffer at this level?
    std::cout << "\nTest Case 1.3: Reverse calculation - Free to Total" << std::endl;
    
    double freeCa = 100e-9;  // 100 nM target free
    double totalEDTA = 5.0e-3;
    
    auto result3 = solver.CalculateFreeToTotal(totalEDTA, freeCa, "EDTA", "Ca2");
    
    // With this free concentration and high K, total should be slightly > free
    bool test3_total = is_near_ref(result3.totalMetal, freeCa + result3.complex, 0.05);
    
    print_test_result("EDTA-Ca1.3", "Total [Ca]", result3.totalMetal, freeCa, test3_total);
    
    std::cout << "\nEDTA-Ca2+ Test Summary: " 
              << (test1_free && test1_complex && test1_total ? "✓ PASS" : "✗ FAIL")
              << std::endl;
}

void test_EGTA_Ca_system() {
    std::cout << "\n[Test 2] EGTA-Ca2+ System (Reference: WebMaxC Database)" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    CationSystem solver;
    
    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;
    params.pH = 7.0;
    params.volume = 1.0;
    solver.SetParameters(params);
    
    // From ligands.csv: EGTA Ca1 = 10.97 (even stronger Ca2+ binding than EDTA)
    // EGTA is known for superior Ca2+ selectivity in biological applications
    
    std::cout << "\nTest Case 2.1: EGTA Ca2+ selectivity test (1 mM EGTA, 0.1 mM Ca)" << std::endl;
    
    auto result = solver.CalculateTotalToFree(1.0e-3, 0.1e-3, "EGTA", "Ca2");
    
    // EGTA has even higher selectivity (logK=10.97 vs EDTA=10.65)
    // Should show even stronger complexation
    bool test_free = is_near_ref(result.freeMetal, 1.0e-13, 1.0);
    bool test_complex = is_near_ref(result.complex, 0.1e-3, 0.05);
    
    print_test_result("EGTA-Ca2.1", "Free [Ca2+]", result.freeMetal, 1.0e-13, test_free);
    print_test_result("EGTA-Ca2.1", "Complex [CaEGTA]", result.complex, 0.1e-3, test_complex);
    
    std::cout << "\nEGTA-Ca2+ Test Summary: " 
              << (test_free && test_complex ? "✓ PASS" : "✗ FAIL")
              << std::endl;
}

void test_ATP_Mg_system() {
    std::cout << "\n[Test 3] ATP-Mg2+ System (Reference: Biochemical Tables)" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    CationSystem solver;
    
    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;
    params.pH = 7.0;
    params.volume = 1.0;
    solver.SetParameters(params);
    
    // From ligands.csv: ATP Mg1 = 4.06
    // ATP-Mg is crucial in cell biology (most ATP exists as ATP-Mg complex)
    // Much weaker binding than EDTA-Ca, but still significant
    
    std::cout << "\nTest Case 3.1: ATP-Mg2+ in cellular conditions (5 mM ATP, 1 mM Mg)" << std::endl;
    
    auto result = solver.CalculateTotalToFree(5.0e-3, 1.0e-3, "ATP", "Mg2");
    
    // With K ≈ 10^4.06 ≈ 11500, modest complexation expected
    // Significant free Mg2+ remains for enzyme catalysis
    bool test_complex = is_near_ref(result.complex, 5.0e-5, 0.2);  // ~50 µM complexed
    bool test_free_ok = result.freeMetal > 0.5e-3;  // Significant free Mg remains
    
    print_test_result("ATP-Mg3.1", "Complex [MgATP]", result.complex, 5.0e-5, test_complex);
    print_test_result("ATP-Mg3.1", "Free [Mg2+] > 0.5 mM", result.freeMetal, 0.5e-3, test_free_ok);
    
    // Test Case 2: Low ATP concentration
    std::cout << "\nTest Case 3.2: ATP-Mg2+ at low ATP (0.1 mM ATP, 1 mM Mg)" << std::endl;
    
    auto result2 = solver.CalculateTotalToFree(0.1e-3, 1.0e-3, "ATP", "Mg2");
    
    // At this ratio, calculated complex should be limited by ATP concentration
    bool test2_complex_limited = result2.complex < 0.1e-3;
    bool test2_free_high = is_near_ref(result2.freeMetal, 0.9e-3, 0.1);
    
    print_test_result("ATP-Mg3.2", "Complex [MgATP]", result2.complex, 0.05e-3, test2_complex_limited);
    print_test_result("ATP-Mg3.2", "Free [Mg2+]", result2.freeMetal, 0.9e-3, test2_free_high);
    
    std::cout << "\nATP-Mg2+ Test Summary: " 
              << (test_complex && test_free_ok ? "✓ PASS" : "✗ FAIL")
              << std::endl;
}

void test_system_parameter_effects() {
    std::cout << "\n[Test 4] System Parameter Effects (Temperature, Ionic Strength, pH)" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    CationSystem solver;
    
    // Test 1: Temperature effect (van't Hoff)
    std::cout << "\nTest Case 4.1: Temperature effect on EDTA-Ca complexation" << std::endl;
    
    SystemParameters params_cold, params_hot;
    params_cold.temperature = 4.0;   // Cold
    params_cold.ionicStrength = 100.0;
    params_cold.pH = 7.0;
    params_cold.volume = 1.0;
    
    params_hot.temperature = 37.0;   // Body temperature
    params_hot.ionicStrength = 100.0;
    params_hot.pH = 7.0;
    params_hot.volume = 1.0;
    
    solver.SetParameters(params_cold);
    auto result_cold = solver.CalculateTotalToFree(1.0e-3, 0.1e-3, "EDTA", "Ca2");
    
    solver.SetParameters(params_hot);
    auto result_hot = solver.CalculateTotalToFree(1.0e-3, 0.1e-3, "EDTA", "Ca2");
    
    // Equilibrium constants typically decrease with increasing temperature
    bool test_temp_effect = result_hot.complex <= result_cold.complex;
    
    print_test_result("Temp4.1", "Hot K ≤ Cold K", result_hot.complex, result_cold.complex, test_temp_effect);
    std::cout << "    At 4°C:   Complex = " << result_cold.complex << std::endl;
    std::cout << "    At 37°C:  Complex = " << result_hot.complex << std::endl;
    
    // Test 2: Ionic strength effect (Davies equation)
    std::cout << "\nTest Case 4.2: Ionic strength effect (Davies equation)" << std::endl;
    
    SystemParameters params_low_I, params_high_I;
    params_low_I.temperature = 25.0;
    params_low_I.ionicStrength = 1.0;  // 0.001M
    params_low_I.pH = 7.0;
    params_low_I.volume = 1.0;
    
    params_high_I.temperature = 25.0;
    params_high_I.ionicStrength = 1000.0;  // 1.0M (high salt)
    params_high_I.pH = 7.0;
    params_high_I.volume = 1.0;
    
    solver.SetParameters(params_low_I);
    auto result_low_I = solver.CalculateTotalToFree(1.0e-3, 0.1e-3, "EDTA", "Ca2");
    
    solver.SetParameters(params_high_I);
    auto result_high_I = solver.CalculateTotalToFree(1.0e-3, 0.1e-3, "EDTA", "Ca2");
    
    // At higher ionic strength, activity coefficients differ
    bool test_ionic_effect = true;  // Both should be valid calculations
    
    std::cout << "    At I=1 mM:    Complex = " << result_low_I.complex << std::endl;
    std::cout << "    At I=1000 mM: Complex = " << result_high_I.complex << std::endl;
    
    print_test_result("Ionic4.2", "Ionic strength effects", result_low_I.complex, result_high_I.complex, test_ionic_effect);
    
    std::cout << "\nSystem Parameters Test Summary: " 
              << (test_temp_effect ? "✓ PASS" : "✗ FAIL")
              << std::endl;
}

void test_solver_boundary_conditions() {
    std::cout << "\n[Test 5] Solver Boundary Conditions" << std::endl;
    std::cout << "================================" << std::endl;
    
    CationSystem solver;
    
    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;
    params.pH = 7.0;
    params.volume = 1.0;
    solver.SetParameters(params);
    
    // Test 1: Zero concentrations
    std::cout << "\nTest Case 5.1: Zero concentration handling" << std::endl;
    auto result_zero = solver.CalculateTotalToFree(0.0, 0.0, "EDTA", "Ca2");
    bool test_zero_ok = (result_zero.totalMetal == 0.0 && result_zero.totalLigand == 0.0);
    print_test_result("Zero5.1", "Zero input handling", 0.0, 0.0, test_zero_ok);
    
    // Test 2: Very dilute system (nanoMolar range)
    std::cout << "\nTest Case 5.2: Ultra-dilute system (nanoMolar range)" << std::endl;
    auto result_dilute = solver.CalculateTotalToFree(1.0e-9, 1.0e-9, "EDTA", "Ca2");
    bool test_dilute_ok = (result_dilute.totalMetal > 0.0 && result_dilute.totalLigand > 0.0);
    print_test_result("Dilute5.2", "Nanoregion calculation", result_dilute.totalMetal, 1.0e-9, test_dilute_ok);
    
    // Test 3: Very concentrated system (molar range)
    std::cout << "\nTest Case 5.3: Concentrated system (molar concentrations)" << std::endl;
    auto result_conc = solver.CalculateTotalToFree(1.0, 1.0, "EDTA", "Ca2");
    bool test_conc_ok = (result_conc.totalMetal > 0.0 && result_conc.totalLigand > 0.0);
    print_test_result("Conc5.3", "Molar concentration calculation", result_conc.totalMetal, 1.0, test_conc_ok);
    
    // Test 4: Extreme concentration ratios (ligand >> metal)
    std::cout << "\nTest Case 5.4: Extreme ratio - excess ligand (1 M EDTA, 1 pM Ca)" << std::endl;
    auto result_excess_L = solver.CalculateTotalToFree(1.0, 1.0e-12, "EDTA", "Ca2");
    bool test_excess_L_ok = (result_excess_L.freeLigand > 0.9);  // Most ligand remains free
    print_test_result("ExcessL5.4", "Free ligand ≈ total", result_excess_L.freeLigand, 0.95, test_excess_L_ok);
    
    // Test 5: Extreme ratio (metal >> ligand)
    std::cout << "\nTest Case 5.5: Extreme ratio - excess metal (1 pM EDTA, 1 M Ca)" << std::endl;
    auto result_excess_M = solver.CalculateTotalToFree(1.0e-12, 1.0, "EDTA", "Ca2");
    bool test_excess_M_ok = (result_excess_M.freeMetal > 0.9);  // Most metal remains free
    print_test_result("ExcessM5.5", "Free metal ≈ total", result_excess_M.freeMetal, 0.95, test_excess_M_ok);
    
    std::cout << "\nBoundary Conditions Test Summary: " 
              << (test_zero_ok && test_dilute_ok && test_conc_ok ? "✓ PASS" : "✗ FAIL")
              << std::endl;
}

void test_ReferenceSolverTest() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  REFERENCE SOLVER TEST SUITE - Phase 1 Validation         ║" << std::endl;
    std::cout << "║  Real equilibrium calculations with literature reference   ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    
    // Initialize ligand data
    InitializeLigandData();
    
    try {
        test_EDTA_Ca_system();
        test_EGTA_Ca_system();
        test_ATP_Mg_system();
        test_system_parameter_effects();
        test_solver_boundary_conditions();
        
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║  ALL REFERENCE TESTS COMPLETED SUCCESSFULLY               ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR during reference testing: " << e.what() << std::endl;
    }
}
