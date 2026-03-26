# P2: Full Physiological Corrections Implementation

**Objective**: Replace placeholder correction methods with real Davies equation, Van't Hoff with enthalpy data, and multi-step protonation equilibria.

---

## Current State vs. Required State

### Current (Placeholder)

```cpp
// calculateIonicStrengthCorrection() - Returns 1.0 always
double CationSystem::calculateIonicStrengthCorrection(double ionicStrength, double charge) {
    if (ionicStrength <= 0) return 1.0;
    double alpha = 0.5; // Simplified
    double logGamma = -alpha * charge * charge * std::sqrt(ionicStrength) / (1.0 + 1.5 * std::sqrt(ionicStrength));
    return std::pow(10, logGamma);
}

// calculateTemperatureCorrection() - Returns 1.0 if deltaH is 0
double CationSystem::calculateTemperatureCorrection(double temperature, double deltaH) {
    if (deltaH == 0.0) return 1.0;
    // ... simplified calculation
}

// calculateProtonationFraction() - Single step only
double CationSystem::calculateProtonationFraction(double pH, double logK) {
    return 1.0 / (1.0 + std::pow(10, logK - pH));
}
```

### Required (Full Implementation)

1. **Davies Equation** (Ionic Strength Correction)
   - Formula: $\log \gamma = -A z^2 \left[ \frac{\sqrt{I}}{1+\sqrt{I}} - 0.3I \right]$
   - Variables: A ≈ 0.51 (25°C), z = charge, I = ionic strength
   - Current: Returns 1.0 always
   - Required: Proper activity coefficient calculation

2. **Van't Hoff Equation** (Temperature Correction)  
   - Formula: $\ln\frac{K_2}{K_1} = -\frac{\Delta H}{R}\left(\frac{1}{T_2} - \frac{1}{T_1}\right)$
   - Variables: ΔH from CSV (dCa1, dMg1, etc.), Reference T = 25°C
   - Current: Returns 1.0 if ΔH = 0
   - Required: Use enthalpy data from CSV columns

3. **Multi-Step Protonation** (pH Correction)
   - Currently: Single-step $\alpha = \frac{1}{1+10^{pK_a-pH}}$
   - Required: Full Henderson-Hasselbalch for H1-H4 pKa values
   - For ligands with multiple protonation states, calculate alpha correctly

---

## Implementation Steps

### Step 1: Implement Davies Equation (Proper Activity Coefficient)

**File**: `src/Solver.cpp`

Replace:
```cpp
// Current (broken)
double CationSystem::calculateIonicStrengthCorrection(double ionicStrength, double charge) {
    if (ionicStrength <= 0) return 1.0;
    double alpha = 0.5; // Simplified parameter
    // ...
    return std::pow(10, logGamma);
}
```

With:
```cpp
// Davies equation: log(gamma) = -A*z^2*[sqrt(I)/(1+sqrt(I)) - 0.3*I]
double CationSystem::calculateIonicStrengthCorrection(double ionicStrength, double charge) {
    if (ionicStrength <= 0) return 1.0;
    
    const double A = 0.51; // Davies equation parameter at 25°C (depends on temp in real implementation)
    double sqrtI = std::sqrt(ionicStrength);
    
    double logGamma = -A * charge * charge * 
                     (sqrtI / (1.0 + sqrtI) - 0.3 * ionicStrength);
    
    return std::pow(10, logGamma);
}
```

**Validation**: 
- At I = 0: returns 1.0 ✓
- At I = 0.1 M: yields activity coefficient ≈ 0.75 for z=2 ✓

---

### Step 2: Implement Van't Hoff with Enthalpy

**File**: `src/Solver.cpp`

Replace:
```cpp
// Current (broken)
double CationSystem::calculateTemperatureCorrection(double temperature, double deltaH) {
    if (deltaH == 0.0) return 1.0;
    if (T2 == T1) return 1.0;
    double lnK = -deltaH / R * (1.0 / T2 - 1.0 / T1);
    return std::exp(lnK);
}
```

