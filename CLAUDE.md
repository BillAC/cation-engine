# Project: Cation-Ligand Equilibrium Engine (Win32 C++)

## Scientific Goal
Calculate free vs. total concentrations for divalent cations (Ca2+,	Mg2+,	Ba2+,	Cd2+,	Sr2+,	Mn2+,	Cu2+,	Zn2+) in complexing solutions using an iterative bounding method.

### Scientific Implementation Details
- **Algorithm:** The technique is an iterative one providing upper and lower bounds for unknowns without requiring initial guesses. It must work for concentrations many orders of magnitude apart.
- **Scope:** Restricted to one cation binding to a given ligand at a time (1:1 stoichiometry per site). Applicable to large molecules with multiple independent "sub-ligands".
- **Physiological Corrections:** Implement the Van 't Hoff equation for temperature correction and the Davies (or Extended Debye-Hückel) equation for ionic strength correction within `CationSystem::calculateStabilityConstant`.
- **Protonation:** Ensure the protonation/pH-dependent binding fractions for ligands are calculated using the multi-step protonation constants ($pK_a$ values) from the 'ligands.CSV'.

## Functional Specification
- **Core Function:** Compute free ions and ion-ligand complexes for arbitrary numbers of divalent cations and ligands.
- **Iterative Solver:** Must provide upper and lower bounds for unknowns. No initial guesses required. Must remain stable even when concentrations span many orders of magnitude.
- **Assumptions:** Single-cation binding per site (1:1 stoichiometry per sub-ligand). Supports multi-site molecules where sub-ligands are independent.
- **Two-Way Solver:** - Determine Free from known Total.
    - Determine Total required to produce specific Free concentrations.

## UI Layout (Native Win32)
- **Ligands Section:** 16 field pairs. Ligands are from Column 1 in 'ligands.csv'. Drop-down box (Ligand selection) + Input box (mM concentration). Default: <None>.
- **Cations Section:** 8 rows (Ca2+,	Mg2+,	Ba2+,	Cd2+,	Sr2+,	Mn2+,	Cu2+,	Zn2).
    - Col 1: "Free Concentration" Input + Drop-down (nM, uM, mM).
    - Col 2: "Total Concentration" Input (mM).
    - Constraint: Radio-style exclusivity (User enters either Free or Total per row).
- **Solution Parameters:** Temperature (ºC), Ionic Strength (mM), and pH.
- **Menu System:** Ligand Editor for viewing/saving -logK, Enthalpy (H), valence, and standard conditions. Automatic backups on save.

## Technical & Scientific Constraints
- **Target:** Native Windows 11 GUI (Win32 API). x86_64-w64-mingw32-g++.
- **Precision:** `double` or `long double` for all concentration matrices.
- **Chemistry:** Account for pH (protonation), ionic strength (Davies/Debye-Hückel), and temperature (Van 't Hoff) corrections.
- **Data Source:** Ligand data and structures are managed via `ligands.csv`.

## Implemented solver behavior (2026-03-28)
- `CationSystem::calculateStabilityConstant(ligand, metalName)` now applies direct pH correction by multiplying K with protonation fraction `alpha` and returns `log10(K_eff)`.
- `CationSystem::solveCoupledEquilibriumNewton(...)` solves n-metal + ligand mass balance with Jacobian + damping and falls back to bisection when unstable.
- `CalculateTotalToFreeMulti(...)` uses the coupled solver plus fallback to robustly compute free Ligand/Metal states.
- `CalculateConstrainedMulti(...)` now correctly handles case `knownTotalMetals` empty (target-only calculation) and competitive cases.

## Updated test coverage
- `test/SolverTest.cpp` now includes three regression scenarios:
  1. EGTA/Ca free-from-total (10 mM EGTA, 5 mM Ca, pH 7.2)
  2. EDTA/Mg total-from-free (10 mM EDTA, target free 1 mM Mg, pH 7.0)
  3. BAPTA competitive constrained case (5 mM BAPTA, target free Ca 100 nM, 1 mM Mg total, pH 7.2)

## Directory Logic
1. **Architecture:** Headers in `/include`, implementation in `/src`.
2. **Legacy Reference:** Reference JavaScript (WebMaxC) and metadata in `legacy_code/`.
3. **Documentation:** Analysis reports in `/docs`, build logs in `build_errors.log`.

## Build & Project Rules
- **Build System:** Standardize on CMake. Always use an out-of-source `build/` directory.
- **Build Commands:**
  - Full Build: `cmake -B build && cmake --build build`
  - Clean: `rm -rf build/*`
- **Test Commands:**
  - Run All Tests: `ctest --test-dir build --output-on-failure`
  - Run Validation: `./build/cation_engine_validation`
- **Protocol:** If build fails, review `build_errors.log` (if present) or `stderr`. Fix the code or `CMakeLists.txt`.

## Naming Conventions
- **Classes:** `PascalCase` (e.g., `CationSystem`)
- **Methods:** `camelCase` (e.g., `calculateStabilityConstant`)
- **Variables:** `camelCase` (e.g., `freeConcentration`)
- **Files:** `PascalCase` for headers/source (e.g., `Solver.cpp`)
- **Constants:** `kPascalCase` or `UPPER_SNAKE_CASE` (e.g., `kBoltzmannConstant`)

## Architectural Guidelines
1. **Header Locations:** All public headers MUST reside in `include/`. Private implementation headers MAY reside in `src/`.
2. **Solver Parity:** The current 1:1 solver is a placeholder. Implementation MUST evolve to the "7-cation matrix" simultaneous solver described in the Scientific Goal.
3. **Redundancy:** Unify `CationEngine` and `CationSystem` to reduce API confusion.

## Data Governance & CSV Integrity
1. **Read-Only Constants:** The file `ligands.csv` is the validated scientific "Source of Truth." The agent is **strictly forbidden** from overwriting, "correcting," or modifying this file's contents unless specifically instructed to add a *new* ligand. 
2. **DO NOT MODIFY `ligands.csv`:** The file `ligands.csv` is the validated scientific "Source of Truth". You are strictly forbidden from altering, "correcting", or overwriting this file
3. **Parser Alignment:** If the solver produces unexpected results, the agent must first verify its own CSV parsing logic (column mapping) before questioning the data in the file.
4. **Backup Requirement:** If a new ligand is added by the user, the agent MUST create a backup (e.g., `ligands.csv.bak`) before the modification.

## Build Instructions (Cross-Compilation)
- **Toolchain:** x86_64-w64-mingw32-g++ (MinGW-w64)
- **Standard Build Command:**
  ```bash
  x86_64-w64-mingw32-g++ -I./include src/*.cpp -o cation_engine.exe \
  -static-libgcc -static-libstdc++ -static -lpthread \
  -luser32 -lgdi32 -lcomctl32 -lcomdlg32

## Verification Protocols
- **Data Integrity:** Cross-reference legacy LogK/ΔH values against NIST/IUPAC via web-search.
- **Discrepancy Log:** Document differences >0.05 LogK in `docs/CONSTANTS_LOG.md`. Modern values prioritize NIST/IUPAC standards.

## Git & Attribution Protocol
1. **Commit Messages:** When preparing a commit, always include the attribution block:
   - Supervisor: Gemini
   - Agent: Claude Code
   - Local LLM: Qwen-Coder (Ollama)
2. **Formatting:** Use the template structure: `[Module]: Description`.
3. **Atomic Commits:** Commit after each successful validation (e.g., after fixing the CSV parser).