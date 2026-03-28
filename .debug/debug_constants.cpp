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
    
    // Check constants
    std::cout << "=== Stability Constants ===\n";
    std::cout << "EGTA-Ca2: " << solver.GetAdjustedEquilibriumConstant("EGTA", "Ca2") << " (logK)\n";
    std::cout << "EDTA-Mg2: " << solver.GetAdjustedEquilibriumConstant("EDTA", "Mg2") << " (logK)\n";
    std::cout << "BAPTA-Ca2: " << solver.GetAdjustedEquilibriumConstant("BAPTA", "Ca2") << " (logK)\n";
    std::cout << "BAPTA-Mg2: " << solver.GetAdjustedEquilibriumConstant("BAPTA", "Mg2") << " (logK)\n";
    
    // Check raw constants from CSV
    const Ligand* egta = GetLigandByName("EGTA");
    const Ligand* edta = GetLigandByName("EDTA");
    const Ligand* bapta = GetLigandByName("BAPTA");
    
    if (egta) {
        std::cout << "\nEGTA raw constants:\n";
        std::cout << "  Ca1: " << egta->constants.Ca1 << "\n";
        std::cout << "  Valence: " << egta->valence << "\n";
        std::cout << "  pK1: " << egta->constants.log_K1 << "\n";
        std::cout << "  pK2: " << egta->constants.log_K2 << "\n";
    }
    
    return 0;
}
