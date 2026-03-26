#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <vector>

// Enum for different cation types (metals)
enum class CationType {
    Ca,  // Calcium
    Mg,  // Magnesium
    Ba,  // Barium
    Cd,  // Cadmium
    Sr,  // Strontium
    Mn,  // Manganese
    X,   // Unknown/Other
    Cu,  // Copper
    Zn   // Zinc
};

// Function to get cation type from string
CationType GetCationType(const std::string& name);

// Function to get cation name from type
std::string GetCationName(CationType type);

// Function to get cation charge from type
int GetCationCharge(CationType type);

// Function to get atomic weight from type
double GetAtomicWeight(CationType type);

#endif // CONSTANTS_H