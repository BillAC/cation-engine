#include "Solver.h"
#include "Ligand.h"
#include "LigandCSVLoader.h"
#include <iostream>
#include <iomanip>

int main() {
    InitializeLigandData();
    CationSystem solver;
    
    // Add all ligands and metals
    for (auto& ligand : GetAllLigands()) {
        solver.AddLigand(ligand);
    }
    for (auto& metal : GetAllMetals()) {
        solver.AddMetal(metal);
    }
    
    // Set parameters
    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0; // 100 mM
    params.pH = 7.2;
    params.volume = 1.0;
    solver.SetParameters(params);
    
    std::cout << "=== Test Case 1: 10 mM EGTA, 5 mM total Ca2+ ===\n";
    auto result1 = solver.CalculateTotalToFree(10.0, 5.0, "EGTA", "Ca2");
    std::cout << "Free Ca2+: " << result1.freeMetal * 1e9 << " nM\n";
    std::cout << "Expected: ~67.8 nM\n\n";
    
    std::cout << "=== Test Case 2: 10 mM EDTA, desired Free Mg2+ = 1 mM ===\n";
    auto result2 = solver.CalculateFreeToTotal(10.0, 1.0, "EDTA", "Mg2");
    std::cout << "Total Mg2+: " << result2.totalMetal << " mM\n";
    std::cout << "Expected: ~10.97 mM\n\n";
    
    std::cout << "=== Test Case 3: 5 mM BAPTA, pH 7.2 ===\n";
    std::cout << "Desired Free Ca2+ = 100 nM, Total Mg2+ = 1 mM\n";
    auto result3 = solver.CalculateTotalToFree(5.0, 1.0, "BAPTA", "Mg2");
    std::cout << "Free Mg2+: " << result3.freeMetal * 1e3 << " mM\n";
    std::cout << "Expected: ~0.879 mM\n";
    
    // For Ca2+ with BAPTA, we need to calculate what total Ca2+ gives 100 nM free
    auto result4 = solver.CalculateFreeToTotal(5.0, 100e-9, "BAPTA", "Ca2");
    std::cout << "Total Ca2+ for 100 nM free: " << result4.totalMetal * 1e3 << " mM\n";
    std::cout << "Expected: ~2.18 mM\n";
    
    return 0;
}
