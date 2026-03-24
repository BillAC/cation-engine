#include "Ligand.h"
#include <iostream>

int main() {
    std::cout << "Loaded " << LIGANDS.size() << " ligands from CSV file:" << std::endl;

    // Display first few ligands
    for (size_t i = 0; i < std::min(size_t(5), LIGANDS.size()); ++i) {
        const Ligand& ligand = LIGANDS[i];
        std::cout << "Ligand: " << ligand.name
                  << " (valence: " << ligand.valence << ")" << std::endl;

        // Display some stability constants
        std::cout << "  H1: " << ligand.constants.H1
                  << ", H2: " << ligand.constants.H2
                  << ", H3: " << ligand.constants.H3
                  << ", H4: " << ligand.constants.H4 << std::endl;

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
        std::cout << "EDTA H1: " << ligand->constants.H1 << std::endl;
        std::cout << "EDTA Ca1: " << ligand->constants.Ca1 << std::endl;
    } else {
        std::cout << "\nEDTA ligand not found!" << std::endl;
    }

    return 0;
}