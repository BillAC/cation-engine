#include "Solver.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>

// Constructor
CationSystem::CationSystem() {
    // Initialize with default ligands and metals
    for (const auto& ligand : LIGANDS) {
        ligands.push_back(ligand);
    }

    for (const auto& metal : METALS) {
        metals.push_back(metal);
    }
}

// Destructor
CationSystem::~CationSystem() {
    Clear();
}

// Initialize with specific ligands and metals
void CationSystem::Initialize(const std::vector<std::string>& ligandNames,
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

// Set system parameters
void CationSystem::SetParameters(const SystemParameters& systemParams) {
    params = systemParams;
}

// Calculate protonation fraction (alpha) for a ligand at given pH
double CationSystem::calculateProtonationFraction(double pH, double logK) {
    // Simplified calculation for single protonation step
    // For multiple protonation steps, we'd need to consider all steps
    return 1.0 / (1.0 + std::pow(10, logK - pH));
}

// Calculate ionic strength correction factor (Debye-Hückel)
double CationSystem::calculateIonicStrengthCorrection(double ionicStrength, double charge) {
    if (ionicStrength <= 0) return 1.0;

    // Debye-Hückel equation for activity coefficient
    // For simplicity, using a simplified version
    double alpha = 0.5; // Simplified parameter
    double logGamma = -alpha * charge * charge * std::sqrt(ionicStrength) / (1.0 + 1.5 * std::sqrt(ionicStrength));
    return std::pow(10, logGamma);
}

// Calculate temperature correction using van't Hoff equation
double CationSystem::calculateTemperatureCorrection(double temperature, double deltaH) {
    // Simplified temperature correction
    // Using the relationship: ln(K2/K1) = -ΔH/R * (1/T2 - 1/T1)
    // For 25°C as reference (298.15K)
    double T1 = 298.15; // Reference temperature (25°C)
    double T2 = temperature + 273.15; // Convert to Kelvin
    double R = 8.314; // Gas constant in J/mol·K

    // If deltaH is not provided (0), return 1.0 (no temperature correction)
    if (deltaH == 0.0) return 1.0;

    if (T2 == T1) return 1.0;

    double lnK = -deltaH / R * (1.0 / T2 - 1.0 / T1);
    return std::exp(lnK);
}

// Calculate adjusted stability constant considering pH, ionic strength, and temperature
double CationSystem::calculateStabilityConstant(double logK, double pH, double ionicStrength) {
    // Convert logK to K
    double K = std::pow(10, logK);

    // Apply pH correction (simplified - for 1:1 complex formation)
    double pHCorrection = 1.0; // For 1:1 complexes, pH correction is minimal

    // Apply ionic strength correction
    double ionicCorrection = 1.0; // Simplified - would be more complex in real implementation

    // Apply temperature correction (simplified - no deltaH in data)
    double tempCorrection = 1.0; // Simplified - would use deltaH if available

    // Combined correction
    double adjustedK = K * pHCorrection * ionicCorrection * tempCorrection;

    // Return log of adjusted K
    return std::log10(adjustedK);
}

// Iterative bounding solver for 1:1 complex formation
double CationSystem::solveForFreeMetal(double totalMetal, double totalLigand, double complexFormationConstant,
                                      double tolerance, int maxIterations) {
    // For a simple 1:1 complex formation: ML = K * M * L
    // Where M = totalMetal - complex, L = totalLigand - complex
    // So: complex = K * (totalMetal - complex) * (totalLigand - complex)
    // This is a quadratic equation in complex: a*complex^2 + b*complex + c = 0

    // Simplified approach: iterative bounding method
    double low = 0.0;
    double high = std::min(totalMetal, totalLigand);

    // If the system is very dilute, return 0
    if (high < tolerance) return 0.0;

    // Iterative approach
    for (int i = 0; i < maxIterations; ++i) {
        double complex = (low + high) / 2.0;

        // Calculate free concentrations
        double freeMetal = totalMetal - complex;
        double freeLigand = totalLigand - complex;

        // Calculate predicted complex concentration using equilibrium
        double predictedComplex = complexFormationConstant * freeMetal * freeLigand;

        // Adjust bounds
        if (predictedComplex > complex) {
            low = complex;
        } else {
            high = complex;
        }

        // Check for convergence
        if (std::abs(predictedComplex - complex) < tolerance) {
            return complex;
        }
    }

    // Return the final estimate if not converged
    return (low + high) / 2.0;
}

// Calculate equilibrium concentrations (Free-to-Total)
EquilibriumResult CationSystem::calculateFreeToTotal(double freeLigand, double freeMetal,
                                                   const std::string& ligandName, const std::string& metalName) {
    EquilibriumResult result;

    // Get ligand and metal
    const Ligand* ligand = GetLigandByName(ligandName);
    const Metal* metal = GetMetalByName(metalName);

    if (!ligand || !metal) {
        return result;
    }

    // Calculate complex formation constant
    double complexFormationConstant = std::pow(10, ligand->stability_constants.log_K1);

    // Apply corrections to the formation constant
    double adjustedK = calculateStabilityConstant(ligand->stability_constants.log_K1, params.pH,
                                                params.ionicStrength);

    // Calculate complex concentration using iterative method
    double complex = solveForFreeMetal(freeMetal, freeLigand, adjustedK);

    // Calculate free concentrations
    double freeLigandFinal = freeLigand - complex;
    double freeMetalFinal = freeMetal - complex;

    // Calculate total concentrations
    double totalLigand = freeLigandFinal + complex;
    double totalMetal = freeMetalFinal + complex;

    result.freeLigand = freeLigandFinal;
    result.freeMetal = freeMetalFinal;
    result.complex = complex;
    result.totalLigand = totalLigand;
    result.totalMetal = totalMetal;
    result.ionicStrength = params.ionicStrength;
    result.pH = params.pH;

    return result;
}

// Calculate equilibrium concentrations (Total-to-Free)
EquilibriumResult CationSystem::calculateTotalToFree(double totalLigand, double totalMetal,
                                                   const std::string& ligandName, const std::string& metalName) {
    EquilibriumResult result;

    // Get ligand and metal
    const Ligand* ligand = GetLigandByName(ligandName);
    const Metal* metal = GetMetalByName(metalName);

    if (!ligand || !metal) {
        return result;
    }

    // Calculate complex formation constant
    double complexFormationConstant = std::pow(10, ligand->stability_constants.log_K1);

    // Apply corrections to the formation constant
    double adjustedK = calculateStabilityConstant(ligand->stability_constants.log_K1, params.pH,
                                                params.ionicStrength);

    // Calculate complex concentration using iterative method
    double complex = solveForFreeMetal(totalMetal, totalLigand, adjustedK);

    // Calculate free concentrations
    double freeLigandFinal = totalLigand - complex;
    double freeMetalFinal = totalMetal - complex;

    result.freeLigand = freeLigandFinal;
    result.freeMetal = freeMetalFinal;
    result.complex = complex;
    result.totalLigand = totalLigand;
    result.totalMetal = totalMetal;
    result.ionicStrength = params.ionicStrength;
    result.pH = params.pH;

    return result;
}

// Calculate equilibrium concentrations (both directions)
EquilibriumResult CationSystem::CalculateEquilibrium(double totalLigand, double totalMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateTotalToFree(totalLigand, totalMetal, ligandName, metalName);
}

// Calculate equilibrium concentrations (Free-to-Total)
EquilibriumResult CationSystem::CalculateFreeToTotal(double freeLigand, double freeMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateFreeToTotal(freeLigand, freeMetal, ligandName, metalName);
}

// Calculate equilibrium concentrations (Total-to-Free)
EquilibriumResult CationSystem::CalculateTotalToFree(double totalLigand, double totalMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateTotalToFree(totalLigand, totalMetal, ligandName, metalName);
}

// Add ligand to the system
void CationSystem::AddLigand(const Ligand& ligand) {
    ligands.push_back(ligand);
}

// Add metal to the system
void CationSystem::AddMetal(const Metal& metal) {
    metals.push_back(metal);
}

// Clear all data
void CationSystem::Clear() {
    ligands.clear();
    metals.clear();
    adjustedConstants.clear();
}

// Get list of available ligands
std::vector<std::string> CationSystem::GetAvailableLigands() const {
    std::vector<std::string> names;
    for (const auto& ligand : ligands) {
        names.push_back(ligand.name);
    }
    return names;
}

// Get list of available metals
std::vector<std::string> CationSystem::GetAvailableMetals() const {
    std::vector<std::string> names;
    for (const auto& metal : metals) {
        names.push_back(metal.name);
    }
    return names;
}

// Get adjusted equilibrium constant for a ligand-metal pair
double CationSystem::GetAdjustedEquilibriumConstant(const std::string& ligandName, const std::string& metalName) {
    std::string key = ligandName + "_" + metalName;
    auto it = adjustedConstants.find(key);
    if (it != adjustedConstants.end()) {
        return it->second;
    }

    // If not found, calculate it
    const Ligand* ligand = GetLigandByName(ligandName);
    if (!ligand) return 0.0;

    double adjustedK = calculateStabilityConstant(ligand->stability_constants.log_K1, params.pH,
                                                params.ionicStrength);
    adjustedConstants[key] = adjustedK;
    return adjustedK;
}