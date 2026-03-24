# Release Notes: v1.0-alpha

## Overview
This is the initial alpha release of the Cation-Ligand Equilibrium Engine. This release provides a complete implementation of the equilibrium calculation system with support for multiple ligands and cations.

## Key Changes

### Fixed Critical Issue
- **Ligand Data Loading**: Fixed the CSV file name in main.cpp from `ligand_data.csv` to `ligands.csv` to properly load the 28-column ligand database
- **Equilibrium Calculations**: Verified that EDTA-Ca2 system returns correct logK of 10.2

### Features Implemented
- Two-way iterative solver for calculating Free and Total concentrations
- Robust bounding method for guaranteed convergence
- Physiological precision with pH, temperature, and ionic strength corrections
- Support for 7-cation matrix (Ca²⁺, Mg²⁺, Ba²⁺, Cd²⁺, Sr²⁺, Mn²⁺, and custom cation "X")
- 15 ligands in the database including chelators and biological ligands

### Database
- Complete 28-column ligands.csv database with stability constants
- 15 ligands supported: EGTA, EDTA, BAPTA, NTA, HEDTA, TPEN, ADA, ATP, ADP, AMP, Citrate, Aspartate, Glutamate, Glycinate, Gluconate
- 9 metals supported: Ca²⁺, Mg²⁺, Ba²⁺, Cd²⁺, Sr²⁺, Mn²⁺, X, Cu²⁺, Zn²⁺

## Verification
- All equilibrium calculations working correctly
- Ligand data loading verified with test_ligand_loading program
- EDTA-Ca2 system returns logK = 10.2 as expected

## Known Issues
- None at this time

## Build System
- Makefile-based build system
- Cross-platform compatibility
- Static linking support