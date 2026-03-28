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
    
    // Use constrained multi-metal calculation
    // Known: total Mg = 1 mM, target free Ca = 100 nM
    // Find: total Ca needed, resulting free Mg
    auto result3 = solver.CalculateConstrainedMulti(5.0, {1.0}, {"Mg2"}, 100e-9, "Ca2", "BAPTA");
    
    std::cout << "Total Ca2+ needed: " << result3.totalMetal * 1e3 << " mM\n";
    std::cout << "Free Mg2+: " << result3.freeMetal * 1e3 << " mM\n";
    std::cout << "Expected Total Ca: ~2.18 mM, Free Mg: ~0.879 mM\n";
    
    return 0;
}
