#include "Ligand.h"
#include "Constants.h"
#include <algorithm>
#include <cctype>

// Common metals with their properties (as per CSV standard with 9 metals: Ca, Mg, Ba, Cd, Sr, Mn, X, Cu, Zn)
const std::vector<Metal> METALS = {
    {"Ca2", 2, 40.08},    // Calcium(II)
    {"Mg2", 2, 24.31},    // Magnesium(II)
    {"Ba2", 2, 137.33},   // Barium(II)
    {"Cd2", 2, 112.41},   // Cadmium(II)
    {"Sr2", 2, 87.62},    // Strontium(II)
    {"Mn2", 2, 54.94},    // Manganese(II)
    {"X1", 1, 1.008},     // Unknown/Other (1+)
    {"Cu2", 2, 63.55},    // Copper(II)
    {"Zn2", 2, 65.38}     // Zinc(II)
};

// Function to get all metals
const std::vector<Metal>& GetAllMetals() {
    return METALS;
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