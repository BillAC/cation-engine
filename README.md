# Cation-Ligand Equilibrium Engine (CLEE)

[![Version](https://img.shields.io/badge/version-1.1.1-blue.svg)](https://github.com/BillAC/cation-engine/releases)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://github.com/BillAC/cation-engine/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**Cation-Ligand Equilibrium Engine** is a high-precision scientific application for physiological chemistry. It calculates the concentrations of free and complexed ions in solutions with multiple competing ligands and metals, offering both legacy compatibility and modern thermodynamic rigor.

---

## 🚀 Key Features

*   **Dual Calculation Modes:**
    *   **WebMaxC Legacy Mode:** Perfectly synchronized with the **NIST v8 (MaxChelator)** benchmark. Uses empirical dielectric fits and specific activity corrections ($TH$).
    *   **Industry Standard Mode:** Rigorous thermodynamic modeling using the **Davies Equation**, **Marshall-Franck $pK_w$** corrections, and automatic **Hydroxide ($MOH$)** speciation.
*   **Two-Way Iterative Calculation:** The engine can calculate either **Free** or **Total** concentrations depending on which value is entered in the interface (ideal for designing precise calcium buffers).
*   **Expanded Multi-Species Solver:** Correctly models complex competitive matrices for **9 cations** (Ca, Mg, Ba, Cd, Sr, Mn, Fe²⁺, Cu²⁺, Zn²⁺) and up to 10 simultaneous ligands using high-precision Newton-Raphson iterations.
*   **Physiological speciation:** Accounts for $ML$, $MHL$ (Metal-Protonated), and $MOH$ (Metal-Hydroxide) species.
*   **Native Windows GUI:** Modern **scrollable** Win32 interface with real-time mode switching and unit scaling (nM, µM, mM).
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

---

## 🖥 User Interface

*   **Mode Selector:** Found under `Options -> Mode`.
*   **Ligand Panel:** Configure up to 10 ligands simultaneously from the database.
*   **Ligand Editor:** View and refine stability constants directly in the database (`Edit -> Ligand Editor`).
*   **Cation Matrix:** Toggle between calculating "Total from Free" or "Free from Total" by entering values in the respective columns.
*   **Scrolling:** Full support for scrollbar and mouse wheel navigation.

---

## 🛠 Building from Source

### Option 1: Native Windows (Microsoft Visual C++)
Run `build_msvc.bat`.

### Option 2: Linux Cross-Compile (MinGW-w64)
Run `./build_windows.sh`. This generates a **statically linked** `cation_engine.exe`.

---

## 👥 Credits

*   **Inspiration:** 
    *   **MaxChelator** (Bers et al.) and **WebMaxC** (UC Davis).
    *   **CaBuf** (developed by G. Droogmans): A widely used software program designed to calculate the free calcium ($Ca^{2+}$) concentration in solutions containing calcium buffers.
*   **Data Sources:** NIST SRD 46 (Critically Selected Stability Constants).
*   **Development:** Architected and synchronized via **Gemini CLI** (Gemini 2.0 Flash & Claude Code).
