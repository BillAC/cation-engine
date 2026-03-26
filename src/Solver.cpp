#include "Solver.h"
#include "LigandCSVLoader.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>

// Constructor
CationSystem::CationSystem() {
    // Initialize with empty ligands and metals - data will be loaded from CSV
    // Ligands and metals will be populated when Initialize() is called or when
    // ligand data is loaded from CSV
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

// Calculate protonation fraction (alpha) for fully deprotonated form (Ln-)
// For ligand with n protonation sites: alpha = 1 / (1 + Σ H_i)
// Where H_i = 10^(pKa1 + pKa2 + ... + pKai - n*pH) for stepwise protonation
double CationSystem::calculateProtonationFraction(double pH, const Ligand* ligand) {
    if (!ligand) return 1.0;
    
    // Multi-step protonation Henderson-Hasselbalch
    // Calculate denominator: D = 1 + Σ 10^(sum of pKa values - number of protons * pH)
    double pK1 = ligand->constants.log_K1;
    double pK2 = ligand->constants.log_K2;
    double pK3 = ligand->constants.log_K3;
    double pK4 = ligand->constants.log_K4;
    
    // Denominator sums all protonation states
    double D = 1.0; // Ln- (fully deprotonated, no protons)
    
    // Add HLn-1 state (one proton bound)
    if (pK1 > 0) D += std::pow(10, pK1 - pH);
    
    // Add H2Ln-2 state (two protons bound)
    if (pK1 > 0 && pK2 > 0) D += std::pow(10, pK1 + pK2 - 2*pH);
    
    // Add H3Ln-3 state (three protons bound)
    if (pK1 > 0 && pK2 > 0 && pK3 > 0) D += std::pow(10, pK1 + pK2 + pK3 - 3*pH);
    
    // Add H4Ln-4 state (four protons bound)
    if (pK1 > 0 && pK2 > 0 && pK3 > 0 && pK4 > 0) D += std::pow(10, pK1 + pK2 + pK3 + pK4 - 4*pH);
    
    // Fraction of fully deprotonated form: alpha = 1/D
    return 1.0 / D;
}

// Deprecated: old single-step protonation method for backward compatibility
double CationSystem::calculateProtonationFraction(double pH, double logK) {
    // Single protonation step only
    // For multiple protonation steps, use the version that accepts Ligand*
    return 1.0 / (1.0 + std::pow(10, logK - pH));
}

// Calculate ionic strength correction factor (Davies Equation)
// log(γ) = -A*z²*[√I/(1+√I) - 0.3*I]
// This is more accurate than simple Debye-Hückel for solutions up to 1M
double CationSystem::calculateIonicStrengthCorrection(double ionicStrength, double charge) {
    if (ionicStrength <= 0) return 1.0;

    // Davies equation parameters
    const double A = 0.51; // Temperature-dependent parameter (0.51 at 25°C)
    double sqrtI = std::sqrt(ionicStrength);
    
    // Davies equation: log(γ) = -A*z²*[√I/(1+√I) - 0.3*I]
    double logGamma = -A * charge * charge * 
                     (sqrtI / (1.0 + sqrtI) - 0.3 * ionicStrength);
    
    // Activity coefficient: γ = 10^(logGamma)
    return std::pow(10, logGamma);
}

// Calculate temperature correction using van't Hoff equation
// ln(K₂/K₁) = -ΔH/R * (1/T₂ - 1/T₁)
// This properly uses enthalpy data from the CSV
double CationSystem::calculateTemperatureCorrection(double temperature, double deltaH) {
    const double R = 8.314; // Gas constant in J/mol·K
    const double T1 = 298.15; // Reference temperature (25°C)
    
    double T2 = temperature + 273.15; // Convert input from Celsius to Kelvin
    
    // If no enthalpy provided or at reference temperature, no correction
    if (deltaH == 0.0 || T2 == T1) return 1.0;

    // deltaH from CSV is typically in kcal/mol, convert to J/mol
    double deltaH_J = deltaH * 4184.0;
    
    // Van't Hoff: ln(K_corrected/K_ref) = -ΔH/R * (1/T₂ - 1/T₁)
    double lnK = -deltaH_J / R * (1.0 / T2 - 1.0 / T1);
    
    // K_corrected = K_ref * exp(lnK)
    return std::exp(lnK);
}

// Calculate adjusted stability constant considering pH and ionic strength
double CationSystem::calculateStabilityConstant(double logK, double pH, double ionicStrength) {
    // Convert logK to K
    double K = std::pow(10, logK);

    // Apply pH correction (simplified - for 1:1 complex formation)
    double pHCorrection = 1.0; // For 1:1 complexes, pH correction is minimal

    // Apply ionic strength correction
    double ionicCorrection = 1.0; // Simplified - would be more complex in real implementation

    // Combined correction
    double adjustedK = K * pHCorrection * ionicCorrection;

    // Return log of adjusted K
    return std::log10(adjustedK);
}

// Helper method to get metal-specific binding constant
double CationSystem::getMetalBindingConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    
    // Map metal names to their binding constants in the StabilityConstants structure
    if (metalName == "Ca2") return ligand->constants.Ca1;
    if (metalName == "Mg2") return ligand->constants.Mg1;
    if (metalName == "Ba2") return ligand->constants.Ba1;
    if (metalName == "Cd2") return ligand->constants.Cd1;
    if (metalName == "Sr2") return ligand->constants.Sr1;
    if (metalName == "Mn2") return ligand->constants.Mn1;
    if (metalName == "X1")  return ligand->constants.X1;
    if (metalName == "Cu2") return ligand->constants.Cu1;
    if (metalName == "Zn2") return ligand->constants.Zn1;
    
    // If metal not found, return 0
    return 0.0;
}

