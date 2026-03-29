#include "Solver.h"
#include "Ligand.h"
#include "LigandCSVLoader.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <cassert>

int main() {
    InitializeLigandData();
    CationSystem solver;
    
    // Set parameters
    SystemParameters params;
    params.temperature = 25.0;
    params.ionicStrength = 100.0;
    params.pH = 7.2;
    params.volume = 1.0;
    solver.SetParameters(params);
    
    std::cout << "=== Comprehensive Test: Multi-metal BAPTA ===\n";
    
    double totalBAPTA = 5.0e-3; // 5 mM
    std::vector<std::string> metals = {"Ca2", "Mg2", "Ba2", "Sr2", "Mn2", "X1"};
    std::vector<double> totals = {1.0e-3, 1.0e-3, 0.1e-3, 0.1e-3, 0.05e-3, 0.05e-3};
    
    auto result = solver.CalculateTotalToFreeMulti(totalBAPTA, totals, "BAPTA", metals);
    
    std::cout << "Total BAPTA: " << totalBAPTA * 1000.0 << " mM\n";
    std::cout << "Free BAPTA:  " << result.freeLigand * 1000.0 << " mM\n";
    std::cout << "\nResults for metals:\n";
    std::cout << std::left << std::setw(10) << "Metal" << std::setw(15) << "Total (mM)" << std::setw(15) << "Free (mM)" << std::setw(15) << "Complex (mM)" << "\n";
    std::cout << "------------------------------------------------------------\n";
    
    double sumComplex = 0.0;
    for (size_t i = 0; i < result.metalNames.size(); ++i) {
        std::cout << std::left << std::setw(10) << result.metalNames[i] 
                  << std::setw(15) << result.totalMetals[i] * 1000.0
                  << std::setw(15) << result.freeMetals[i] * 1000.0
                  << std::setw(15) << result.complex[i] * 1000.0 << "\n";
        sumComplex += result.complex[i];
    }
    
    std::cout << "------------------------------------------------------------\n";
    std::cout << "Sum of Complexes: " << sumComplex * 1000.0 << " mM\n";
    std::cout << "L_free + Sum(Complex): " << (result.freeLigand + sumComplex) * 1000.0 << " mM\n";
    
    // Validation: Mass balance for ligand
    if (std::abs(result.freeLigand + sumComplex - totalBAPTA) < 1e-9) {
        std::cout << "Ligand mass balance: OK\n";
    } else {
        std::cout << "Ligand mass balance: FAILED! Diff: " << std::abs(result.freeLigand + sumComplex - totalBAPTA) << "\n";
    }
    
    return 0;
}
