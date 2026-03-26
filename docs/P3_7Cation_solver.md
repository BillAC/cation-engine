# P3: 7-Cation Matrix Solver

This document summarizes implementation status and usage for the new 7-cation matrix solver in CationSystem.

## Implemented in
- `include/Solver.h`: `EquilibriumMultiResult`, `CalculateTotalToFreeMulti`, `CalculateFreeToTotalMulti`
- `src/Solver.cpp`: multi-metal solver routines + stability correction function updates
- `test/ReferenceSolverTest.cpp`: `test_7Cation_matrix_solver()` scenario

## Solver algorithm
1. For each metal (up to 7 supported, currently tested with 3): retrieve corrected logK using
   `CationSystem::calculateStabilityConstant(ligand, metalName)`
2. For total-to-free: solve for ligand free fraction using bisection on
   `f(Lfree) = totalLigand - Lfree - Σ C_i(Lfree)`
3. For free-to-total: compute each complex using `Ci = (Ki * Lfree * Mi_total) / (1 + Ki * Lfree)`

## Physical corrections
- Ionic strength: `calculateIonicStrengthCorrection()` (Davies equation)
- Temperature: `calculateTemperatureCorrection()` (Van 't Hoff) using ligand `deltaH` (kcal/mol)
- Protonation: `calculateProtonationFraction(params.pH, ligand)` with stepwise pKa states

## Validation
- Full test suite passes via `./build/bin/cation_engine_test`
- Specific test in `test_ReferenceSolverTest.cpp` ensures free[metal] << total under high ligand excess

## Next improvements
- add optional custom metal charge support in Davies correction
- include temperature and ionic corrections in pKa-dependent pH fraction calculations
