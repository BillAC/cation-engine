#ifndef LIGAND_CSV_LOADER_H
#define LIGAND_CSV_LOADER_H

#include "Ligand.h"
#include <string>

// Function to load ligand data from CSV file
void LoadLigandDataFromCSV(const std::string& filename);

// Function to initialize ligand data (calls LoadLigandDataFromCSV)
void InitializeLigandData();

// Function to get ligand by name
const Ligand* GetLigandByName(const std::string& name);

// Function to get all ligands
const std::vector<Ligand>& GetAllLigands();

#endif // LIGAND_CSV_LOADER_H