# Scientific Basis: Cation-Ligand Equilibrium Engine (CLEE)

This document describes the mathematical and physical foundations of the **Cation-Ligand Equilibrium Engine**. The engine supports two distinct physical models to accommodate both legacy compatibility and modern thermodynamic rigor.

## 1. Fundamental Equilibrium Equations

The system solves for the equilibrium state of multiple metals ($M_j$) and multiple ligands ($L_i$). The engine accounts for three primary species for every ligand-metal pair:

1.  **ML Complex:** $M + L \rightleftharpoons ML$
    $$[ML] = K_{ML} \cdot [M_f] \cdot [L_f]$$
2.  **MHL Complex:** $M + H + L \rightleftharpoons MHL$
    $$[MHL] = K_{MHL} \cdot K_{H1} \cdot [H] \cdot [M_f] \cdot [L_f]$$
3.  **MOH Complex (Hydroxide):** $M + OH \rightleftharpoons MOH$
    $$[MOH] = K_{MOH} \cdot [M_f] \cdot [OH]$$

Where $[L_f]$ is the concentration of the **fully deprotonated** ligand species.

### Mass Balance Constraints
The solver must satisfy conservation of mass for all components:

**Metal Mass Balance ($Mt_j$):**
$$Mt_j = [M_{f,j}] + \sum_{i} ([ML_{ij}] + [MHL_{ij}]) + [MOH_j]$$

**Ligand Mass Balance ($Lt_i$):**
$$Lt_i = [L_{f,i}] \cdot ZSumL_i + \sum_{j} ([ML_{ij}] + [MHL_{ij}])$$

**Protonation Sum ($ZSumL$):**
Accounts for all protonated states of the free ligand ($HL, H_2L, H_3L, H_4L$):
$$ZSumL = 1 + \sum_{n=1}^{4} \left( [H]^n \cdot \prod_{k=1}^{n} K_{H,k} \right)$$

---

## 2. Solver Modes

### A. WebMaxC Legacy Mode (NIST v8)
This mode provides bit-for-bit mathematical mimicry of the original MaxChelator logic.

*   **Valence counting ($VaC$):** Charges for ionic strength correction are determined by the count of non-zero hydrogen constants in the database, rather than the physical valence.
*   **Empirical Activity ($TH$):** Uses a specific empirical correction factor ($TH$) to convert $pH$ to $[H^+]$ concentration:
    $$B = 0.5229 \cdot e^{0.0327 \cdot T} + 4.0159$$
    $$TH = 0.145 \cdot e^{-B \cdot I} + 0.0635 \cdot e^{-43.97 \cdot I} + 0.6956$$
*   **Dielectric Correction:** Uses a temperature-dependent polynomial for the dielectric constant of water to calculate $L_f$ activity factors.
*   **Speciation:** Limited to $ML$ and $MHL$.

### B. Industry Standard Mode (Davies)
A rigorous thermodynamic model aligned with generalized geochemical frameworks (e.g., PHREEQC).

*   **Activity Coefficients:** Uses the **Davies Equation** ($I \leq 0.5 M$):
    $$\log_{10} \gamma_i = -A \cdot Z_i^2 \left( \frac{\sqrt{I}}{1 + \sqrt{I}} - 0.3 I \right)$$
*   **Water Speciation:** Calculates $pK_w$ dynamically using the **Marshall-Franck** equation.
*   **Global Competition:** Automatically includes **Metal-Hydroxide ($MOH$)** speciation for all metals, which is critical for accurate results in neutral/alkaline solutions.
*   **Rigorous Van 't Hoff:** Uses the exact exponential form for temperature adjustments.

---

## 3. Numerical Implementation

### Newton-Raphson Solver
The engine solves the system of $N+M$ non-linear equations using a damped Newton-Raphson method. The Jacobian matrix ($\mathbf{J}$) is constructed from the partial derivatives of the mass-balance functions:

$$J_{ii} = \frac{\partial f_i}{\partial x_i}, \quad J_{ij} = \frac{\partial f_i}{\partial x_j}$$

The solver uses Gaussian elimination with partial pivoting to compute the correction vector $\Delta \mathbf{x}$ at each step until a tolerance of $10^{-12}$ is reached.

### WebMaxC Iterative Solver
For Legacy Mode, the engine also supports the "successive approximation" method used in the original MaxChelator:
$$[L_f]_{next} = \frac{Lt}{ZSumL + \sum (K_{app} \cdot [M_f])}$$
$$[M_f]_{next} = \frac{Mt}{1 + \sum (K_{app} \cdot [L_f])}$$
This method is stable but slower to converge than Newton-Raphson.
