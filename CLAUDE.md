# Project: Cation-Ligand Equilibrium Engine (Win32 C++)

## Scientific Goal
Calculate free vs. total concentrations for divalent cations (Ca2+, Mg2+, Ba2+, Cd2+, Sr2+, Mn2+, X) in complexing solutions using an iterative bounding method.

## Functional Specification
- **Core Function:** Compute free ions and ion-ligand complexes for arbitrary numbers of divalent cations and ligands.
- **Iterative Solver:** Must provide upper and lower bounds for unknowns. No initial guesses required. Must remain stable even when concentrations span many orders of magnitude.
- **Assumptions:** Single-cation binding per site (1:1 stoichiometry per sub-ligand). Supports multi-site molecules where sub-ligands are independent.
- **Two-Way Solver:** - Determine Free from known Total.
    - Determine Total required to produce specific Free concentrations.

## UI Layout (Native Win32)
- **Ligands Section:** 10 field pairs. Drop-down box (Ligand selection) + Input box (mM concentration). Default: <None>.
- **Cations Section:** 7 rows (Ca2+, Mg2+, Ba2+, Cd2+, Sr2+, Mn2+, X).
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

## Directory Logic
1. **Architecture:** Headers in `/include`, implementation in `/src`.
2. **Legacy Reference:** Reference JavaScript (WebMaxC) and metadata in `legacy_code/`.
3. **Documentation:** Analysis reports in `/docs`, build logs in `build_errors.log`.

## Build & Project Rules
- **Commands:** `./build.sh` (Full Build), `rm -rf build/*` (Clean).
- **Compilation:** Use `-static` and `-mwindows` for the final .exe.
- **Protocol:** If build fails, read `build_errors.log` and self-correct the code or `build.sh`.

## Verification Protocols
- **Data Integrity:** Cross-reference legacy LogK/ΔH values against NIST/IUPAC via web-search.
- **Discrepancy Log:** Document differences >0.05 LogK in `docs/CONSTANTS_LOG.md`. Modern values prioritize NIST/IUPAC standards.