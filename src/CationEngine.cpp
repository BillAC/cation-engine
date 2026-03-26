#include "CationEngine.h"
#include "LigandCSVLoader.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// Constructor
CationEngine::CationEngine() {
    // Initialize with empty ligands and metals - data will be loaded from CSV
    // Ligands and metals will be populated when Initialize() is called or when
    // ligand data is loaded from CSV
}

// Destructor
CationEngine::~CationEngine() {
    Clear();
}

// Initialize with specific ligands and metals
void CationEngine::Initialize(const std::vector<std::string>& ligandNames,
                              const std::vector<std::string>& metalNames) {
    ligands.clear();
    metals.clear();

    // Add requested ligands
    for (const auto& name : ligandNames) {
        const Ligand* ligand = GetLigandByName(name);
        if (ligand) {
            ligands.push_back(*ligand);
        }
    }

    // Add requested metals
    for (const auto& name : metalNames) {
        const Metal* metal = GetMetalByName(name);
        if (metal) {
            metals.push_back(*metal);
        }
    }
}

// Calculate equilibrium concentrations
EquilibriumConcentrations CationEngine::CalculateEquilibrium(double totalLigand,
                                                            double totalMetal,
                                                            double pH,
                                                            const std::string& ligandName,
                                                            const std::string& metalName) {
    EquilibriumConcentrations result = {0, 0, 0, 0};

    // Get ligand and metal
    const Ligand* ligand = GetLigandByName(ligandName);
    const Metal* metal = GetMetalByName(metalName);

    if (!ligand || !metal) {
        return result;
    }

    // Simplified equilibrium calculation (this would be more complex in a full implementation)
    // For now, we'll provide a basic framework

    // Calculate protonation fractions (simplified - using log_K1 instead of H1)
    double alpha1 = 1.0 / (1.0 + std::pow(10, ligand->constants.log_K1 - pH));
    double alpha2 = 1.0 / (1.0 + std::pow(10, ligand->constants.log_K2 - pH));
    double alpha3 = 1.0 / (1.0 + std::pow(10, ligand->constants.log_K3 - pH));
    double alpha4 = 1.0 / (1.0 + std::pow(10, ligand->constants.log_K4 - pH));

    // For a simple 1:1 complex formation (this is a simplification)
    // In reality, this would involve solving a system of equations
    double complexFormationConstant = std::pow(10, ligand->constants.log_K1); // Simplified

    // Basic equilibrium calculation
    double freeLigand = totalLigand * alpha1;  // Simplified
    double freeMetal = totalMetal;             // Simplified

    // Complex concentration calculation
    double complexConcentration = 0.0;
    if (freeLigand > 0 && freeMetal > 0) {
        complexConcentration = (freeLigand * freeMetal) / (complexFormationConstant + freeLigand * freeMetal);
    }

    result.ligand = freeLigand;
    result.metal = freeMetal;
    result.complex = complexConcentration;
    result.total = totalLigand + totalMetal;

    return result;
}

// Get stability constants for a ligand
StabilityConstants CationEngine::GetStabilityConstants(const std::string& ligandName) {
    const Ligand* ligand = GetLigandByName(ligandName);
    if (ligand) {
        return ligand->constants;
    }
    return StabilityConstants();
}

// Get metal properties
Metal CationEngine::GetMetalProperties(const std::string& metalName) {
    const Metal* metal = GetMetalByName(metalName);
    if (metal) {
        return *metal;
    }
    return Metal();
}

// Set equilibrium constants (for custom calculations)
void CationEngine::SetEquilibriumConstant(const std::string& key, double constant) {
    equilibriumConstants[key] = constant;
}

// Get equilibrium constant
double CationEngine::GetEquilibriumConstant(const std::string& key) {
    auto it = equilibriumConstants.find(key);
    if (it != equilibriumConstants.end()) {
        return it->second;
    }
    return 0.0;
}

// Add ligand to the system
void CationEngine::AddLigand(const Ligand& ligand) {
    ligands.push_back(ligand);
}

// Add metal to the system
void CationEngine::AddMetal(const Metal& metal) {
    metals.push_back(metal);
}

// Clear all data
void CationEngine::Clear() {
    ligands.clear();
    metals.clear();
    equilibriumConstants.clear();
}

// Get list of available ligands
std::vector<std::string> CationEngine::GetAvailableLigands() const {
    std::vector<std::string> names;
    for (const auto& ligand : ligands) {
        names.push_back(ligand.name);
    }
    return names;
}

// Get list of available metals
std::vector<std::string> CationEngine::GetAvailableMetals() const {
    std::vector<std::string> names;
    for (const auto& metal : metals) {
        names.push_back(metal.name);
    }
    return names;
}