#include "Solver.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "Testing Solver implementation..." << std::endl;

    // Create solver instance
    CationSystem solver;

    // Set system parameters
    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;  // 100 mM
    params.pH = 7.0;
    params.volume = 1.0;
    solver.SetParameters(params);

    // Test getting available ligands
    auto ligands = solver.GetAvailableLigands();
    std::cout << "Available ligands: " << ligands.size() << std::endl;
    for (const auto& ligand : ligands) {
        std::cout << "  - " << ligand << std::endl;
    }

    // Test getting available metals
    auto metals = solver.GetAvailableMetals();
    std::cout << "Available metals: " << metals.size() << std::endl;
    for (const auto& metal : metals) {
        std::cout << "  - " << metal << std::endl;
    }

    // Test getting equilibrium constant for EDTA-Ca2
    double K = solver.GetAdjustedEquilibriumConstant("EDTA", "Ca2");
    std::cout << "EDTA-Ca2 equilibrium constant: " << K << std::endl;

    // Test equilibrium calculation (Free-to-Total)
    std::cout << "\nTesting EDTA-Ca2 equilibrium calculation (Free-to-Total):" << std::endl;
    auto result1 = solver.CalculateFreeToTotal(1.0e-3, 1.0e-4, "EDTA", "Ca2");
    std::cout << "Free ligand concentration: " << result1.freeLigand << " M" << std::endl;
    std::cout << "Free metal concentration: " << result1.freeMetal << " M" << std::endl;
    std::cout << "Complex concentration: " << result1.complex << " M" << std::endl;
    std::cout << "Total ligand concentration: " << result1.totalLigand << " M" << std::endl;
    std::cout << "Total metal concentration: " << result1.totalMetal << " M" << std::endl;
    std::cout << "pH: " << result1.pH << std::endl;
    std::cout << "Ionic strength: " << result1.ionicStrength << " mM" << std::endl;

    // Test equilibrium calculation (Total-to-Free)
    std::cout << "\nTesting EDTA-Ca2 equilibrium calculation (Total-to-Free):" << std::endl;
    auto result2 = solver.CalculateTotalToFree(1.0e-3, 1.0e-4, "EDTA", "Ca2");
    std::cout << "Free ligand concentration: " << result2.freeLigand << " M" << std::endl;
    std::cout << "Free metal concentration: " << result2.freeMetal << " M" << std::endl;
    std::cout << "Complex concentration: " << result2.complex << " M" << std::endl;
    std::cout << "Total ligand concentration: " << result2.totalLigand << " M" << std::endl;
    std::cout << "Total metal concentration: " << result2.totalMetal << " M" << std::endl;
    std::cout << "pH: " << result2.pH << std::endl;
    std::cout << "Ionic strength: " << result2.ionicStrength << " mM" << std::endl;

    std::cout << "Solver test completed successfully!" << std::endl;
    return 0;
}