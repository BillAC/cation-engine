# Skill: Equilibrium Chemistry Solver
- **Mass Balance:** $T_{cat} = [Free_{cat}] + \sum [Complexes]$
- **Binding Equation:** $[ML] = \frac{[M][L]}{10^{-logK}}$
- **Temperature Correction:** Use Van 't Hoff equation with Enthalpy (H1, H2).
- **Ionic Strength:** Ensure constants are adjusted if the solution deviates from the "Standard State" of the constant.
- **Iterative Logic:** Use the "Bounding" method described (Binary search or Newton-Raphson with safety bounds) to ensure convergence even at nanomolar scales.
