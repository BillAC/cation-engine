#ifndef SOLVER_H
#define SOLVER_H

#include "Ligand.h"
#include <string>
#include <vector>
#include <map>

// System parameters structure
struct SystemParameters {
    double temperature;     // Temperature in Celsius
    double ionicStrength;   // Ionic strength in mM
    double pH;              // pH value
    double volume;          // Volume in liters

    SystemParameters() : temperature(25.0), ionicStrength(100.0), pH(7.0), volume(1.0) {}
};

// Equilibrium result structure
struct EquilibriumResult {
    double freeLigand;
    double freeMetal;
    double complex;
    double totalLigand;
    double totalMetal;
    double ionicStrength;
    double pH;

    EquilibriumResult() : freeLigand(0.0), freeMetal(0.0), complex(0.0),
                         totalLigand(0.0), totalMetal(0.0), ionicStrength(0.0), pH(0.0) {}
};

// Cation system class
class CationSystem {
private:
    std::vector<Ligand> ligands;
    std::vector<Metal> metals;
    SystemParameters params;
    std::map<std::string, double> adjustedConstants;

    // Helper functions
    double calculateProtonationFraction(double pH, double logK); // Single step (deprecated)
    double calculateProtonationFraction(double pH, const Ligand* ligand); // Multi-step (new)
    double calculateIonicStrengthCorrection(double ionicStrength, double charge);
    double calculateTemperatureCorrection(double temperature, double deltaH);
    double calculateStabilityConstant(double logK, double pH, double ionicStrength);
    double getMetalBindingConstant(const Ligand* ligand, const std::string& metalName);
    double getMetalEnthalpyConstant(const Ligand* ligand, const std::string& metalName);
    double solveForFreeMetal(double totalMetal, double totalLigand, double complexFormationConstant,
                            double tolerance = 1e-12, int maxIterations = 1000);

    // Internal calculation functions
    EquilibriumResult calculateFreeToTotal(double freeLigand, double freeMetal,
                                         const std::string& ligandName, const std::string& metalName);
    EquilibriumResult calculateTotalToFree(double totalLigand, double totalMetal,
                                         const std::string& ligandName, const std::string& metalName);

public:
    // Constructor and destructor
    CationSystem();
    ~CationSystem();

    // Initialization
    void Initialize(const std::vector<std::string>& ligandNames,
                   const std::vector<std::string>& metalNames);

    // Set system parameters
    void SetParameters(const SystemParameters& systemParams);

    // Calculation methods
    EquilibriumResult CalculateEquilibrium(double totalLigand, double totalMetal,
                                         const std::string& ligandName, const std::string& metalName);
    EquilibriumResult CalculateFreeToTotal(double freeLigand, double freeMetal,
                                         const std::string& ligandName, const std::string& metalName);
    EquilibriumResult CalculateTotalToFree(double totalLigand, double totalMetal,
                                         const std::string& ligandName, const std::string& metalName);

    // Data management
    void AddLigand(const Ligand& ligand);
    void AddMetal(const Metal& metal);
    void Clear();

    // Getters
    std::vector<std::string> GetAvailableLigands() const;
    std::vector<std::string> GetAvailableMetals() const;
    double GetAdjustedEquilibriumConstant(const std::string& ligandName, const std::string& metalName);

    // Get stability constants for a ligand (from unified API)
    StabilityConstants GetStabilityConstants(const std::string& ligandName) const;

    // Get metal properties (from unified API)
    Metal GetMetalProperties(const std::string& metalName) const;

    // Set equilibrium constants (from unified API)
    void SetEquilibriumConstant(const std::string& key, double constant);

    // Get equilibrium constant (from unified API)
    double GetEquilibriumConstant(const std::string& key) const;
};

#endif // SOLVER_H