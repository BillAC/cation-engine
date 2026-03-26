#include "Constants.h"
#include <stdexcept>

// Function to get cation type from string
CationType GetCationType(const std::string& name) {
    if (name == "Ca") return CationType::Ca;
    if (name == "Mg") return CationType::Mg;
    if (name == "Ba") return CationType::Ba;
    if (name == "Cd") return CationType::Cd;
    if (name == "Sr") return CationType::Sr;
    if (name == "Mn") return CationType::Mn;
    if (name == "X") return CationType::X;
    if (name == "Cu") return CationType::Cu;
    if (name == "Zn") return CationType::Zn;

    throw std::invalid_argument("Unknown cation type: " + name);
}

// Function to get cation name from type
std::string GetCationName(CationType type) {
    switch (type) {
        case CationType::Ca: return "Ca";
        case CationType::Mg: return "Mg";
        case CationType::Ba: return "Ba";
        case CationType::Cd: return "Cd";
        case CationType::Sr: return "Sr";
        case CationType::Mn: return "Mn";
        case CationType::X: return "X";
        case CationType::Cu: return "Cu";
        case CationType::Zn: return "Zn";
        default: return "Unknown";
    }
}

// Function to get cation charge from type
int GetCationCharge(CationType type) {
    switch (type) {
        case CationType::Ca: return 2;
        case CationType::Mg: return 2;
        case CationType::Ba: return 2;
        case CationType::Cd: return 2;
        case CationType::Sr: return 2;
        case CationType::Mn: return 2;
        case CationType::X: return 1;
        case CationType::Cu: return 2;
        case CationType::Zn: return 2;
        default: return 0;
    }
}

// Function to get atomic weight from type
double GetAtomicWeight(CationType type) {
    switch (type) {
        case CationType::Ca: return 40.08;
        case CationType::Mg: return 24.31;
        case CationType::Ba: return 137.33;
        case CationType::Cd: return 112.41;
        case CationType::Sr: return 87.62;
        case CationType::Mn: return 54.94;
        case CationType::X: return 1.008;  // Default for unknown
        case CationType::Cu: return 63.55;
        case CationType::Zn: return 65.38;
        default: return 0.0;
    }
}