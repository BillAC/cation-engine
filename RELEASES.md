# Releases

## v1.0.0 (Latest) - 2026-03-28

### 🚀 High-Precision Cation Equilibrium Solver

This is the first stable release of the Cation-Ligand Equilibrium Engine. This version marks the transition from alpha to a fully verified scientific tool capable of handling multi-ligand, multi-cation scenarios with high precision.

#### [Download cation_engine.exe (Windows 10/11)](https://github.com/BillAC/cation-engine/releases/download/v1.0.0/cation_engine.exe)

---

### What's New

*   **Robust Multi-Species Solver:** New Newton-Raphson implementation for simultaneous multi-ligand and multi-metal equilibrium.
*   **Scientifically Verified:** BAPTA multi-metal test cases now match NIST/literature values with high accuracy.
*   **Refined Win32 Interface:**
    *   Rebuilt menu structure (File, Edit, Help).
    *   New "Ligand Editor" pseudo-modal window for database management.
    *   "Lookup Stability Constants" integration with external reference resources.
    *   Automatic unit conversion (nM, uM, mM) for free concentration inputs.
*   **Physiological Corrections:** Fully functional pH, temperature (Van 't Hoff), and ionic strength (Davies) adjustments.

### Technical Details
*   **Version:** 1.0.0
*   **Platform:** Windows (x86_64)
*   **Architecture:** Statically linked Win32 C++
*   **Database:** `ligands.csv` (28-column format)

### Credits
Developed through AI-collaborative engineering with mathematical oversight from Gemini 2.0 Flash and implementation by Claude 3.7 Sonnet.
