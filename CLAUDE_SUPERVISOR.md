# Claude Code Supervisor Instructions

## CRITICAL DIRECTIVES
1. **DO NOT MODIFY `ligands.csv`:** The file `ligands.csv` is the validated scientific "Source of Truth". You are strictly forbidden from altering, "correcting", or overwriting this file. If the user edits ligands via the GUI, save changes to a new file or ensure a backup (`ligands.csv.bak`) is created first, but as an agent, do not manually modify the seed file.
2. **Target Platform:** The end product MUST be an executable program (`cation_engine.exe`) that runs under Windows 11, utilizing standard native Win32 menus and GUI components.

## Objective: Windows 11 Cation Equilibrium GUI
Your goal is to build a Windows 11 GUI program to calculate the free or total concentrations of cations in an aqueous solution of known pH and osmotic strength, achieving parity with legacy tools (Maxchelator/WebMaxC).

### Graphical User Interface (GUI) Layout & Function
- **Ligands Section:** 10 field pairs. Each has a drop-down box for ligand selection (Default: `<None>`) and an adjacent input box for concentration. The text "mM" must be printed to the right of the input box.
- **Cations Section:** Cations and ligands from the 'ligands.csv' file. Each row has:
  - Column 1: "Free Concentration" input box + drop-down (nM, uM, mM).
  - Column 2: "Total Concentration" input box (mM) with "mM" written to the right.
  - *Constraint:* Radio-style exclusivity; the user can enter either Free OR Total per row, not both.
- **Solution Parameters Section:** Inputs for Temperature (ºC), Ionic Strength (mM), and pH.
- **Calculate Function:** A "Calculate" button. If Total is entered, calculate Free. If Free is entered, calculate Total.
- **Menu / Ligand Editor:** A menu entry to access available ligands from the 'ligands.csv' file.
  - Displays: valence, -logK1, H1, -logK2, H2 for H, and each of the cations
  - Also displays editable Temperature and Ionic Strength for each ligand.
  - Default ligands to load: EGTA, EDTA, BAPTA, ATP, ADP, AMP, Citrate, Aspartate, Glutamate, Glycinate, and Gluconate.
  - Features: Ability to add new ligands. If changes are made, prompt the user to save the ligand configuration file and automatically create a backup.

### Scientific Implementation Details
- **Algorithm:** The technique is an iterative one providing upper and lower bounds for unknowns without requiring initial guesses. It must work for concentrations many orders of magnitude apart.
- **Scope:** Restricted to one cation binding to a given ligand at a time (1:1 stoichiometry per site). Applicable to large molecules with multiple independent "sub-ligands".
- **Physiological Corrections:** Implement the Van 't Hoff equation for temperature correction and the Davies (or Extended Debye-Hückel) equation for ionic strength correction within `CationSystem::calculateStabilityConstant`.
- **Protonation:** Ensure the protonation/pH-dependent binding fractions for ligands are calculated using the multi-step protonation constants ($pK_a$ values) from the CSV.

### Architectural Cleanup
- **Build System:** Standardize the project on CMake. Ensure `CMakeLists.txt` correctly handles cross-compilation for Windows using MinGW (`x86_64-w64-mingw32-g++`) and links necessary Win32 libraries (`-luser32 -lgdi32 -lcomctl32 -lcomdlg32`).
- **Unify API:** Consolidate `CationEngine` and `CationSystem` into a single, cohesive API supporting the UI.

## Guidelines
- Follow the naming conventions and project rules established in `CLAUDE.md`.
- Use the custom skills (`equilibrium-solver.md`, `scientific-verification.md`) for complex logic.
- Ensure the UI code (Win32 API) is cleanly separated from the scientific solver code (`src/Solver.cpp`).

## Success Criteria
- [ ] A functioning native Windows 11 GUI (`.exe`) is generated matching the layout specifications.
- [ ] Core solver logic accurately computes Free vs Total concentrations for the 7-cation matrix.
- [ ] `ligands.csv` remains completely unmodified by the agent.
