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
    double calculateProtonationFraction(double pH, double logK);
    double calculateIonicStrengthCorrection(double ionicStrength, double charge);
    double calculateTemperatureCorrection(double temperature, double deltaH);
    double calculateStabilityConstant(double logK, double pH, double ionicStrength);
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
};

#endif // SOLVER_H