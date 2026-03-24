#include "CationEngine.h"
#include <iostream>
#include <cassert>

int main() {
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
    std::cout << "  H1: " << stability.H1 << std::endl;
    std::cout << "  H2: " << stability.H2 << std::endl;
    std::cout << "  H3: " << stability.H3 << std::endl;
    std::cout << "  H4: " << stability.H4 << std::endl;

    // Test getting metal properties
    auto metalProps = engine.GetMetalProperties("Ca2");
    std::cout << "Ca2 properties:" << std::endl;
    std::cout << "  Charge: " << metalProps.charge << std::endl;
    std::cout << "  Atomic weight: " << metalProps.atomicWeight << std::endl;

    std::cout << "CationEngine test completed successfully!" << std::endl;
    return 0;
}