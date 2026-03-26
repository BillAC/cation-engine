#include "Solver.h"
#include "Ligand.h"
#include "LigandCSVLoader.h"
#include <iostream>

int main() {
    std::cout << "Cation-Ligand Equilibrium Solver Demo" << std::endl;
    std::cout << "=====================================" << std::endl;

    // Initialize ligand data from CSV
    InitializeLigandData();

    // Create solver instance
    CationSystem solver;

    // Add all available ligands to the system
    auto allLigands = GetAllLigands();
    for (const auto& ligand : allLigands) {
        solver.AddLigand(ligand);
    }

    // Add all available metals to the system
    auto allMetals = GetAllMetals();
    for (const auto& metal : allMetals) {
        solver.AddMetal(metal);
    }

    // Set system parameters
    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;  // 100 mM
    params.pH = 7.0;
    params.volume = 1.0;
    solver.SetParameters(params);

    // Display available ligands
    std::cout << "Available ligands:" << std::endl;
    auto ligands = solver.GetAvailableLigands();
    for (const auto& ligand : ligands) {
        std::cout << "  - " << ligand << std::endl;
    }

    // Display available metals
    std::cout << "\nAvailable metals:" << std::endl;
    auto metals = solver.GetAvailableMetals();
    for (const auto& metal : metals) {
        std::cout << "  - " << metal << std::endl;
    }

    // Test equilibrium calculation for EDTA-Ca2 system (Free-to-Total)
    std::cout << "\nTesting EDTA-Ca2 equilibrium calculation (Free-to-Total):" << std::endl;
    auto result1 = solver.CalculateFreeToTotal(1.0e-3, 1.0e-4, "EDTA", "Ca2");

    std::cout << "Free ligand concentration: " << result1.freeLigand << " M" << std::endl;
    std::cout << "Free metal concentration: " << result1.freeMetal << " M" << std::endl;
    std::cout << "Complex concentration: " << result1.complex << " M" << std::endl;
    std::cout << "Total ligand concentration: " << result1.totalLigand << " M" << std::endl;
    std::cout << "Total metal concentration: " << result1.totalMetal << " M" << std::endl;
    std::cout << "pH: " << result1.pH << std::endl;
    std::cout << "Ionic strength: " << result1.ionicStrength << " mM" << std::endl;

    // Test equilibrium calculation for EDTA-Ca2 system (Total-to-Free)
    std::cout << "\nTesting EDTA-Ca2 equilibrium calculation (Total-to-Free):" << std::endl;
    auto result2 = solver.CalculateTotalToFree(1.0e-3, 1.0e-4, "EDTA", "Ca2");

    std::cout << "Free ligand concentration: " << result2.freeLigand << " M" << std::endl;
    std::cout << "Free metal concentration: " << result2.freeMetal << " M" << std::endl;
    std::cout << "Complex concentration: " << result2.complex << " M" << std::endl;
    std::cout << "Total ligand concentration: " << result2.totalLigand << " M" << std::endl;
    std::cout << "Total metal concentration: " << result2.totalMetal << " M" << std::endl;
    std::cout << "pH: " << result2.pH << std::endl;
    std::cout << "Ionic strength: " << result2.ionicStrength << " mM" << std::endl;

    // Display equilibrium constant for EDTA-Ca2 system
    std::cout << "\nEDTA-Ca2 equilibrium constant:" << std::endl;
    double K = solver.GetAdjustedEquilibriumConstant("EDTA", "Ca2");
    std::cout << "  K: " << K << std::endl;

    return 0;
}