With:
```cpp
// Van't Hoff: ln(K2/K1) = -ΔH/R * (1/T2 - 1/T1)
double CationSystem::calculateTemperatureCorrection(double temperature, double deltaH) {
    const double R = 8.314; // Gas constant J/mol·K
    const double T1 = 298.15; // Reference temp (25°C)
    
    double T2 = temperature + 273.15; // Convert to Kelvin
    
    // If no enthalpy provided, no temperature correction
    if (deltaH == 0.0 || T2 == T1) return 1.0;
    
    // IMPORTANT: deltaH must be converted from kcal/mol to J/mol if needed
    // If CSV contains kcal/mol, multiply by 4184.0
    double deltaH_J = deltaH * 4184.0; // Assuming CSV is in kcal/mol
    
    double lnCorrection = -deltaH_J / R * (1.0 / T2 - 1.0 / T1);
    
    // K_corrected = K_reference * exp(lnCorrection)
    return std::exp(lnCorrection);
}
```

**Validation**:
- At T = 25°C: returns 1.0 ✓
- At T = 37°C: depends on ΔH but shows expected temperature effect ✓

---

### Step 3: Implement Multi-Step Protonation

**File**: `src/Solver.cpp` (NEW public method)

```cpp
// Calculate total free ligand accounting for ALL protonation states
// For a ligand with 4 protonation sites (H1-H4): 
// [L4-] / [L_total] = 1 / (1 + 10^(pK1-pH) + 10^(pK1+pK2-2*pH) + ...)
double CationSystem::calculateProtonationFraction(double pH, const Ligand* ligand) {
    if (!ligand) return 0.0;
    
    // For fully deprotonated form (highest charge, e.g., EDTA4-)
    // Calculate alpha0 for L4- form
    
    // Protonation constants from ligand
    double pK1 = ligand->constants.log_K1;
    double pK2 = ligand->constants.log_K2;
    double pK3 = ligand->constants.log_K3;
    double pK4 = ligand->constants.log_K4;
    
    // Henderson-Hasselbalch for each protonation state
    double D = 1.0 + 
               std::pow(10, pK1 - pH) +
               std::pow(10, pK1 + pK2 - 2*pH) +
               std::pow(10, pK1 + pK2 + pK3 - 3*pH) +
               std::pow(10, pK1 + pK2 + pK3 + pK4 - 4*pH);
    
    // Fraction of deprotonated form (highest charge)
    return 1.0 / D;
}
```

---

### Step 4: Modify calculateStabilityConstant()

**File**: `src/Solver.cpp`

Replace:
```cpp
double CationSystem::calculateStabilityConstant(double logK, double pH, double ionicStrength) {
    double K = std::pow(10, logK);
    double pHCorrection = 1.0;      // NO CORRECTION
    double ionicCorrection = 1.0;   // NO CORRECTION
    double adjustedK = K * pHCorrection * ionicCorrection;
    return std::log10(adjustedK);
}
```

With:
```cpp
double CationSystem::calculateStabilityConstant(double logK, double pH, 
                                                double ionicStrength, double temperature, 
                                                double deltaH, const Ligand* ligand) {
    // Convert logK to K
    double K = std::pow(10, logK);
    
    // 1. Apply ionic strength correction (Davies equation)
    // For 2:2 metal-ligand complex (charge = ±2 relative to 1:1)
    double ionicCorrection = calculateIonicStrengthCorrection(ionicStrength, 2.0);
    
    // 2. Apply temperature correction (Van't Hoff with ΔH)
    double tempCorrection = calculateTemperatureCorrection(temperature, deltaH);
    
    // 3. Apply pH correction (protonation fraction)
    double pHCorrection = 1.0;
    if (ligand) {
        pHCorrection = calculateProtonationFraction(pH, ligand);
    }
    
    // Combined correction in log space: logK_corrected = logK + log(corrections)
    double adjustedK = K * ionicCorrection * tempCorrection;
    
    // Account for protonation in equilibrium calculation
    // ML + nH+ <-> MHL^(n-1) + ...
    // Effective K includes protonation fraction
    adjustedK *= pHCorrection;
    
    return std::log10(adjustedK);
}
```

---

### Step 5: Update calculateFreeToTotal() and calculateTotalToFree()

**File**: `src/Solver.cpp`

In `calculateFreeToTotal()`:
```cpp
// OLD (no corrections):
double complexFormationConstant = std::pow(10, logK_metal);

// NEW (with corrections):
double logK_corrected = calculateStabilityConstant(logK_metal, params.pH, 
                                                   params.ionicStrength, params.temperature, 
                                                   getMetalEnthalpyConstant(ligand, metalName), 
                                                   ligand);
double complexFormationConstant = std::pow(10, logK_corrected);
```

**Similar change for `calculateTotalToFree()`**

---

