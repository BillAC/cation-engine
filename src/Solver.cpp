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

    // Convert ionic strength from mM to M
    double I_molar = ionicStrength / 1000.0;

    // Davies equation parameters
    const double A = 0.51; // Temperature-dependent parameter (0.51 at 25°C)
    double sqrtI = std::sqrt(I_molar);
    
    // Davies equation: log(γ) = -A*z²*[√I/(1+√I) - 0.3*I]
    double logGamma = -A * charge * charge * 
                     (sqrtI / (1.0 + sqrtI) - 0.3 * I_molar);
    
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
    if (logK <= 0.0) return 0.0;

    // Convert logK to K
    double K = std::pow(10.0, logK);

    // pH correction is assumed via protonation fraction of ligand-specific behaviour
    double pHCorrection = 1.0;
    
    // For metal-ligand complex formation, we need activity coefficients for all species
    // Ca2+ + L4- ⇌ CaL2-
    double gamma_Ca = calculateIonicStrengthCorrection(ionicStrength, 2.0);      // Ca2+ charge +2
    double gamma_L = calculateIonicStrengthCorrection(ionicStrength, -4.0);     // L4- charge -4
    double gamma_complex = calculateIonicStrengthCorrection(ionicStrength, -2.0); // CaL2- charge -2
    
    // Conditional K' = K_thermo * gamma_Ca * gamma_L / gamma_complex
    double ionicCorrection = gamma_Ca * gamma_L / gamma_complex;
    
    double adjustedK = K * pHCorrection * ionicCorrection;
    if (adjustedK <= 0.0) return 0.0;
    return std::log10(adjustedK);
}

// Calculate adjusted stability constant for a specific ligand-metal pair
// using pH, ionic strength, and temperature corrections.
double CationSystem::calculateStabilityConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;

    double logK = getMetalBindingConstant(ligand, metalName);
    if (logK <= 0.0) return 0.0;

    double K = std::pow(10.0, logK);
    double deltaH = getMetalEnthalpyConstant(ligand, metalName);

    double tempCorr = calculateTemperatureCorrection(params.temperature, deltaH);
    // ionic strength correction is available in separate method (not used here or set to 1)
    double ionicCorr = 1.0;

    // Direct pH-corrected stability constant via protonation fraction
    double alpha = calculateProtonationFraction(params.pH, ligand);

    double effectiveK = K * tempCorr * ionicCorr * alpha;
    if (effectiveK < 1e-300) {
        // Underflow protection
        effectiveK = 0.0;
    }

    return effectiveK > 0.0 ? std::log10(effectiveK) : 0.0;
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

