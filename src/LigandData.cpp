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