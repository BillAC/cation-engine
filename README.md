# Cation-Ligand Equilibrium Engine (CLEE)

[![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)](https://github.com/BillAC/cation-engine/releases)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://github.com/BillAC/cation-engine/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**Cation-Ligand Equilibrium Engine** is a high-precision scientific application for physiological chemistry. It calculates the concentrations of free and complexed ions in solutions with multiple competing ligands and metals, offering both legacy compatibility and modern thermodynamic rigor.

---

## 🚀 Key Features

*   **Dual Calculation Modes:**
    *   **WebMaxC Legacy Mode:** Perfectly synchronized with the **NIST v8 (MaxChelator)** benchmark. Uses empirical dielectric fits and specific activity corrections ($TH$).
    *   **Industry Standard Mode:** Rigorous thermodynamic modeling using the **Davies Equation**, **Marshall-Franck $pK_w$** corrections, and automatic **Hydroxide ($MOH$)** speciation.
*   **Simultaneous Multi-Species Solver:** Correctly models complex competitive matrices (e.g., ATP + EGTA competing for Ca²⁺ and Mg²⁺) using high-precision Newton-Raphson iterations.
*   **Physiological speciation:** Accounts for $ML$, $MHL$ (Metal-Protonated), and $MOH$ (Metal-Hydroxide) species.
*   **Native Windows GUI:** Modern Win32 interface with real-time mode switching and unit scaling (nM, µM, mM).
*   **Expanded Data Format:** Supports a 46-column CSV format for comprehensive stability constant and enthalpy data management.

---

## 🧪 Scientific Validation

### **1. WebMaxC Synchronization (Legacy Mode)**
CLEE has been bit-for-bit synchronized against the WebMaxC benchmark (pH 7.2, 25°C, I=150mM).

| ATP (mM) | EGTA (mM) | Total Ca (mM) | Total Mg (mM) | WinMaxC Free Ca (µM) | **CLEE Legacy Ca (µM)** |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 0.1 | 0.1 | 1.0 | 1.0 | 871.99 | **871.99** |
| 1.0 | 1.0 | 1.0 | 1.0 | 8.62 | **8.62** |
| 5.0 | 1.0 | 3.0 | 3.0 | 455.17 | **455.17** |
| 5.0 | 5.0 | 1.0 | 1.0 | 0.04 | **0.04** |

### **2. Scientific Rigor (Mode Comparison)**
The "Industry Standard" mode provides more conservative binding estimates by rigorously accounting for activity coefficients and hydroxide competition.

| Scenario | Legacy Free Ca (µM) | **Industry Standard Ca (µM)** | Impact |
| :--- | :--- | :--- | :--- |
| Physiological ATP/EGTA | 455.17 | **329.77** | ~27% Tighter Binding |
| High-EGTA Buffer | 0.04 | **0.035** | Speciation Shift |

---

## 🖥 User Interface

*   **Mode Selector:** Found under `Options -> Mode`. Switch logic sets instantly.
*   **Ligand Panel:** Configure up to 10 ligands simultaneously.
*   **Cation Matrix:** Toggle between calculating "Total from Free" or "Free from Total".
*   **Results Console:** Detailed report of active mode, adjusted constants, and total bound fractions.

---

## 📅 Mathematical Foundation

For detailed derivations of the Newton-Raphson mass balance equations, the Marshall-Franck $pK_w$ model, and the Davies activity coefficients, see [**PHYSICS.md**](PHYSICS.md).

---

## 🛠 Building from Source

### Project Structure
*   `src/`: Contains all C++ source and header files.
*   `src/test/`: Contains the scientific validation suite and test runner.
*   `ligands.csv`: The primary database of stability constants and enthalpies.
*   `metals.csv`: Atomic weights and charges for supported cations.

### Option 1: Native Windows (Microsoft Visual C++)
Recommended for Windows developers using Visual Studio.
1. Ensure **Visual Studio** (with C++ Desktop Development) and **CMake** are installed.
2. Run the provided batch script:
   ```cmd
   build_msvc.bat
   ```
   *Note: The script is pre-configured for VS 2022. You may need to update the `VCVARS_PATH` variable inside the script if your installation path differs.*

### Option 2: Linux Cross-Compile (MinGW-w64)
Used for generating portable Windows executables from a Linux environment.
1. Ensure `mingw-w64` and `cmake` are installed on your Linux system.
2. Run the build script:
   ```bash
   ./build_windows.sh
   ```
3. This generates a **statically linked** `cation_engine.exe` that does not require any external DLLs (like `libgcc_s_seh-1.dll`) to run.

---

## 👥 Credits

*   **Inspiration:** MaxChelator (Bers et al.) and WebMaxC (UC Davis).
*   **Data Sources:** NIST SRD 46 (Critically Selected Stability Constants).
*   **Development:** Architected and synchronized via **Gemini CLI** (Gemini 2.0 Flash & Claude Code).
