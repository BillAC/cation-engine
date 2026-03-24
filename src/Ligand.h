#ifndef LIGAND_H
#define LIGAND_H

#include <string>
#include <vector>

// Structure to hold stability constants for a ligand
struct StabilityConstants {
    double log_K1, log_K2, log_K3, log_K4, log_K5, log_K6, log_K7, log_K8;

    StabilityConstants(double k1 = 0.0, double k2 = 0.0, double k3 = 0.0, double k4 = 0.0,
                       double k5 = 0.0, double k6 = 0.0, double k7 = 0.0, double k8 = 0.0)
        : log_K1(k1), log_K2(k2), log_K3(k3), log_K4(k4),
          log_K5(k5), log_K6(k6), log_K7(k7), log_K8(k8) {}
};

// Structure to hold metal properties
struct Metal {
    std::string name;
    int charge;
    double atomic_mass;

    Metal(const std::string& n, int c, double am)
        : name(n), charge(c), atomic_mass(am) {}
};

// Structure to hold ligand properties
struct Ligand {
    std::string name;
    int charge;
    StabilityConstants stability_constants;

    Ligand(const std::string& n, int c, const StabilityConstants& sc)
        : name(n), charge(c), stability_constants(sc) {}
};

// Function declarations
const Ligand* GetLigandByName(const std::string& name);
const Metal* GetMetalByName(const std::string& name);

// CSV loader functions
void LoadLigandDataFromCSV(const std::string& filename);
void InitializeLigandData();

#endif // LIGAND_H