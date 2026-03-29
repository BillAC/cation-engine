#include "Solver.h"
#include "LigandCSVLoader.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>

// Constructor
CationSystem::CationSystem() {
    // Initialize with empty ligands and metals - data will be loaded from CSV
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
double CationSystem::calculateProtonationFraction(double pH, const Ligand* ligand) {
    if (!ligand) return 1.0;
    
    double pK1 = ligand->constants.log_K1;
    double pK2 = ligand->constants.log_K2;
    double pK3 = ligand->constants.log_K3;
    double pK4 = ligand->constants.log_K4;
    
    double D = 1.0; // Ln- (fully deprotonated)
    if (pK1 > 0) D += std::pow(10, pK1 - pH);
    if (pK1 > 0 && pK2 > 0) D += std::pow(10, pK1 + pK2 - 2*pH);
    if (pK1 > 0 && pK2 > 0 && pK3 > 0) D += std::pow(10, pK1 + pK2 + pK3 - 3*pH);
    if (pK1 > 0 && pK2 > 0 && pK3 > 0 && pK4 > 0) D += std::pow(10, pK1 + pK2 + pK3 + pK4 - 4*pH);
    
    return 1.0 / D;
}

double CationSystem::calculateProtonationFraction(double pH, double logK) {
    return 1.0 / (1.0 + std::pow(10, logK - pH));
}

double CationSystem::calculateIonicStrengthCorrection(double ionicStrength, double charge) {
    if (ionicStrength <= 0) return 1.0;
    double I_molar = ionicStrength / 1000.0;
    const double A = 0.51;
    double sqrtI = std::sqrt(I_molar);
    double logGamma = -A * charge * charge * (sqrtI / (1.0 + sqrtI) - 0.3 * I_molar);
    return std::pow(10, logGamma);
}

double CationSystem::calculateTemperatureCorrection(double temperature, double deltaH) {
    const double R = 8.314;
    const double T1 = 298.15;
    double T2 = temperature + 273.15;
    if (deltaH == 0.0 || T2 == T1) return 1.0;
    double deltaH_J = deltaH * 4184.0;
    double lnK = -deltaH_J / R * (1.0 / T2 - 1.0 / T1);
    return std::exp(lnK);
}

double CationSystem::calculateStabilityConstant(double logK, double pH, double ionicStrength) {
    if (logK <= 0.0) return 0.0;
    double K = std::pow(10.0, logK);
    double gamma_Ca = calculateIonicStrengthCorrection(ionicStrength, 2.0);
    double gamma_L = calculateIonicStrengthCorrection(ionicStrength, -4.0);
    double gamma_complex = calculateIonicStrengthCorrection(ionicStrength, -2.0);
    double adjustedK = K * (gamma_Ca * gamma_L / gamma_complex);
    return adjustedK > 0.0 ? std::log10(adjustedK) : 0.0;
}

double CationSystem::calculateStabilityConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    double logK = getMetalBindingConstant(ligand, metalName);
    if (logK <= 0.0) return 0.0;
    double K = std::pow(10.0, logK);
    double deltaH = getMetalEnthalpyConstant(ligand, metalName);
    double tempCorr = calculateTemperatureCorrection(params.temperature, deltaH);
    double alpha = calculateProtonationFraction(params.pH, ligand);
    double effectiveK = K * tempCorr * alpha;
    return effectiveK > 0.0 ? std::log10(effectiveK) : 0.0;
}

double CationSystem::getMetalBindingConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    if (metalName == "Ca2") return ligand->constants.Ca1;
    if (metalName == "Mg2") return ligand->constants.Mg1;
    if (metalName == "Ba2") return ligand->constants.Ba1;
    if (metalName == "Cd2") return ligand->constants.Cd1;
    if (metalName == "Sr2") return ligand->constants.Sr1;
    if (metalName == "Mn2") return ligand->constants.Mn1;
    if (metalName == "X1")  return ligand->constants.X1;
    if (metalName == "Cu2") return ligand->constants.Cu1;
    if (metalName == "Zn2") return ligand->constants.Zn1;
    return 0.0;
}

