# Cation-Ligand Equilibrium Engine

A C++ library for calculating cation-ligand equilibrium concentrations based on stability constants from the WebMaxC database.

## Features

- Support for multiple ligands with their stability constants
- Support for common metals with their properties
- Equilibrium concentration calculations
- Extensible design for adding new ligands and metals

## Supported Ligands

The library includes stability constants for the following ligands from the WebMaxC Extended database:

- ADA (Adenosine diphosphate)
- AMP-5 (Adenosine monophosphate)
- ADP (Adenosine diphosphate)
- ATP (Adenosine triphosphate)
- BAPTA (1,2-Bis(2-aminooxy)ethane-N,N,N',N'-tetraacetic acid)
- CITRATE (Citric acid)
- DiBrBAP (1,2-Dibromo-1,2-bis(2-aminooxy)ethane)
- EDTA (Ethylenediaminetetraacetic acid)
- EGTA (Ethyleneglycol-bis(beta-aminoethyl ether)-N,N,N',N'-tetraacetic acid)
- HEDTA (Hydroxyethylenediaminetriacetic acid)
- NTA (Nitrilotriacetic acid)
- TPEN (N,N,N',N'-Tetraacetyl-2,2-dimethyl-1,3-propanediamine)

## Supported Metals

- Ag1 (Silver(I))
- Ca2 (Calcium(II))
- Co2 (Cobalt(II))
- Fe2 (Iron(II))
- Fe3 (Iron(III))
- Mg2 (Magnesium(II))
- Mn2 (Manganese(II))
- Ni2 (Nickel(II))
- Pb2 (Lead(II))

## Usage

```cpp
#include "CationEngine.h"

CationEngine engine;
auto result = engine.CalculateEquilibrium(0.1, 0.01, 7.0, "EDTA", "Ca2");
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## License

MIT License