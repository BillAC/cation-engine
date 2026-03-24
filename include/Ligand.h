#ifndef LIGAND_H
#define LIGAND_H

#include <string>
#include <vector>
#include <array>

// Structure to hold stability constants for a ligand
struct StabilityConstants {
    double H1, H2, H3, H4;     // LogK values for 1-4 protonation steps
    double dH1, dH2, dH3, dH4; // Enthalpy values for 1-4 protonation steps

    StabilityConstants() : H1(0), H2(0), H3(0), H4(0), dH1(0), dH2(0), dH3(0), dH4(0) {}
    StabilityConstants(double h1, double h2, double h3, double h4,
                      double dh1, double dh2, double dh3, double dh4)
        : H1(h1), H2(h2), H3(h3), H4(h4), dH1(dh1), dH2(dh2), dH3(dh3), dH4(dh4) {}
};

// Structure to hold ligand information
struct Ligand {
    std::string name;           // Ligand name (e.g., "EDTA", "EGTA")
    int valence;                // Charge valence (e.g., 4 for EDTA, 2 for Mg2+)
    StabilityConstants constants; // Stability constants for this ligand

    // Constructor
    Ligand(const std::string& n, int v, const StabilityConstants& c)
        : name(n), valence(v), constants(c) {}

    // Default constructor
    Ligand() : valence(0) {}
};

// Structure to hold metal information
struct Metal {
    std::string name;           // Metal name (e.g., "Ca2", "Mg2")
    int charge;                 // Charge of the metal (e.g., 2 for Ca2+, Mg2+)
    double atomicWeight;        // Atomic weight for calculations

    Metal(const std::string& n, int c, double weight)
        : name(n), charge(c), atomicWeight(weight) {}

    Metal() : charge(0), atomicWeight(0) {}
};

// Constants for common metals
extern const std::vector<Metal> METALS;

// Constants for common ligands with their stability constants
extern const std::vector<Ligand> LIGANDS;

// Function to get ligand by name
const Ligand* GetLigandByName(const std::string& name);

// Function to get metal by name
const Metal* GetMetalByName(const std::string& name);

#endif // LIGAND_H