double CationSystem::getMetalEnthalpyConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    if (metalName == "Ca2") return ligand->constants.dCa1;
    if (metalName == "Mg2") return ligand->constants.dMg1;
    if (metalName == "Ba2") return ligand->constants.dBa1;
    if (metalName == "Cd2") return ligand->constants.dCd1;
    if (metalName == "Sr2") return ligand->constants.dSr1;
    if (metalName == "Mn2") return ligand->constants.dMn1;
    if (metalName == "X1")  return ligand->constants.dX1;
    if (metalName == "Cu2") return ligand->constants.dCu1;
    if (metalName == "Zn2") return ligand->constants.dZn1;
    return 0.0;
}

double CationSystem::calculateComplexFromLigandFree(double totalMetal, double K, double freeLigand) {
    if (K <= 0.0 || totalMetal <= 0.0 || freeLigand <= 0.0) return 0.0;
    double part = K * freeLigand;
    return totalMetal * part / (1.0 + part);
}

double CationSystem::solveForFreeMetal(double totalMetal, double totalLigand, double complexFormationConstant,
                                      double tolerance, int maxIterations) {
    double low = 0.0;
    double high = std::min(totalMetal, totalLigand);
    if (high < tolerance) return 0.0;
    for (int i = 0; i < maxIterations; ++i) {
        double complex = (low + high) / 2.0;
        double predictedComplex = complexFormationConstant * (totalMetal - complex) * (totalLigand - complex);
        if (predictedComplex > complex) low = complex;
        else high = complex;
        if (std::abs(predictedComplex - complex) < tolerance) return complex;
    }
    return (low + high) / 2.0;
}

bool CationSystem::solveCoupledEquilibriumNewton(double totalLigand, const std::vector<double>& totalMetals,
                                                 const std::vector<double>& Kvalues, double& freeLigand,
                                                 std::vector<double>& freeMetals, double tolerance, int maxIterations) {
    size_t n = totalMetals.size();
    if (n == 0 || totalLigand <= 0) return false;
    if (freeLigand <= 0.0 || freeLigand > totalLigand) freeLigand = totalLigand * 0.5;
    freeMetals.resize(n);
    for (size_t i = 0; i < n; ++i) freeMetals[i] = totalMetals[i] / (1.0 + Kvalues[i] * freeLigand);

    for (int iter = 0; iter < maxIterations; ++iter) {
        std::vector<double> res(n + 1);
        double sumComplex = 0.0;
        for (size_t i = 0; i < n; ++i) {
            double c = Kvalues[i] * freeMetals[i] * freeLigand;
            sumComplex += c;
            res[i + 1] = totalMetals[i] - freeMetals[i] - c;
        }
        res[0] = totalLigand - freeLigand - sumComplex;

        double maxErr = 0.0;
        for (double r : res) maxErr = std::max(maxErr, std::abs(r));
        if (maxErr < tolerance) return true;

        std::vector<std::vector<double>> J(n + 1, std::vector<double>(n + 1, 0.0));
        double sumKM = 0.0;
        for (size_t i = 0; i < n; ++i) sumKM += Kvalues[i] * freeMetals[i];
        J[0][0] = -1.0 - sumKM;
        for (size_t i = 0; i < n; ++i) {
            J[0][i+1] = -Kvalues[i] * freeLigand;
            J[i+1][0] = -Kvalues[i] * freeMetals[i];
            J[i+1][i+1] = -1.0 - Kvalues[i] * freeLigand;
        }

        std::vector<std::vector<double>> A = J;
        std::vector<double> b = res;
        for (size_t i = 0; i < n + 1; ++i) {
            size_t pivot = i;
            for (size_t k = i + 1; k < n + 1; ++k) if (std::abs(A[k][i]) > std::abs(A[pivot][i])) pivot = k;
            std::swap(A[i], A[pivot]); std::swap(b[i], b[pivot]);
            if (std::abs(A[i][i]) < 1e-20) return false;
            for (size_t k = i + 1; k < n + 1; ++k) {
                double f = A[k][i] / A[i][i];
                for (size_t l = i; l < n + 1; ++l) A[k][l] -= f * A[i][l];
                b[k] -= f * b[i];
            }
        }
        std::vector<double> dx(n + 1);
        for (int i = (int)n; i >= 0; --i) {
            double s = 0;
            for (size_t k = i + 1; k < n + 1; ++k) s += A[i][k] * dx[k];
            dx[i] = (b[i] - s) / A[i][i];
        }

        double damping = 0.5;
        freeLigand = std::max(1e-18, std::min(totalLigand, freeLigand - damping * dx[0]));
        for (size_t i = 0; i < n; ++i) freeMetals[i] = std::max(1e-18, std::min(totalMetals[i], freeMetals[i] - damping * dx[i+1]));
    }
    return false;
}

