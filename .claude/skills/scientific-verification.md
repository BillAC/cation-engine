# Skill: Scientific Data Verifier
- **Objective:** Ensure all chemical constants are physically sound.
- **Tools:** Use Tavily/Brave Search to find "Stability Constants" + [Ligand Name].
- **Focus:** Always look for the specific conditions: pH, Temperature, and Ionic Strength ($I$).
- **Conversions:**
  - $K \iff log_{10} K$ ($K = 10^{log K}$).
  - Concentrations: mM $\to$ M (divide by 1000).
  - Temperature: $^\circ\text{C} \to \text{K}$ ($T + 273.15$).
- **Output:** Generate a table comparing Legacy vs. Modern values for the user to review.
- **Validation:** Check the `validation_suite.cpp` and `SolverTest.cpp` against expected values (e.g., EDTA-Ca logK $\approx$ 10.7 at 25ºC).