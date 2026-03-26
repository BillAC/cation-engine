#include "Ligand.h"
#include "LigandCSVLoader.h"
#include <iostream>

// This is a test function, not a main function
void test_ligand_loading() {
    // Initialize ligand data from CSV
    LoadLigandDataFromCSV("ligands.csv");

    std::cout << "Loaded " << LIGANDS.size() << " ligands from CSV file:" << std::endl;

    // Display first few ligands
    for (size_t i = 0; i < std::min(size_t(5), LIGANDS.size()); ++i) {
        const Ligand& ligand = LIGANDS[i];
        std::cout << "Ligand: " << ligand.name
                  << " (valence: " << ligand.valence << ")" << std::endl;

        // Display some stability constants
        std::cout << "  log_K1: " << ligand.constants.log_K1
                  << ", log_K2: " << ligand.constants.log_K2
                  << ", log_K3: " << ligand.constants.log_K3
                  << ", log_K4: " << ligand.constants.log_K4 << std::endl;

        // Display metal constants
        std::cout << "  Ca1: " << ligand.constants.Ca1
                  << ", Mg1: " << ligand.constants.Mg1
                  << ", Zn1: " << ligand.constants.Zn1 << std::endl;
    }

    std::cout << "\nLoaded " << METALS.size() << " metals:" << std::endl;
    for (const auto& metal : METALS) {
        std::cout << "Metal: " << metal.name
                  << " (charge: " << metal.charge
                  << ", atomic weight: " << metal.atomicWeight << ")" << std::endl;
    }

    // Test getting specific ligand
    const Ligand* ligand = GetLigandByName("EDTA");
    if (ligand) {
        std::cout << "\nFound EDTA ligand!" << std::endl;
        std::cout << "EDTA log_K1: " << ligand->constants.log_K1 << std::endl;
        std::cout << "EDTA Ca1: " << ligand->constants.Ca1 << std::endl;
    } else {
        std::cout << "\nEDTA ligand not found!" << std::endl;
    }
}