EquilibriumResult CationSystem::calculateFreeToTotal(double totalLigand, double freeMetal,
                                                   const std::string& ligandName, const std::string& metalName) {
    EquilibriumResult res;
    const Ligand* l = GetLigandByName(ligandName);
    if (!l) return res;
    double logK = calculateStabilityConstant(l, metalName);
    double K = std::pow(10.0, logK);
    double freeL = totalLigand / (1.0 + K * freeMetal);
    double c = K * freeMetal * freeL;
    res.freeLigand = freeL; res.freeMetal = freeMetal; res.complex = c;
    res.totalLigand = totalLigand; res.totalMetal = freeMetal + c;
    res.pH = params.pH; res.ionicStrength = params.ionicStrength;
    return res;
}

EquilibriumResult CationSystem::calculateTotalToFree(double totalLigand, double totalMetal,
                                                   const std::string& ligandName, const std::string& metalName) {
    EquilibriumResult res;
    const Ligand* l = GetLigandByName(ligandName);
    if (!l) return res;
    double logK = calculateStabilityConstant(l, metalName);
    double K = std::pow(10.0, logK);
    double c = solveForFreeMetal(totalMetal, totalLigand, K);
    res.freeLigand = totalLigand - c; res.freeMetal = totalMetal - c; res.complex = c;
    res.totalLigand = totalLigand; res.totalMetal = totalMetal;
    res.pH = params.pH; res.ionicStrength = params.ionicStrength;
    return res;
}

EquilibriumMultiResult CationSystem::CalculateTotalToFreeMulti(double totalLigand, const std::vector<double>& totalMetals,
                                                    const std::string& ligandName, const std::vector<std::string>& metalNames) {
    EquilibriumMultiResult res;
    res.totalLigand = totalLigand; res.totalMetals = totalMetals; res.metalNames = metalNames;
    const Ligand* l = GetLigandByName(ligandName);
    if (!l) return res;
    size_t n = metalNames.size();
    std::vector<double> K(n);
    for (size_t i = 0; i < n; ++i) K[i] = std::pow(10.0, calculateStabilityConstant(l, metalNames[i]));
    
    double Lf = totalLigand * 0.5;
    std::vector<double> Mf(n);
    if (!solveCoupledEquilibriumNewton(totalLigand, totalMetals, K, Lf, Mf)) {
        // Fallback to bisection
        double low = 0, high = totalLigand;
        for (int i = 0; i < 100; ++i) {
            Lf = (low + high) / 2.0;
            double sumC = 0;
            for (size_t j = 0; j < n; ++j) sumC += calculateComplexFromLigandFree(totalMetals[j], K[j], Lf);
            if (totalLigand - Lf - sumC > 0) low = Lf; else high = Lf;
        }
        for (size_t j = 0; j < n; ++j) Mf[j] = totalMetals[j] / (1.0 + K[j] * Lf);
    }
    res.freeLigand = Lf; res.freeMetals = Mf; res.complex.resize(n);
    for (size_t i = 0; i < n; ++i) res.complex[i] = totalMetals[i] - Mf[i];
    return res;
}

EquilibriumMultiResult CationSystem::CalculateFreeToTotalMulti(double freeLigand, const std::vector<double>& freeMetals,
                                                    const std::string& ligandName, const std::vector<std::string>& metalNames) {
    EquilibriumMultiResult res;
    res.freeLigand = freeLigand; res.freeMetals = freeMetals; res.metalNames = metalNames;
    const Ligand* l = GetLigandByName(ligandName);
    if (!l) return res;
    size_t n = metalNames.size();
    res.totalMetals.resize(n); res.complex.resize(n);
    double sumC = 0;
    for (size_t i = 0; i < n; ++i) {
        double K = std::pow(10.0, calculateStabilityConstant(l, metalNames[i]));
        res.complex[i] = K * freeLigand * freeMetals[i];
        res.totalMetals[i] = freeMetals[i] + res.complex[i];
        sumC += res.complex[i];
    }
    res.totalLigand = freeLigand + sumC;
    return res;
}