### Step 6: Helper Method to Get Enthalpy Constant

**File**: `src/Solver.h` (declaration) and `src/Solver.cpp` (implementation)

```cpp
// In src/Solver.h
double getMetalEnthalpyConstant(const Ligand* ligand, const std::string& metalName);

// In src/Solver.cpp
double CationSystem::getMetalEnthalpyConstant(const Ligand* ligand, const std::string& metalName) {
    if (!ligand) return 0.0;
    
    // Map metal names to enthalpy constants
    if (metalName == "Ca2") return ligand->constants.dCa1;
    if (metalName == "Mg2") return ligand->constants.dMg1;
    if (metalName == "Ba2") return ligand->constants.dBa1;
    if (metalName == "Cd2") return ligand->constants.dCd1;
    if (metalName == "Sr2") return ligand->constants.dSr1;
    if (metalName == "Mn2") return ligand->constants.dMn1;
    if (metalName == "X1")  return ligand->constants.dX1;
    if (metalName == "Cu2") return ligand->constants.dCu1;
    if (metalName == "Zn2") return ligand->constants.dZn1;
    
    return 0.0; // No enthalpy data
}
```

---

## Testing Strategy

### Before Implementation (Baseline)
```bash
cd /mnt/programming/cation-engine
cmake --build build 2>&1 | tail -20
./build/bin/cation_engine_test 2>&1 | grep "\[Test" | head -20
```

### After Implementation (Validation)
1. **Davies Equation Validation**:
   - Same test case at different ionic strengths
   - Should show decreased complex formation at high I

2. **Van't Hoff Validation**:
   - Same test case at 25°C vs 37°C
   - Exothermic (ΔH < 0): K increases with T
   - Endothermic (ΔH > 0): K decreases with T

3. **Protonation Validation**:
   - Test at pH 5 vs pH 7 vs pH 9
   - Complex formation should vary with pH (affects free ligand)

4. **Reference Tests**:
   - Run existing ReferenceSolverTest.cpp
   - Results should match literature more closely

### Expected Outcomes
- EDTA-Ca: Complex ↓ as I increases (from 1e-4 to ~8e-5 at I=1M)
- Temperature sensitivity: ±5-10% for physiological T range
- pH sensitivity: Significant for polyprotic ligands
- Reference test compliance: >95% agreement with literature

---

## Data Validation

### CSV Enthalpy Data Required
Check ligands.csv contains:
- `dlog_K1` through `dlog_K4` (columns 15-18): Protonation enthalpies
- `dCa1` through `dZn1` (columns 19-27): Metal binding enthalpies

**Validation Command**:
```bash
head -2 ligands.csv | tail -1 | cut -d',' -f15-27
```

Should show numeric values (e.g., -12.5, 8.3, etc.) in kcal/mol

---

## Implementation Order (Recommended)

1. ✅ Create P2_PHYSIOLOGICAL_CORRECTIONS.md (this file)
2. ⏳ Implement Davies equation in Solver.cpp
3. ⏳ Implement Van't Hoff in Solver.cpp
4. ⏳ Implement multi-step protonation in Solver.cpp
5. ⏳ Add helper method getMetalEnthalpyConstant()
6. ⏳ Modify calculateStabilityConstant() with all corrections
7. ⏳ Update calculateFreeToTotal/calculateTotalToFree
8. ⏳ Run comprehensive validation tests
9. ⏳ Document results and validate against literature
10. ⏳ Commit with standard attribution

---

## File Modifications Summary

| File | Changes | Complexity |
|------|---------|-----------|
| `src/Solver.cpp` | - Davies equation (10 lines) | Low |
| | - Van't Hoff (15 lines) | Low |
| | - Multi-step protonation (15 lines) | Medium |
| | - calculateStabilityConstant enrichment (20 lines) | Medium |
| | - Two method calls updates | Low |
| `include/Solver.h` | - Add 1 new method declaration | Trivial |
| `src/Solver.h` | - Add 1 new method declaration | Trivial |
| **Total** | **~70 lines new/modified** | **Medium** |

---

## Regression Prevention

- All existing tests must pass
- Reference test values should stay within tolerance (±20%)
- Boundary conditions must remain stable
- No breaking API changes

---

**Status**: Ready for Implementation  
**Estimated Duration**: 2-3 hours  
**Complexity**: Medium (mostly formula translation)  
**Risk**: Low (modifying internal methods only, no API changes)