// Multispecies helper: calculate complex for 1:1 metal ion at fixed free ligand
// From Ci = Ki * Mi_free * L_free and Mi_free = Mi_total - Ci:
// Ci = (Ki * L_free * Mi_total) / (1 + Ki * L_free)
double CationSystem::calculateComplexFromLigandFree(double totalMetal, double K, double freeLigand) {
    if (K <= 0.0 || totalMetal <= 0.0 || freeLigand <= 0.0) {
        return 0.0;
    }
    double part = K * freeLigand;
    return totalMetal * part / (1.0 + part);
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

// Newton-Raphson solver for coupled multi-metal ligand equilibria
bool CationSystem::solveCoupledEquilibriumNewton(double totalLigand,
                                                 const std::vector<double>& totalMetals,
                                                 const std::vector<double>& Kvalues,
                                                 double& freeLigand,
                                                 std::vector<double>& freeMetals,
                                                 double tolerance,
                                                 int maxIterations) {
    size_t n = totalMetals.size();
    if (n == 0 || totalLigand <= 0 || Kvalues.size() != n) {
        return false;
    }

    // Initialize variables with reasonable bounds
    if (freeLigand <= 0.0 || freeLigand > totalLigand) {
        freeLigand = std::max(1e-12, totalLigand * 0.5);
    }

    freeMetals.resize(n);
    for (size_t i = 0; i < n; ++i) {
        freeMetals[i] = totalMetals[i];
    }

    for (int iter = 0; iter < maxIterations; ++iter) {
        // Calculate complexes and residuals
        std::vector<double> complex(n);
        std::vector<double> residue(n+1);

        double sumComplex = 0.0;
        for (size_t i = 0; i < n; ++i) {
            complex[i] = Kvalues[i] * freeMetals[i] * freeLigand;
            sumComplex += complex[i];
            residue[i+1] = totalMetals[i] - freeMetals[i] - complex[i];
        }
        residue[0] = totalLigand - freeLigand - sumComplex;

        // Check convergence on all equations
        double maxErr = 0.0;
        for (double r : residue) {
            maxErr = std::max(maxErr, std::abs(r));
        }
        if (maxErr < tolerance) {
            return true;
        }

        // Build Jacobian matrix (n+1 x n+1) as dense vector of vectors
        std::vector<std::vector<double>> J(n+1, std::vector<double>(n+1, 0.0));

        // d(res0)/d(Lfree)
        double sumKf = 0.0;
        for (size_t i = 0; i < n; ++i) {
            sumKf += Kvalues[i] * freeMetals[i];
        }
        J[0][0] = -1.0 - sumKf;

        // d(res0)/d(Mfree_i)
        for (size_t i = 0; i < n; ++i) {
            J[0][i+1] = -Kvalues[i] * freeLigand;
        }

        for (size_t i = 0; i < n; ++i) {
            // d(res_i+1)/d(Lfree)
            J[i+1][0] = -Kvalues[i] * freeMetals[i];
            // d(res_i+1)/d(Mfree_i)
            J[i+1][i+1] = -1.0 - Kvalues[i] * freeLigand;
        }

        // Solve J * dx = residue with simple Gaussian elimination
        // We need dx = J^{-1} * residue, where we treat residue vector as RHS
        // Pivot matrix and RHS
        std::vector<std::vector<double>> A = J;
        std::vector<double> b(n+1);
        for (size_t i = 0; i < n+1; ++i) b[i] = residue[i];

        // Gaussian elimination
        for (size_t i = 0; i < n+1; ++i) {
            // Partial pivot
            size_t pivot = i;
            double maxVal = std::abs(A[i][i]);
            for (size_t j = i+1; j < n+1; ++j) {
                double val = std::abs(A[j][i]);
                if (val > maxVal) {
                    maxVal = val;
                    pivot = j;
                }
            }
            if (maxVal < 1e-18) {
                // Singular matrix; fallback
                return false;
            }
            if (pivot != i) {
                std::swap(A[i], A[pivot]);
                std::swap(b[i], b[pivot]);
            }

            double diag = A[i][i];
            for (size_t j = i; j < n+1; ++j) {
                A[i][j] /= diag;
            }
            b[i] /= diag;

            for (size_t k = i+1; k < n+1; ++k) {
                double factor = A[k][i];
                for (size_t j = i; j < n+1; ++j) {
                    A[k][j] -= factor * A[i][j];
                }
                b[k] -= factor * b[i];
            }
        }

        // Back substitution for dx
        std::vector<double> dx(n+1, 0.0);
        for (int i = (int)n; i >= 0; --i) {
            double sum = b[i];
            for (size_t j = i+1; j < n+1; ++j) {
                sum -= A[i][j] * dx[j];
            }
            dx[i] = sum;
        }

        // Update unknowns with damping to avoid overshoot
        double damping = 0.5;
        freeLigand -= damping * dx[0];
        freeLigand = std::min(std::max(freeLigand, 0.0), totalLigand);

        for (size_t i = 0; i < n; ++i) {
            freeMetals[i] -= damping * dx[i+1];
            freeMetals[i] = std::max(0.0, std::min(freeMetals[i], totalMetals[i]));
        }

        // Ensure ligand is mass-balanced if we drifted
        if (freeLigand <= 0.0) freeLigand = 1e-15;
    }

    return false;
}

// Calculate equilibrium concentrations (Free-to-Total)
EquilibriumResult CationSystem::calculateFreeToTotal(double totalLigand, double freeMetal,
                                                   const std::string& ligandName, const std::string& metalName) {
    EquilibriumResult result;

    // Get ligand and metal
    const Ligand* ligand = GetLigandByName(ligandName);
    const Metal* metal = GetMetalByName(metalName);

    if (!ligand || !metal) {
        return result;
    }

    // Get adjusted binding constant for metal-ligand complex
    double logK_metal = calculateStabilityConstant(ligand, metalName);
    if (logK_metal <= 0.0) {
        return result;
    }

    double complexFormationConstant = std::pow(10.0, logK_metal);

    // K_metal is already corrected for pH in calculateStabilityConstant
    double effectiveK = complexFormationConstant;

    // For Free-to-Total: we know totalLigand and freeMetal, find totalMetal
    // We need to solve for freeLigand:
    // totalLigand = freeLigand + complex
    // complex = effectiveK * freeMetal * freeLigand
    // So: totalLigand = freeLigand + effectiveK * freeMetal * freeLigand
    // freeLigand = totalLigand / (1 + effectiveK * freeMetal)
    
    double freeLigand = totalLigand / (1.0 + effectiveK * freeMetal);
    double complex = effectiveK * freeMetal * freeLigand;
    double totalMetal = freeMetal + complex;

    result.freeLigand = freeLigand;
    result.freeMetal = freeMetal;
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

    double logK_metal = calculateStabilityConstant(ligand, metalName);
    if (logK_metal <= 0.0) {
        return result;
    }

    double complexFormationConstant = std::pow(10.0, logK_metal);

    // K_metal is already corrected for pH in calculateStabilityConstant
    double effectiveK = complexFormationConstant;

    // Calculate complex concentration using iterative method
    double complex = solveForFreeMetal(totalMetal, totalLigand, effectiveK, 1e-10, 1000);

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

// Multi-metal equilibrium: Total-to-Free (7-cation matrix solver)
EquilibriumMultiResult CationSystem::CalculateTotalToFreeMulti(double totalLigand,
                                                    const std::vector<double>& totalMetals,
                                                    const std::string& ligandName,
                                                    const std::vector<std::string>& metalNames) {
    EquilibriumMultiResult result;
    result.totalLigand = totalLigand;
    result.ionicStrength = params.ionicStrength;
    result.pH = params.pH;
    result.metalNames = metalNames;
    result.totalMetals = totalMetals;

    if (metalNames.size() != totalMetals.size() || metalNames.empty() || totalLigand <= 0) {
        return result;
    }

    const Ligand* ligand = GetLigandByName(ligandName);
    if (!ligand) {
        return result;
    }

    size_t n = metalNames.size();
    std::vector<double> Kvalues(n, 0.0);
    for (size_t i = 0; i < n; ++i) {
        double logK = calculateStabilityConstant(ligand, metalNames[i]);
        Kvalues[i] = (logK > 0.0) ? std::pow(10.0, logK) : 0.0;
    }

    // Solve for free ligand using bisection on f(L_free) = totalLigand - L_free - sum_i C_i(L_free)
    auto f = [&](double Lfree) {
        double sumComplex = 0.0;
        for (size_t i = 0; i < n; ++i) {
            sumComplex += calculateComplexFromLigandFree(totalMetals[i], Kvalues[i], Lfree);
        }
        return totalLigand - Lfree - sumComplex;
    };

    double lower = 0.0;
    double upper = totalLigand;
    double f_lower = f(lower);
    double f_upper = f(upper);

    // If at upper bound it is still positive, all ligand is free and no complex forms
    if (f_upper > 0.0) {
        result.freeLigand = totalLigand;
        result.freeMetals = totalMetals;
        result.complex.assign(n, 0.0);
        return result;
    }

    // Try Newton-Raphson on the full coupled system
    double Lfree = totalLigand * 0.5;
    std::vector<double> Mfree = totalMetals;
    bool nrOk = solveCoupledEquilibriumNewton(totalLigand, totalMetals, Kvalues,
                                              Lfree, Mfree, 1e-12, 200);

    if (nrOk) {
        result.freeLigand = Lfree;
        result.freeMetals = Mfree;
        result.complex.resize(n);

        for (size_t i = 0; i < n; ++i) {
            result.complex[i] = totalMetals[i] - result.freeMetals[i];
        }
        return result;
    }

    // Newton failed; fallback to bisection on single variable Lfree
    double mid = 0.0;
    for (int iter = 0; iter < 200; ++iter) {
        mid = (lower + upper) / 2.0;
        double f_mid = f(mid);

        if (std::abs(f_mid) < 1e-12) {
            break;
        }

        if (f_mid > 0) {
            lower = mid;
        } else {
            upper = mid;
        }
    }

    Lfree = mid;
    result.freeLigand = Lfree;
    result.complex.resize(n);
    result.freeMetals.resize(n);

    for (size_t i = 0; i < n; ++i) {
        result.complex[i] = calculateComplexFromLigandFree(totalMetals[i], Kvalues[i], Lfree);
        result.freeMetals[i] = totalMetals[i] - result.complex[i];
    }

    return result;
}

// Multi-metal equilibrium: Free-to-Total
EquilibriumMultiResult CationSystem::CalculateFreeToTotalMulti(double freeLigand,
                                                    const std::vector<double>& freeMetals,
                                                    const std::string& ligandName,
                                                    const std::vector<std::string>& metalNames) {
    EquilibriumMultiResult result;
    result.freeLigand = freeLigand;
    result.ionicStrength = params.ionicStrength;
    result.pH = params.pH;
    result.metalNames = metalNames;
    result.freeMetals = freeMetals;

    if (metalNames.size() != freeMetals.size() || metalNames.empty() || freeLigand < 0) {
        return result;
    }

    const Ligand* ligand = GetLigandByName(ligandName);
    if (!ligand) {
        return result;
    }

    size_t n = metalNames.size();
    result.totalMetals.resize(n);
    result.complex.resize(n);

    double Lfree = freeLigand;
    double sumComplex = 0.0;

    for (size_t i = 0; i < n; ++i) {
        double logK = calculateStabilityConstant(ligand, metalNames[i]);
        double K = (logK > 0.0) ? std::pow(10.0, logK) : 0.0;

        if (K <= 0.0 || freeMetals[i] < 0.0) {
            result.complex[i] = 0.0;
            result.totalMetals[i] = freeMetals[i];
        } else {
            result.complex[i] = calculateComplexFromLigandFree(freeMetals[i], K, Lfree);
            result.totalMetals[i] = freeMetals[i] + result.complex[i];
        }
        sumComplex += result.complex[i];
    }

    result.totalLigand = freeLigand + sumComplex;
    return result;
}

// Constrained multi-metal: find total concentration of one metal to achieve target free concentration
EquilibriumResult CationSystem::CalculateConstrainedMulti(double totalLigand,
                                                         const std::vector<double>& knownTotalMetals,
                                                         const std::vector<std::string>& knownMetalNames,
                                                         double targetFreeMetal,
                                                         const std::string& targetMetalName,
                                                         const std::string& ligandName) {
    EquilibriumResult result;

    if (knownTotalMetals.size() != knownMetalNames.size()) {
        return result;
    }

    const Ligand* ligand = GetLigandByName(ligandName);
    const Metal* targetMetal = GetMetalByName(targetMetalName);
    if (!ligand || !targetMetal) {
        return result;
    }

    // Get binding constants
    size_t n_known = knownMetalNames.size();
    std::vector<double> K_known(n_known, 0.0);
    for (size_t i = 0; i < n_known; ++i) {
        double logK = calculateStabilityConstant(ligand, knownMetalNames[i]);
        K_known[i] = (logK > 0.0) ? std::pow(10.0, logK) : 0.0;
    }

    double K_target = 0.0;
    double logK_target = calculateStabilityConstant(ligand, targetMetalName);
    K_target = (logK_target > 0.0) ? std::pow(10.0, logK_target) : 0.0;

    // Function to evaluate: for a given total_target, what is free_target?
    auto evaluateFreeTarget = [&](double totalTarget) -> double {
        // Iterative solution for competitive binding
        double Lfree = totalLigand; // Initial guess

        for (int iter = 0; iter < 100; ++iter) {
            // Calculate free concentrations
            double free_target = totalTarget / (1.0 + K_target * Lfree);

            std::vector<double> free_known(n_known);
            for (size_t i = 0; i < n_known; ++i) {
                free_known[i] = knownTotalMetals[i] / (1.0 + K_known[i] * Lfree);
            }

            // Calculate new free ligand
            double denominator = 1.0 + K_target * free_target;
            for (size_t i = 0; i < n_known; ++i) {
                denominator += K_known[i] * free_known[i];
            }
            double new_Lfree = totalLigand / denominator;

            // Check convergence
            double diff = std::abs(new_Lfree - Lfree);
            Lfree = new_Lfree;

            if (diff < 1e-12) break;
        }

        // Now calculate the actual free target
        double free_target = totalTarget / (1.0 + K_target * Lfree);

        return free_target;
    };

    // Find total_target such that free_target = targetFreeMetal
    double total_lower = targetFreeMetal; // Minimum possible
    double total_upper = targetFreeMetal * 1e5; // Large upper bound
    double best_total = targetFreeMetal;
    double best_error = 1e9;

    for (int iter = 0; iter < 50; ++iter) {
        double total_mid = (total_lower + total_upper) / 2.0;
        double free_result = evaluateFreeTarget(total_mid);
        double error = std::abs(free_result - targetFreeMetal);

        if (error < best_error) {
            best_error = error;
            best_total = total_mid;
        }

        if (free_result > targetFreeMetal) {
            total_upper = total_mid;
        } else {
            total_lower = total_mid;
        }

        if (error < 1e-12) {
            break;
        }
    }

    // Now compute the final result using the best total
    double final_Lfree = 0.0;
    auto f_final = [&](double Lfree) -> double {
        double sumComplex = 0.0;
        for (size_t i = 0; i < n_known; ++i) {
            sumComplex += calculateComplexFromLigandFree(knownTotalMetals[i], K_known[i], Lfree);
        }
        double complex_target = (K_target * best_total * Lfree) / (1.0 + K_target * Lfree);
        sumComplex += complex_target;
        return totalLigand - Lfree - sumComplex;
    };

    // Find final Lfree
    double L_lower = 0.0;
    double L_upper = totalLigand;
    for (int iter = 0; iter < 100; ++iter) {
        final_Lfree = (L_lower + L_upper) / 2.0;
        double f_val = f_final(final_Lfree);
        if (std::abs(f_val) < 1e-12) {
            break;
        }
        if (f_val > 0) {
            L_lower = final_Lfree;
        } else {
            L_upper = final_Lfree;
        }
    }

    // Calculate final values
    double final_complex_target = (K_target * best_total * final_Lfree) / (1.0 + K_target * final_Lfree);
    double final_free_target = best_total - final_complex_target;

    // Calculate free known metals
    std::vector<double> free_known(n_known);
    for (size_t i = 0; i < n_known; ++i) {
        double complex_known = calculateComplexFromLigandFree(knownTotalMetals[i], K_known[i], final_Lfree);
        free_known[i] = knownTotalMetals[i] - complex_known;
    }

    result.totalMetal = best_total;
    if (n_known > 0) {
        result.freeMetal = free_known[0]; // Free known metal (first in vector)
    } else {
        // With no known competing metal, the target metal is the only metal
        double target_complex = (K_target * best_total * final_Lfree) / (1.0 + K_target * final_Lfree);
        result.freeMetal = best_total - target_complex;
    }
    result.freeLigand = final_Lfree;
    result.complex = final_complex_target;
    result.totalLigand = totalLigand;
    result.ionicStrength = params.ionicStrength;
    result.pH = params.pH;

    return result;
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

// Calculate equilibrium concentrations (both directions)
EquilibriumResult CationSystem::CalculateEquilibrium(double totalLigand, double totalMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateTotalToFree(totalLigand, totalMetal, ligandName, metalName);
}

// Calculate equilibrium concentrations (Free-to-Total)
EquilibriumResult CationSystem::CalculateFreeToTotal(double totalLigand, double freeMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateFreeToTotal(totalLigand, freeMetal, ligandName, metalName);
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