EquilibriumResult CationSystem::CalculateConstrainedMulti(double totalLigand, const std::vector<double>& knownTotals,
                                               const std::vector<std::string>& knownNames, double targetFree,
                                               const std::string& targetName, const std::string& ligandName) {
    EquilibriumResult res;
    const Ligand* l = GetLigandByName(ligandName);
    if (!l) return res;
    size_t n = knownNames.size();
    std::vector<double> Kk(n);
    for (size_t i = 0; i < n; ++i) Kk[i] = std::pow(10.0, calculateStabilityConstant(l, knownNames[i]));
    double Kt = std::pow(10.0, calculateStabilityConstant(l, targetName));

    auto f = [&](double Lf) {
        double sumC = Kt * Lf * targetFree;
        for (size_t i = 0; i < n; ++i) sumC += (Kk[i] * Lf * knownTotals[i]) / (1.0 + Kk[i] * Lf);
        return totalLigand - Lf - sumC;
    };

    double low = 0, high = totalLigand, Lf = 0;
    for (int i = 0; i < 100; ++i) {
        Lf = (low + high) / 2.0;
        if (f(Lf) > 0) low = Lf; else high = Lf;
    }
    res.freeLigand = Lf; res.freeMetal = targetFree;
    res.complex = Kt * Lf * targetFree;
    res.totalMetal = targetFree + res.complex;
    res.totalLigand = totalLigand;
    if (n > 0) res.freeMetal = knownTotals[0] / (1.0 + Kk[0] * Lf);
    return res;
}

CationSystem::MultiSpeciesResult CationSystem::CalculateSimultaneousEquilibrium(
    const std::vector<std::string>& ligandNames, const std::vector<double>& totalLigands,
    const std::vector<std::string>& metalNames, const std::vector<double>& totalMetals) {

    MultiSpeciesResult result;
    result.ligandNames = ligandNames; result.totalLigands = totalLigands;
    result.metalNames = metalNames; result.totalMetals = totalMetals;
    result.converged = false;
    size_t m = ligandNames.size(), n = metalNames.size(), dim = m + n;
    if (m == 0 || n == 0) return result;

    std::vector<std::vector<double>> K(m, std::vector<double>(n));
    for (size_t i = 0; i < m; ++i) {
        const Ligand* l = GetLigandByName(ligandNames[i]);
        for (size_t j = 0; j < n; ++j) K[i][j] = std::pow(10.0, calculateStabilityConstant(l, metalNames[j]));
    }

    std::vector<double> x(dim);
    for (size_t i = 0; i < m; ++i) x[i] = totalLigands[i] * 0.5;
    for (size_t j = 0; j < n; ++j) x[m + j] = totalMetals[j] * 0.5;

    for (int iter = 0; iter < 200; ++iter) {
        std::vector<double> res(dim);
        for (size_t i = 0; i < m; ++i) {
            double sumC = 0;
            for (size_t j = 0; j < n; ++j) sumC += K[i][j] * x[i] * x[m + j];
            res[i] = totalLigands[i] - x[i] - sumC;
        }
        for (size_t j = 0; j < n; ++j) {
            double sumC = 0;
            for (size_t i = 0; i < m; ++i) sumC += K[i][j] * x[i] * x[m + j];
            res[m + j] = totalMetals[j] - x[m + j] - sumC;
        }

        double maxErr = 0;
        for (double r : res) maxErr = std::max(maxErr, std::abs(r));
        if (maxErr < 1e-12) { result.converged = true; break; }

        std::vector<std::vector<double>> J(dim, std::vector<double>(dim, 0.0));
        for (size_t i = 0; i < m; ++i) {
            double sumKM = 0;
            for (size_t j = 0; j < n; ++j) sumKM += K[i][j] * x[m+j];
            J[i][i] = -1.0 - sumKM;
            for (size_t j = 0; j < n; ++j) J[i][m+j] = -K[i][j] * x[i];
        }
        for (size_t j = 0; j < n; ++j) {
            double sumKL = 0;
            for (size_t i = 0; i < m; ++i) sumKL += K[i][j] * x[i];
            J[m+j][m+j] = -1.0 - sumKL;
            for (size_t i = 0; i < m; ++i) J[m+j][i] = -K[i][j] * x[m+j];
        }

        std::vector<std::vector<double>> A = J;
        std::vector<double> b = res;
        for (size_t i = 0; i < dim; ++i) {
            size_t pivot = i;
            for (size_t k = i+1; k < dim; ++k) if (std::abs(A[k][i]) > std::abs(A[pivot][i])) pivot = k;
            std::swap(A[i], A[pivot]); std::swap(b[i], b[pivot]);
            if (std::abs(A[i][i]) < 1e-20) continue;
            for (size_t k = i+1; k < dim; ++k) {
                double f = A[k][i] / A[i][i];
                for (size_t l = i; l < dim; ++l) A[k][l] -= f * A[i][l];
                b[k] -= f * b[i];
            }
        }
        std::vector<double> dx(dim);
        for (int i = (int)dim-1; i >= 0; --i) {
            double s = 0;
            for (size_t k = i+1; k < dim; ++k) s += A[i][k] * dx[k];
            dx[i] = (b[i] - s) / A[i][i];
        }
        double damping = 0.5;
        for (size_t i = 0; i < m; ++i) x[i] = std::max(1e-18, std::min(totalLigands[i], x[i] - damping * dx[i]));
        for (size_t j = 0; j < n; ++j) x[m+j] = std::max(1e-18, std::min(totalMetals[j], x[m+j] - damping * dx[m+j]));
    }
    result.freeLigands.assign(x.begin(), x.begin() + m);
    result.freeMetals.assign(x.begin() + m, x.end());
    result.complexes.assign(m, std::vector<double>(n));
    for (size_t i = 0; i < m; ++i) for (size_t j = 0; j < n; ++j) result.complexes[i][j] = K[i][j] * result.freeLigands[i] * result.freeMetals[j];
    return result;
}

