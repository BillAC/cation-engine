# Scientific Basis: Cation-Ligand Equilibrium Equations

This document describes the mathematical derivation of the solver used in the **Cation-Ligand Equilibrium Engine**. The engine solves for the equilibrium state of a system containing multiple metals ($M_1, M_2, ... M_n$) and multiple ligands ($L_1, L_2, ... L_m$).

## 1. Fundamental Equations

For each metal $j$ and ligand $i$ forming a 1:1 complex $C_{ij}$:
$$C_{ij} = K_{ij} \cdot [L_{f,i}] \cdot [M_{f,j}]$$

Where:
- $[L_{f,i}]$ is the free concentration of ligand $i$.
- $[M_{f,j}]$ is the free concentration of metal $j$.
- $K_{ij}$ is the effective stability constant (adjusted for pH, temperature, and ionic strength).

### Mass Balance Constraints
The system must satisfy the conservation of mass for every species:

**Metal Mass Balance ($f_{M,j}$):**
$$f_{M,j} = [M_{total,j}] - [M_{f,j}] - \sum_{i=1}^{m} C_{ij} = 0$$

**Ligand Mass Balance ($f_{L,i}$):**
$$f_{L,i} = [L_{total,i}] - [L_{f,i}] - \sum_{j=1}^{n} C_{ij} = 0$$

## 2. Newton-Raphson Derivation

To solve this system of $n + m$ nonlinear equations, we use the Newton-Raphson method. We define a vector of functions $\mathbf{F}(\mathbf{x}) = 0$, where $\mathbf{x}$ contains the free concentrations:
$$\mathbf{x} = [[L_{f,1}], ..., [L_{f,m}], [M_{f,1}], ..., [M_{f,n}]]^T$$

The iteration step is:
$$\mathbf{x}_{k+1} = \mathbf{x}_k - \mathbf{J}^{-1} \mathbf{F}(\mathbf{x}_k)$$

### The Jacobian Matrix ($\mathbf{J}$)

The Jacobian is the matrix of partial derivatives of the mass-balance functions with respect to the free concentrations.

#### For Ligand Equations ($f_{L,i}$):
1. **With respect to its own free concentration ($L_{f,i}$):**
   $$\frac{\partial f_{L,i}}{\partial [L_{f,i}]} = -1 - \sum_{j=1}^{n} K_{ij} [M_{f,j}]$$
2. **With respect to another ligand ($L_{f,k}$):**
   $$\frac{\partial f_{L,i}}{\partial [L_{f,k}]} = 0 \quad (i \neq k)$$
3. **With respect to a metal ($M_{f,j}$):**
   $$\frac{\partial f_{L,i}}{\partial [M_{f,j}]} = -K_{ij} [L_{f,i}]$$

#### For Metal Equations ($f_{M,j}$):
1. **With respect to its own free concentration ($M_{f,j}$):**
   $$\frac{\partial f_{M,j}}{\partial [M_{f,j}]} = -1 - \sum_{i=1}^{m} K_{ij} [L_{f,i}]$$
2. **With respect to another metal ($M_{f,k}$):**
   $$\frac{\partial f_{M,j}}{\partial [M_{f,k}]} = 0 \quad (j \neq k)$$
3. **With respect to a ligand ($L_{f,i}$):**
   $$\frac{\partial f_{M,j}}{\partial [L_{f,i}]} = -K_{ij} [M_{f,j}]$$

## 3. Implementation Details

In `src/Solver.cpp`, the matrix is constructed as follows (for the multi-species case):

- Diagonal elements $(i,i)$ for ligands: `-1.0 - sum(K_ij * M_f,j)`
- Diagonal elements $(m+j, m+j)$ for metals: `-1.0 - sum(K_ij * L_f,i)`
- Off-diagonal elements $(i, m+j)$: `-K_ij * L_f,i`
- Off-diagonal elements $(m+j, i)$: `-K_ij * M_f,j`

The solver uses Gaussian elimination with partial pivoting to solve the linear system $\mathbf{J} \Delta \mathbf{x} = \mathbf{F}$ at each step, with a damping factor to ensure stability in highly non-linear regions (e.g., near-zero free concentrations).
