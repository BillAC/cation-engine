#ifndef LIGAND_H
#define LIGAND_H

#include <string>
#include <vector>
#include <array>

// Structure to hold stability constants for a ligand
struct StabilityConstants {
    double log_K1, log_K2, log_K3, log_K4;     // LogK values for 1-4 protonation steps
    double dlog_K1, dlog_K2, dlog_K3, dlog_K4; // Enthalpy values for 1-4 protonation steps

    // Metal-specific stability constants (ML complexes)
    double Ca1, Mg1, Ba1, Cd1, Sr1, Mn1, X1, Cu1, Zn1;
    
    // Metal-hydrogen-ligand constants (MHL complexes)
    double CaH1, MgH1, BaH1, CdH1, SrH1, MnH1, XH1, CuH1, ZnH1;

    // Enthalpy values for ML complexes
    double dCa1, dMg1, dBa1, dCd1, dSr1, dMn1, dX1, dCu1, dZn1;
    
    // Enthalpy values for MHL complexes
    double dCaH1, dMgH1, dBaH1, dCdH1, dSrH1, dMnH1, dXH1, dCuH1, dZnH1;

    StabilityConstants() : log_K1(0), log_K2(0), log_K3(0), log_K4(0), dlog_K1(0), dlog_K2(0), dlog_K3(0), dlog_K4(0),
                          Ca1(0), Mg1(0), Ba1(0), Cd1(0), Sr1(0), Mn1(0), X1(0), Cu1(0), Zn1(0),
                          CaH1(0), MgH1(0), BaH1(0), CdH1(0), SrH1(0), MnH1(0), XH1(0), CuH1(0), ZnH1(0),
                          dCa1(0), dMg1(0), dBa1(0), dCd1(0), dSr1(0), dMn1(0), dX1(0), dCu1(0), dZn1(0),
                          dCaH1(0), dMgH1(0), dBaH1(0), dCdH1(0), dSrH1(0), dMnH1(0), dXH1(0), dCuH1(0), dZnH1(0) {}
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
extern std::vector<Ligand> LIGANDS;

// Function to get ligand by name
const Ligand* GetLigandByName(const std::string& name);

// Function to get metal by name
const Metal* GetMetalByName(const std::string& name);

// Function to get all metals
const std::vector<Metal>& GetAllMetals();

#endif // LIGAND_H