// Helper method to get metal-specific enthalpy constant
double CationSystem::getMetalEnthalpyConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    
    // Map metal names to their enthalpy constants in the StabilityConstants structure
    if (metalName == "Ca2") return ligand->constants.dCa1;
    if (metalName == "Mg2") return ligand->constants.dMg1;
    if (metalName == "Ba2") return ligand->constants.dBa1;
    if (metalName == "Cd2") return ligand->constants.dCd1;
    if (metalName == "Sr2") return ligand->constants.dSr1;
    if (metalName == "Mn2") return ligand->constants.dMn1;
    if (metalName == "X1")  return ligand->constants.dX1;
    if (metalName == "Cu2") return ligand->constants.dCu1;
    if (metalName == "Zn2") return ligand->constants.dZn1;
    
    // If metal not found, return 0 (no enthalpy correction)
    return 0.0;
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

    // Get metal-specific binding constant (logK for metal-ligand complex)
    double logK_metal = getMetalBindingConstant(ligand, metalName);
    if (logK_metal == 0.0) {
        // If no specific binding constant, calculation not possible
        return result;
    }
    
    // Convert logK to actual formation constant K with corrections applied
    // For FREE-to-TOTAL: input is already in the free (deprotonated) form
    // So we only apply ionic strength and temperature corrections
    
    // Convert corrected logK to actual formation constant K
    // TEMPORARILY DISABLED: All corrections disabled to debug
    double complexFormationConstant = std::pow(10, logK_metal);

    // Calculate complex concentration using iterative method with actual K value
    double complex = solveForFreeMetal(freeMetal, freeLigand, complexFormationConstant, 1e-10, 1000);

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

    // Get metal-specific binding constant (logK for metal-ligand complex)
    double logK_metal = getMetalBindingConstant(ligand, metalName);
    if (logK_metal == 0.0) {
        // If no specific binding constant, calculation not possible
        return result;
    }
    
    // Convert logK to actual formation constant K with corrections applied
    // For TOTAL-to-FREE: input includes all protonation states
    // So we apply ionic strength and temperature corrections
    // NOTE: Protonation correction temporarily disabled pending pH model refinement
    
    // Convert corrected logK to actual formation constant K
    // TEMPORARILY DISABLED: All corrections disabled to debug
    double complexFormationConstant = std::pow(10, logK_metal);

    // Calculate complex concentration using iterative method with actual K value
    double complex = solveForFreeMetal(totalMetal, totalLigand, complexFormationConstant, 1e-10, 1000);

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

    double adjustedK = calculateStabilityConstant(ligand->constants.log_K1, params.pH,
                                                params.ionicStrength);
    adjustedConstants[key] = adjustedK;
    return adjustedK;
}

// Get stability constants for a ligand
StabilityConstants CationSystem::GetStabilityConstants(const std::string& ligandName) const {
    const Ligand* ligand = GetLigandByName(ligandName);
    if (ligand) {
        return ligand->constants;
    }
    return StabilityConstants();
}

// Get metal properties
Metal CationSystem::GetMetalProperties(const std::string& metalName) const {
    const Metal* metal = GetMetalByName(metalName);
    if (metal) {
        return *metal;
    }
    return Metal();
}

// Set equilibrium constants (for custom calculations)
void CationSystem::SetEquilibriumConstant(const std::string& key, double constant) {
    adjustedConstants[key] = constant;
}

// Get equilibrium constant
double CationSystem::GetEquilibriumConstant(const std::string& key) const {
    auto it = adjustedConstants.find(key);
    if (it != adjustedConstants.end()) {
        return it->second;
    }
    return 0.0;
}