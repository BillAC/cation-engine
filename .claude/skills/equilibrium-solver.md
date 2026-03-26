# Skill: Equilibrium Chemistry Solver
- **Mass Balance:** $T_{cat} = [Free_{cat}] + \sum [Complexes]$
- **Binding Equation:** $[ML] = \frac{[M][L]}{10^{-logK_{adj}}}$
- **Temperature Correction (Van 't Hoff):** 
  $log K_{T} = log K_{T_{std}} - \frac{\Delta H}{2.303 R} (\frac{1}{T} - \frac{1}{T_{std}})$
- **Ionic Strength (Davies Equation):** 
  $log \gamma = -A \cdot z^2 (\frac{\sqrt{I}}{1+\sqrt{I}} - 0.3 I)$
  $log K_{adj} = log K_{0} + \Delta log \gamma$
- **Iterative Logic:** Use the "Bounding" method described (Binary search or Newton-Raphson with safety bounds) to ensure convergence even at nanomolar scales.
- **Multidimensional Solving:** Implement simultaneous mass-balance for $n$ cations and $m$ ligands using a Jacobian-based Newton method or the bounding matrix approach.
