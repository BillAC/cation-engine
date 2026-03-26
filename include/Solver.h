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

// Result struct for multi-metal equilibrium calculations
struct EquilibriumMultiResult {
    double freeLigand;
    double totalLigand;
    std::vector<std::string> metalNames;
    std::vector<double> freeMetals;
    std::vector<double> totalMetals;
    std::vector<double> complex;
    double ionicStrength;
    double pH;

    EquilibriumMultiResult() : freeLigand(0), totalLigand(0), ionicStrength(0), pH(0) {}
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
    double calculateProtonationFraction(double pH, double logK); // Single step (deprecated)
    double calculateProtonationFraction(double pH, const Ligand* ligand); // Multi-step (new)
    double calculateIonicStrengthCorrection(double ionicStrength, double charge);
    double calculateTemperatureCorrection(double temperature, double deltaH);
    double calculateStabilityConstant(double logK, double pH, double ionicStrength);
    double calculateStabilityConstant(const Ligand* ligand, const std::string& metalName);
    
    // Get metal-specific binding constant for a ligand-metal pair
    double getMetalBindingConstant(const Ligand* ligand, const std::string& metalName);
    
    // Get metal-specific enthalpy constant for a ligand-metal pair
    double getMetalEnthalpyConstant(const Ligand* ligand, const std::string& metalName);

    // Iterative bounding solver for 1:1 complex formation
    double solveForFreeMetal(double totalMetal, double totalLigand, double complexFormationConstant,
                           double tolerance = 1e-10, int maxIterations = 1000);

    // Helper for multispecies 1:1 complex formation, given free ligand
    double calculateComplexFromLigandFree(double totalMetal, double K, double freeLigand);

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
    EquilibriumResult CalculateFreeToTotal(double totalLigand, double freeMetal,
                                         const std::string& ligandName, const std::string& metalName);

    // Calculate equilibrium concentrations (Total-to-Free)
    EquilibriumResult CalculateTotalToFree(double totalLigand, double totalMetal,
                                         const std::string& ligandName, const std::string& metalName);

    // Multi-metal equilibrium (7-cation matrix solver)
    EquilibriumMultiResult CalculateTotalToFreeMulti(double totalLigand,
                                                    const std::vector<double>& totalMetals,
                                                    const std::string& ligandName,
                                                    const std::vector<std::string>& metalNames);

    EquilibriumMultiResult CalculateFreeToTotalMulti(double freeLigand,
                                                    const std::vector<double>& freeMetals,
                                                    const std::string& ligandName,
                                                    const std::vector<std::string>& metalNames);

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

    // Get stability constants for a ligand
    StabilityConstants GetStabilityConstants(const std::string& ligandName) const;

    // Get metal properties
    Metal GetMetalProperties(const std::string& metalName) const;

    // Set equilibrium constants (for custom calculations)
    void SetEquilibriumConstant(const std::string& key, double constant);

    // Get equilibrium constant
    double GetEquilibriumConstant(const std::string& key) const;
};

#endif // SOLVER_H