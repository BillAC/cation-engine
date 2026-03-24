#ifndef SOLVER_H
#define SOLVER_H

#include "Ligand.h"
#include <vector>
#include <string>
#include <map>

// Structure to hold equilibrium results
struct EquilibriumResult {
    double freeLigand;
    double freeMetal;
    double complex;
    double totalLigand;
    double totalMetal;
    double ionicStrength;
    double pH;

    EquilibriumResult() : freeLigand(0), freeMetal(0), complex(0),
                         totalLigand(0), totalMetal(0), ionicStrength(0), pH(0) {}
};

// Structure to hold system parameters
struct SystemParameters {
    double temperature;      // Temperature in Celsius
    double ionicStrength;    // Ionic strength in mM
    double pH;               // pH value
    double volume;           // Volume in liters

    SystemParameters() : temperature(25.0), ionicStrength(0.0), pH(7.0), volume(1.0) {}
};

// Main cation-ligand system solver
class CationSystem {
private:
    // System parameters
    SystemParameters params;

    // Ligands and metals in the system
    std::vector<Ligand> ligands;
    std::vector<Metal> metals;

    // Equilibrium constants adjusted for pH and ionic strength
    std::map<std::string, double> adjustedConstants;

    // Internal helper methods
    double calculateProtonationFraction(double pH, double logK);
    double calculateIonicStrengthCorrection(double ionicStrength, double charge);
    double calculateTemperatureCorrection(double temperature, double deltaH);
    double calculateStabilityConstant(double logK, double pH, double ionicStrength, double deltaH);

    // Iterative bounding solver for 1:1 complex formation
    double solveForFreeMetal(double totalMetal, double totalLigand, double complexFormationConstant,
                           double tolerance = 1e-10, int maxIterations = 1000);

    // Two-way calculation methods
    EquilibriumResult calculateFreeToTotal(double freeLigand, double freeMetal,
                                         const std::string& ligandName, const std::string& metalName);

    EquilibriumResult calculateTotalToFree(double totalLigand, double totalMetal,
                                         const std::string& ligandName, const std::string& metalName);

public:
    // Constructor
    CationSystem();

    // Destructor
    ~CationSystem();

    // Initialize with specific ligands and metals
    void Initialize(const std::vector<std::string>& ligandNames,
                   const std::vector<std::string>& metalNames);

    // Set system parameters
    void SetParameters(const SystemParameters& systemParams);

    // Calculate equilibrium concentrations (Free-to-Total)
    EquilibriumResult CalculateFreeToTotal(double freeLigand, double freeMetal,
                                         const std::string& ligandName, const std::string& metalName);

    // Calculate equilibrium concentrations (Total-to-Free)
    EquilibriumResult CalculateTotalToFree(double totalLigand, double totalMetal,
                                         const std::string& ligandName, const std::string& metalName);

    // Calculate equilibrium concentrations (both directions)
    EquilibriumResult CalculateEquilibrium(double totalLigand, double totalMetal,
                                         const std::string& ligandName, const std::string& metalName);

    // Add ligand to the system
    void AddLigand(const Ligand& ligand);

    // Add metal to the system
    void AddMetal(const Metal& metal);

    // Clear all data
    void Clear();

    // Get list of available ligands
    std::vector<std::string> GetAvailableLigands() const;

    // Get list of available metals
    std::vector<std::string> GetAvailableMetals() const;

    // Get adjusted equilibrium constant for a ligand-metal pair
    double GetAdjustedEquilibriumConstant(const std::string& ligandName, const std::string& metalName);
};

#endif // SOLVER_H