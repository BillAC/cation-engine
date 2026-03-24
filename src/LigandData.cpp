#include "Ligand.h"
#include <algorithm>
#include <cctype>

// Common metals with their properties
const std::vector<Metal> METALS = {
    {"Ag1", 1, 107.87},   // Silver(I)
    {"Ca2", 2, 40.08},    // Calcium(II)
    {"Co2", 2, 58.93},    // Cobalt(II)
    {"Fe2", 2, 55.85},    // Iron(II)
    {"Fe3", 3, 55.85},    // Iron(III)
    {"Mg2", 2, 24.31},    // Magnesium(II)
    {"Mn2", 2, 54.94},    // Manganese(II)
    {"Ni2", 2, 58.69},    // Nickel(II)
    {"Pb2", 2, 207.2}     // Lead(II)
};

// Stability constants for ligands (from WebMaxC Extended.html)
const std::vector<Ligand> LIGANDS = {
    // ADA ligand
    Ligand("ADA", 4, StabilityConstants(6.710, 2.300, 1.590, 0.000, -2.900, 0.000, 0.000, 0.000)),

    // AMP-5 ligand
    Ligand("AMP-5", 4, StabilityConstants(6.202, 3.854, 0.000, 0.000, 0.800, -4.000, 0.000, 0.000)),

    // ADP ligand
    Ligand("ADP", 4, StabilityConstants(6.374, 3.924, 0.000, 0.000, 0.600, -4.000, 0.000, 0.000)),

    // ATP ligand
    Ligand("ATP", 4, StabilityConstants(6.476, 4.039, 0.000, 0.000, 0.500, -3.600, 0.000, 0.000)),

    // BAPTA ligand
    Ligand("BAPTA", 4, StabilityConstants(6.372, 5.491, 3.260, 2.200, -4.000, -3.000, 0.000, 0.000)),

    // CITRATE ligand
    Ligand("CITRATE", 4, StabilityConstants(5.696, 4.363, 2.919, 0.000, 0.460, -0.750, -1.080, 0.000)),

    // DiBrBAP ligand
    Ligand("DiBrBAP", 4, StabilityConstants(5.600, 4.570, 1.000, 1.000, 0.000, 0.000, 0.000, 0.000)),

    // EDTA ligand
    Ligand("EDTA", 4, StabilityConstants(10.262, 6.186, 2.677, 2.004, -5.600, -4.200, 1.500, 0.300)),

    // EGTA ligand
    Ligand("EGTA", 4, StabilityConstants(9.476, 8.866, 2.738, 1.913, -5.900, -5.800, -2.600, -0.400)),

    // HEDTA ligand
    Ligand("HEDTA", 4, StabilityConstants(9.955, 5.423, 2.612, 1.600, -6.600, -3.100, 1.100, 0.000)),

    // NTA ligand
    Ligand("NTA", 4, StabilityConstants(9.719, 2.521, 1.811, 0.983, -4.600, 0.200, 0.400, 2.000)),

    // TPEN ligand
    Ligand("TPEN", 4, StabilityConstants(7.194, 4.866, 3.353, 2.958, -5.800, -4.200, -3.800, -5.600))
};

// Function to get ligand by name
const Ligand* GetLigandByName(const std::string& name) {
    auto it = std::find_if(LIGANDS.begin(), LIGANDS.end(),
        [&name](const Ligand& ligand) {
            return ligand.name == name;
        });

    if (it != LIGANDS.end()) {
        return &(*it);
    }
    return nullptr;
}

// Function to get metal by name
const Metal* GetMetalByName(const std::string& name) {
    auto it = std::find_if(METALS.begin(), METALS.end(),
        [&name](const Metal& metal) {
            return metal.name == name;
        });

    if (it != METALS.end()) {
        return &(*it);
    }
    return nullptr;
}