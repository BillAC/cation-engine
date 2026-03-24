#ifndef CATION_ENGINE_H
#define CATION_ENGINE_H

#include "Ligand.h"
#include <vector>
#include <string>
#include <map>

// Structure to hold equilibrium concentrations
struct EquilibriumConcentrations {
    double ligand;      // Free ligand concentration
    double metal;       // Free metal concentration
    double complex;     // Complex concentration
    double total;       // Total concentration
};

// Main cation-ligand equilibrium engine
class CationEngine {
private:
    // Internal data structures
    std::vector<Ligand> ligands;
    std::vector<Metal> metals;

    // Equilibrium constants for different metal-ligand combinations
    std::map<std::string, double> equilibriumConstants;

public:
    // Constructor
    CationEngine();

    // Destructor
    ~CationEngine();

    // Initialize with specific ligands and metals
    void Initialize(const std::vector<std::string>& ligandNames,
                   const std::vector<std::string>& metalNames);

    // Calculate equilibrium concentrations
    EquilibriumConcentrations CalculateEquilibrium(double totalLigand,
                                                   double totalMetal,
                                                   double pH,
                                                   const std::string& ligandName,
                                                   const std::string& metalName);

    // Get stability constants for a ligand
    StabilityConstants GetStabilityConstants(const std::string& ligandName);

    // Get metal properties
    Metal GetMetalProperties(const std::string& metalName);

    // Set equilibrium constants (for custom calculations)
    void SetEquilibriumConstant(const std::string& key, double constant);

    // Get equilibrium constant
    double GetEquilibriumConstant(const std::string& key);

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
};

#endif // CATION_ENGINE_H