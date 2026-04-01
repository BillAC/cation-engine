#ifndef CATION_ENGINE_H
#define CATION_ENGINE_H

// DEPRECATION NOTICE:
// This header is deprecated. CationEngine has been unified with CationSystem.
// All functionality is now available through the CationSystem class in Solver.h.
//
// Migration path:
//   Old: #include "CationEngine.h"     CationEngine engine;
//   New: #include "Solver.h"            CationSystem engine;
//
// The CationSystem class provides all features from both CationEngine and the
// original CationSystem, including system parameters, iterative solving,
// physiological corrections, and two-way calculations.

#include "Solver.h"

// Compatibility alias (deprecated - use CationSystem directly)
using CationEngine = CationSystem;

#endif // CATION_ENGINE_H
