  Critical Bug Analysis

  The application has a fundamental bug in src/LigandCSVLoader.cpp in the LoadLigandDataFromCSV function where the column mapping is incorrect, causing ligand stability constants
   to be loaded from wrong CSV columns.

  Root Cause

  The current column mapping in lines 86-122 of src/LigandCSVLoader.cpp incorrectly assigns:
  - H1-H4 protonation constants (columns 2-5) to wrong positions
  - Metal-specific constants (columns 6-14) to wrong positions
  - Enthalpy values (columns 15-27) to wrong positions

  Impact

  This bug causes the solver to calculate incorrect equilibrium concentrations, making all application results inaccurate.

  Solution

  The column indices need to be corrected to properly map:
  - H1-H4 constants: columns 2-5 (0-indexed: 2,3,4,5)
  - Metal constants: columns 6-14 (0-indexed: 6,7,8,9,10,11,12,13,14)
  - Enthalpy values: columns 15-27 (0-indexed: 115,16,17,18,19,20,21,22,23,24,25,26,27)

  This is a critical bug that must be fixed to ensure accurate results from the cation-ligand equilibrium solver.