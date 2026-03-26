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
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open ligand data file: " << filename << std::endl;
        return;
    }

    std::string line;
    bool firstLine = true;

    // Skip header line
    if (std::getline(file, line)) {
        firstLine = false;
    }

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> cells;

        // Parse CSV line
        while (std::getline(ss, cell, ',')) {
            cells.push_back(trim(cell));
        }

        // Check if we have enough columns (28 columns total for the required structure)
        if (cells.size() < 28) continue;

        // Extract ligand data
        std::string name = trim(cells[0]);
        int charge = 0;
        try {
            charge = std::stoi(trim(cells[1]));
        } catch (...) {
            charge = 0;
        }

        // Extract stability constants using correct 28-column mapping
        // CSV columns: Ligand(0), Valence(1), 
        //   H1-H4(2-5), Ca1-Zn1(6-14), dH1-dH4(15-18), dCa1-dZn1(19-27)
        StabilityConstants sc(
            // H1-H4 protonation constants (indices 2-5)
            stringToDouble(cells[2]),  // log_K1
            stringToDouble(cells[3]),  // log_K2
            stringToDouble(cells[4]),  // log_K3
            stringToDouble(cells[5]),  // log_K4

            // H1-H4 enthalpy values (indices 15-18)
            stringToDouble(cells[15]), // dlog_K1
            stringToDouble(cells[16]), // dlog_K2
            stringToDouble(cells[17]), // dlog_K3
            stringToDouble(cells[18]), // dlog_K4

            // Metal-specific stability constants (indices 6-14: Ca1-Zn1)
            stringToDouble(cells[6]),  // Ca1
            stringToDouble(cells[7]),  // Mg1
            stringToDouble(cells[8]),  // Ba1
            stringToDouble(cells[9]),  // Cd1
            stringToDouble(cells[10]), // Sr1
            stringToDouble(cells[11]), // Mn1
            stringToDouble(cells[12]), // X1
            stringToDouble(cells[13]), // Cu1
            stringToDouble(cells[14]), // Zn1

            // Metal-specific enthalpy values (indices 19-27: dCa1-dZn1)
            stringToDouble(cells[19]), // dCa1
            stringToDouble(cells[20]), // dMg1
            stringToDouble(cells[21]), // dBa1
            stringToDouble(cells[22]), // dCd1
            stringToDouble(cells[23]), // dSr1
            stringToDouble(cells[24]), // dMn1
            stringToDouble(cells[25]), // dX1
            stringToDouble(cells[26]), // dCu1
            stringToDouble(cells[27])  // dZn1
        );

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
    // Load ligand data from CSV file
    LoadLigandDataFromCSV("ligands.csv");
}