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

// Structure for hydroxide binding constants
struct HydroxideConstants {
    double logK_MOH;   // M + OH <-> MOH (stability constant)
    double dH_MOH;     // Enthalpy in kcal/mol
};

// Internal database of hydroxide binding constants (from NIST/IUPAC)
// These are used as global competitors in Industry Standard mode.
const HydroxideConstants* GetHydroxideConstants(const std::string& metalName);

#endif // CONSTANTS_H