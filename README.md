# Cation-Ligand Equilibrium Engine (CLEE)

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/BillAC/cation-engine/releases)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://github.com/BillAC/cation-engine/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**Cation-Ligand Equilibrium Engine** is a high-precision Win32 application designed for physiological chemistry and biochemical research. It calculates the equilibrium concentrations of free and complexed divalent cations in solutions containing multiple competing ligands.

---

## 🚀 Key Features

*   **Advanced Simultaneous Solver:** Solves coupled mass-balance equations for arbitrary numbers of ligands and cations using a robust Newton-Raphson method with Jacobian-based iteration.
*   **Two-Way Iterative Calculation:** 
    *   Compute **Free ion concentrations** from known Total concentrations.
    *   Compute **Total ion concentrations** required to achieve target Free levels (ideal for calcium buffering).
*   **Physiological Accuracy:**
    *   **pH Correction:** Automatic adjustment via multi-step ligand protonation fractions ($H_1$ through $H_4$).
    *   **Temperature Correction:** Thermodynamic adjustments using the Van 't Hoff equation and validated Enthalpy ($\Delta H$) data.
    *   **Ionic Strength:** Correction via the Davies equation for experimental accuracy in non-ideal solutions.
*   **7-Cation Matrix:** Native support for $Ca^{2+}$, $Mg^{2+}$, $Ba^{2+}$, $Cd^{2+}$, $Sr^{2+}$, $Mn^{2+}$, and $Fe^{2+}$ (implemented as $FeII$). 
*   **Integrated Ligand Editor:** Manage and backup your constant database directly within the GUI. New ligands can be added. 

---

## 🖥 User Interface

The application features a lightweight, zero-dependency native Win32 GUI designed for Windows 10 and 11.

*   **Solution Parameters:** Real-time adjustment of Temperature, Ionic Strength, and pH.
*   **Ligand Panel:** Supports up to 10 simultaneous ligands with drop-down selection.
*   **Cation Matrix:** 7-column matrix for simultaneous calculation of competing ions. Toggle between Free and Total input/output with automatic unit scaling (nM, µM, mM).

---

## 🧪 Scientific Validation

The engine has been verified against NIST Standard Reference Database 46 and literature values for complex competitive systems:

| Test Case | Conditions | Expected Result | Actual Result | Status |
| :--- | :--- | :--- | :--- | :--- |
| **EGTA-Ca** | 10mM L, 5mM Ca, pH 7.2 | Free Ca: ~67.8 nM | 67.78 nM | ✅ PASS |
| **EDTA-Mg** | 10mM L, Free Mg 1mM, pH 7.2 | Total Mg: ~10.97 mM | 10.98 mM | ✅ PASS |
| **BAPTA Mixed** | 5mM L, 1mM Total Mg, pH 7.2 | Total Ca for 100nM Free: ~2.18mM | 2.185 mM | ✅ PASS |

---

## 📅 Roadmap & TODO

### Upcoming Features
*   **Matrix Expansion:** UI support for $Cu^{2+}$ and $Zn^{2+}$ (already supported in the core solver).
*   **Extended Metal Support:** Integration of $Ag^{+}$, $Co^{2+}$, $Fe^{3+}$, $Ni^{2+}$, and $Pb^{2+}$ from the internal database into the calculation engine.
*   **Import/Export:** Support for JSON-based ligand sharing and experimental session saving.
*   **Graphing:** Visualization of $pCa$ vs. Total concentration curves.

---

## 🛠 Building from Source

### Prerequisites
*   [x86_64-w64-mingw32-g++](https://www.mingw-w64.org/) (for Windows builds)
*   CMake (optional) or included Bash scripts.

### Build Instructions
```bash
# Clone the repository
git clone https://github.com/BillAC/cation-engine.git
cd cation-engine

# Build for Windows (from Linux or Windows)
./build_windows.sh
```

---

## 👥 Credits & Acknowledgments

The **Cation-Ligand Equilibrium Engine** is built upon decades of scientific research and modern AI-collaborative engineering.
*   **Credits:** Program development was inspired by excellent resouces, including
    *   **WebMaxC:** https://somapp.ucdmc.ucdavis.edu/pharmacology/bers/maxchelator/webmaxc/webmaxcS.htm
    *   **MAXCHELATOR:** https://somapp.ucdmc.ucdavis.edu/pharmacology/bers/maxchelator/
    *   **CaBuf:** CaBuf, developed by G. Droogmans at KU Leuven, a classic tool used to calculate free calcium concentrations in solutions containing chelators (like EGTA or BAPTA) for electrophysiology and biochemical experiments

### Scientific Foundations
*   **Iterative Bounding Method:** The core solver architecture is based on the iterative method for computing free ion concentrations in competitive binding environments. See [PHYSICS.md](PHYSICS.md) for a detailed mathematical derivation.
*   **Data Sources:** Stability constants and enthalpy data are derived from:
    *   **NIST Standard Reference Database 46** (Critically Selected Stability Constants of Metal Complexes).
    *   **IUPAC Stability Constants Database**.
    *   **WebMaxC Extended Database**.

### AI Collaborative Engineering Team
This project is a showcase of multi-agent software engineering:
*   **Initial Development of the framework:**    
    *   **Gemini 2.0 Flash:** Project Supervisor, Architect, and Scientific Auditor.
    *   **Claude Code:** Primary Implementation Agent and Win32 UI Specialist.
    *   **Qwen-2.5-Coder:** Logic Troubleshooting and Verification specialist.
*   **Code development:** 
    *   **Github CoPilot:** The bulk of code development.
*   **Finalization:**  
    *   **Gemini CLI (Gemini 3):** Rebuilding the menu structure, code corrections and cleanup. Verifying calculations, stability constants and accuracy, documentation, GitHub. 
*   **Human:**  
    *   **Many excellent scientists and coders:** See above for Credits 
    *   **BillAC:** Project oversight, GUI design, testing 

### Upstream Tools
*   Statically linked against **MinGW-w64** runtime.
*   Developed using the **Gemini CLI** ecosystem.

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
