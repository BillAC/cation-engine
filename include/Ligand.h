#ifndef LIGAND_H
#define LIGAND_H

#include <string>
#include <vector>
#include <array>

// Structure to hold stability constants for a ligand
struct StabilityConstants {
    double log_K1, log_K2, log_K3, log_K4;     // LogK values for 1-4 protonation steps
    double dlog_K1, dlog_K2, dlog_K3, dlog_K4; // Enthalpy values for 1-4 protonation steps

    // Metal-specific stability constants (for 9 different metals)
    double Ca1, Mg1, Ba1, Cd1, Sr1, Mn1, X1, Cu1, Zn1;  // LogK values for metal complexes
    double dCa1, dMg1, dBa1, dCd1, dSr1, dMn1, dX1, dCu1, dZn1;  // Enthalpy values for metal complexes

    StabilityConstants() : log_K1(0), log_K2(0), log_K3(0), log_K4(0), dlog_K1(0), dlog_K2(0), dlog_K3(0), dlog_K4(0),
                          Ca1(0), Mg1(0), Ba1(0), Cd1(0), Sr1(0), Mn1(0), X1(0), Cu1(0), Zn1(0),
                          dCa1(0), dMg1(0), dBa1(0), dCd1(0), dSr1(0), dMn1(0), dX1(0), dCu1(0), dZn1(0) {}
    StabilityConstants(double k1, double k2, double k3, double k4,
                      double dk1, double dk2, double dk3, double dk4,
                      double ca1, double mg1, double ba1, double cd1, double sr1, double mn1, double x1, double cu1, double zn1,
                      double dca1, double dmg1, double dba1, double dcd1, double dsr1, double dmn1, double dx1, double dcu1, double dzn1)
        : log_K1(k1), log_K2(k2), log_K3(k3), log_K4(k4), dlog_K1(dk1), dlog_K2(dk2), dlog_K3(dk3), dlog_K4(dk4),
          Ca1(ca1), Mg1(mg1), Ba1(ba1), Cd1(cd1), Sr1(sr1), Mn1(mn1), X1(x1), Cu1(cu1), Zn1(zn1),
          dCa1(dca1), dMg1(dmg1), dBa1(dba1), dCd1(dcd1), dSr1(dsr1), dMn1(dmn1), dX1(dx1), dCu1(dcu1), dZn1(dzn1) {}
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