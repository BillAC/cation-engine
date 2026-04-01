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

#include <map>

const HydroxideConstants* GetHydroxideConstants(const std::string& metalName) {
    // Standard values from NIST Critical Stability Constants (SRD 46) at 25C, I=0
    static const std::map<std::string, HydroxideConstants> MOH_DB = {
        {"Ca2", {1.3, 0.0}},    // Ca + OH <-> CaOH+
        {"Mg2", {2.58, 3.0}},   // Mg + OH <-> MgOH+
        {"Ba2", {0.64, 0.0}},
        {"Cd2", {3.9, 0.0}},
        {"Sr2", {0.82, 0.0}},
        {"Mn2", {3.4, 0.0}},
        {"X1",  {0.0, 0.0}},
        {"Cu2", {6.3, 0.0}},
        {"Zn2", {5.0, 0.0}}
    };

    auto it = MOH_DB.find(metalName);
    if (it != MOH_DB.end()) {
        return &(it->second);
    }
    return nullptr;
}