# Cation-Ligand Equilibrium Engine (v1.0-alpha)

A high-precision Win32 C++ application for calculating the equilibrium concentrations of divalent cations in complex aqueous solutions. This engine uses an iterative bounding method to solve mass-balance equations, ensuring convergence even at concentration extremes where traditional initial-guess methods fail.



## 🚀 Key Features

* **Two-Way Iterative Solver:** * Calculate **Free** concentrations from known Total concentrations.
    * Calculate **Total** concentrations required to achieve a target Free concentration.
* **Robust Bounding Method:** Guaranteed convergence across many orders of magnitude without requiring user-provided initial guesses.
* **Physiological Precision:** Built-in corrections for:
    * **pH:** Multi-step ligand protonation ($H_1$ through $H_4$).
    * **Temperature:** Thermodynamic adjustments via the Van 't Hoff equation and validated Enthalpy ($\Delta H$) data.
    * **Ionic Strength:** Activity coefficient corrections for experimental accuracy.
* **7-Cation Matrix:** Simultaneous calculation for $Ca^{2+}$, $Mg^{2+}$, $Ba^{2+}$, $Cd^{2+}$, $Sr^{2+}$, $Mn^{2+}$, and a custom cation "X" (plus tracking for $Cu^{2+}$ and $Zn^{2+}$ contaminants).
* **Native Win32 GUI:** A lightweight, zero-dependency Windows 11 interface with a built-in Ligand Editor and configuration backup system.

## 🧪 Validated Ligand Database

The engine is powered by a strictly audited `ligands.csv` containing constants standardized to $0.1M$ ionic strength at $25^\circ C$. Supported ligands include:

## ✅ Recent test cases added

Automatic cases in `test/SolverTest.cpp` now include:

1. `EGTA` + `Ca2+` (10 mM ligand, 5 mM Ca total, pH 7.2) => Free Ca ≈ 6.78e-8 M
2. `EDTA` + `Mg2+` (10 mM ligand, target free 1 mM, pH 7.0) => Total Mg ≈ 1.10e-2 M
3. `BAPTA` + competitive `Ca2+/Mg2+` (5 mM ligand, Ca free target 100 nM, Mg total 1 mM, pH 7.2) => Total Ca ≈ 2.18e-3 M, Free Mg ≈ 8.79e-4 M

These cases verify direct pH-corrected stability constants and the new mixed-constrained solver path.

## 🧪 Validation CLI

* **Chelators:** EGTA, EDTA, BAPTA, NTA, HEDTA, TPEN, ADA.
* **Biological/Organic:** ATP, ADP, AMP, Citrate, Aspartate, Glutamate, Glycinate, Gluconate.

## 🛠 The AI Development Stack

This project is a showcase of multi-agent collaborative engineering:

* **Supervisor & Architect (Gemini 3 Flash):** Mathematical oversight, scientific auditing of stability constants, and cross-referencing NIST 46/IUPAC databases.
* **Implementation Agent (Claude Code):** Win32 GUI integration, C++ architecture, and legacy code translation.
* **Logic & Inference (Qwen-2.5-Coder via Ollama):** Local GPU-accelerated logic troubleshooting and stoichiometry matrix validation.

## 🖥 Usage & Interface

The application provides a dedicated "Cations" panel where users can toggle between entering Free or Total values. The "Solution Parameters" section allows for real-time adjustments to environmental conditions.

```cpp
// Internal Solver Example:
CationSystem system;
system.setSolutionParams(7.20, 25.0, 100.0); // pH, Temp, Ionic Strength
system.addLigand("EGTA", 10.0);
system.setCationTotal("Ca2+", 5.0);
system.solve();

double freeCa = system.getFreeConcentration("Ca2+"); // Returns ~155 nM
```

# Full Build (requires x86_64-w64-mingw32-g++)
./build.sh

## 🖥 Building from Source
# The final executable is statically linked:
# cation_engine.exe

## 🖥 Verification
./build_tests.sh && ./validation_suite

## 👥 Contributors

* GitHub Copilot (code implementation and solver integration)
* Gemini 3 Flash (project supervision)
* Qwen-2.5-Coder (local inference and validation)
