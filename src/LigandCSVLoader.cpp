#include "Ligand.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

// Global vector to store ligand data
std::vector<Ligand> LIGANDS;

// Helper function to trim whitespace from string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

// Helper function to convert string to double
double stringToDouble(const std::string& str) {
    if (str.empty() || str == "NULL" || str == "null") {
        return 0.0;
    }
    try {
        return std::stod(str);
    } catch (...) {
        return 0.0;
    }
}

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

// Function to load ligand data from CSV file
void LoadLigandDataFromCSV(const std::string& filename) {
    LIGANDS.clear();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open ligand data file: " << filename << std::endl;
        return;
    }

    std::string line;

    // Skip header line
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> cells;

        // Parse CSV line
        while (std::getline(ss, cell, ',')) {
            cells.push_back(trim(cell));
        }

        // Check if we have enough columns (minimum 28, full 46 for MHL support)
        if (cells.size() < 28) continue;

        // Extract ligand data
        std::string name = trim(cells[0]);
        int charge = 0;
        try {
            charge = std::stoi(trim(cells[1]));
        } catch (...) {
            charge = 0;
        }

        StabilityConstants sc;
        // H1-H4 protonation constants (indices 2-5)
        sc.log_K1 = stringToDouble(cells[2]);
        sc.log_K2 = stringToDouble(cells[3]);
        sc.log_K3 = stringToDouble(cells[4]);
        sc.log_K4 = stringToDouble(cells[5]);

        // Metal-specific stability constants (ML complexes) (indices 6-14: Ca1-Zn1)
        sc.Ca1 = stringToDouble(cells[6]);
        sc.Mg1 = stringToDouble(cells[7]);
        sc.Ba1 = stringToDouble(cells[8]);
        sc.Cd1 = stringToDouble(cells[9]);
        sc.Sr1 = stringToDouble(cells[10]);
        sc.Mn1 = stringToDouble(cells[11]);
        sc.X1  = stringToDouble(cells[12]);
        sc.Cu1 = stringToDouble(cells[13]);
        sc.Zn1 = stringToDouble(cells[14]);

        // H1-H4 enthalpy values (indices 15-18)
        sc.dlog_K1 = stringToDouble(cells[15]);
        sc.dlog_K2 = stringToDouble(cells[16]);
        sc.dlog_K3 = stringToDouble(cells[17]);
        sc.dlog_K4 = stringToDouble(cells[18]);

        // Metal-specific enthalpy values (ML complexes) (indices 19-27: dCa1-dZn1)
        sc.dCa1 = stringToDouble(cells[19]);
        sc.dMg1 = stringToDouble(cells[20]);
        sc.dBa1 = stringToDouble(cells[21]);
        sc.dCd1 = stringToDouble(cells[22]);
        sc.dSr1 = stringToDouble(cells[23]);
        sc.dMn1 = stringToDouble(cells[24]);
        sc.dX1  = stringToDouble(cells[25]);
        sc.dCu1 = stringToDouble(cells[26]);
        sc.dZn1 = stringToDouble(cells[27]);

        // New MHL columns (indices 28-36: CaH1-ZnH1 and 37-45: dCaH1-dZnH1)
        if (cells.size() >= 46) {
            sc.CaH1 = stringToDouble(cells[28]);
            sc.MgH1 = stringToDouble(cells[29]);
            sc.BaH1 = stringToDouble(cells[30]);
            sc.CdH1 = stringToDouble(cells[31]);
            sc.SrH1 = stringToDouble(cells[32]);
            sc.MnH1 = stringToDouble(cells[33]);
            sc.XH1  = stringToDouble(cells[34]);
            sc.CuH1 = stringToDouble(cells[35]);
            sc.ZnH1 = stringToDouble(cells[36]);

            sc.dCaH1 = stringToDouble(cells[37]);
            sc.dMgH1 = stringToDouble(cells[38]);
            sc.dBaH1 = stringToDouble(cells[39]);
            sc.dCdH1 = stringToDouble(cells[40]);
            sc.dSrH1 = stringToDouble(cells[41]);
            sc.dMnH1 = stringToDouble(cells[42]);
            sc.dXH1  = stringToDouble(cells[43]);
            sc.dCuH1 = stringToDouble(cells[44]);
            sc.dZnH1 = stringToDouble(cells[45]);
        }

        // Add ligand to vector
        LIGANDS.push_back(Ligand(name, charge, sc));
    }

    file.close();
}

// Function to get all ligands
const std::vector<Ligand>& GetAllLigands() {
    return LIGANDS;
}

// Function to initialize ligand data
void InitializeLigandData() {
    // Try to find ligands.csv in several common locations
    const char* paths[] = {
        "ligands.csv",
        "../ligands.csv",
        "../../ligands.csv",
        "bin/ligands.csv",
        "../bin/ligands.csv"
    };

    bool loaded = false;
    for (const char* path : paths) {
        std::ifstream file(path);
        if (file.good()) {
            file.close();
            LoadLigandDataFromCSV(path);
            if (!LIGANDS.empty()) {
                loaded = true;
                break;
            }
        }
    }

    if (!loaded) {
        std::cerr << "Critical Error: Could not find ligands.csv in any expected location." << std::endl;
    }
}