EquilibriumResult CationSystem::CalculateEquilibrium(double totalLigand, double totalMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateTotalToFree(totalLigand, totalMetal, ligandName, metalName);
}

EquilibriumResult CationSystem::CalculateFreeToTotal(double totalLigand, double freeMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateFreeToTotal(totalLigand, freeMetal, ligandName, metalName);
}

EquilibriumResult CationSystem::CalculateTotalToFree(double totalLigand, double totalMetal,
                                                    const std::string& ligandName, const std::string& metalName) {
    return calculateTotalToFree(totalLigand, totalMetal, ligandName, metalName);
}

void CationSystem::AddLigand(const Ligand& ligand) {
    ligands.push_back(ligand);
}

void CationSystem::AddMetal(const Metal& metal) {
    metals.push_back(metal);
}

void CationSystem::Clear() {
    ligands.clear();
    metals.clear();
    adjustedConstants.clear();
}

std::vector<std::string> CationSystem::GetAvailableLigands() const {
    std::vector<std::string> names;
    for (const auto& ligand : ligands) names.push_back(ligand.name);
    return names;
}

std::vector<std::string> CationSystem::GetAvailableMetals() const {
    std::vector<std::string> names;
    for (const auto& metal : metals) names.push_back(metal.name);
    return names;
}

double CationSystem::GetAdjustedEquilibriumConstant(const std::string& ligandName, const std::string& metalName) {
    std::string key = ligandName + "_" + metalName;
    auto it = adjustedConstants.find(key);
    if (it != adjustedConstants.end()) return it->second;
    const Ligand* ligand = GetLigandByName(ligandName);
    if (!ligand) return 0.0;
    double adjustedK = std::pow(10.0, calculateStabilityConstant(ligand, metalName));
    adjustedConstants[key] = adjustedK;
    return adjustedK;
}

StabilityConstants CationSystem::GetStabilityConstants(const std::string& ligandName) const {
    const Ligand* ligand = GetLigandByName(ligandName);
    if (ligand) return ligand->constants;
    return StabilityConstants();
}

Metal CationSystem::GetMetalProperties(const std::string& metalName) const {
    const Metal* metal = GetMetalByName(metalName);
    if (metal) return *metal;
    return Metal();
}

void CationSystem::SetEquilibriumConstant(const std::string& key, double constant) {
    adjustedConstants[key] = constant;
}

double CationSystem::GetEquilibriumConstant(const std::string& key) const {
    auto it = adjustedConstants.find(key);
    if (it != adjustedConstants.end()) return it->second;
    return 0.0;
}
