#include "CationEngine.h"
#include <iostream>
#include <cassert>

void test_CationEngineTest() {
    std::cout << "Testing CationEngine implementation..." << std::endl;

    // Create engine instance
    CationEngine engine;

    // Test getting available ligands
    auto ligands = engine.GetAvailableLigands();
    std::cout << "Available ligands: " << ligands.size() << std::endl;
    for (const auto& ligand : ligands) {
        std::cout << "  - " << ligand << std::endl;
    }

    // Test getting available metals
    auto metals = engine.GetAvailableMetals();
    std::cout << "Available metals: " << metals.size() << std::endl;
    for (const auto& metal : metals) {
        std::cout << "  - " << metal << std::endl;
    }

    // Test getting stability constants for EDTA
    auto stability = engine.GetStabilityConstants("EDTA");
    std::cout << "EDTA stability constants:" << std::endl;
    std::cout << "  log_K1: " << stability.log_K1 << std::endl;
    std::cout << "  log_K2: " << stability.log_K2 << std::endl;
    std::cout << "  log_K3: " << stability.log_K3 << std::endl;
    std::cout << "  log_K4: " << stability.log_K4 << std::endl;

    // Test getting metal properties
    auto metalProps = engine.GetMetalProperties("Ca2");
    std::cout << "Ca2 properties:" << std::endl;
    std::cout << "  Charge: " << metalProps.charge << std::endl;
    std::cout << "  Atomic weight: " << metalProps.atomicWeight << std::endl;

    std::cout << "CationEngine test completed successfully!" << std::endl;
}