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

        // Check if we have enough columns
        if (cells.size() < 12) continue;

        // Extract ligand data
        std::string name = trim(cells[0]);
        int charge = 0;
        try {
            charge = std::stoi(trim(cells[1]));
        } catch (...) {
            charge = 0;
        }

        // Extract stability constants (K1 through K8)
        StabilityConstants sc(
            stringToDouble(cells[2]),  // K1
            stringToDouble(cells[3]),  // K2
            stringToDouble(cells[4]),  // K3
            stringToDouble(cells[5]),  // K4
            stringToDouble(cells[6]),  // K5
            stringToDouble(cells[7]),  // K6
            stringToDouble(cells[8]),  // K7
            stringToDouble(cells[9])   // K8
        );

        // Add ligand to vector
        LIGANDS.push_back(Ligand(name, charge, sc));
    }

    file.close();
}

// Function to initialize ligand data
void InitializeLigandData() {
    // Load ligand data from CSV file
    LoadLigandDataFromCSV("ligand_data.csv");
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