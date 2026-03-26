#include "CationEngine.h"
#include "LigandCSVLoader.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// Constructor
CationSystem::CationSystem() {
    // Initialize with empty ligands and metals - data will be loaded from CSV
    // Ligands and metals will be populated when Initialize() is called or when
    // ligand data is loaded from CSV
}

// Destructor
CationSystem::~CationSystem() {
    Clear();
}